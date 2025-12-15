// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModelClassifierCore.h"
#include "FileType.h"
#include "ISettingsModule.h"
#include "ModelClassifierCorePage.h"
#include "EditorFramework/AssetImportData.h"
#include "FBX/ExportFBX.h"
#include "FBX/InMemoryFBXStream.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Render/RenderMesh.h"
#include "Utilities/StringHelper.h"

static const FName ModelClassifierCoreTabName("MCC Tools");

#define LOCTEXT_NAMESPACE "FModelClassifierCoreModule"

namespace ModelClassifierCore
{
	TArray<FString> FModelClassifierCoreModule::ThirdPartyAssetPaths;
	
	void FModelClassifierCoreModule::CheckLibraryFile(FString BaseDir, FString Filename)
	{
		FString LibraryPath;
#if PLATFORM_WINDOWS
		UE_LOG(LogTemp, Log, TEXT("WIN 64 BaseDir: %s"), *BaseDir);
		LibraryPath = FPaths::Combine(*BaseDir, FString::Printf(TEXT("Binaries/Win64/%s.dll"), *Filename));
#elif PLATFORM_MAC
		UE_LOG(LogTemp, Log, TEXT("MACOS BaseDir: %s"), *BaseDir);
		LibraryPath = FPaths::Combine(*BaseDir,  FString::Printf(TEXT("Binaries/Mac/%s.dylib"), *Filename);
	#elif PLATFORM_LINUX
		UE_LOG(LogTemp, Log, TEXT("LINUX 64 BaseDir: %s"), *BaseDir);
		LibraryPath = FPaths::Combine(*BaseDir, FString::Printf(TEXT("Binaries/Linux/%s.so"), *Filename));
	#endif // PLATFORM_WINDOWS
	
		if (FPaths::FileExists(LibraryPath))
		{
			// Call the test function in the third party library that opens a message box
			UE_LOG(LogTemp, Log, TEXT("Successfully imported the file at : %s"), *LibraryPath);
		}
		else
		{
			FText FormattedMessage = FText::Format(NSLOCTEXT("ThirdPartyLibrary",
				"ThirdPartyLibraryError",
				"Missing import: {0}.dll\nFailed to import the file at: {1}"),
				FText::FromString(Filename),
				FText::FromString(LibraryPath));
			FMessageDialog::Open(EAppMsgType::Ok, FormattedMessage);
		}
	}

	FString FModelClassifierCoreModule::GenerateRandomFolderName(int32 Length)
	{
		const FString Charset = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
		FString Result;
		FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

		for (int32 i = 0; i < Length; ++i)
		{
			int32 Index = FMath::RandHelper(Charset.Len());
			Result += Charset[Index];
		}

		Result += Timestamp;
		return Result;
	}

	TWeakObjectPtr<UModelClassifierCoreSetting> FModelClassifierCoreModule::GetConfig()
	{
		return CoreEditorSetting;
	}

	TArray<FString> FModelClassifierCoreModule::GetAllThirdPartyAssetPaths(FString BaseDir, FString FilePath)
	{
		TArray<FString> Files;

		FString Path = FPaths::Combine(BaseDir, FilePath);
		if (FPaths::FileExists(Path))
		{
			if (FFileHelper::LoadFileToStringArray(Files, *Path))
			{
				return Files;
			}
		}

		return Files;
	}

	TSharedRef<SDockTab> FModelClassifierCoreModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
	{
		return FModelClassifierCorePage::OnSpawnPluginTab(SpawnTabArgs);
	}

	void FModelClassifierCoreModule::StartupModule()
	{
		// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
		FModelClassifierCoreStyle::Initialize();
		FModelClassifierCoreStyle::ReloadTextures();
		
		FModelClassifierCoreCommands::Register();
		
		// Get default config
		CoreEditorSetting = GetMutableDefault<UModelClassifierCoreSetting>();
		
		UE_LOG(LogTemp, Log, TEXT("Load ClassifierNNEModelData..."));
		ClassifierNNEModelData = GetConfig()->ClassifierNNEModelData.LoadSynchronous();
		
		UE_LOG(LogTemp, Log, TEXT("Load TextFeaturesAsset..."));
		TextFeaturesAsset = GetConfig()->TextFeaturesAsset.LoadSynchronous();
		
		UE_LOG(LogTemp, Log, TEXT("Load ImageEncoderDataAsset..."));
		ImageEncoderDataAsset = GetConfig()->ImageEncoderDataAsset.LoadSynchronous();
		
		RegisterSettings();
		
		PluginCommands = MakeShareable(new FUICommandList);

		PluginCommands->MapAction(
			FModelClassifierCoreCommands::Get().PluginAction,
			FExecuteAction::CreateRaw(this, &FModelClassifierCoreModule::PluginButtonClicked),
			FCanExecuteAction());

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FModelClassifierCoreModule::RegisterMenus));
		
