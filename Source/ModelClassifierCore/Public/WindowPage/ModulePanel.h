#pragma once

#include "ModulePage.h"
#include "SettingDataMap.h"

class FModulePanel
{

protected:
	static IModulePage* CurrentPage;
	
	TSharedPtr<SBox> MainPanel;
	TSharedPtr<SBox> PanelBox;
	
	FSettingDataMap AllSettingModules;
	TMap<FString, TSharedPtr<SVerticalBox>> LeftBoxSettings;
	TMap<FString, TSharedPtr<SVerticalBox>> RightBoxSettings;
	TMap<FString, TSharedPtr<SBox>> BoxSettings;
	FString CurrentSettingPageName;
	bool bAutoOpenModuleSetting = true;
	
public:
	FModulePanel();
	virtual ~FModulePanel();
	virtual TSharedPtr<SBox> CreatePanel(float Padding);
	virtual TSharedPtr<SBox> CreateEntryPage();
	
	virtual void AddPage();
	virtual void InsertSlotButtonScrollBox(TSharedPtr<SButton> Button, int32 Index);
	virtual void SetSelectButton(TSharedPtr<SButton> Button);
	
	TSharedPtr<SBox> GetCategoryBoxSettings(FString Name);
	TMap<FString, TSharedPtr<SBox>> GetCategoryBoxSettings();
	void SetCategoryBoxSetting(::FString Name, TSharedPtr<SBox> SettingBox);
	TSharedPtr<SVerticalBox> GetLeftBoxSettings(FString Name);
	void SetLeftBoxSetting(::FString Name, TSharedPtr<SVerticalBox> SettingBox);
	TSharedPtr<SVerticalBox> GetRightBoxSettings(FString Name);
	void SetRightBoxSettings(::FString Name, TSharedPtr<SVerticalBox> SettingBox);
	
	void AddSettingModule(::FString Name, ::IModulePage* Owner, TMap<FString, FSettingData<TSharedPtr<void>>> SettingActions);
	void CreateAllSettingInModule(IModulePage* Owner);
	void ComposeSettingButton(TSharedPtr<SButton> Button, IModulePage* Owner);
	FString GetCurrentSettingPageName();
	void SetCurrentSettingPageName(FString Name);
	bool GetAutoOpenModuleSetting();
	
	static void ShowSettingIf(FString Name, TSharedPtr<TFunction<EVisibility()>> Action);

	template<typename T>
	static FSettingData<TSharedPtr<void>> MakeSettingData(TSharedPtr<T> Value)
	{
		return FSettingData<TSharedPtr<void>>(StaticCastSharedPtr<void>(Value));
	}
	template<typename T>
	static FSettingData<TSharedPtr<void>> MakeSettingData(TSharedPtr<T> Value, FString SettingName, SettingType SettingType, TDelegate<void(TSharedPtr<void>)> Action, FString Category = "Default")
	{
		TDelegate<TSharedPtr<void>(void)> GetAction;
		GetAction.BindLambda([]()->TSharedPtr<void>{return TSharedPtr<void>();});
		
		FSettingData<TSharedPtr<void>> SettingData = MakeSettingData<T>(Value);
		SettingData.Name = SettingName;
		SettingData.SettingType = SettingType;
		SettingData.GetAction = GetAction;
		SettingData.SetAction = Action;
		SettingData.Category = Category;

		return SettingData;
	}
	template<typename T>
	static FSettingData<TSharedPtr<void>> MakeSettingData(TSharedPtr<T> Value, FString SettingName, SettingType SettingType, TDelegate<TSharedPtr<void>(void)> GetAction, TDelegate<void(TSharedPtr<void>)> SetAction, FString Category = "Default")
	{
		FSettingData<TSharedPtr<void>> SettingData = MakeSettingData<T>(Value);
		SettingData.Name = SettingName;
		SettingData.SettingType = SettingType;
		SettingData.GetAction = GetAction;
		SettingData.SetAction = SetAction;
		SettingData.Category = Category;

		return SettingData;
	}
	
	TSharedPtr<SBox> PanelPage;
	TMap<FString, IModulePage*> Pages;
	TSharedPtr<SScrollBox> ScrollButtonTab;
	TSharedPtr<SButton> SelectedButton;
	TMap<FString, TSharedPtr<SButton>> ButtonPages;
	TAttribute<const FSlateBrush*> ButtonSlateIcon = FSlateIcon(FName("ChaosVDStyle"), "ChaosVD.OpenPluginWindow").GetIcon();
};
