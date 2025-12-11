// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "ModelClassifierCoreStyle.h"

class FModelClassifierCoreCommands : public TCommands<FModelClassifierCoreCommands>
{
public:

	FModelClassifierCoreCommands()
		: TCommands<FModelClassifierCoreCommands>(TEXT("ModelClassifierCoreEditor"), NSLOCTEXT("Contexts", "ModelClassifierCoreEditor", "ModelClassifierCoreEditor Plugin"), NAME_None, FModelClassifierCoreStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