		FModelClassifierCorePage::Initialize(this);
		
		// Get the base directory of this plugin
		FString BaseDir = IPluginManager::Get().FindPlugin("ModelClassifierCore")->GetBaseDir();
		PluginPath = BaseDir;

		// Start Python Runner
		PythonRunner = new FPythonRunner(this);

		// Add on the relative location of the third party dll and load it
		CheckLibraryFile(BaseDir, "assimp");
		CheckLibraryFile(BaseDir, "libfbxsdk");

		//Set Temp Path
		if (CoreEditorSetting->TempPath.IsEmpty())
		{
			FString TempSavePath = FPaths::Combine(BaseDir, "Temp");
			if (!FPaths::DirectoryExists(TempSavePath))
			{
				IFileManager::Get().MakeDirectory(*TempSavePath, true);
			}
			CoreEditorSetting->TempPath = TempSavePath;
		}

		FString TempRenderSavePath = FPaths::Combine(CoreEditorSetting->TempPath, "RenderTemp");
		if (!FPaths::DirectoryExists(TempRenderSavePath))
		{
			IFileManager::Get().MakeDirectory(*TempRenderSavePath, true);
		}
		TempRenderPath = TempRenderSavePath;

		ThirdPartyAssetPaths = GetAllThirdPartyAssetPaths(BaseDir, "Source/ThirdParty/ModelClassifierCoreLibrary/ThirdPartyAssets.txt");

