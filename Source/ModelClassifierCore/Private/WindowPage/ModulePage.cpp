#include "WindowPage/ModulePage.h"
#include "WindowPage/ModulePanel.h"
#include "WindowPage/SettingComponent.h"

IModulePage::IModulePage()
{
}

IModulePage::~IModulePage()
{
}

void IModulePage::ClearVisibilitySettingBoxes()
{
	if (VisibilitySettingBoxes.Num() > 0)
	{
		VisibilitySettingBoxes.Empty();
	}
}

EVisibility IModulePage::GetSettingVisibilityFunction(TSharedPtr<TFunction<EVisibility()>>* Function, FString ModuleName, FString Category, FString ID)
{
	EVisibility Visibility = EVisibility::Visible;
	if (Function)
	{
		if (Function->IsValid())
		{
			Visibility = (**Function)();
		}
	}

	//UE_LOG(LogTemp, Warning, TEXT("Visibility: %s"), *Visibility.ToString());
	AutoRegisterVisibilitySettingEntry(ModuleName, Category, ID, Visibility);
	return Visibility;
}

EVisibility IModulePage::GetAllSettingVisibilityEntry(FString ModuleName, FString Category)
{
	/*UE_LOG(LogTemp, Error, TEXT("GetAllSettingVisibilityEntry ------>"));
	for (auto VisibilityEntry : VisibilitySettingEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("Key (- %s -)"), *VisibilityEntry.Key);
		for (auto VisibilityValue : VisibilityEntry.Value)
		{
			UE_LOG(LogTemp, Warning, TEXT("|_ Value (- %s -) Key"), *VisibilityValue.Key);
			for (auto Value : VisibilityValue.Value)
			{
				UE_LOG(LogTemp, Warning, TEXT("--|_ Key (- %s -)"), *Value);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("----------------------------------------------"));
	}*/
	
	if (VisibilitySettingEntry.Contains(ModuleName))
	{
		//UE_LOG(LogTemp, Warning, TEXT("VisibilitySettingEntry[ModuleName] Num %d"), VisibilitySettingEntry[ModuleName].Num());
		if (VisibilitySettingEntry[ModuleName].Contains(Category))
		{
			//UE_LOG(LogTemp, Warning, TEXT("VisibilitySettingEntry[ModuleName][Category] Num %d"), VisibilitySettingEntry[ModuleName][Category].Num());
			return VisibilitySettingEntry[ModuleName][Category].Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
		}
	}

	return EVisibility::Collapsed;
}

void IModulePage::AutoRegisterVisibilitySettingEntry(FString ModuleName, FString Category, FString ID, EVisibility Visibility)
{
	/*UE_LOG(LogTemp, Error, TEXT("AutoRegisterVisibilitySettingEntry ------>"));
	for (auto VisibilityEntry : VisibilitySettingEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("Key (- %s -)"), *VisibilityEntry.Key);
		for (auto VisibilityValue : VisibilityEntry.Value)
		{
			UE_LOG(LogTemp, Warning, TEXT("|_ Value (- %s -) Key"), *VisibilityValue.Key);
			for (auto Value : VisibilityValue.Value)
			{
				UE_LOG(LogTemp, Warning, TEXT("--|_ Key (- %s -)"), *Value);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("----------------------------------------------"));
	}
	UE_LOG(LogTemp, Warning, TEXT("ModuleName : %s | Category : %s | ID : %s"), *ModuleName, *Category, *ID);*/
	
	if (VisibilitySettingEntry.Contains(ModuleName))
	{
		if (VisibilitySettingEntry[ModuleName].Contains(Category))
		{
			if (Visibility == EVisibility::Collapsed && VisibilitySettingEntry[ModuleName][Category].Contains(ID))
			{
				VisibilitySettingEntry[ModuleName][Category].Remove(ID);
			}else if (Visibility == EVisibility::Visible && !VisibilitySettingEntry[ModuleName][Category].Contains(ID))
			{
				VisibilitySettingEntry[ModuleName][Category].Add(ID);
			}
		}else
		{
			VisibilitySettingEntry[ModuleName].Add(Category, {ID});
		}
	} else
	{
		if (Visibility == EVisibility::Visible)
		{
			TMap<FString, TArray<FString>> NewVisibilitySetting;
			NewVisibilitySetting.Add(Category, {ID});
			VisibilitySettingEntry.Add(ModuleName, NewVisibilitySetting);
		}
	}
}

