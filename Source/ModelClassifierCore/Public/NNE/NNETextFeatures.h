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
#include "TextFeatures/TextFeaturesAsset.h"
#include "Utilities/Collections.h"

namespace ModelClassifierCore
{
	class MODELCLASSIFIERCORE_API FNNETextFeatures
	{
		UClassifierNNEModel* ClassifierNNEModelData;
		UTextFeaturesAsset* TextFeaturesAsset;
		
		ENNEInstanceType RuntimeType = ENNEInstanceType::GPU;
		
		TSharedPtr<UE::NNE::IModelInstanceCPU> TextModelInstance;
		TSharedPtr<UE::NNE::IModelCPU> TextModel;
		TArray<UE::NNE::FTensorBindingCPU> InputBindings;
		TArray<UE::NNE::FTensorBindingCPU> OutputBindings;
		
		TArray<TSharedPtr<TArray<uint8>>> InputRawBuffers;
		
		int CurrentProgress = 0;
		int MaxProgress = 0;
		int TotalLabels = 0;
		
	public:
		FNNETextFeatures() = default;
		~FNNETextFeatures() = default;
		
		//Import & Setting
		void SetClassifierNNEModelData(UClassifierNNEModel* InNNEModelData);
		void SetTextFeaturesAsset(UTextFeaturesAsset* InTextFeaturesData);
		void SetRuntimeType(ENNEInstanceType InRuntimeType);
		bool GenerateTextFeatures();
		void NormalizeCLIPTextFeatures(TArray<float>& Features, int32 NumRows, int32 FeatureDim);
		void SetOutputToTextFeaturesAsset(const TArray<FString>& Labels, const TArray<float>& NormalizedFeatures, int32 FeatureDim);
		bool Initialize();
		void Cleanup();
		
		int GetMaxProgress();
		int GetProgress();
	};
}