		int i = 1;
		for (auto ThirdPartyAssetPath : ThirdPartyAssetPaths)
		{
			UE_LOG(LogTemp, Log, TEXT("ThirdPartyAssetPath (i) :%s"), *ThirdPartyAssetPath);
			i++;
		}
		
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ModelClassifierCoreTabName, FOnSpawnTab::CreateRaw(this, &FModelClassifierCoreModule::OnSpawnPluginTab))
			.SetDisplayName(LOCTEXT("FModelClassifierCoreModule", "MCC Tools"))
			.SetMenuType(ETabSpawnerMenuType::Enabled);
	}

	void FModelClassifierCoreModule::ShutdownModule()
	{
		// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
		// we call this function before unloading the module.
		
		if (CoreEditorSetting.IsValid()) CoreEditorSetting.Reset();
		if (ClassifierNNEModelData.IsValid()) ClassifierNNEModelData.Reset();
		if (TextFeaturesAsset.IsValid()) TextFeaturesAsset.Reset();
		if (ImageEncoderDataAsset.IsValid()) ImageEncoderDataAsset.Reset();
		
		UToolMenus::UnRegisterStartupCallback(this);

		UToolMenus::UnregisterOwner(this);
	
		UnregisterSettings();

		FModelClassifierCoreStyle::Shutdown();

		FModelClassifierCoreCommands::Unregister();
		
		UE_LOG(LogTemp, Log, TEXT("Begin Shutdown ModelClassifierCore Module"));
		
		// Free the dll handle
		FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
		ExampleLibraryHandle = nullptr;

		// Shutdown Python Runner
		PythonRunner = nullptr;
		
		UE_LOG(LogTemp, Log, TEXT("Shutdown ModelClassifierCore Module Complete!"));
	}
	
	void FModelClassifierCoreModule::PluginButtonClicked()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(ModelClassifierCoreTabName);
	}

	void FModelClassifierCoreModule::RegisterMenus()
	{
		// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
		FToolMenuOwnerScoped OwnerScoped(this);
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FModelClassifierCoreCommands::Get().PluginAction, PluginCommands);
		}

		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FModelClassifierCoreCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}

	void FModelClassifierCoreModule::RegisterSettings()
	{
		if (CoreEditorSetting == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("CoreEditorSetting is null"));
			return;
		}
		
		UE_LOG(LogTemp, Log, TEXT("RegisterSettings"));
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->RegisterSettings(
				"Project",
				"Plugins",
				"ModelClassifierCore",
				NSLOCTEXT("ModelClassifierCore", "SettingsName", "Model Classifier Core"),
				NSLOCTEXT("ModelClassifierCore", "SettingsDesc", "Settings for Model Classifier Core plugin."),
				CoreEditorSetting.Get()
			);
		}
	}

	void FModelClassifierCoreModule::UnregisterSettings()
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "ModelClassifierCore");
		}
		
		UE_LOG(LogTemp, Log, TEXT("UnregisterSettings"));
	}

	FPythonRunner* FModelClassifierCoreModule::GetPythonRunner(bool& bIsValid)
	{
		bIsValid = PythonRunner != nullptr;

		return PythonRunner;
	}

	void FModelClassifierCoreModule::RunDownloadModelPython()
	{
		bool bIsValid = false;
		FPythonRunner* Runner = GetPythonRunner(bIsValid);

		if (!bIsValid) return;

		Runner->RunDownloadAndConvertModelToONNXModel();
	}

	void FModelClassifierCoreModule::RunDownloadCLIPTokenizerPython()
	{
		bool bIsValid = false;
		FPythonRunner* Runner = GetPythonRunner(bIsValid);

		if (!bIsValid) return;
		
		Runner->RunDownloadTokenizer();
	}

	bool FModelClassifierCoreModule::RunGenerateTokenization(UTextFeaturesAsset* InDataAsset, FString& OutJsonPath)
	{
		bool bIsValid = false;
		FPythonRunner* Runner = GetPythonRunner(bIsValid);

		if (!bIsValid) return false;
		
		return  Runner->RunGenerateTokenization(InDataAsset, OutJsonPath);
	}

	bool FModelClassifierCoreModule::RunGetTokenLength(FString OnnxPath, FString& OutTokenLengthPath)
	{
		bool bIsValid = false;
		FPythonRunner* Runner = GetPythonRunner(bIsValid);

		if (!bIsValid) return false;
		
		return Runner->RunGetTokenLength(OnnxPath, OutTokenLengthPath);
	}

	bool FModelClassifierCoreModule::RunExportCLIPImageNormalization(const FString ModelName, FString& OutSavePath)
	{
		bool bIsValid = false;
		FPythonRunner* Runner = GetPythonRunner(bIsValid);

		if (!bIsValid) return false;
		
		return Runner->RunExportCLIPImageNormalization(ModelName, OutSavePath);
	}

	TWeakObjectPtr<UClassifierNNEModel> FModelClassifierCoreModule::GetClassifierNNEModelData()
	{
		if (!GetConfig()->ClassifierNNEModelData.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("ClassifierNNEModelData not valid"));
		}
		
		if (ClassifierNNEModelData == nullptr)
		{
			ClassifierNNEModelData = GetConfig()->ClassifierNNEModelData.LoadSynchronous();
		}
		
		return ClassifierNNEModelData;
	}

	TWeakObjectPtr<UTextFeaturesAsset> FModelClassifierCoreModule::GetTextFeaturesAsset()
	{
		if (!GetConfig()->TextFeaturesAsset.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("TextFeaturesAsset not valid"));
		}
		
		if (TextFeaturesAsset == nullptr)
		{
			TextFeaturesAsset = GetConfig()->TextFeaturesAsset.LoadSynchronous();
		}
		
		return TextFeaturesAsset;
	}

	TWeakObjectPtr<UImageEncoderDataAsset> FModelClassifierCoreModule::GetImageEncoderDataAsset()
	{
		if (!GetConfig()->ImageEncoderDataAsset.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("ImageEncoderDataAsset not valid"));
		}
		
		if (ImageEncoderDataAsset == nullptr)
		{
			ImageEncoderDataAsset = GetConfig()->ImageEncoderDataAsset.LoadSynchronous();
		}
		
		return ImageEncoderDataAsset;
	}

	FString FModelClassifierCoreModule::GetPluginPath()
	{
		return PluginPath;
	}

	TSharedPtr<FModelClassifierHandler> FModelClassifierCoreModule::CreateHandler(FString Path, bool& bIsValid)
	{
		UE_LOG(LogTemp, Log, TEXT("Creating handler"));
		
		FSoftObjectPath MeshPath(Path);
		TWeakObjectPtr<UStaticMesh> Mesh = Cast<UStaticMesh>(MeshPath.ResolveObject());

		if (!Mesh.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create a new handler"));
			bIsValid = false;
			return nullptr;
		}

		TObjectPtr<UAssetImportData> AssetImportData = Mesh->GetAssetImportData();
		if (!AssetImportData)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create a new handler"));
			bIsValid = false;
			return nullptr;
		}

		FString AssetLocalPath = AssetImportData->GetFirstFilename();
		UE_LOG(LogTemp, Log, TEXT("Local path: %s"), *AssetLocalPath);

		bool bIsFileExists = FPaths::FileExists(*AssetLocalPath);
		EFileType FileType = bIsFileExists ? (FPaths::GetExtension(AssetLocalPath, false) == "fbx" ||
			FPaths::GetExtension(AssetLocalPath, false) == "FBX" ? RawFBX : RawOBJ) : FBXStream;

		UE_LOG(LogTemp, Log, TEXT("Model import mode : %s"), (bIsFileExists ? (FPaths::GetExtension(AssetLocalPath, false) == "fbx" ||
			FPaths::GetExtension(AssetLocalPath, false) == "FBX" ? *FString("RawFBX") : *FString("RawOBJ")) : *FString("FBXStream")));
		
		UE_LOG(LogTemp, Log, TEXT("Create handler complete"));
		return MakeShareable<FModelClassifierHandler>(new FModelClassifierHandler(this, Mesh->GetName(), FileType, bIsFileExists ? AssetLocalPath : Path));
	}

	FString FModelClassifierCoreModule::GetThirdPartyAssetPath(int index)
	{
		FScopeLock Lock(&AssetPathMutex);
		
		if (ThirdPartyAssetPaths.IsEmpty())
		{
			return FString();
		}
		else
		{
			return ThirdPartyAssetPaths[index];
		}
	}

	FString FModelClassifierCoreModule::GetTempRenderPath()
	{
		FScopeLock Lock(&AssetPathMutex);

		FString RenderPath = FPaths::Combine(TempRenderPath, FStringHelper::GenerateRandomStringWithDate(20));
		UE_LOG(LogTemp, Log, TEXT("TempRenderPat: %s"), *RenderPath);
		
		if (!IFileManager::Get().DirectoryExists(*RenderPath))
		{
			IFileManager::Get().MakeDirectory(*RenderPath, true);
		}
		
		return RenderPath;
	}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FModelClassifierCoreModule, ModelClassifierCore)
}