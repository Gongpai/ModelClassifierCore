#pragma once
#include "TextFeaturesAsset.h"
#include "NNE/ClassifierNNEModel.h"

namespace ModelClassifierCore
{
	enum MODELCLASSIFIERCORE_API FileFormat
	{
		TEXT,
		IMAGE_NET_JSON,
		CSV
	};
	
	class MODELCLASSIFIERCORE_API FTextFeatures
	{
		static TArray<FString> ParseNames(const FString& Input);
		static TArray<FString> ConvertImageNetToStringArray(FString FileContent, const bool bIsPrimaryNameOnly);
		static TArray<FString> ConvertCSVToStringArray(FString FileContent, const bool bIsPrimaryNameOnly);
		
	public:
		static void GenerateTextFeatures(FString VocabPath, FString MergesPath, UClassifierNNEModel* ClassifierNNEModel, UTextFeaturesAsset* TextFeaturesAsset, TFunction<void()> OnComplete);
		static void GenerateLabel(FString LabelClassPath, FileFormat Format, const bool bIsPrimaryNameOnly, UTextFeaturesAsset* TextFeaturesAsset, TFunction<void()> OnComplete);
		static void LoadCLIPTokenizer(FString FilePath, UTextFeaturesAsset* TextFeaturesAsset, TFunction<void()> OnComplete);
	};
}
