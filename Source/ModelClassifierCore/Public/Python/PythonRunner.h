#pragma once

#include "IPythonScriptPlugin.h"
#include "ModelClassifierCore.h"

namespace ModelClassifierCore
{
	class MODELCLASSIFIERCORE_API FPythonRunner
	{
		FModelClassifierCoreModule *Core;
	
	public:
		FPythonRunner(FModelClassifierCoreModule *InCoreModule);
		~FPythonRunner();

		FString GetDefaultPythonWorkingDirectory();
		bool EnvExists(const FString& EnvPath);
		bool EnvUsable(const FString& EnvPath);
		bool DeleteEnv(const FString& EnvPath);
		bool CreateEnv(const FString& PythonExe, const FString& EnvPath);
		bool EnsurePythonEnv(const FString& PythonExe, const FString& EnvPath);
		bool CreatePythonEnv();
		void InstallPackages();
		bool RunDownloadAndConvertModelToONNXModel();
		bool RunDownloadTokenizer();
		bool RunGenerateTokenization(UTextFeaturesAsset* TextFeaturesAsset, FString& OutJsonPath);
		bool RunGetTokenLength(FString OnnxPath, FString& OutTokenLengthPath);
		bool RunExportCLIPImageNormalization(const FString ModelName, FString& OutSavePath);
	};
}