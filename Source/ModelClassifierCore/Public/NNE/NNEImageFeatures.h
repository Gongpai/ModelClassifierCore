#pragma once

#include "NNE.h"
#include "NNEImageFeatures.h"
#include "IPythonScriptPlugin.h"
#include <vector>
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ClassifierNNEModel.h"
#include "NNEModelData.h"
#include "NNERuntimeCPU.h"
#include "NNEInstanceType.h"
#include "ImageEncoder/ImageEncoderDataAsset.h"
#include "TextFeatures/TextFeaturesAsset.h"
#include "Utilities/Collections.h"

namespace ModelClassifierCore
{
	class MODELCLASSIFIERCORE_API FNNEImageFeatures
	{
		TObjectPtr<UClassifierNNEModel> ClassifierNNEModelData;
		UTextFeaturesAsset* TextFeaturesAsset;
		UImageEncoderDataAsset* ImageEncoderDataAsset;
		
		ENNEInstanceType RuntimeType = ENNEInstanceType::GPU;
		
		std::vector<float> InputData;
		TArray<std::vector<float>> OutputDataLists;
		
		TSharedPtr<UE::NNE::IModelInstanceCPU> ImageModelInstance;
		TSharedPtr<UE::NNE::IModelCPU> ImageModel;
		TArray<UE::NNE::FTensorBindingCPU> InputBindings;
		TArray<UE::NNE::FTensorBindingCPU> OutputBindings;
		
		bool bIsRunning = false;
		
	public:
		FNNEImageFeatures(){};
		~FNNEImageFeatures() = default;
		
		//Import & Setting
		void SetTextFeaturesAsset(UTextFeaturesAsset* InDataAsset);
		void SetImageEncoderDataAsset(UImageEncoderDataAsset* InDataAsset);
		void SetClassifierNNEModelData(TObjectPtr<UClassifierNNEModel> InNNEModelData);
		void SetRuntimeType(ENNEInstanceType InRuntimeType);
		void SetInputData(const std::vector<unsigned char>& Pixels, ImageSize ImageSize, bool bFlipY = false);
		bool RunClassifyRenderedImage(ImageSize ImageSize, int& OutBestIdx);
		bool Initialize();
		void Cleanup();
	};
}
