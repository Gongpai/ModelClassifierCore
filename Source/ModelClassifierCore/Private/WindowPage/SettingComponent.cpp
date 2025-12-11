#include "WindowPage/SettingComponent.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"

TSharedPtr<STextBlock> FSettingComponent::CreateSettingName(FString Name)
{
	TSharedPtr<SBox> TextBox = SNew(SBox)
	.Padding(0.0f)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	.HeightOverride(35.0F);
	
	TSharedPtr<STextBlock> TextBlock = SNew(STextBlock)
	.Text(FText::FromString(*Name))
	.Justification(ETextJustify::Left)
	.ColorAndOpacity(FLinearColor::White)
	.Font(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular", 10.0f)));
	
	TextBox->SetContent(TextBlock.ToSharedRef());
	return TextBlock;
}

TSharedPtr<SButton> FSettingComponent::CreateButton(TSharedPtr<FSettingData<TSharedPtr<FButtonData>>> SettingData)
{
	TSharedPtr<SBox> Box = SNew(SBox)
	.Padding(0.0f)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	.HeightOverride(35.0F);
	
	TSharedPtr<SButton> Button = SNew(SButton)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.OnClicked_Lambda([&, SettingData]()
	{
		if (SettingData->SetAction.IsBound())
		{
			SettingData->SetAction.Execute(SettingData->GetValue());
		}
		
		return FReply::Handled();
	})
	[
		SNew(SBox)
		.Padding(0.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text_Lambda([&, SettingData]()->FText
			{
				return FText::FromString(SettingData->GetValue()->ButtonLabel);
			})
		]
	];
	
	Box->SetContent(Button.ToSharedRef());
	return Button;
}

TSharedPtr<SSlider> FSettingComponent::CreateSlider(TSharedPtr<FSettingData<TSharedPtr<FSliderData>>> SettingData)
{
	TSharedPtr<SBox> Box = SNew(SBox)
	.Padding(0.0f)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	.HeightOverride(35.0F);
	
	TSharedPtr<SSlider> Slider = SNew(SSlider)
	.MinValue(SettingData->GetValue()->Min)
	.MaxValue(SettingData->GetValue()->Max)
	.Value_Lambda([&, SettingData]() -> float
	{
		return SettingData->GetValue()->Value;
	})
	.OnValueChanged_Lambda([&, SettingData](float NewValue)
	{
		SettingData->GetValue()->Value = NewValue;
		
		if (SettingData->SetAction.IsBound())
		{
			SettingData->SetAction.Execute(SettingData->GetValue());
		}
	});
	
	Box->SetContent(Slider.ToSharedRef());
	return Slider;
}

TSharedPtr<SCheckBox> FSettingComponent::CreateCheckBox(TSharedPtr<FSettingData<TSharedPtr<ECheckBoxState>>> SettingData)
{	
	TSharedPtr<SCheckBox> CheckBox = SNew(SCheckBox)
	.Padding(0.0)
	.IsChecked_Lambda([&, SettingData]() -> ECheckBoxState
	{
		return SettingData->GetValue().ToSharedRef().Get();
	})
	.OnCheckStateChanged_Lambda([&, SettingData](ECheckBoxState NewState) 
	{
		if (SettingData->SetAction.IsBound())
		{
			SettingData->SetAction.Execute(MakeShared<ECheckBoxState>(NewState));
		}

		SettingData->SetValue(MakeShared<ECheckBoxState>(NewState));
	});
	
	return CheckBox;
}

TSharedPtr<SComboBox<TSharedPtr<FString>>> FSettingComponent::CreateDropDown(TSharedPtr<FSettingData<TSharedPtr<FDropDownData>>> SettingData)
{
	UE_LOG(LogTemp, Warning, TEXT("Option Num : %d"), SettingData->GetValue()->Options.Num());
	
	TSharedPtr<STextBlock> OptionText = SNew(STextBlock)
	.Text_Lambda([&, SettingData]() -> FText {
		if (SettingData->GetValue()->SelectedOption > SettingData->GetValue()->Options.Num() - 1)
		{
			UE_LOG(LogTemp, Error, TEXT("Key not found in options!"));
			return FText::FromString("");
		}else
		{
			//UE_LOG(LogTemp, Log, TEXT("Key found in options!"));
			return FText::FromString(*SettingData->GetValue()->Options[SettingData->GetValue()->SelectedOption].ToSharedRef().Get());
		}
	});
	
	TWeakPtr<STextBlock> WeakOptionText = OptionText;
	
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBox = SNew(SComboBox<TSharedPtr<FString>>)
	.OptionsSource(&SettingData->GetValue()->Options)
	.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>{
		return SNew(SBox)
		.Padding(0.0f)
		.HeightOverride(35.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		[
			SNew(STextBlock)
			.Text(FText::FromString(*Item))
			.Justification(ETextJustify::Left)
			.Font(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular", 10.0f)))
		];
	})
	.OnSelectionChanged_Lambda([&, SettingData, WeakOptionText](TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
	{
		int SelectedOptionIndex = SettingData->GetValue()->Options.IndexOfByKey(NewSelection);
		
		UE_LOG(LogTemp, Warning, TEXT("Selected: %s (Index: %d)"), **NewSelection, SelectedOptionIndex);
		
		if (TSharedPtr<STextBlock> NewOptionText = WeakOptionText.Pin())
		{
			NewOptionText->SetText(FText::FromString(*NewSelection));
		}
		
		if (SettingData->SetAction.IsBound())
		{
			SettingData->GetValue()->SelectedOption = SelectedOptionIndex;
			SettingData->SetAction.Execute(SettingData->GetValue());
		}
	})
	[
		OptionText.ToSharedRef()
	];
	
	return ComboBox;
}

TSharedPtr<SEditableTextBox> FSettingComponent::CreateEditableTextBox(
	TSharedPtr<FSettingData<TSharedPtr<FEditableTextBoxData>>> SettingData,
	TSharedPtr<SBox>& Box)
{
	if (!SettingData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid setting!"));
		return SNew(SEditableTextBox);
	}
	
	if (!SettingData->GetValue().IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SettingData->GetValue() is null for setting: %s"), *SettingData->Name);
		return SNew(SEditableTextBox);
	}
	
	TSharedPtr<SEditableTextBox> EditableTextBox = SNew(SEditableTextBox)
	.Padding(0.0)
	.Text_Lambda([&, SettingData](){
		return FText::FromString(*SettingData->GetValue()->Text);
	})
	.Justification(ETextJustify::Left)
	.Font(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular", 10.0f)))
	.HintText_Lambda([&, SettingData]() {
		return FText::FromString(*SettingData->GetValue()->HintText);
	})
	.OnTextCommitted_Lambda([&, SettingData](const FText& NewText, ETextCommit::Type CommitType) {
		if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
		{
			SettingData->GetValue()->Text = NewText.ToString();
				
			if (SettingData->SetAction.IsBound())
			{
				UE_LOG(LogTemp, Log, TEXT("Text committed: %s"), *SettingData->GetValue()->Text);
				SettingData->SetAction.Execute(SettingData->GetValue());
			}
		}
	});
	
	TSharedPtr<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);
	HorizontalBox->AddSlot()
	.AutoWidth().FillWidth(SettingData->SettingType == FileEditableTextBox ? 0.7f : 1.0f)
	[
		EditableTextBox.ToSharedRef()
	];
	
	if (SettingData->SettingType == FileEditableTextBox)
	{
		HorizontalBox->AddSlot()
		.AutoWidth().FillWidth(0.3f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([&, SettingData]()->FReply
			{
				TArray<FString> Files;
				IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

				if(DesktopPlatform)
				{
					DesktopPlatform->OpenFileDialog(
					nullptr,
					TEXT("Select a File"),
					TEXT(""),
					TEXT(""),
					TEXT("Json Files (*.json)|*.json|Text Files (*.txt)|*.txt|Merges Files (*.bpe)|*.bpe|ONNX Files (*.onnx)|*.onnx|All Files (*.*)|*.*"),
					EFileDialogFlags::None,
					Files
					);
				}
				
				if (Files.Num() > 0)
				{
					SettingData->GetValue()->File = Files[0];
				}
				
				SettingData->GetValue()->Text = SettingData->GetValue()->File;
				
				if (SettingData->SetAction.IsBound())
				{
					UE_LOG(LogTemp, Log, TEXT("Text committed: %s"), *SettingData->GetValue()->Text);
					SettingData->SetAction.Execute(SettingData->GetValue());
				}
				
				return FReply::Handled();
			})
			[
				SNew(SBox)
				.Padding(0.0f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center).Text(FText::FromString("Browse"))
				]
			]
		];
	}
	
	Box = SNew(SBox)
	.Padding(0.0)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		HorizontalBox.ToSharedRef()
	];
	
	return EditableTextBox;
}

TSharedPtr<SObjectPropertyEntryBox> FSettingComponent::CreateObjectPropertyEntryBox(
	TSharedPtr<FSettingData<TSharedPtr<FObjectPropertyEntryBoxData>>> SettingData)
{
	if (SettingData->GetValue()->Class == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("SettingData->GetValue()->Class = nullptr"));
	}
	
	if (SettingData->GetValue()->Asset == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("SettingData->GetValue()->Asset = nullptr"));
	}
	
	if (SettingData->GetValue() == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("SettingData->GetValue() = nullptr"));
	}
	
	TSharedPtr<SObjectPropertyEntryBox> EntryBox = SNew(SObjectPropertyEntryBox)
	.AllowedClass(SettingData->GetValue()->Class)
	.AllowClear(true)
	.ObjectPath_Lambda([&, SettingData]() -> FString
	{
		return SettingData->GetValue()->Asset == nullptr ? FString("") : *SettingData->GetValue()->Asset.GetObjectPathString();
	})
	.OnObjectChanged(FOnSetObject::CreateLambda([&, SettingData](const FAssetData& NewAssetData) {
		UE_LOG(LogTemp, Log, TEXT("New AssetData Changed"));
		
		SettingData->GetValue()->Asset = NewAssetData;
		if (SettingData->SetAction.IsBound())
		{
			UE_LOG(LogTemp, Log, TEXT("Set AssetData"));
			SettingData->SetAction.Execute(SettingData->GetValue());
		}
	}));
	
	return EntryBox;
}
