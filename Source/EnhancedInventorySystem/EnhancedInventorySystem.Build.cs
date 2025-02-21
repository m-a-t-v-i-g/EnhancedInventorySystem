// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class EnhancedInventorySystem : ModuleRules
{
	public EnhancedInventorySystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"GameplayTags",
			"ModularGameplay"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"NetCore"
		});
		
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Framework"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Framework/Inventory"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Framework/Item"));
		
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Framework"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Framework/Inventory"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Framework/Item"));
	}
}
