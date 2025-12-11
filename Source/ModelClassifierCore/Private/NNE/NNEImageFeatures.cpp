#include "NNE/NNEImageFeatures.h"

#include "Sampling/MeshMapEvaluator.h"

namespace ModelClassifierCore
{
	void FNNEImageFeatures::SetTextFeaturesAsset(UTextFeaturesAsset* InDataAsset)
	{
		TextFeaturesAsset = InDataAsset;
	}

	void FNNEImageFeatures::SetImageEncoderDataAsset(UImageEncoderDataAsset* InDataAsset)
	{
		ImageEncoderDataAsset = InDataAsset;
	}

	void FNNEImageFeatures::SetClassifierNNEModelData(TObjectPtr<UClassifierNNEModel> InNNEModelData)
	{
		ClassifierNNEModelData = InNNEModelData;
	}

	void FNNEImageFeatures::SetRuntimeType(ENNEInstanceType InRuntimeType)
	{
		RuntimeType = InRuntimeType;
	}

	void FNNEImageFeatures::SetInputData(const std::vector<unsigned char>& Pixels, ImageSize ImageSize, bool bFlipY)
	{
		// utility: convert image bytes (uint8) to float NCHW normalized
		// inputBytes: size = Width*Height*Channels (Channels = 3 or 4)
		// channelsIn: 3 or 4
		
		if (ImageEncoderDataAsset == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ImageEncoderDataAsset not valid"));
			return;
		}
		
		constexpr int C = 3;
		const int HW = ImageSize.width * ImageSize.height;

		TArray<double> Mean = ImageEncoderDataAsset->Mean;
		TArray<double> Std = ImageEncoderDataAsset->Std;

		std::vector<float> Out;
		Out.resize(C * HW);
		
		// Debug: sample raw pixels (first, center, last)
		if (Pixels.size() >= 4)
		{
			auto logPixel = [&](int px) {
				int idx = px * 4;
				if (idx + 3 < (int)Pixels.size())
				{
					UE_LOG(LogTemp, Log, TEXT("RawPixel[%d] RGBA = %d,%d,%d,%d"),
						px, Pixels[idx], Pixels[idx+1], Pixels[idx+2], Pixels[idx+3]);
				}
			};
			logPixel(0);
			logPixel((ImageSize.width * ImageSize.height) / 2);
			logPixel(ImageSize.width * ImageSize.height - 1);
		}

		for (int y = 0; y < ImageSize.height; ++y)
		{
			int srcY = bFlipY ? (ImageSize.height - 1 - y) : y;

			for (int x = 0; x < ImageSize.width; ++x)
			{
				int srcIdx = (srcY * ImageSize.width + x) * 4;
				int dstIdx = y * ImageSize.width + x;

				float r = Pixels[srcIdx + 0] / 255.0f;
				float g = Pixels[srcIdx + 1] / 255.0f;
				float b = Pixels[srcIdx + 2] / 255.0f;

				r = (r - Mean[0]) / Std[0];
				g = (g - Mean[1]) / Std[1];
				b = (b - Mean[2]) / Std[2];

				Out[0 * HW + dstIdx] = r;
				Out[1 * HW + dstIdx] = g;
				Out[2 * HW + dstIdx] = b;
			}
		}
		
		// Debug: print first few normalized floats
		for (int i = 0; i < FMath::Min(10, HW); ++i)
		{
			int pix = i; // pixel index
			UE_LOG(LogTemp, Log, TEXT("NCHW pixel %d: R=%f G=%f B=%f"),
				pix,
				Out[0 * HW + pix],
				Out[1 * HW + pix],
				Out[2 * HW + pix]);
			if (i >= 4) break;
		}
		
		InputData = Out;
	}

