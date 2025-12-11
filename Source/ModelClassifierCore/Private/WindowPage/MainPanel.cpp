#include "WindowPage/MainPanel.h"

#include "WindowPage/GeneralPage.h"
#include "WindowPage/ModelsPage.h"

FMainPanel::~FMainPanel()
{
}

TSharedPtr<SBox> FMainPanel::CreatePanel(float Padding)
{
	return FModulePanel::CreatePanel(Padding);
}

TSharedPtr<SBox> FMainPanel::CreateEntryPage()
{
	return FModulePanel::CreateEntryPage();
}

void FMainPanel::AddPage()
{
	FGeneralPage* GeneralPage = new FGeneralPage();
	Pages.Add(GeneralPage->ModulePageName, GeneralPage);
	
	FModelsPage* ModelsPage = new FModelsPage();
	Pages.Add(ModelsPage->ModulePageName, ModelsPage);
	
	FModulePanel::AddPage();
}

void FMainPanel::InsertSlotButtonScrollBox(TSharedPtr<SButton> Button, int32 Index)
{
	FModulePanel::InsertSlotButtonScrollBox(Button, Index);
}

void FMainPanel::SetSelectButton(TSharedPtr<SButton> Button)
{
	FModulePanel::SetSelectButton(Button);
}
