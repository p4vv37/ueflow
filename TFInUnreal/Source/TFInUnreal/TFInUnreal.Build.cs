// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class TFInUnreal : ModuleRules
{

	public string ProjectRoot
	{
		get
		{
			return System.IO.Path.GetFullPath(
				System.IO.Path.Combine(ModuleDirectory, "../../")
			);
		}
	}


	private string CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
	{
		string BinariesDir = Path.Combine(ProjectRoot, "Binaries", Target.Platform.ToString());
		string Filename = Path.GetFileName(Filepath);

		//convert relative path 
		string FullBinariesDir = Path.GetFullPath(BinariesDir);

		if (!Directory.Exists(FullBinariesDir))
		{
			Directory.CreateDirectory(FullBinariesDir);
		}

		string FullExistingPath = Path.Combine(FullBinariesDir, Filename);
		bool ValidFile = false;

		//File exists, check if they're the same
		if (File.Exists(FullExistingPath))
		{
			ValidFile = true;
		}

		//No valid existing file found, copy new dll
		if (!ValidFile)
		{
			File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
		}
		return FullExistingPath;
	}

	public TFInUnreal(ReadOnlyTargetRules Target) : base(Target)
	{


		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true 

		PublicIncludePaths.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "include"));

		// RuntimeDependencies.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "lib", "tensorflow.dll"));
		// PublicAdditionalLibraries.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "lib", "tensorflow.lib"));
		PublicAdditionalLibraries.Add("D:/git/cppflow/examples/efficientnet/build/Release/tensorflow.lib");
	}
}
