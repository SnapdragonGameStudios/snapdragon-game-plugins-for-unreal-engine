//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

using UnrealBuildTool;
using System;
using System.IO;

public class ANF : ModuleRules
{
	public ANF(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]{
				// ... add public include paths required here ...
			}
			);

		PrivateIncludePaths.AddRange(
			new string[]{
				// ... add other private include paths required here ...
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add other public dependencies that you statically link with here ...
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"ANFTUModule",
                "ANFFrameGenModule",
                "ANFAPI"
				// ... add private dependencies that you statically link with here ...
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		PrecompileForTargets = PrecompileTargetsType.Any;
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Content/..."), StagedFileType.NonUFS);
	}
}