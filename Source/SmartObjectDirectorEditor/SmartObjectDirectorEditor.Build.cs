// Copyright 2026 Silvan Teufel All Rights Reserved.

using UnrealBuildTool;

public class SmartObjectDirectorEditor : ModuleRules
{
	public SmartObjectDirectorEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"SmartObjectDirector",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"SmartObjectsModule",
		});
	}
}
