#pragma once

#include "CoreMinimal.h"
#include "NNE/ClassifierNNEModel.h"
#include "NNE/TextFeatures/TextFeaturesAsset.h"
#include "Engine/DeveloperSettings.h"
#include "NNE/ImageEncoder/ImageEncoderDataAsset.h"
#include "ModelClassifierCoreSetting.generated.h"

UCLASS(Config=ModelClassifierCore, DefaultConfig, meta=(DisplayName="Model Classifier Core"))
class MODELCLASSIFIERCORE_API UModelClassifierCoreSetting : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	UPROPERTY(Config, EditAnywhere, Category="General")
	int32 MaxCPUUsage = 16;
	
	UPROPERTY(Config, EditAnywhere, Category="General")
	int32 MaxMemoryUsage = 16;

	UPROPERTY(Config, EditAnywhere, Category="Temp")
	FString TempPath;
	
	UPROPERTY(Config, EditAnywhere, Category="Python")
	FString PythonExe = "C:/Python312/python.exe";
	
	UPROPERTY(Config, EditAnywhere, Category="Python")
	FString PythonENV;
	
	UPROPERTY(Config, EditAnywhere, Category="Python")
	TArray<FString> Packages = {
		TEXT("ninja"), 
		TEXT("pillow"), 
		TEXT("torch"), 
		TEXT("numpy==2.1.0"), 
		TEXT("transformers"),
		TEXT("sentencepiece"),
		TEXT("scipy"),
		TEXT("onnxscript")
	};
	
	UPROPERTY(Config, EditAnywhere, Category="Python")
	TArray<FString> ExternalPackages = {
		TEXT("https://github.com/openai/CLIP.git")
	};
	
	UPROPERTY(Config, EditAnywhere, Category="Python")
	FString PythonOutputPath;
	
	UPROPERTY(Config, EditAnywhere, Category="Python")
	bool bIsSkipInstallPackages = false;
	
	UPROPERTY(Config, EditAnywhere, Category="Downloads")
	FString ModelName = "openai/clip-vit-large-patch14";
	
	UPROPERTY(Config, EditAnywhere, Category="Downloads")
	FIntVector2 InputResolution = FIntVector2(224, 224);
	
	UPROPERTY(Config, EditAnywhere, Category="Models")
	TSoftObjectPtr<UClassifierNNEModel> ClassifierNNEModelData;
	
	UPROPERTY(Config, EditAnywhere, Category="Models")
	TSoftObjectPtr<UTextFeaturesAsset> TextFeaturesAsset;
	
	UPROPERTY(Config, EditAnywhere, Category="Models")
	TArray<FString> PromptTemplates = {
		TEXT("3D rendered model of a {}."),
		TEXT("a rendered image of a {}."),
		TEXT("an rendered image of a {}."),
		TEXT("a 3d rendered image  of a {}."),
		TEXT("a 3d rendered model of a {} with a gray background."), 
		TEXT("a rendered image of a {} on a gray table.")
	};
	
	UPROPERTY(Config, EditAnywhere, Category="Models")
	TSoftObjectPtr<UImageEncoderDataAsset> ImageEncoderDataAsset;
};