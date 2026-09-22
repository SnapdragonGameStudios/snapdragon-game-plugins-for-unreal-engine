//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

using UnrealBuildTool;
using System;
using System.IO;

public class ANFFrameGenModule : ModuleRules
{
	public ANFFrameGenModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        if (Target.Version.MinorVersion > 0)
        {
            PublicIncludePaths.AddRange(
                new string[]{
                    Path.Combine(GetModuleDirectory("SlateCore"), "Public"),
                    Path.Combine(GetModuleDirectory("SlateRHIRenderer"), "Public"),
                }
                );

            PrivateIncludePaths.AddRange(
                new string[]{
                   Path.Combine(GetModuleDirectory("Renderer"), "Private"),
                   Path.Combine(GetModuleDirectory("VulkanRHI"), "Private"),
                   Path.Combine(GetModuleDirectory("Renderer"), "Internal"),
                   Path.Combine(GetModuleDirectory("SlateRHIRenderer"), "Private"),
                }
                );

        }
        else
        {
            PublicIncludePaths.AddRange(
                new string[]{
                    EngineDirectory + "/Source/Runtime/SlateCore/Public",
                    EngineDirectory + "/Source/Runtime/SlateRHIRenderer/Public",
                }
                );

            PrivateIncludePaths.AddRange(
                new string[]{
                   Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Private"),
                   Path.Combine(EngineDirectory, "Source/Runtime/VulkanRHI/Private"),
                   Path.Combine(EngineDirectory, "Source/Runtime/SlateRHIRenderer/Private"),
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
                "SlateCore",
				"Slate",
                "SlateRHIRenderer"
			}
			);

		PrecompileForTargets = PrecompileTargetsType.Any;
	}
}