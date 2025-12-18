#include "NNE/NNETextFeatures.h"

#include "NNE/TextFeatures/TextFeatureVector.h"
#include "Sampling/MeshMapEvaluator.h"

namespace ModelClassifierCore
{
	void FNNETextFeatures::SetClassifierNNEModelData(UClassifierNNEModel* InNNEModelData)
	{
		ClassifierNNEModelData = InNNEModelData;
	}

	void FNNETextFeatures::SetTextFeaturesAsset(UTextFeaturesAsset* InTextFeaturesData)
	{
		TextFeaturesAsset = InTextFeaturesData;
	}

	void FNNETextFeatures::SetRuntimeType(ENNEInstanceType InRuntimeType)
	{
		RuntimeType = InRuntimeType;
	}

	bool FNNETextFeatures::GenerateTextFeatures()
	{
		using namespace UE::NNE;
		
		if (TextModelInstance == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("TextModelInstance is nullptr"));
			return false;
		}
		
		UE_LOG(LogTemp, Log, TEXT("Start Generate Text Features"));
		
		TMap<FString, FClipTokens> TokensMap = TextFeaturesAsset->ClipTokens;
		TotalLabels = TokensMap.Num();
		const int32 SeqLen = TextFeaturesAsset->InputTokenLength;   // 77
		const int32 FeatureDim = TextFeaturesAsset->OutputTokenLength; // 768

		// Flatten labels order
		TArray<FString> Labels;
		Labels.Reserve(TotalLabels);
		TArray<TArray<int64>> Tokens;
		Tokens.Reserve(TotalLabels);
		
		UE_LOG(LogTemp, Log, TEXT("Setup TotalLabels (%d), SeqLen (%d), FeatureDim (%d)"), TotalLabels, SeqLen, FeatureDim);
		
		for (const auto& Pair : TokensMap)
		{
			Labels.Add(Pair.Key);
			Tokens.Add(Pair.Value.Tokens);
		}
		
		// Inspect model input desc
		TConstArrayView<FTensorDesc> InputDescs = TextModelInstance->GetInputTensorDescs();
	    if (InputDescs.Num() != 1)
	    {
	        UE_LOG(LogTemp, Error, TEXT("Unexpected input count"));
	        return false;
	    }

	    const FTensorDesc& InputDesc = InputDescs[0];
	    UE_LOG(LogTemp, Log, TEXT("Input tensor name: %s"), *InputDesc.GetName());
	    UE_LOG(LogTemp, Log, TEXT("Input data type: %d"), static_cast<int>(InputDesc.GetDataType()));

	    FSymbolicTensorShape SymIn = InputDesc.GetShape();
	    const TConstArrayView<int32> SymInDims = SymIn.GetData();

	    FString SymStr;
	    for (int32 d : SymInDims) SymStr += FString::Printf(TEXT("%d,"), d);
	    UE_LOG(LogTemp, Log, TEXT("Symbolic input dims: %s"), *SymStr); // e.g. "-1,77"

	    // Check if first dim is fixed
	    const int32 SymBatch = (SymInDims.Num() > 0 ? SymInDims[0] : -1);
	    bool BatchFixed = (SymBatch != -1);

	    // If batch fixed and not equal TotalLabels, we must chunk.
	    int32 RequestedBatch = TotalLabels;
	    if (BatchFixed && SymBatch != RequestedBatch)
	    {
	        UE_LOG(LogTemp, Warning, TEXT("Model input batch dim is fixed to %d. Will chunk labels into batches of %d."),
	            SymBatch, SymBatch);
	    }

	    // Also check input dtype expected (Int64 vs Int32)
	    const bool ModelExpectsInt64 = (InputDesc.GetDataType() == ENNETensorDataType::Int64);

	    // We'll accumulate outputs here
	    TArray<float> OutputAllBuffer;
	    OutputAllBuffer.Empty();

	    // Decide chunk size:
	    int32 ChunkSize = BatchFixed ? SymBatch : 64; // if batch fixed use that as chunk, else use 64 or choose smaller if memory issue
	    if (ChunkSize <= 0) ChunkSize = 1;

	    UE_LOG(LogTemp, Log, TEXT("TotalLabels=%d, SeqLen=%d, FeatureDim=%d, ChunkSize=%d, ModelExpectsInt64=%d"),
	        TotalLabels, SeqLen, FeatureDim, ChunkSize, ModelExpectsInt64 ? 1 : 0);

