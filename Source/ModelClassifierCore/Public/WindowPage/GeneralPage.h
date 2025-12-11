#pragma once
#include "ModulePage.h"

class FGeneralPage : public IModulePage
{
public:
	FGeneralPage();
	virtual ~FGeneralPage() override;
	virtual void StartPage(FModulePanel* Panel) override;
	virtual void OpenPage() override;
	virtual void ClosePage() override;
	virtual void ShutdownPage() override;
	virtual void RegisterPage() override;
	virtual void UnregisterPage() override;
	virtual TSharedPtr<SBox> CreatePage() override;
	virtual void SaveConfigSettings(FString Key, FString Value) override;
	virtual FString LoadConfigSettings(FString Key) override;
	virtual TSharedPtr<SButton> GetTabButton() override;
	virtual void SetTabButton(TSharedPtr<SButton> Button) override;
};
