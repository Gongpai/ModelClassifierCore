#pragma once
#include "ModelsPage.h"
#include "ModelsPage.h"
#include "SettingData.h"
#include "Widgets/Input/SSlider.h"
#include "PropertyCustomizationHelpers.h"
#include "SettingComponent.h"
#include "Widgets/Notifications/SProgressBar.h"

class FSettingComponent
{
public:
	static TSharedPtr<STextBlock> CreateSettingName(FString Name);
	static TSharedPtr<SButton> CreateButton(TSharedPtr<FSettingData<TSharedPtr<FButtonData>>> SettingData);
	static TSharedPtr<SSlider> CreateSlider(TSharedPtr<FSettingData<TSharedPtr<FSliderData>>> SettingData);
	static TSharedPtr<SCheckBox> CreateCheckBox(TSharedPtr<FSettingData<TSharedPtr<ECheckBoxState>>> SettingData);
	static TSharedPtr<SComboBox<TSharedPtr<FString>>> CreateDropDown(TSharedPtr<FSettingData<TSharedPtr<FDropDownData>>> SettingData);
	static TSharedPtr<SEditableTextBox> CreateEditableTextBox(TSharedPtr<FSettingData<TSharedPtr<FEditableTextBoxData>>> SettingData, TSharedPtr<SBox>& Box);
	static TSharedPtr<SObjectPropertyEntryBox> CreateObjectPropertyEntryBox(TSharedPtr<FSettingData<TSharedPtr<FObjectPropertyEntryBoxData>>> SettingData);
	static TSharedPtr<SProgressBar> CreateProgressBar(TSharedPtr<FSettingData<TSharedPtr<void>>> SettingData, TSharedPtr<SBox>& OutBox);
};
