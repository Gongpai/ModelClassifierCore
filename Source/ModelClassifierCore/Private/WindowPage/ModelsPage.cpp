#include "WindowPage/ModelsPage.h"
#include "ModelClassifierCorePage.h"
#include "NNE/NNETextFeatures.h"
#include "NNE/TextFeatures/TextFeatures.h"
#include "NNE/TextFeatures/TextFeaturesAsset.h"
#include "WindowPage/ModulePanel.h"

FModelsPage::FModelsPage()
{
	ModulePageName = "Models";
}

FModelsPage::~FModelsPage()
{
}

void FModelsPage::StartPage(FModulePanel* Panel)
{
	IModulePage::StartPage(Panel);
}

void FModelsPage::OpenPage()
{
	IModulePage::OpenPage();
}

void FModelsPage::ClosePage()
{
	IModulePage::ClosePage();
}

void FModelsPage::ShutdownPage()
{
	IModulePage::ShutdownPage();
}

void FModelsPage::RegisterPage()
{
	IModulePage::RegisterPage();
}

void FModelsPage::UnregisterPage()
{
	IModulePage::UnregisterPage();
}

TSharedPtr<SBox> FModelsPage::CreatePage()
{
	return IModulePage::CreatePage();
}

void FModelsPage::SaveConfigSettings(FString Key, FString Value)
{
	IModulePage::SaveConfigSettings(Key, Value);
}

FString FModelsPage::LoadConfigSettings(FString Key)
{
	return IModulePage::LoadConfigSettings(Key);
}

TSharedPtr<SButton> FModelsPage::GetTabButton()
{
	return IModulePage::GetTabButton();
}

void FModelsPage::SetTabButton(TSharedPtr<SButton> Button)
{
	IModulePage::SetTabButton(Button);
}

