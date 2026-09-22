//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

using UnrealBuildTool;
using System;
using System.IO;

public class ANFTUModule : ModuleRules
{
	public ANFTUModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]{
				EngineDirectory + "/Source/Runtime/Renderer/Private",
			}
			);
            
        if (Target.Version.MinorVersion > 0)
        {
            PrivateIncludePaths.AddRange(
                new string[]{
                   Path.Combine(GetModuleDirectory("Renderer"), "Private"),
                   Path.Combine(GetModuleDirectory("Renderer"), "Internal"),
                }
                );

        }
        else
        {
            PrivateIncludePaths.AddRange(
                new string[]{
                    Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Private"),
                }
                );

        }

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"Projects",
				"RenderCore",
				"Renderer",
				"RHI",
				"CoreUObject",
				"DeveloperSettings",
                "ANFAPI",
			}
			);

		PrecompileForTargets = PrecompileTargetsType.Any;
	}
}