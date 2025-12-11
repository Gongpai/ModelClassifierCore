#pragma once
#include "ModulePanel.h"

class FMainPanel : public FModulePanel
{
public:
	virtual ~FMainPanel() override;
	virtual TSharedPtr<SBox> CreatePanel(float Padding) override;
	virtual TSharedPtr<SBox> CreateEntryPage() override;
	virtual void AddPage() override;
	virtual void InsertSlotButtonScrollBox(TSharedPtr<SButton> Button, int32 Index) override;
	virtual void SetSelectButton(TSharedPtr<SButton> Button) override;
};