void FModelsPage::SetupSetting()
{
	// Download Model Action --------------------------------
	TSharedPtr<FButtonData> DownloadModelButtonData = MakeShared<FButtonData>("Download");
	TDelegate<void(TSharedPtr<void>)> DownloadModelAction;
	DownloadModelAction.BindLambda([](TSharedPtr<void> Value)
	{
		FModelClassifierCorePage::GetCore()->RunDownloadModelPython();
	});
	FSettingData<TSharedPtr<void>> DownloadModelSetting = FModulePanel::MakeSettingData<FButtonData>(DownloadModelButtonData, "Download model", Button,DownloadModelAction, "Download Tool");
	Settings.Add(DownloadModelSetting.Name, DownloadModelSetting);
	// Download Model Action --------------------------------
	
	// Download CLIPTokenizer Action --------------------------------
	TSharedPtr<FButtonData> DownloadCLIPTokenizerButtonData = MakeShared<FButtonData>("Download");
	TDelegate<void(TSharedPtr<void>)> DownloadCLIPTokenizerAction;
	DownloadCLIPTokenizerAction.BindLambda([](TSharedPtr<void> Value)
	{
		FModelClassifierCorePage::GetCore()->RunDownloadCLIPTokenizerPython();
	});
	FSettingData<TSharedPtr<void>> DownloadCLIPTokenizerSetting = FModulePanel::MakeSettingData<FButtonData>(DownloadCLIPTokenizerButtonData, "Download CLIPTokenizer", Button,DownloadCLIPTokenizerAction, "Download Tool");
	Settings.Add(DownloadCLIPTokenizerSetting.Name, DownloadCLIPTokenizerSetting);
	// Download CLIPTokenizer Action --------------------------------
	
	// Import Label Class Action --------------------------------
	TDelegate<TSharedPtr<void>(void)> GetLabelFilePathAction;
	GetLabelFilePathAction.BindLambda([this]()
	{
		return MakeShared<FEditableTextBoxData>(FEditableTextBoxData(LabelClassFile, "Path:"));
	});
	TDelegate<void(TSharedPtr<void>)> SetLabelFilePathAction;
	SetLabelFilePathAction.BindLambda([this](TSharedPtr<void> Value)
	{
		LabelClassFile = StaticCastSharedPtr<FEditableTextBoxData>(Value).Get()->File;
		UE_LOG(LogTemp, Log, TEXT("Set LabelClassFile : %s"), *LabelClassFile);
	});
	FSettingData<TSharedPtr<void>> LabelFilePathSetting = FModulePanel::MakeSettingData<FEditableTextBoxData>(MakeShared<FEditableTextBoxData>(FEditableTextBoxData(LabelClassFile, "Path:")), "Import label class", FileEditableTextBox, GetLabelFilePathAction, SetLabelFilePathAction, "Import Label Tool");
	Settings.Add(LabelFilePathSetting.Name, LabelFilePathSetting);
	// Import Label Class Action --------------------------------
	
	// Select Label File Format Action --------------------------------
	SelectedFileFormat = MakeShared<FDropDownData>(0);
	TArray<TSharedPtr<FString>> Options;
	Options.Add(MakeShared<FString>("Text"));
	Options.Add(MakeShared<FString>("ImageNet Json"));
	Options.Add(MakeShared<FString>("CSV"));
	
	SelectedFileFormat->Options = Options;
	TDelegate<TSharedPtr<void>(void)> GetLabelFileFormatAction;
	GetLabelFileFormatAction.BindLambda([this]()
	{
		return SelectedFileFormat;
	});
	TDelegate<void(TSharedPtr<void>)> SetLabelFileFormatAction;
	SetLabelFileFormatAction.BindLambda([this](TSharedPtr<void> Value)
	{
		SelectedFileFormat = StaticCastSharedPtr<FDropDownData>(Value);
	});
	FSettingData<TSharedPtr<void>> LabelFileFormatSetting = FModulePanel::MakeSettingData<FDropDownData>(SelectedFileFormat, "Select file format", DropDown, GetLabelFileFormatAction, SetLabelFileFormatAction, "Import Label Tool");
	Settings.Add(LabelFileFormatSetting.Name, LabelFileFormatSetting);
	// Select Label File Format Action --------------------------------
	
	// Convert Label File To Asset Action --------------------------------
	TSharedPtr<FButtonData> ConvertLabelFileButtonData = MakeShared<FButtonData>("Import");
	TDelegate<void(TSharedPtr<void>)> ConvertLabelFileAction;
	ConvertLabelFileAction.BindLambda([this](TSharedPtr<void> Value)
	{
		UE_LOG(LogTemp, Log, TEXT("LabelClassFile : %s"), *LabelClassFile);
		
		if (!LabelClassFile.IsEmpty())
		{
			if (ImportLabelTextFeaturesAsset)
			{
				ModelClassifierCore::FTextFeatures::GenerateLabel(LabelClassFile, static_cast<ModelClassifierCore::FileFormat>(SelectedFileFormat->SelectedOption), true, ImportLabelTextFeaturesAsset, []()
				{
					UE_LOG(LogTemp, Log, TEXT("Import label class"));
				});
			}else
			{
				UE_LOG(LogTemp, Error, TEXT("TextFeaturesAsset not set"));
			}
		}
	});
	FSettingData<TSharedPtr<void>> ConvertLabelFileSetting = FModulePanel::MakeSettingData<FButtonData>(ConvertLabelFileButtonData, "Import labels", Button,ConvertLabelFileAction, "Import Label Tool");
	Settings.Add(ConvertLabelFileSetting.Name, ConvertLabelFileSetting);
	// Convert Label File To Asset Action --------------------------------
	
	// Text Feature Asset Data (Import Labels) Action --------------------------------
	TSharedPtr<FObjectPropertyEntryBoxData> ImportLabelTextFeatureAssetButtonData = MakeShared<FObjectPropertyEntryBoxData>(UTextFeaturesAsset::StaticClass());
	TDelegate<TSharedPtr<void>(void)> GetImportLabelTextFeatureAssetAction;
	GetImportLabelTextFeatureAssetAction.BindLambda([&ImportLabelTextFeatureAssetButtonData]()
	{
		return ImportLabelTextFeatureAssetButtonData;
	});
	TDelegate<void(TSharedPtr<void>)> SetImportLabelTextFeatureAssetAction;
	SetImportLabelTextFeatureAssetAction.BindLambda([this](TSharedPtr<void> Value)
	{
		FObjectPropertyEntryBoxData* TextFeatureAsset = StaticCastSharedPtr<FObjectPropertyEntryBoxData>(Value).Get();
		UE_LOG(LogTemp, Log, TEXT("Text Feature Asset : %s"), *TextFeatureAsset->Asset.GetObjectPathString());
		ImportLabelTextFeaturesAsset = Cast<UTextFeaturesAsset>(TextFeatureAsset->Asset.GetAsset());
	});
	FSettingData<TSharedPtr<void>> LabelTextTextFeatureAssetSetting = FModulePanel::MakeSettingData<FObjectPropertyEntryBoxData>(ImportLabelTextFeatureAssetButtonData, "Text feature asset for import label", ObjectPropertyEntryBox, GetImportLabelTextFeatureAssetAction, SetImportLabelTextFeatureAssetAction, "Import Label Tool");
	Settings.Add(LabelTextTextFeatureAssetSetting.Name, LabelTextTextFeatureAssetSetting);
	// Text Feature Asset Data (Import Labels) Action--------------------------------
	
	// Text Feature Asset Data (Save ClipTokens) Action --------------------------------
	TSharedPtr<FObjectPropertyEntryBoxData> ClipTokensData = MakeShared<FObjectPropertyEntryBoxData>(UTextFeaturesAsset::StaticClass());
	TDelegate<TSharedPtr<void>(void)> GetClipTokensAction;
	GetClipTokensAction.BindLambda([&ClipTokensData]()
	{
		return ClipTokensData;
	});
	TDelegate<void(TSharedPtr<void>)> SetClipTokensAction;
	SetClipTokensAction.BindLambda([this](TSharedPtr<void> Value)
	{
		FObjectPropertyEntryBoxData* TextFeatureAsset = StaticCastSharedPtr<FObjectPropertyEntryBoxData>(Value).Get();
		UE_LOG(LogTemp, Log, TEXT("Text Feature Asset : %s"), *TextFeatureAsset->Asset.GetObjectPathString());
		ImportClipTokensTextFeaturesAsset = Cast<UTextFeaturesAsset>(TextFeatureAsset->Asset.GetAsset());
	});
	FSettingData<TSharedPtr<void>> ClipTokensSetting = FModulePanel::MakeSettingData<FObjectPropertyEntryBoxData>(ClipTokensData, "Text feature asset for save Clip Tokens", ObjectPropertyEntryBox, GetClipTokensAction, SetClipTokensAction, "Tokenization Tool");
	Settings.Add(ClipTokensSetting.Name, ClipTokensSetting);
	// Text Feature Asset Data (Save ClipTokens) Action --------------------------------
	
	// Import ONNXModel Action --------------------------------
	TDelegate<TSharedPtr<void>(void)> GetONNXModelFilePathAction;
	GetONNXModelFilePathAction.BindLambda([this]()
	{
		return MakeShared<FEditableTextBoxData>(FEditableTextBoxData(ONNXModelPath, "Path:"));
	});
	TDelegate<void(TSharedPtr<void>)> SetONNXModelFilePathAction;
	SetONNXModelFilePathAction.BindLambda([this](TSharedPtr<void> Value)
	{
		ONNXModelPath = StaticCastSharedPtr<FEditableTextBoxData>(Value).Get()->File;
		UE_LOG(LogTemp, Log, TEXT("Set ONNXModelPath : %s"), *ONNXModelPath);
	});
	FSettingData<TSharedPtr<void>> ONNXModelFilePathSetting = FModulePanel::MakeSettingData<FEditableTextBoxData>(MakeShared<FEditableTextBoxData>(FEditableTextBoxData(ONNXModelPath, "Path:")), "Import ONNXModel file", FileEditableTextBox, GetONNXModelFilePathAction, SetONNXModelFilePathAction, "Tokenization Tool");
	Settings.Add(ONNXModelFilePathSetting.Name, ONNXModelFilePathSetting);
	// Import ONNXModel Action --------------------------------
	
	// Generated Tokenization Action --------------------------------
	TSharedPtr<FButtonData> GeneratedTokenizationButtonData = MakeShared<FButtonData>("Generate");
	TDelegate<void(TSharedPtr<void>)> GeneratedTokenizationAction;
	GeneratedTokenizationAction.BindLambda([this](TSharedPtr<void> Value)
	{
		if (ImportClipTokensTextFeaturesAsset)
		{
			UE_LOG(LogTemp, Log, TEXT("Start Run Generate Tokenization"));
			bool bIsGenerateComplete = FModelClassifierCorePage::GetCore()->RunGenerateTokenization(ImportClipTokensTextFeaturesAsset, ClipTokensPath);
			
			if (!bIsGenerateComplete) return;
			
			UE_LOG(LogTemp, Log, TEXT("Start Run Get Token Length"));
			FString OutTokenLengthPath;
			bool bIsGetInOutComplete = FModelClassifierCorePage::GetCore()->RunGetTokenLength(ONNXModelPath, OutTokenLengthPath);
			
			if (!bIsGetInOutComplete) return;
			
			UE_LOG(LogTemp, Log, TEXT("Start Load Token Length File"));
			TArray<FString> TokenLengths;
			int32 InputTokenLength = 0;
			if (FFileHelper::LoadFileToStringArray(TokenLengths, *OutTokenLengthPath))
			{
				InputTokenLength = FCString::Atoi(*TokenLengths[0]);
				UE_LOG(LogTemp, Log, TEXT("Completed to load file"));
				
				ImportClipTokensTextFeaturesAsset->InputTokenLength = InputTokenLength;
				ImportClipTokensTextFeaturesAsset->OutputTokenLength = FCString::Atoi(*TokenLengths[1]);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load file"));
			}
			
			UE_LOG(LogTemp, Log, TEXT("Start Load Clip Tokens File"));
			FString JsonText;
			if (!FFileHelper::LoadFileToString(JsonText, *ClipTokensPath))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load json: %s"), *ClipTokensPath);
				return;
			}
			
			TSharedPtr<FJsonObject> Root;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
			
			if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid json"));
				return;
			}
			
			for (const auto& LabelPair : Root->Values)
			{
				const FString& Label = LabelPair.Key;
				
				const TSharedPtr<FJsonObject>* PromptObject;
				if (!LabelPair.Value->TryGetObject(PromptObject))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid prompt object for label: %s"), *Label);
					continue;
				}

				for (const auto& PromptPair : (*PromptObject)->Values)
				{
					const FString& Prompt = PromptPair.Key;

					const TArray<TSharedPtr<FJsonValue>>* JsonTokens;
					if (!PromptPair.Value->TryGetArray(JsonTokens))
					{
						UE_LOG(LogTemp, Warning, TEXT("Invalid token array: %s"), *Prompt);
						continue;
					}

					TArray<int64> TokenRow;
					TokenRow.Reserve(JsonTokens->Num());

					for (const TSharedPtr<FJsonValue>& V : *JsonTokens)
					{
						TokenRow.Add(static_cast<int64>(V->AsNumber()));
					}

					// Sanity check
					if (TokenRow.Num() != InputTokenLength)
					{
						UE_LOG(LogTemp, Warning,
							TEXT("Token length != %d for label: %s | prompt: %s"),
							InputTokenLength, *Label, *Prompt);
					}

					ImportClipTokensTextFeaturesAsset->ClipTokens.Add(
						Prompt,
						FClipTokens(Label, TokenRow)
					);
				}
			}
			
			UE_LOG(LogTemp, Log, TEXT("Complete Load All Files"));
		}
	});
	FSettingData<TSharedPtr<void>> GeneratedTokenizationSetting = FModulePanel::MakeSettingData<FButtonData>(GeneratedTokenizationButtonData, "Generated Tokenization", Button,GeneratedTokenizationAction, "Tokenization Tool");
	Settings.Add(GeneratedTokenizationSetting.Name, GeneratedTokenizationSetting);
	// Generated Tokenization Action --------------------------------
	
	// Generate Text Features Action --------------------------------
	TSharedPtr<FObjectPropertyEntryBoxData> GenerateTextFeaturesData = MakeShared<FObjectPropertyEntryBoxData>(UTextFeaturesAsset::StaticClass());
	TDelegate<TSharedPtr<void>(void)> GetGenerateTextFeaturesAction;
	GetGenerateTextFeaturesAction.BindLambda([&GenerateTextFeaturesData]()
	{
		return GenerateTextFeaturesData;
	});
	TDelegate<void(TSharedPtr<void>)> SetGenerateTextFeaturesAction;
	SetGenerateTextFeaturesAction.BindLambda([this](TSharedPtr<void> Value)
	{
		FObjectPropertyEntryBoxData* TextFeatureAsset = StaticCastSharedPtr<FObjectPropertyEntryBoxData>(Value).Get();
		UE_LOG(LogTemp, Log, TEXT("Text Feature Asset : %s"), *TextFeatureAsset->Asset.GetObjectPathString());
		GenerateTextFeaturesAsset = Cast<UTextFeaturesAsset>(TextFeatureAsset->Asset.GetAsset());
	});
	FSettingData<TSharedPtr<void>> GenerateTextFeaturesSetting = FModulePanel::MakeSettingData<FObjectPropertyEntryBoxData>(GenerateTextFeaturesData, "Text feature asset for save Text Features", ObjectPropertyEntryBox, GetGenerateTextFeaturesAction, SetGenerateTextFeaturesAction, "TextFeatures Tool");
	Settings.Add(GenerateTextFeaturesSetting.Name, GenerateTextFeaturesSetting);
	// Generate Text Features Action --------------------------------
	
	// Generate Text Features Button Action --------------------------------
	TSharedPtr<FButtonData> GenerateTextFeaturesButtonData = MakeShared<FButtonData>("Generate");
	TDelegate<void(TSharedPtr<void>)> GenerateTextFeaturesAction;
	GenerateTextFeaturesAction.BindLambda([this](TSharedPtr<void> Value)
	{
		if (!GenerateTextFeaturesAsset)
		{
			UE_LOG(LogTemp, Error, TEXT("GenerateTextFeaturesAsset is null"));
		}
		
		UClassifierNNEModel* ClassifierNNEModelData = FModelClassifierCorePage::GetCore()->GetConfig()->ClassifierNNEModelData.LoadSynchronous();
		
		if (!FModelClassifierCorePage::GetCore()->GetConfig()->ClassifierNNEModelData || !ClassifierNNEModelData)
		{
			UE_LOG(LogTemp, Error, TEXT("ClassifierNNEModelData is null"));
		}
		
		Async(EAsyncExecution::Thread, [this, ClassifierNNEModelData]()
		{
			using namespace ModelClassifierCore;
		
			NNETextRuntime = new FNNETextFeatures();
			
			NNETextRuntime->SetClassifierNNEModelData(ClassifierNNEModelData);
			NNETextRuntime->SetTextFeaturesAsset(GenerateTextFeaturesAsset);
			NNETextRuntime->SetRuntimeType(ENNEInstanceType::CPU);
			
			if (NNETextRuntime->Initialize())
			{
				UE_LOG(LogTemp, Log, TEXT("Completed to initialize NNETextRuntime"));
				NNETextRuntime->GenerateTextFeatures();
				NNETextRuntime->Cleanup();
				NNETextRuntime = nullptr;
			}else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to initialize NNETextRuntime"));
			}
		});
	});
	FSettingData<TSharedPtr<void>> GenerateTextFeaturesButtonSetting = FModulePanel::MakeSettingData<FButtonData>(GenerateTextFeaturesButtonData, "Generate Text Features", Button,GenerateTextFeaturesAction, "TextFeatures Tool");
	Settings.Add(GenerateTextFeaturesButtonSetting.Name, GenerateTextFeaturesButtonSetting);
	// Generate Text Features Button Action --------------------------------
	
	// Assign Image Normalization Action --------------------------------
	TSharedPtr<FObjectPropertyEntryBoxData> ImageNormalizationData = MakeShared<FObjectPropertyEntryBoxData>(UImageEncoderDataAsset::StaticClass());
	TDelegate<TSharedPtr<void>(void)> GetImportImageNormalizationAction;
	GetImportImageNormalizationAction.BindLambda([&ImageNormalizationData]()
	{
		return ImageNormalizationData;
	});
	TDelegate<void(TSharedPtr<void>)> SetImportImageNormalizationAction;
	SetImportImageNormalizationAction.BindLambda([this](TSharedPtr<void> Value)
	{
		FObjectPropertyEntryBoxData* ImageEncoderAsset = StaticCastSharedPtr<FObjectPropertyEntryBoxData>(Value).Get();
		UE_LOG(LogTemp, Log, TEXT("Text Feature Asset : %s"), *ImageEncoderAsset->Asset.GetObjectPathString());
		ImageNormalizationDataAsset = Cast<UImageEncoderDataAsset>(ImageEncoderAsset->Asset.GetAsset());
	});
	FSettingData<TSharedPtr<void>> ImportImageNormalizationSetting = FModulePanel::MakeSettingData<FObjectPropertyEntryBoxData>(ImageNormalizationData, "Image encoder asset for save normalize", ObjectPropertyEntryBox, GetImportImageNormalizationAction, SetImportImageNormalizationAction, "Image Normalization Tool");
	Settings.Add(ImportImageNormalizationSetting.Name, ImportImageNormalizationSetting);
	// Assign Image Normalization Action --------------------------------
	
	// Import Label Class Action --------------------------------
	TDelegate<TSharedPtr<void>(void)> GetONNXModelNormalizationAction;
	GetONNXModelNormalizationAction.BindLambda([this]()
	{
		return MakeShared<FEditableTextBoxData>(FEditableTextBoxData(ImageEncoderModelName, "Name:"));
	});
	TDelegate<void(TSharedPtr<void>)> SetONNXModelNormalizationAction;
	SetONNXModelNormalizationAction.BindLambda([this](TSharedPtr<void> Value)
	{
		ImageEncoderModelName = StaticCastSharedPtr<FEditableTextBoxData>(Value).Get()->Text;
		UE_LOG(LogTemp, Log, TEXT("Set ImageEncoderModelName : %s"), *ImageEncoderModelName);
	});
	FSettingData<TSharedPtr<void>> ONNXModelNormalizationSetting = FModulePanel::MakeSettingData<FEditableTextBoxData>(MakeShared<FEditableTextBoxData>(FEditableTextBoxData(LabelClassFile, "Name:")), "Set onnx model name", EditableTextBox, GetONNXModelNormalizationAction, SetONNXModelNormalizationAction, "Image Normalization Tool");
	Settings.Add(ONNXModelNormalizationSetting.Name, ONNXModelNormalizationSetting);
	// Import Label Class Action --------------------------------
	
	// Get Image Normalization Action --------------------------------
	TSharedPtr<FButtonData> ImageNormalizationButtonData = MakeShared<FButtonData>("Import");
	TDelegate<void(TSharedPtr<void>)> GetImageNormalizationAction;
	GetImageNormalizationAction.BindLambda([this](TSharedPtr<void> Value)
	{
		FString OutSavePath;
		FModelClassifierCorePage::GetCore()->RunExportCLIPImageNormalization(ImageEncoderModelName, OutSavePath);
		
		FString JsonText;
		if (FFileHelper::LoadFileToString(JsonText, *OutSavePath))
		{
			UE_LOG(LogTemp, Log, TEXT("Loaded Image Normalization"));
			
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
			
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				TArray<TSharedPtr<FJsonValue>> MeanArray;
				TArray<TSharedPtr<FJsonValue>> StdArray;
				
				if (JsonObject->HasField(TEXT("mean")))
				{
					UE_LOG(LogTemp, Log, TEXT("HasField mean"));
					MeanArray = JsonObject->GetArrayField(TEXT("mean"));
				}else
				{
					UE_LOG(LogTemp, Error, TEXT("Missing mean"));
					return;
				}
				
				if (JsonObject->HasField(TEXT("std")))
				{
					UE_LOG(LogTemp, Log, TEXT("HasField std"));
					StdArray = JsonObject->GetArrayField(TEXT("std"));
				}else
				{
					UE_LOG(LogTemp, Error, TEXT("Missing std"));
					return;
				}
				
				if (MeanArray.Num() < 3 || StdArray.Num() < 3)
				{
					UE_LOG(LogTemp, Error, TEXT("Data is not vector"));
				}
				
				for (TSharedPtr<FJsonValue> MeanValue : MeanArray)
				{
					ImageNormalizationDataAsset->Mean.Add(MeanValue->AsNumber());
				}

				for (TSharedPtr<FJsonValue> StdValue : StdArray)
				{
					ImageNormalizationDataAsset->Std.Add(StdValue->AsNumber());
				}
	            
				UE_LOG(LogTemp, Display, TEXT("Successfully loaded Image Normalization"));
			}
			else
			{
				
			}
		}
	});
	FSettingData<TSharedPtr<void>> ImageNormalizationSetting = FModulePanel::MakeSettingData<FButtonData>(ImageNormalizationButtonData, "Get Image normalization from onnx model", Button,GetImageNormalizationAction, "Image Normalization Tool");
	Settings.Add(ImageNormalizationSetting.Name, ImageNormalizationSetting);
	// Get Image Normalization Action --------------------------------
	
	IModulePage::SetupSetting();
}
