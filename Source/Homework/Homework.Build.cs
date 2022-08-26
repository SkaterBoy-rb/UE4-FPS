// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Homework : ModuleRules
{
	public Homework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", 
			"Engine", "InputCore", "HeadMountedDisplay", "PhysicsCore", "UMG", "NavigationSystem" });
	}
}