void IModulePage::StartPage(FModulePanel* Panel)
{
	UE_LOG(LogTemp, Warning, TEXT("Start Page %s"), *ModulePageName);
	ModulePanel = Panel;
	RegisterPage();
}

void IModulePage::OpenPage()
{
	UE_LOG(LogTemp, Warning, TEXT("Open Page %s"), *ModulePageName);
}

void IModulePage::ClosePage()
{
	VisibilitySettingEntry.Empty();
	
	UE_LOG(LogTemp, Warning, TEXT("Close Page %s"), *ModulePageName);
}

void IModulePage::ShutdownPage()
{
	UnregisterPage();
}

void IModulePage::RegisterPage()
{
	SetupSetting();
}

void IModulePage::UnregisterPage()
{
	Settings.Empty();
}

TSharedPtr<SBox> IModulePage::CreatePage()
{
	return CreateSettingPage().ToSharedRef();
}

void IModulePage::SetupSetting()
{
	if (!Settings.IsEmpty())
	{
		ModulePanel->AddSettingModule(ModulePageName, this, Settings);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Setup Setting %s In Module"), *ModulePageName);
}

TSharedPtr<SButton> IModulePage::GetTabButton()
{
	return TabButton;
}

void IModulePage::SetTabButton(TSharedPtr<SButton> Button)
{
	TabButton = Button;
}

void IModulePage::SaveConfigSettings(FString Key, FString Value)
{
	// Add Save Here
}

FString IModulePage::LoadConfigSettings(FString Key)
{
	FString Value;
	// Add load here
	
	return Value;
}

TSharedPtr<SBox> IModulePage::CreateSettingPage()
{
	if (!SettingModuleName.IsValid())
	{
		TSharedPtr<STextBlock> TextBlock = SNew(STextBlock)
		.Text(FText::FromString("Select settings from the top bar."))
		.Justification(ETextJustify::Left)
		.ColorAndOpacity(FLinearColor::White)
		.Font(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular", 10.0f)));

		SettingModuleName = SNew(SBox)
		.Padding(10.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		.HeightOverride(35.0f)
		[
			TextBlock.ToSharedRef()
		];

		ModulePanel->SetCategoryBoxSetting("None", SettingModuleName);
	}
	
	ClearVisibilitySettingBoxes();
	
	ScrollBoxSettingList = SNew(SScrollBox);
	
	CreateSettingList();

	return SNew(SBox)
	.Padding(0.0f)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		ScrollBoxSettingList.ToSharedRef()
	];		
}

void IModulePage::CreateSettingList()
{
	if (!ScrollBoxSettingList.IsValid())
	{
		ScrollBoxSettingList = SNew(SScrollBox);
	}
	
	ScrollBoxSettingList.ToSharedRef()->ClearChildren();
	
	//UE_LOG(LogTemp, Warning, TEXT("Create Setting List"));

	for (auto Box : ModulePanel->GetCategoryBoxSettings())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Create Setting Category %s"), *Box.Key);
	
		ScrollBoxSettingList.ToSharedRef()->AddSlot()
		.AutoSize()
		[
			Box.Value.ToSharedRef()
		];
		
		ScrollBoxSettingList.ToSharedRef()->AddSlot()
		[
			SNew(SBox)
			.Padding(10.0f, 0.0f, 0.0f, 0.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SSplitter)
				+ SSplitter::Slot()
				.Value(0.4f)
				[
					SNew(SBox)
					.Padding(0.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						ModulePanel->GetLeftBoxSettings(Box.Key).ToSharedRef()
					]
				]
	
				+ SSplitter::Slot()
				.Value(0.6f)
				[
					SNew(SBox)
					.Padding(0.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)
				
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Fill)
						.HAlign(HAlign_Fill)
						.MaxWidth(300.0f)
						[
							ModulePanel->GetRightBoxSettings(Box.Key).ToSharedRef()
						]
						
						+ SHorizontalBox::Slot()
						.FillWidth(0.0f)
						.VAlign(VAlign_Fill)
						.HAlign(HAlign_Fill)
						[
							SNew(SBox)
						]
					]
				]
			]
		];
	}
}

