#include "WindowPage/GeneralPage.h"

FGeneralPage::FGeneralPage()
{
	ModulePageName = "General";
}

FGeneralPage::~FGeneralPage()
{
}

void FGeneralPage::StartPage(FModulePanel* Panel)
{
	IModulePage::StartPage(Panel);
}

void FGeneralPage::OpenPage()
{
	IModulePage::OpenPage();
}

void FGeneralPage::ClosePage()
{
	IModulePage::ClosePage();
}

void FGeneralPage::ShutdownPage()
{
	IModulePage::ShutdownPage();
}

void FGeneralPage::RegisterPage()
{
	IModulePage::RegisterPage();
}

void FGeneralPage::UnregisterPage()
{
	IModulePage::UnregisterPage();
}

TSharedPtr<SBox> FGeneralPage::CreatePage()
{
	return IModulePage::CreatePage();
}

void FGeneralPage::SaveConfigSettings(FString Key, FString Value)
{
	IModulePage::SaveConfigSettings(Key, Value);
}

FString FGeneralPage::LoadConfigSettings(FString Key)
{
	return IModulePage::LoadConfigSettings(Key);
}

TSharedPtr<SButton> FGeneralPage::GetTabButton()
{
	return IModulePage::GetTabButton();
}

void FGeneralPage::SetTabButton(TSharedPtr<SButton> Button)
{
	IModulePage::SetTabButton(Button);
}
