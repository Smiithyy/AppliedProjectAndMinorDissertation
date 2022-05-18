// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Banishment : ModuleRules
{
	public Banishment(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
			{"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG", "AIModule"});
		
		PrivateDependencyModuleNames.AddRange((new string[] 
			{ "Slate", "SlateCore"}));
	}
}