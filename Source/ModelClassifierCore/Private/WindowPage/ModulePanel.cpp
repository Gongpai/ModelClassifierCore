#include "WindowPage/ModulePanel.h"

IModulePage* FModulePanel::CurrentPage = nullptr;

FModulePanel::FModulePanel()
{
	
}

FModulePanel::~FModulePanel()
{
	if (CurrentPage != nullptr)
	{
		CurrentPage->ClosePage();
		CurrentPage = nullptr;
	}
	
	for (auto Page :Pages)
	{
		Page.Value->ShutdownPage();
	}

	Pages.Empty();
}

TSharedPtr<SBox> FModulePanel::CreatePanel(float Padding)
{
	if (!MainPanel.IsValid())
	{
		MainPanel = SNew(SBox)
		.Padding(Padding);
	}
	
	PanelPage = SNew(SBox)
	.Padding(0.0f)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill);

	PanelBox = CreateEntryPage();
	
	MainPanel.ToSharedRef()->SetContent(PanelBox.ToSharedRef());
	
	//Open First Page
	if (!Pages.IsEmpty())
	{
		PanelPage.Get()->SetContent(Pages.begin()->Value->CreatePage().ToSharedRef());
		CurrentPage = Pages.begin()->Value;
		
		if (Pages.begin()->Value->GetTabButton().IsValid())
		{
			Pages.begin()->Value->GetTabButton().ToSharedRef().Get().SetBorderBackgroundColor(FAppStyle::Get().GetSlateColor("Colors.Primary").GetSpecifiedColor());
			SelectedButton = Pages.begin()->Value->GetTabButton();
		}	
	}
	
	return MainPanel;
}

TSharedPtr<SBox> FModulePanel::CreateEntryPage()
{
	//Register Page
	AddPage();
	
	ScrollButtonTab = SNew(SScrollBox)
	.Orientation(EOrientation::Orient_Horizontal)
	+ SScrollBox::Slot()
	.AutoSize();
	
	for (TTuple<FString, IModulePage*> Page :Pages)
	{
		Page.Value->StartPage(this);
		
		UE_LOG(LogTemp, Warning, TEXT("Create Button Page %s"), *Page.Key);
		
		TSharedPtr<SButton> Button = SNew(SButton)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center);

		Page.Value->SetTabButton(Button);
		
		Button.ToSharedRef().Get().SetOnClicked(FOnClicked::CreateLambda([&, Page]() -> FReply {

			if (CurrentPage != nullptr)
			{
				CurrentPage->ClosePage();
				CurrentPage = nullptr;
			}
			
			FString PageName = Page.Key;
			IModulePage* ShowPage = Page.Value;
			PanelPage.Get()->SetContent(ShowPage->CreatePage().ToSharedRef());
				
			if (ShowPage->GetTabButton())
			{	
				SetSelectButton(ShowPage->GetTabButton());
			}

			CurrentPage = ShowPage;
			Page.Value->OpenPage();
			
			return FReply::Handled();
		}));
		
		Button.ToSharedRef().Get().SetContent(
		SNew(SBox)
		.Padding(0)
		.HAlign(HAlign_Left)
		[
			SNew(STextBlock)
			.Text(FText::FromString(*Page.Key))
			.Justification(ETextJustify::Left)
			.ColorAndOpacity(FLinearColor::White)
			.Font(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", 12.0f)))
		]);
		
		ButtonPages.Add(Page.Value->ModulePageName, Button);
	}
	
	return SNew(SBox)
	.Padding(0)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.AutoHeight()
		[
			//Tab bar
			ScrollButtonTab.ToSharedRef()
		]
		
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(0.0f)
			.BorderBackgroundColor(FLinearColor::Gray)
			[
				PanelPage.ToSharedRef()
			]
		]
	];
}

void FModulePanel::AddPage()
{
}

void FModulePanel::InsertSlotButtonScrollBox(TSharedPtr<SButton> Button, int32 Index)
{
	ScrollButtonTab.ToSharedRef()->InsertSlot(Index)
	[
		SNew(SBox)
		.HeightOverride(40)
		[
			Button.ToSharedRef()
		]
	];
}

void FModulePanel::SetSelectButton(TSharedPtr<SButton> Button)
{
	if (SelectedButton && SelectedButton.IsValid())
	{
		SelectedButton.ToSharedRef().Get().SetBorderBackgroundColor(FLinearColor::White);
	}
				
	Button.ToSharedRef().Get().SetBorderBackgroundColor(FAppStyle::Get().GetSlateColor("Colors.Primary").GetSpecifiedColor());
	SelectedButton = Button;
}

TSharedPtr<SBox> FModulePanel::GetCategoryBoxSettings(FString Name)
{
	if (!BoxSettings.Contains(Name))
	{
		BoxSettings.Add(Name, SNew(SBox));
	}
	return BoxSettings[Name];
}

TMap<FString, TSharedPtr<SBox>> FModulePanel::GetCategoryBoxSettings()
{
	return BoxSettings;
}

void FModulePanel::SetCategoryBoxSetting(FString Name, TSharedPtr<SBox> SettingBox)
{
	if (!SettingBox.IsValid() || SettingBox.Get() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SettingBox is invalid for category %s"), *Name);
		return;
	}
	
	BoxSettings.Add(Name, SettingBox);
}

