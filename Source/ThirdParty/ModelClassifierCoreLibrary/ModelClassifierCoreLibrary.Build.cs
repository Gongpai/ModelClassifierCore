// Fill out your copyright notice in the Description page of Project Settings.

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

public class ModelClassifierCoreLibrary : ModuleRules
{
	/** irajsb - UE4_Assimp **/
	public string BinFolder(ReadOnlyTargetRules Target)
	{
		if (Target.Platform == UnrealTargetPlatform.Mac)
			return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/Mac/"));
		else if (Target.Platform == UnrealTargetPlatform.IOS)
			return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/IOS/"));
		if (Target.Platform == UnrealTargetPlatform.Win64)
			return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/Win64/"));
		if (Target.Platform == UnrealTargetPlatform.Android)
			return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/Android/"));
		if (Target.Platform == UnrealTargetPlatform.Linux)
			return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Binaries/Linux/"));
		return "";
	}
	
	/** irajsb - UE4_Assimp **/
	public ModelClassifierCoreLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		bUseRTTI = true;
		
		string BinaryFolder = BinFolder(Target);
		GetFile("assimp", "assimp", BinaryFolder);
		GetFile("fbxsdk", "libfbxsdk", BinaryFolder);
		GetFile("glfw", "glfw3", BinaryFolder);
		GetFile("glm", "glm", BinaryFolder);

		List<string> ThirdPartyAssetFils = new List<string>()
		{
			Path.Combine(ModuleDirectory, "pbr-shader")
		};
		
		string ThirdPartyAssets = Path.Combine(ModuleDirectory, "ThirdPartyAssets.txt");
		
		if (File.Exists(ThirdPartyAssets))
		{
			File.Delete(ThirdPartyAssets);
		}
		
		File.WriteAllLines(ThirdPartyAssets, ThirdPartyAssetFils.ToArray());
	}

	private void GetFile(string name, string fileName, string binFolder, bool bIsAddon = false)
	{
		if (!bIsAddon)
		{
			string IncludePath = Path.Combine(ModuleDirectory, name, "include");
			if (Directory.Exists(IncludePath))
				PublicIncludePaths.Add(IncludePath);

			string codePath = Path.Combine(ModuleDirectory, name, "code");
			if (Directory.Exists(codePath))
				PublicIncludePaths.Add(codePath);
			
			if (!Directory.Exists(IncludePath) && !Directory.Exists(codePath))
				PublicIncludePaths.Add(Path.Combine(ModuleDirectory, name));
			
			string PrivateIncludePath = Path.Combine(ModuleDirectory, name, "src");
			if (Directory.Exists(PrivateIncludePath))
				PrivateIncludePaths.Add(PrivateIncludePath);
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			string LibPath = Path.Combine(ModuleDirectory, name, "lib", "Release", $"{fileName}.lib");
			
			// For Fbx 2020.2.1
			if (!File.Exists(LibPath))
			{
				LibPath = Path.Combine(ModuleDirectory, name, "lib", "vs2019", "x64", "release", $"{fileName}.lib");
			}
			
			// For Fbx 2020.3.7
			if (!File.Exists(LibPath))
			{
				LibPath = Path.Combine(ModuleDirectory, name, "lib", "x64", "release", $"{fileName}.lib");
			}
			
			if (File.Exists(LibPath))
				PublicAdditionalLibraries.Add(LibPath);

			// Delay-load the DLL
			string DllPath = Path.Combine(ModuleDirectory, name, "bin", "Release", $"{fileName}.dll");
			
			// .DLL a file doesn't exist.
			if (!File.Exists(DllPath))
			{
				DllPath = Path.Combine(ModuleDirectory, name, "lib", "Release", $"{fileName}.dll");
			}

			// For Fbx 2020.2.1
			if (!File.Exists(DllPath) && !bIsAddon)
			{
				DllPath	= Path.Combine(ModuleDirectory, name, "lib", "vs2019", "x64", "release", $"{fileName}.dll");
			}
			
			// For Fbx 2020.3.7
			if (!File.Exists(DllPath) && !bIsAddon)
			{
				DllPath	= Path.Combine(ModuleDirectory, name, "lib", "x64", "release", $"{fileName}.dll");
			}
			
			if (!bIsAddon && File.Exists(DllPath))
				PublicDelayLoadDLLs.Add(DllPath);
			
			// Create Directory
			Directory.CreateDirectory(binFolder);
			
			// Copy .lib To Binaries
			string BinLibPath = Path.Combine(binFolder, Path.GetFileName(LibPath));
			CopyFile(LibPath, BinLibPath);

			// Copy .dll To Binaries
			if (!bIsAddon && File.Exists(DllPath))
			{
				string BinDllPath = Path.Combine(binFolder, Path.GetFileName(DllPath));
				CopyFile(DllPath, BinDllPath);
				RuntimeDependencies.Add($"$(TargetOutputDir)/{fileName}.dll", DllPath);
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, name, "bin", $"{fileName}.dylib"));

			Directory.CreateDirectory(binFolder);
			string FileDylib = Path.Combine(ModuleDirectory, name, "bin", $"{fileName}.dylib");
			string BinPath = Path.Combine(ModuleDirectory, binFolder, $"{fileName}.dylib");

			CopyFile(FileDylib, BinPath);
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, name, "bin", $"{fileName}.so"));

			Directory.CreateDirectory(binFolder);
			string FileSo = Path.Combine(ModuleDirectory, name, "bin", $"{fileName}.so");
			string BinPath = Path.Combine(ModuleDirectory, binFolder, $"{fileName}.so");

			CopyFile(FileSo, BinPath);
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, name, "bin", "Release", $"{fileName}.so"));
		}
	}
	
	/** irajsb - UE4_Assimp **/
	public void CopyFile(string Source, string Dest)
	{
		System.Console.WriteLine("Copying {0} to {1}", Source, Dest);
		if (System.IO.File.Exists(Dest))
		{
			System.IO.File.SetAttributes(Dest, System.IO.File.GetAttributes(Dest) & ~System.IO.FileAttributes.ReadOnly);
		}
		try
		{
			//Make Folder
			System.IO.File.Copy(Source, Dest, true);
		}
		catch (System.Exception ex)
		{
			System.Console.WriteLine("Failed to copy file: {0}", ex.Message);
		}
	}
}
