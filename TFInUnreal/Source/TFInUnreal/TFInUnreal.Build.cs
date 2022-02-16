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


		CppStandard = CppStandardVersion.Cpp17;
	
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicIncludePaths.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "include"));
		PublicAdditionalLibraries.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "lib", "tensorflow.lib"));
		RuntimeDependencies.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "lib", "tensorflow.dll"));
		if(!System.IO.File.Exists(Path.Combine(ProjectRoot, "Binaries", Target.Platform.ToString(), "tensorflow.dll")))
		{
			System.IO.File.Copy(Path.Combine(ProjectRoot, "Source", "ThirdParty", "lib", "tensorflow.dll"), Path.Combine(ProjectRoot, "Binaries", Target.Platform.ToString(), "tensorflow.dll"), true);
		}
	}
}
