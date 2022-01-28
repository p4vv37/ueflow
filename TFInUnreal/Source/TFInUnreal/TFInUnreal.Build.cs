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

	public TFInUnreal(ReadOnlyTargetRules Target) : base(Target)
	{


		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AssetTools", "SlateCore", "RHI" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true 

		PublicIncludePaths.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "include"));
		RuntimeDependencies.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "lib", "tensorflow.dll"));
		System.IO.File.Copy(Path.Combine(ProjectRoot, "Source", "ThirdParty", "lib", "tensorflow.dll"), Path.Combine(ProjectRoot, "Binaries", Target.Platform.ToString(), "tensorflow.dll"), true);
	

		PublicAdditionalLibraries.Add("D:/git/cppflow/examples/efficientnet/build/Release/tensorflow.lib");
	}
}
