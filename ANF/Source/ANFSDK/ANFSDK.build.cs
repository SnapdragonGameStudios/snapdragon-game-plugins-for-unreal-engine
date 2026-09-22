//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

using UnrealBuildTool;
using System;
using System.IO;

public class ANFSDK : ModuleRules
{
	public ANFSDK(ReadOnlyTargetRules Target) : base(Target)
	{
        Type = ModuleType.External;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
			);


        if (Target.Version.MinorVersion > 0)
        {
            PrivateIncludePaths.AddRange(
                new string[] {
                    Path.Combine(GetModuleDirectory("Renderer"), "Private"),
                   Path.Combine(GetModuleDirectory("VulkanRHI"), "Private"),
                   Path.Combine(GetModuleDirectory("Renderer"), "Internal"),
                }
                );

        }
        else
        {
            PrivateIncludePaths.AddRange(
                new string[] {
                     Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Private"),
                   Path.Combine(EngineDirectory, "Source/Runtime/VulkanRHI/Private"),
                }
                );

        }

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                    "Vulkan",
					"VulkanRHI",
			}
			);

		PrecompileForTargets = PrecompileTargetsType.Any;

		bEnableExceptions = true;
        PublicIncludePathModuleNames.Add("Vulkan");
        AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            // This XML will be executed AFTER trying to add libraries
            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "ANFSDK.xml"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "libs/arm64-v8a/libanf.so"));
        }
	}
}
