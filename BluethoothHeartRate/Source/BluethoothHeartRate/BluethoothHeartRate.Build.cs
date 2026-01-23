// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class BluethoothHeartRate : ModuleRules
{
	public BluethoothHeartRate(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Required for C++/WinRT (WinRT APIs throw exceptions via hresult_error)
		bEnableExceptions = true;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput","WebSockets" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.AddRange(new string[] { "runtimeobject.lib" });

			// C++/WinRT projection headers live in: Windows Kits\10\Include\<ver>\cppwinrt\winrt\...
			string SdkDir = Target.WindowsPlatform.WindowsSdkDir;
			string SdkVer = Target.WindowsPlatform.WindowsSdkVersion;
			string CppWinRTInclude = Path.Combine(SdkDir, "Include", SdkVer, "cppwinrt");
			PublicSystemIncludePaths.Add(CppWinRTInclude);
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