		UE_LOG(LogTemp, Log, TEXT("========================= Start Process Token ========================="));
	    for (int32 start = 0; start < TotalLabels; start += ChunkSize)
	    {
	        int32 curBatch = FMath::Min(ChunkSize, TotalLabels - start);
	    	MaxProgress = TotalLabels * curBatch;
	    	CurrentProgress = start;
	    	
	        // 1) Set input tensor shapes [curBatch, SeqLen]
	        {
	            TArray<uint32> InDims;
	            InDims.Add(static_cast<uint32>(curBatch));
	            InDims.Add(static_cast<uint32>(SeqLen));
	            FTensorShape InputShape = FTensorShape::Make(InDims);

	            if (TextModelInstance->SetInputTensorShapes({ InputShape }) != EResultStatus::Ok)
	            {
	                UE_LOG(LogTemp, Error, TEXT("SetInputTensorShapes failed for batch %d (start=%d)"), curBatch, start);
	                return false;
	            }
	        }
	    	
	    	//UE_LOG(LogTemp, Log, TEXT("Input tensor shapes [%d, %d] - (%d/%d)"), curBatch, SeqLen, start, TotalLabels);

	        // 2) Flatten tokens for this chunk (make int64 or int32 buffer)
	        // ensure tokens have expected length
	        for (int32 i = 0; i < curBatch; ++i)
	        {
	            const int32 idx = start + i;
	        	
	            if (Tokens[idx].Num() != SeqLen)
	            {
	                UE_LOG(LogTemp, Error, TEXT("Token length mismatch for index %d: %d != %d"), idx, Tokens[idx].Num(), SeqLen);
	                return false;
	            }
	        	
	        	//UE_LOG(LogTemp, Log, TEXT("Token length match for index %d: %d != %d"), i, Tokens[idx].Num(), SeqLen);
	        }

	        // Build contiguous buffer in the expected dtype
	        // We'll always build int64 then if model expects int32 we cast to int32 buffer
	        TArray<int64> FlatInt64;
	        FlatInt64.Reserve(curBatch * SeqLen);
	        for (int i = 0; i < curBatch; ++i) FlatInt64.Append(Tokens[start + i]);

	        // Bind input
	        InputBindings.SetNumZeroed(1);

	        if (ModelExpectsInt64)
	        {
	            InputBindings[0].Data = FlatInt64.GetData();
	            InputBindings[0].SizeInBytes = static_cast<uint64>(FlatInt64.Num() * sizeof(int64));
	        }
	        else
	        {
	            // convert to int32 buffer
	            TArray<int32> FlatInt32;
	            FlatInt32.SetNumUninitialized(FlatInt64.Num());
	            for (int i = 0; i < FlatInt64.Num(); ++i) FlatInt32[i] = static_cast<int32>(FlatInt64[i]);

	            // store FlatInt32 somewhere with proper lifetime: use a local TArray that persists until RunSync
	            // We'll move it into a TArray<TArray<uint8>> or keep as variable in this scope
	            // Simpler: allocate uint8 buffer and memcpy int32 values
	            InputRawBuffers.Reset();
	            InputRawBuffers.Add( MakeShared<TArray<uint8>>() );
	            InputRawBuffers[0]->SetNumUninitialized(FlatInt32.Num() * sizeof(int32));
	            FMemory::Memcpy(InputRawBuffers[0]->GetData(), FlatInt32.GetData(), FlatInt32.Num() * sizeof(int32));

	            InputBindings[0].Data = InputRawBuffers[0]->GetData();
	            InputBindings[0].SizeInBytes = static_cast<uint64>(InputRawBuffers[0]->Num());
	        }

	        // 3) Prepare output descs (after SetInputTensorShapes)
	        TConstArrayView<FTensorDesc> OutputTensorDescs = TextModelInstance->GetOutputTensorDescs();
	        if (OutputTensorDescs.Num() != 1)
	        {
	            UE_LOG(LogTemp, Error, TEXT("Unexpected output count"));
	            return false;
	        }

	        FSymbolicTensorShape OutputSym = OutputTensorDescs[0].GetShape();
	        FTensorShape OutputShape = FTensorShape::MakeFromSymbolic(OutputSym);
	        const TConstArrayView<uint32> OutDims = OutputShape.GetData();
	        if (OutDims.Num() != 2)
	        {
	            UE_LOG(LogTemp, Error, TEXT("Unexpected output rank: %d"), OutDims.Num());
	            return false;
	        }

	        const uint32 OutB = OutDims[0];
	        const uint32 OutD = OutDims[1];
	        check(OutB == static_cast<uint32>(curBatch));
	        check(OutD == static_cast<uint32>(FeatureDim));

	        uint64 NumOutputElements = static_cast<uint64>(OutB) * static_cast<uint64>(OutD);
	        TArray<float> OutputBuffer;
	        OutputBuffer.SetNumZeroed(static_cast<int32>(NumOutputElements));

	        OutputBindings.SetNumZeroed(1);
	        OutputBindings[0].Data = OutputBuffer.GetData();
	        OutputBindings[0].SizeInBytes = NumOutputElements * sizeof(float);

	        // 4) Run
	        if (TextModelInstance->RunSync(InputBindings, OutputBindings) != EResultStatus::Ok)
	        {
	            UE_LOG(LogTemp, Error, TEXT("RunSync failed at chunk start=%d batch=%d"), start, curBatch);
	            return false;
	        }

	        // 5) Append results
	        const int32 Prev = OutputAllBuffer.Num();
	        OutputAllBuffer.AddUninitialized(OutputBuffer.Num());
	        FMemory::Memcpy(OutputAllBuffer.GetData() + Prev, OutputBuffer.GetData(), OutputBuffer.Num() * sizeof(float));
	    	
	    	//UE_LOG(LogTemp, Log, TEXT("Append results [Prev : %d, OutputData %f]"), Prev, OutputAllBuffer.GetData()[0]);
	    }
		
