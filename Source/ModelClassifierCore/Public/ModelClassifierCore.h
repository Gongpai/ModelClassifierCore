// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModelClassifierData.h"
#include "ModelClassifierCoreSetting.h"
#include "ModelClassifierCoreCommands.h"
#include "Modules/ModuleManager.h"
#include "Python/PythonRunner.h"

namespace ModelClassifierCore
{
	class MODELCLASSIFIERCORE_API FLabels;
	
	class MODELCLASSIFIERCORE_API FModelClassifierCoreModule : public IModuleInterface
	{
		TSharedPtr<UModelClassifierCoreSetting> CoreEditorSetting;
		TSharedPtr<UClassifierNNEModel> ClassifierNNEModelData;
		TSharedPtr<UTextFeaturesAsset> TextFeaturesAsset;
		TSharedPtr<UImageEncoderDataAsset> ImageEncoderDataAsset;
		
		FString PluginPath;
		FString TempRenderPath;
		FCriticalSection AssetPathMutex;
		FPythonRunner* PythonRunner = nullptr;
		
		static TArray<FString> ThirdPartyAssetPaths;
		
		void RegisterMenus();
		void RegisterSettings();
		void UnregisterSettings();
		void CheckLibraryFile(FString BaseDir, FString Filename);
		TArray<FString> GetAllThirdPartyAssetPaths(FString BaseDir, FString FilePath);
		TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
		
	public:

		/** IModuleInterface implementation */
		virtual void StartupModule() override;
		virtual void ShutdownModule() override;
		
		/** This function will be bound to Command. */
		void PluginButtonClicked();

		/** Tools */
		FPythonRunner* GetPythonRunner(bool& bIsValid);
		void RunDownloadModelPython();
		void RunDownloadCLIPTokenizerPython();
		bool RunGenerateTokenization(UTextFeaturesAsset* InDataAsset, FString& OutJsonPath);
		bool RunGetTokenLength(FString OnnxPath, FString& OutTokenLengthPath);
		bool RunExportCLIPImageNormalization(const FString ModelName, FString& OutSavePath);
		
		/** Get Model Asset Data */
		TSharedPtr<UClassifierNNEModel> GetClassifierNNEModelData();
		TSharedPtr<UTextFeaturesAsset> GetTextFeaturesAsset();
		TSharedPtr<UImageEncoderDataAsset> GetImageEncoderDataAsset();
		TSharedPtr<UModelClassifierCoreSetting> GetConfig();
		
		FString GetPluginPath();
		FString GetThirdPartyAssetPath(int index);
		FString GetTempRenderPath();
		FString GenerateRandomFolderName(int32 Length);

		TSharedPtr<FModelClassifierData> CreateClassifierData(FString Path, bool& bIsValid);
		
	private:
		TSharedPtr<class FUICommandList> PluginCommands;
		
		/** Handle to the test dll we will load */
		void* ExampleLibraryHandle;
	};
}
