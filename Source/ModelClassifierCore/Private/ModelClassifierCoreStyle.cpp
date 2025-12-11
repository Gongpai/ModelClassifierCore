// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModelClassifierCoreStyle.h"
#include "ModelClassifierCore.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FModelClassifierCoreStyle::StyleInstance = nullptr;

void FModelClassifierCoreStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FModelClassifierCoreStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FModelClassifierCoreStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ModelClassifierCoreEditorStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FModelClassifierCoreStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ModelClassifierCoreEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ModelClassifierCore")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ModelClassifierCoreEditor.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FModelClassifierCoreStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FModelClassifierCoreStyle::Get()
{
	return *StyleInstance;
}
