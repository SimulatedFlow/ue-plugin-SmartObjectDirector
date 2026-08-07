// Copyright 2026 Silvan Teufel All Rights Reserved.

using UnrealBuildTool;

public class SmartObjectDirector : ModuleRules
{
	public SmartObjectDirector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"GameplayTags",
			"GameplayTasks",
			"GameplayAbilities",
			"SmartObjectsModule",
			"MotionWarping",
			"StateTreeModule",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"NavigationSystem",
		});
	}
}
