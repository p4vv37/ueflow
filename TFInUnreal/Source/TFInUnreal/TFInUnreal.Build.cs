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

		// Compile and copy dll. This bat can be also executed externally.
		System.Diagnostics.Process process = new System.Diagnostics.Process();
		System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
		//startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
		startInfo.WorkingDirectory = Path.Combine(ProjectRoot, "../");
		startInfo.FileName = Path.Combine(ProjectRoot, "../build_dll.bat");
		process.StartInfo = startInfo;
		process.Start();
		process.WaitForExit();


		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true 

		// string model2dllLibPath = CopyToProjectBinaries(Path.Combine(ProjectRoot, "Source/ThirdParty/model2dll.lib"), Target);
		// string tfLibPath = CopyToProjectBinaries(Path.Combine(ProjectRoot, "Source/ThirdParty/tensorflow.lib"), Target);

		CopyToProjectBinaries(Path.Combine(ProjectRoot, "Source", "ThirdParty", "model2dll.dll"), Target);
		CopyToProjectBinaries(Path.Combine(ProjectRoot, "Source", "ThirdParty", "tensorflow.dll"), Target);
		CopyToProjectBinaries(Path.Combine(ProjectRoot, "Source", "ThirdParty", "model2dll.lib"), Target);
		CopyToProjectBinaries(Path.Combine(ProjectRoot, "Source", "ThirdParty", "tensorflow.lib"), Target);
		CopyToProjectBinaries(Path.Combine(ProjectRoot, "Source", "ThirdParty", "model.pb"), Target);

		PublicIncludePaths.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "include"));
		PublicIncludePaths.Add("D:/git/ueflow/model2dll/ThirdParty/libtensorflow/include");
		RuntimeDependencies.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "model2dll.dll"));
		RuntimeDependencies.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "tensorflow.dll"));
		PublicAdditionalLibraries.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "model2dll.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ProjectRoot, "Source", "ThirdParty", "tensorflow.lib"));

		// Just for easier debug:         
		CopyToProjectBinaries(Path.Combine(ProjectRoot, "Source/ThirdParty/model2dll.pdb"), Target);
	}
}
