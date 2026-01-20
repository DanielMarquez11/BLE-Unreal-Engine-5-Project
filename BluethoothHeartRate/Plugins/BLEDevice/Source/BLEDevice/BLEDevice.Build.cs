// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class BLEDevice : ModuleRules
{
	public BLEDevice(ReadOnlyTargetRules Target) : base(Target)
	{
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
			bEnableExceptions = true;
			bUseUnity = false;
			CppStandard = CppStandardVersion.Cpp20;

			PublicIncludePaths.AddRange(
				new string[]
				{
					// ... add public include paths required here ...
				}
			);
			
			// 1. Voeg de noodzakelijke Windows System Libraries toe voor WinRT
			PublicSystemLibraries.AddRange(new string[] { 
				"runtimeobject.lib",
				"Synchronization.lib",
				"shlwapi.lib" 
			});
			
			// 2. Vertel UBT waar de WinRT headers in de Windows SDK staan
			var windowsSdkDir = Target.WindowsPlatform.WindowsSdkDir;
			var windowsSdkVersion = Target.WindowsPlatform.WindowsSdkVersion;
			var cppWinRtPath = Path.Combine(windowsSdkDir, "Include", windowsSdkVersion, "cppwinrt");

			PublicIncludePaths.Add(cppWinRtPath);
			
			// Optioneel: Definieer dat we WinRT gebruiken om conflicten te minimaliseren
			PublicDefinitions.Add("WINRT_LEAN_AND_MEAN");
		}

		PrivateIncludePaths.AddRange(
				new string[]
				{
					// ... add other private include paths required here ...
				}
			);


			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
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
					// ... add private dependencies that you statically link with here ...	
				}
			);


			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{ 
					// ... add any modules that your module loads dynamically here ...
				}
			);
		}
}
