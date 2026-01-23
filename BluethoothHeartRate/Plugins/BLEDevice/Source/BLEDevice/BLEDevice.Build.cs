// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class BLEDevice : ModuleRules
{
	public BLEDevice(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;


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
					"WebSockets",
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
