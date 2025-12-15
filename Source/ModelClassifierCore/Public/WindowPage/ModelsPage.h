#pragma once
#include "ModulePage.h"
#include "NNE/NNETextFeatures.h"
#include "NNE/ImageEncoder/ImageEncoderDataAsset.h"
#include "NNE/TextFeatures/TextFeaturesAsset.h"

class FModulePanel;

class FModelsPage : public IModulePage
{
	FString LabelClassFile;
	FString ClipTokensPath;
	FString ONNXModelPath;
	FString ImageEncoderModelName;
	ModelClassifierCore::FNNETextFeatures* NNETextRuntime;
	TSharedPtr<FDropDownData> SelectedFileFormat;
	UTextFeaturesAsset* ImportLabelTextFeaturesAsset;
	UTextFeaturesAsset* ImportClipTokensTextFeaturesAsset;
	UTextFeaturesAsset* GenerateTextFeaturesAsset;
	UImageEncoderDataAsset* ImageNormalizationDataAsset;
	
public:
	FModelsPage();
	virtual ~FModelsPage() override;
	virtual void StartPage(FModulePanel* Panel) override;
	virtual void OpenPage() override;
	virtual void ClosePage() override;
	virtual void ShutdownPage() override;
	virtual void RegisterPage() override;
	virtual void UnregisterPage() override;
	virtual TSharedPtr<SBox> CreatePage() override;
	
	virtual void SaveConfigSettings(FString Key, FString Value) override;
	virtual FString LoadConfigSettings(FString Key) override;
	
	virtual TSharedPtr<SButton> GetTabButton() override;
	virtual void SetTabButton(TSharedPtr<SButton> Button) override;
	virtual void SetupSetting() override;
};
