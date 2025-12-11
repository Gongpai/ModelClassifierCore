#pragma once
#include "ModelClassifierCore.h"

class FModulePanel;
class ModelClassifierCore::FModelClassifierCoreModule;

class MODELCLASSIFIERCORE_API FModelClassifierCorePage
{
	static ModelClassifierCore::FModelClassifierCoreModule* Core;
	static TSharedPtr<SDockTab> DockTab;
	static TMap<FString, TSharedPtr<SBox>> ModuleBoxButtons;
	static TMap<FString, TSharedPtr<SButton>> ModuleButtons;
	static TSharedPtr<SBox> TextModuleEmpty;
	static TSharedPtr<SScrollBox> ModuleScrollBox;
	static TSharedPtr<STextBlock> TextModuleName;
	static TSharedPtr<SBox> MainPanel;
	static TSharedPtr<SButton> SelectedButton;
	static FModulePanel* ModulePanel;
	
public:
	static void Initialize(ModelClassifierCore::FModelClassifierCoreModule* InCore);
	static void Shutdown();
 	static TSharedRef<class SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
	static TSharedRef<SDockTab> GetPluginTab();
	static ModelClassifierCore::FModelClassifierCoreModule* GetCore();

	static TSharedPtr<SBox> SettingButton;
};