void IModulePage::CreateSettingEntry(TSharedPtr<FSettingData<TSharedPtr<void>>> SettingData)
{
	if (SettingModuleName.IsValid())
	{
		ScrollBoxSettingList->ClearChildren();
		ModulePanel->GetCategoryBoxSettings().Remove("None");
		SettingModuleName = nullptr;
	}
	
	TSharedPtr<SBox>* CategoryBox = ModulePanel->GetCategoryBoxSettings().Find(SettingData->Category);
	if (!CategoryBox)
	{
		TSharedPtr<STextBlock> TextBlock = SNew(STextBlock)
		.Text(FText::FromString("Select settings from the top bar."))
		.Justification(ETextJustify::Left)
		.ColorAndOpacity(FLinearColor::White)
		.Font(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular", 10.0f)));
		
		TextBlock->SetText(FText::FromString(
		SettingData->Category == "Default" ?
		FString::Printf(TEXT("%s Tools"), *ModulePageName) :
		FString::Printf(TEXT("%s"), *SettingData->Category))
		);
		
		UE_LOG(LogTemp, Warning, TEXT("Create Category Box Settings %s"), *SettingData->Category);
		
		TSharedPtr<SBox> Box = SNew(SBox)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		.Visibility_Lambda([this, SettingData]()
		{
			FString& Category = SettingData->Category;
			FString& ModuleName = ModulePageName;
			
			return GetAllSettingVisibilityEntry(ModuleName, Category);
		})
		[
			SNew(SBorder).VAlign(VAlign_Center)
			.Padding(5.0f)
			.HAlign(HAlign_Fill)
			[
				TextBlock.ToSharedRef()
			]
		];
		
		ModulePanel->SetCategoryBoxSetting(SettingData->Category, Box);
	}
	
	if (!VisibilitySettingBoxes.Contains(SettingData->Name))
	{
		TSharedPtr<TFunction<EVisibility()>> VisibilityFunc = MakeShared<TFunction<EVisibility()>>([]() { return EVisibility::Visible;});
		VisibilitySettingBoxes.Add(SettingData->Name, VisibilityFunc);
	}
		
	ModulePanel->GetLeftBoxSettings(SettingData->Category).ToSharedRef()->AddSlot()
	[
		SNew(SBox)
		.Padding(0.0f)
		.HeightOverride(35.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Visibility_Lambda([this, SettingData]() {
			FString& SettingName = SettingData->Name;
			FString& Category = SettingData->Category;
			FString& ModuleName = ModulePageName;
			FString& ID = SettingData->GetID();
	
			return GetSettingVisibilityFunction(VisibilitySettingBoxes.Find(SettingName), ModuleName, Category, ID);
		})
		[
			FSettingComponent::CreateSettingName(SettingData->Name).ToSharedRef()
		]
	];

	switch (SettingData->SettingType) 
	{
		case Button:
		{
			TSharedPtr<FSettingData<TSharedPtr<FButtonData>>> SettingButtonData = MakeShared<FSettingData<TSharedPtr<FButtonData>>>(
			StaticCastSharedPtr<FButtonData>(SettingData->GetValue()),
			SettingData->Name,
			SettingData->SettingType,
			SettingData->GetAction,
			SettingData->SetAction
			);
				
			TSharedPtr<SBox> Box = SNew(SBox)
			.Padding(0.0f)
			.HeightOverride(35.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Visibility_Lambda([this, SettingData]() {
				FString& SettingName = SettingData->Name;
				FString& Category = SettingData->Category;
				FString& ModuleName = ModulePageName;
				FString& ID = SettingData->GetID();
	
				return GetSettingVisibilityFunction(VisibilitySettingBoxes.Find(SettingName), ModuleName, Category, ID);
			})
			[
				FSettingComponent::CreateButton(SettingButtonData).ToSharedRef()
			];
			
			ModulePanel->GetRightBoxSettings(SettingData->Category).ToSharedRef()->AddSlot()
			[
				Box.ToSharedRef()
			];
			break;
		}
		case Slider:
		{
			TSharedPtr<FSettingData<TSharedPtr<FSliderData>>> SettingDataFloat = MakeShared<FSettingData<TSharedPtr<FSliderData>>>(
			StaticCastSharedPtr<FSliderData>(SettingData->GetValue()),
			SettingData->Name,
			SettingData->SettingType,
			SettingData->GetAction,
			SettingData->SetAction
			);
	
			TSharedPtr<SBox> Box = SNew(SBox)
			.Padding(0.0f)
			.HeightOverride(35.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Visibility_Lambda([this, SettingData]() {
				FString& SettingName = SettingData->Name;
				FString& Category = SettingData->Category;
				FString& ModuleName = ModulePageName;
				FString& ID = SettingData->GetID();
	
				return GetSettingVisibilityFunction(VisibilitySettingBoxes.Find(SettingName), ModuleName, Category, ID);
			})
			[
				FSettingComponent::CreateSlider(SettingDataFloat).ToSharedRef()
			];
			
			ModulePanel->GetRightBoxSettings(SettingData->Category).ToSharedRef()->AddSlot()
			[
				Box.ToSharedRef()
			];
			break;
		}
		case CheckBox:
		{
			TSharedPtr<FSettingData<TSharedPtr<ECheckBoxState>>> SettingDataBool = MakeShared<FSettingData<TSharedPtr<ECheckBoxState>>>(
			StaticCastSharedPtr<ECheckBoxState>(SettingData->GetValue()),
			SettingData->Name,
			SettingData->SettingType,
			SettingData->GetAction,
			SettingData->SetAction
			);

			TSharedPtr<SBox> Box = SNew(SBox)
			.Padding(0.0f)
			.HeightOverride(35.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Visibility_Lambda([this, SettingData]() {
				FString& SettingName = SettingData->Name;
				FString& Category = SettingData->Category;
				FString& ModuleName = ModulePageName;
				FString& ID = SettingData->GetID();
	
				return GetSettingVisibilityFunction(VisibilitySettingBoxes.Find(SettingName), ModuleName, Category, ID);
			})
			[
				FSettingComponent::CreateCheckBox(SettingDataBool).ToSharedRef()
			];
			
			ModulePanel->GetRightBoxSettings(SettingData->Category).ToSharedRef()->AddSlot()
			[
				SNew(SBox)
				.Padding(0.0f)
				.HeightOverride(35.0f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					Box.ToSharedRef()
				]
			];
			break;
		}
		case DropDown:
		{
			TSharedPtr<FSettingData<TSharedPtr<FDropDownData>>> SettingDataInt = MakeShared<FSettingData<TSharedPtr<FDropDownData>>>(
			StaticCastSharedPtr<FDropDownData>(SettingData->GetValue()),
			SettingData->Name,
			SettingData->SettingType,
			SettingData->GetAction,
			SettingData->SetAction
			);

			TSharedPtr<SBox> Box = SNew(SBox)
			.Padding(0.0f)
			.HeightOverride(35.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Visibility_Lambda([this, SettingData]() {
				FString& SettingName = SettingData->Name;
				FString& Category = SettingData->Category;
				FString& ModuleName = ModulePageName;
				FString& ID = SettingData->GetID();
	
				return GetSettingVisibilityFunction(VisibilitySettingBoxes.Find(SettingName), ModuleName, Category, ID);
			})
			[
				FSettingComponent::CreateDropDown(SettingDataInt).ToSharedRef()
			];
			
			ModulePanel->GetRightBoxSettings(SettingData->Category).ToSharedRef()->AddSlot()
			[
				SNew(SBox)
				.Padding(0.0f)
				.HeightOverride(35.0f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					Box.ToSharedRef()
				]
			];
			break;
		}
		case ObjectPropertyEntryBox:
		{
			TSharedPtr<FSettingData<TSharedPtr<FObjectPropertyEntryBoxData>>> SettingDataString = MakeShared<FSettingData<TSharedPtr<FObjectPropertyEntryBoxData>>>(
			StaticCastSharedPtr<FObjectPropertyEntryBoxData>(SettingData->GetValue()),
			SettingData->Name,
			SettingData->SettingType,
			SettingData->GetAction,
			SettingData->SetAction
			);
				
			TSharedPtr<SBox> Box = SNew(SBox)
			.Padding(0.0f)
			.HeightOverride(35.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Visibility_Lambda([&, SettingData]() {
				FString& SettingName = SettingData->Name;
				FString& Category = SettingData->Category;
				FString& ModuleName = ModulePageName;
				FString& ID = SettingData->GetID();

				return GetSettingVisibilityFunction(VisibilitySettingBoxes.Find(SettingName), ModuleName, Category, ID);
			})
			[
				FSettingComponent::CreateObjectPropertyEntryBox(SettingDataString).ToSharedRef()
			];
		
			ModulePanel->GetRightBoxSettings(SettingData->Category).ToSharedRef()->AddSlot()
			[
				SNew(SBox)
				.Padding(0.0f)
				.HeightOverride(35.0f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					Box.ToSharedRef()
				]
			];
		}
		default: 
			{
				if (SettingData->SettingType == EditableTextBox || SettingData->SettingType == FileEditableTextBox)
				{
					TSharedPtr<FSettingData<TSharedPtr<FEditableTextBoxData>>> SettingDataString = MakeShared<FSettingData<TSharedPtr<FEditableTextBoxData>>>(
					StaticCastSharedPtr<FEditableTextBoxData>(SettingData->GetValue()),
					SettingData->Name,
					SettingData->SettingType,
					SettingData->GetAction,
					SettingData->SetAction
					);

					TSharedPtr<SBox> Box = SNew(SBox)
					.Padding(0.0f)
					.HeightOverride(35.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Visibility_Lambda([this, SettingData]() {
						FString& SettingName = SettingData->Name;
						FString& Category = SettingData->Category;
						FString& ModuleName = ModulePageName;
						FString& ID = SettingData->GetID();
	
						return GetSettingVisibilityFunction(VisibilitySettingBoxes.Find(SettingName), ModuleName, Category, ID);
					});
					
					TSharedPtr<SBox> EditableTextBox;
					FSettingComponent::CreateEditableTextBox(SettingDataString, EditableTextBox);
					Box->SetContent(EditableTextBox.ToSharedRef());
			
					ModulePanel->GetRightBoxSettings(SettingData->Category).ToSharedRef()->AddSlot()
					[
						SNew(SBox)
						.Padding(0.0f)
						.HeightOverride(35.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							Box.ToSharedRef()
						]
					];
				}
				
			break;
		}
	}
}

void IModulePage::ShowSettingIf(FString Name, TSharedPtr<TFunction<EVisibility()>> Action)
{
	UE_LOG(LogTemp, Warning, TEXT("Setting Name : %s"), *Name);
	VisibilitySettingBoxes.Add(Name, Action);
	
	UE_LOG(LogTemp, Warning, TEXT("Setting Name : %s Func is valid %s"), *Name, *(VisibilitySettingBoxes[Name].IsValid() ? FString("True") : FString("False")));
}