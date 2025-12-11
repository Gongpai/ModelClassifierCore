#include "ModelClassifierCorePage.h"
#include "WindowPage/MainPanel.h"

#define LOCTEXT_NAMESPACE "FModelClassifierCoreModule"

ModelClassifierCore::FModelClassifierCoreModule* FModelClassifierCorePage::Core;
TSharedPtr<SDockTab> FModelClassifierCorePage::DockTab = nullptr;
TMap<FString, TSharedPtr<SBox>> FModelClassifierCorePage::ModuleBoxButtons;
TMap<FString, TSharedPtr<SButton>> FModelClassifierCorePage::ModuleButtons;
TSharedPtr<SBox> FModelClassifierCorePage::TextModuleEmpty;
TSharedPtr<SScrollBox> FModelClassifierCorePage::ModuleScrollBox;
TSharedPtr<STextBlock> FModelClassifierCorePage::TextModuleName;
TSharedPtr<SBox> FModelClassifierCorePage::MainPanel;
TSharedPtr<SButton> FModelClassifierCorePage::SelectedButton;
TSharedPtr<SBox> FModelClassifierCorePage::SettingButton = nullptr;
FModulePanel* FModelClassifierCorePage::ModulePanel = nullptr;

TSharedRef<class SDockTab> FModelClassifierCorePage::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Loading {0} : {1} window's contents..."),
		FText::FromString(TEXT("Model Classifier Tools")),
		FText::FromString(TEXT("ModelClassifierCore.cpp"))
		);

	ModulePanel = new FMainPanel();
	MainPanel = ModulePanel->CreatePanel(2.0f);
		
	if (!MainPanel.IsValid())
	{
		MainPanel = SNew(SBox)
		.Padding(0.0f)
		[
			SNew(STextBlock).Text(FText::FromString("Empty Page."))
         	.Justification(ETextJustify::Center)
         	.ColorAndOpacity(FLinearColor::White)
		];
	}
	
	if (!TextModuleName.IsValid())
	{
		TextModuleName = SNew(STextBlock).Text(FText::FromString("Select a submodule to configure."))
		.Justification(ETextJustify::Center)
		.ColorAndOpacity(FLinearColor::White)
		.Font(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", 16.0f)));
	}
		
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			[
				MainPanel.ToSharedRef()
			]	
		];
}

TSharedRef<SDockTab> FModelClassifierCorePage::GetPluginTab()
{
	if (!DockTab.IsValid())
	{
		DockTab = FGlobalTabmanager::Get()->GetActiveTab();
	}
	
	return DockTab.ToSharedRef();
}

ModelClassifierCore::FModelClassifierCoreModule* FModelClassifierCorePage::GetCore()
{
	return Core;
}

void FModelClassifierCorePage::Initialize(ModelClassifierCore::FModelClassifierCoreModule* InCore)
{
	Core = InCore;
	
	UE_LOG(LogTemp, Log, TEXT("StartupModule: Initializing Page"));
}

void FModelClassifierCorePage::Shutdown()
{
	
}

#undef LOCTEXT_NAMESPACE
