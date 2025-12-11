// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModelClassifierCoreCommands.h"

#define LOCTEXT_NAMESPACE "FModelClassifierCoreModule"

void FModelClassifierCoreCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "MCC Tools", "Open Model Classifier Core Tools Window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
