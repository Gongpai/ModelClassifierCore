#pragma once
#include "SettingData.h"

class FModulePanel;
class MODELCLASSIFIERCORE_API IModulePage
{
private:
	TSharedPtr<void> ComponentPage;

protected:
	TSharedPtr<SBox> PageBox;
	TSharedPtr<SButton> TabButton;
	FModulePanel* ModulePanel;
	TSharedPtr<SBox> SettingModuleName;
	TSharedPtr<SScrollBox> ScrollBoxSettingList;
	TMap<FString, FSettingData<TSharedPtr<void>>> Settings;
	TMap<FString, TSharedPtr<TFunction<EVisibility()>>> VisibilitySettingBoxes;
	TMap<FString, TMap<FString, TArray<FString>>> VisibilitySettingEntry;
	
	void ClearVisibilitySettingBoxes();
	EVisibility GetSettingVisibilityFunction(TSharedPtr<TFunction<EVisibility()>>* Function, ::FString ModuleName, ::FString Category, FString ID);
	EVisibility GetAllSettingVisibilityEntry(FString ModuleName, FString Category);
	void AutoRegisterVisibilitySettingEntry(FString ModuleName, FString Category, FString ID, EVisibility Visibility);

public:
	IModulePage();
	virtual ~IModulePage();
	virtual void StartPage(FModulePanel* Panel);
	virtual void OpenPage();
	virtual void ClosePage();
	virtual void ShutdownPage();
	virtual void RegisterPage();
	virtual void UnregisterPage();
	virtual TSharedPtr<SBox> CreatePage();
	
	virtual void SetupSetting();
	virtual void SaveConfigSettings(FString Key, FString Value);
	virtual FString LoadConfigSettings(FString Key);
	
	virtual TSharedPtr<SButton> GetTabButton();
	virtual void SetTabButton(TSharedPtr<SButton> Button);
	
	TSharedPtr<SBox> CreateSettingPage();
	void CreateSettingEntry(TSharedPtr<FSettingData<TSharedPtr<void>>> SettingData);
	void CreateSettingList();
	void ShowSettingIf(FString Name, TSharedPtr<TFunction<EVisibility()>> Action);

	FString ModulePageName;
};