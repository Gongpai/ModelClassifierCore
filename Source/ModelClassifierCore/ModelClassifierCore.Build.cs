// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ModelClassifierCore : ModuleRules
{
	public ModelClassifierCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GeometryCore",
				"GeometryFramework",
				"GeometryScriptingCore",
				"Projects",
				"ModelClassifierCoreLibrary", 
				"UnrealEd",
				"InputCore",
				"PropertyEditor",
				"ProceduralMeshComponent",
				"Engine",
				"PythonScriptPlugin",
				"ToolMenus",
				"LevelEditor",
				"DeveloperSettings",
				"Json",
				"JsonUtilities",
				"NNERuntimeORT",
				"NNERuntimeBasicCpu",
				"NNEOnnxruntime",
				"NNE"

				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"ProceduralMeshComponent",
				"Projects",
				"MeshDescription", 
				"PropertyEditor",
				"MeshConversion",
				"StaticMeshDescription",
				"ModelClassifierCoreLibrary",
				"MaterialEditor",
				"MaterialUtilities",
				"ProceduralMeshComponent", 
				"GeometryFramework", 
				"PackagesDialog",
				"RenderCore",
				"PythonScriptPlugin",
				"ToolMenus",
				"LevelEditor",
				"DeveloperSettings",
				"Json",
				"JsonUtilities",
				"NNERuntimeORT",
				"NNERuntimeBasicCpu",
				"NNEOnnxruntime"
				
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		// Link with OpenGL system library on Windows
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.Add("opengl32.lib");
		}
	}
}