		UE_LOG(LogTemp, Log, TEXT("========================= End Process Token ========================="));

	    // Now OutputAllBuffer has TotalLabels * FeatureDim floats
	    check(OutputAllBuffer.Num() == TotalLabels * FeatureDim);
		
		// Normalize and save
		NormalizeCLIPTextFeatures(OutputAllBuffer, TotalLabels, FeatureDim);
		SetOutputToTextFeaturesAsset(Labels, OutputAllBuffer, FeatureDim);
		
		UE_LOG(LogTemp, Log, TEXT("Completely generated text features for %d labels"), TotalLabels);
		return true;
	}

	void FNNETextFeatures::NormalizeCLIPTextFeatures(TArray<float>& Features, int32 NumRows, int32 FeatureDim)
	{
		check(Features.Num() == NumRows * FeatureDim);
		
		for (int32 Row = 0; Row < NumRows; ++Row)
		{
			float* Vec = Features.GetData() + Row * FeatureDim;

			float SumSq = 0.0f;
			for (int32 i = 0; i < FeatureDim; ++i)
			{
				SumSq += Vec[i] * Vec[i];
			}

			float InvNorm = 1.0f / (FMath::Sqrt(SumSq) + 1e-6f);

			for (int32 i = 0; i < FeatureDim; ++i)
			{
				Vec[i] *= InvNorm;
			}
		}
	}

	void FNNETextFeatures::SetOutputToTextFeaturesAsset(const TArray<FString>& Labels,
		const TArray<float>& NormalizedFeatures, int32 FeatureDim)
	{
		const int32 NumLabels = Labels.Num();
		check(NormalizedFeatures.Num() == NumLabels * FeatureDim);
		
		TextFeaturesAsset->TextFeatures.SetNum(NumLabels);
		
		for (int32 i = 0; i < NumLabels; ++i)
		{
			FTextFeatureVector& Feature = TextFeaturesAsset->TextFeatures[i];
			Feature.Values.SetNum(FeatureDim);

			FMemory::Memcpy(
				Feature.Values.GetData(),
				NormalizedFeatures.GetData() + i * FeatureDim,
				FeatureDim * sizeof(float));
		}
		
		TextFeaturesAsset->GeneratedTime = FDateTime::Now();
	}

	bool FNNETextFeatures::Initialize()
	{
		UE_LOG(LogTemp, Log, TEXT("Start Initialize NNERuntime"));

		UE_LOG(LogTemp, Log, TEXT("Start Initialize Python"));
		
		// Get NNE Runtime
		TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(TEXT("NNERuntimeORTCpu"));
		if (!Runtime.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("NNERuntimeORTCpu not found. Enable plugin NNERuntimeORTCpu"));
			return false;
		}

		if (ClassifierNNEModelData == nullptr || ClassifierNNEModelData->TextEncoderNNEModel == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("NNEModelData is not valid!"))
			return false;
		}

		// Create model
		TextModel = Runtime->CreateModelCPU(ClassifierNNEModelData->TextEncoderNNEModel);
		if (!TextModel.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create NNE model"));
			return false;
		}

		TextModelInstance = TextModel->CreateModelInstanceCPU();
		if (!TextModelInstance.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create model instance"));
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Run Initialize Complete"));
		return true;
	}

	void FNNETextFeatures::Cleanup()
	{
		InputBindings.Reset();
		OutputBindings.Reset();
		InputRawBuffers.Empty();
	}

	int FNNETextFeatures::GetMaxProgress()
	{
		return MaxProgress;
	}

	int FNNETextFeatures::GetProgress()
	{
		return CurrentProgress;
	}
}