TSharedPtr<SVerticalBox> FModulePanel::GetLeftBoxSettings(FString Name)
{
	if (!LeftBoxSettings.Contains(Name))
	{
		LeftBoxSettings.Add(Name, SNew(SVerticalBox));
	}
	return LeftBoxSettings[Name];
}

void FModulePanel::SetLeftBoxSetting(FString Name, TSharedPtr<SVerticalBox> SettingBox)
{
	LeftBoxSettings.Add(Name, SettingBox);
}

TSharedPtr<SVerticalBox> FModulePanel::GetRightBoxSettings(FString Name)
{
	if (!RightBoxSettings.Contains(Name))
	{
		RightBoxSettings.Add(Name, SNew(SVerticalBox));
	}
	return RightBoxSettings[Name];
}

void FModulePanel::SetRightBoxSettings(FString Name, TSharedPtr<SVerticalBox> SettingBox)
{
	RightBoxSettings.Add(Name, SettingBox);
}

void FModulePanel::AddSettingModule(FString Name, IModulePage* Owner, TMap<FString, FSettingData<TSharedPtr<void>>> SettingActions)
{
	TSharedPtr<SButton> Button = SNew(SButton)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center);

	UE_LOG(LogTemp, Error, TEXT("Add Setting %s Module"), *Name);
	AllSettingModules.Data.Add(Name, SettingActions);
	
	ButtonPages.Add(Owner->ModulePageName, Button);
	ComposeSettingButton(ButtonPages[Owner->ModulePageName], Owner);
	
	Button.ToSharedRef().Get().SetOnClicked(FOnClicked::CreateLambda([&, this, Name, Owner]() -> FReply {
		UE_LOG(LogTemp, Warning, TEXT("Add Setting %s Module"), *Name);
		UE_LOG(LogTemp, Warning, TEXT("CurrentSettingPageName : %s"), *CurrentSettingPageName);
		FString ModuleName = Name;

		/*if (!CurrentSettingPageName.IsEmpty())
		{
			CurrentPage->ClosePage();
			CurrentPage->ModulePageName.Empty();
		}
		*/
		
		auto LastIt = AllSettingModules.Data.Find(Name);
		if (LastIt == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Error: Name not found in AllSettingModules"));
			return FReply::Unhandled();
		}
	
		if (!AllSettingModules.Data.Contains(Name))  
		{
			UE_LOG(LogTemp, Error, TEXT("Key %s not found in AllSettingModules!"), *Name);
		}else
		{
			UE_LOG(LogTemp, Log, TEXT("Key %s found in AllSettingModules!"), *Name);
			
			if (ButtonPages.Contains(Owner->ModulePageName))
			{
				SetSelectButton(ButtonPages[Owner->ModulePageName]);
			}

			CurrentSettingPageName = ModuleName;
			CurrentPage->ModulePageName = CurrentSettingPageName;
			UE_LOG(LogTemp, Warning, TEXT("CurrentSettingPageName : %s"), *CurrentSettingPageName);
			CurrentPage->OpenPage();
			CreateAllSettingInModule(Owner);
		}
		return FReply::Handled();
	}));
		
	Button.ToSharedRef().Get().SetContent(
	SNew(SBox)
	.Padding(0)
	.HAlign(HAlign_Left)
	[
		SNew(STextBlock)
		.Text(FText::FromString(*Name))
		.Justification(ETextJustify::Left)
		.ColorAndOpacity(FLinearColor::White)
		.Font(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", 12.0f)))
	]);
}

void FModulePanel::CreateAllSettingInModule(IModulePage* Owner)
{
	if (Owner != nullptr && AllSettingModules.Data.Contains(Owner->ModulePageName) && CurrentPage != Owner)
	{
		TMap<FString, FSettingData<TSharedPtr<void>>> SettingActions = AllSettingModules.Data[Owner->ModulePageName];
		UE_LOG(LogTemp, Warning, TEXT("Create All Setting In Module"));
		
		BoxSettings.Empty();
		LeftBoxSettings.Empty();
		RightBoxSettings.Empty();
		
		for (auto Setting : SettingActions)
		{
			UE_LOG(LogTemp, Warning, TEXT("Create Setting %s"), *Setting.Key);
			if (Setting.Value.GetAction.IsBound())
			{
				TSharedPtr<void> Value = Setting.Value.GetValue();
				CurrentPage->CreateSettingEntry(MakeShared<FSettingData<TSharedPtr<void>>>(
				Value,
				Setting.Value.Name,
				Setting.Value.SettingType,
				Setting.Value.GetAction,
				Setting.Value.SetAction,
				Setting.Value.Category
				));
			}else
			{
				UE_LOG(LogTemp, Warning, TEXT("Setting Value GetAction Is Not Bound"));
			}
		}

		CurrentPage->CreateSettingList();
	}
}

void FModulePanel::ComposeSettingButton(TSharedPtr<SButton> Button, IModulePage* Owner)
{
	InsertSlotButtonScrollBox(Button, 0);
}

FString FModulePanel::GetCurrentSettingPageName()
{
	return CurrentSettingPageName;
}

void FModulePanel::SetCurrentSettingPageName(FString Name)
{
	CurrentSettingPageName = Name;
}

bool FModulePanel::GetAutoOpenModuleSetting()
{
	return bAutoOpenModuleSetting;
}

void FModulePanel::ShowSettingIf(FString Name, TSharedPtr<TFunction<EVisibility()>> Action)
{
	CurrentPage->ShowSettingIf(Name, Action);
}