	bool FNNEImageFeatures::RunClassifyRenderedImage(ImageSize ImageSize, int& OutResultIndex)
	{
		using namespace UE::NNE;
		
		UE_LOG(LogTemp, Log, TEXT("Start Run ClassifyRenderedImage Function"));
		OutResultIndex = -1;
		
		int OutBestIdx = -1;
		float OutBestSim = -1.0f;
		
		if (!ImageModelInstance.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("ImageModelInstance not valid"));
			return false;
		}
		if (TextFeaturesAsset == nullptr || TextFeaturesAsset->TextFeatures.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("TextFeaturesAsset invalid or empty"));
			return false;
		}
		
		// 1) Set concrete input shape [1, C, H, W]
		TArray<uint32> InDims;
		InDims.Add(1u);
		InDims.Add(3u);
		InDims.Add(static_cast<uint32>(ImageSize.height));
		InDims.Add(static_cast<uint32>(ImageSize.width));
		
		FTensorShape InputShape = FTensorShape::Make(InDims);
		
		if (ImageModelInstance->SetInputTensorShapes({ InputShape }) != EResultStatus::Ok)
		{
			UE_LOG(LogTemp, Error, TEXT("SetInputTensorShapes failed for image"));
			return false;
		}
		
		// 2) Bind input
		InputBindings.SetNumZeroed(1);
		InputBindings[0].Data = InputData.data();
		InputBindings[0].SizeInBytes = static_cast<uint64>(InputData.size() * sizeof(float));
		
		// 4) Prepare outputs
		TConstArrayView<FTensorDesc> OutputTensorDescs = ImageModelInstance->GetOutputTensorDescs();
		if (OutputTensorDescs.Num() < 1)
		{
			UE_LOG(LogTemp, Error, TEXT("Model has no outputs"));
			return false;
		}
		
		// For simplicity take output 0 as image embedding
		FSymbolicTensorShape OutSym = OutputTensorDescs[0].GetShape();
		FTensorShape OutShape = FTensorShape::MakeFromSymbolic(OutSym);
		const TConstArrayView<uint32> OutDims = OutShape.GetData();

		if (OutDims.Num() != 2)
		{
			UE_LOG(LogTemp, Error, TEXT("Unexpected output rank %d (expect 2)"), OutDims.Num());
			return false;
		}
		
		const uint32 OutBatch = OutDims[0];
		const uint32 OutDim = OutDims[1];
		
		if (OutBatch != 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("Output batch != 1 (%d) - continuing but handling first row"), OutBatch);
		}
		
		// allocate output buffer
		uint64 NumOutputElements = static_cast<uint64>(OutBatch) * static_cast<uint64>(OutDim);
		std::vector<float> OutputVec(static_cast<size_t>(NumOutputElements), 0.0f);

		OutputBindings.SetNumZeroed(1);
		OutputBindings[0].Data = OutputVec.data();
		OutputBindings[0].SizeInBytes = NumOutputElements * sizeof(float);
		
		UE_LOG(LogTemp, Log, TEXT("Binding input bytes: %llu (expected %llu)"),
		InputBindings[0].SizeInBytes,
		static_cast<uint64>(1u * 3u * ImageSize.width * ImageSize.height * sizeof(float)));

		
		// 5) Run
		if (ImageModelInstance->RunSync(InputBindings, OutputBindings) != EResultStatus::Ok)
		{
			UE_LOG(LogTemp, Error, TEXT("ImageModelInstance RunSync failed"));
			return false;
		}
		
		// 6) Read embedding (use first row)
		std::vector<float> ImageFeat;
		ImageFeat.resize(static_cast<size_t>(OutDim));
		// copy from OutputVec (first row)
		FMemory::Memcpy(ImageFeat.data(), OutputVec.data(), OutDim * sizeof(float));
		
		if (OutputVec.size() >= 10)
		{
			for (int i = 0; i < 10; ++i)
				UE_LOG(LogTemp, Log, TEXT("RawOutput[%d]=%f"), i, OutputVec[i]);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OutputVec small: %d"), (int)OutputVec.size());
		}
		
		// normalize L2
		{
			double sumsq = 0.0;
			for (uint32 i = 0; i < OutDim; ++i) sumsq += double(ImageFeat[i]) * double(ImageFeat[i]);
			float invnorm = 1.0f / (sqrtf(static_cast<float>(sumsq)) + 1e-6f);
			for (uint32 i = 0; i < OutDim; ++i) ImageFeat[i] *= invnorm;
		}
		
		// 7) Compute similarity with text features (assume TextFeaturesAsset->TextFeatures[i].Values.Num() == OutDim)
		const int NumLabels = TextFeaturesAsset->TextFeatures.Num();
		float bestSim = -1e9f;
		int bestIdx = -1;
		
		for (int li = 0; li < NumLabels; ++li)
		{
			const FTextFeatureVector& tfv = TextFeaturesAsset->TextFeatures[li];
			const TArray<float>& tvals = tfv.Values;
			if (tvals.Num() != static_cast<int32>(OutDim))
			{
				UE_LOG(LogTemp, Warning, TEXT("Text feature dim mismatch label %d: %d vs %d"), li, tvals.Num(), OutDim);
				continue;
			}

			double dot = 0.0;
			for (uint32 d = 0; d < OutDim; ++d)
				dot += double(ImageFeat[d]) * double(tvals[d]);

			float sim = static_cast<float>(dot); // since both normalized -> cosine
			if (sim > bestSim)
			{
				bestSim = sim;
				bestIdx = li;
			}
		}
		
		OutBestIdx = bestIdx;
		OutBestSim = bestSim;

		TArray<FString> ClipKeys;
		TextFeaturesAsset->ClipTokens.GetKeys(ClipKeys);
		FString LabelStr = (bestIdx >= 0 && TextFeaturesAsset->TextFeatures.IsValidIndex(bestIdx)) ? TextFeaturesAsset->ClipTokens[ClipKeys[bestIdx]].Label : FString::Printf(TEXT("%d"), bestIdx);
		UE_LOG(LogTemp, Log, TEXT("Predicted image -> [%d] %s (sim=%.6f)"), bestIdx, *LabelStr, bestSim);

		return bestIdx >= 0;
	}

	bool FNNEImageFeatures::Initialize()
	{
		UE_LOG(LogTemp, Log, TEXT("Start Initialize NNERuntime"));
		
		// Get NNE Runtime
		TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(TEXT("NNERuntimeORTCpu"));
		if (!Runtime.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("NNERuntimeORTCpu not found. Enable plugin NNERuntimeORTCpu"));
			return false;
		}

		if (ClassifierNNEModelData == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("NNEModelData is not valid!"))
			return false;
		}

		// Create model
		ImageModel = Runtime->CreateModelCPU(ClassifierNNEModelData->ImageEncoderNNEModel);
		if (!ImageModel.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create NNE model"));
			return false;
		}

		ImageModelInstance = ImageModel->CreateModelInstanceCPU();
		if (!ImageModelInstance.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create model instance"));
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Run Initialize Complete"));
		return true;
	}

	void FNNEImageFeatures::Cleanup()
	{
		ImageModelInstance.Reset();
		ImageModel.Reset();
		InputBindings.Reset();
		OutputBindings.Reset();
		OutputDataLists.Reset();
	}
}
