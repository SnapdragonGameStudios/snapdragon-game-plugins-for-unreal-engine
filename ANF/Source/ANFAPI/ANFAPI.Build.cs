//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

using UnrealBuildTool;
using System;
using System.IO;

public class ANFAPI : ModuleRules
{
	public ANFAPI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        if (Target.Version.MinorVersion > 0)
        {
            PublicIncludePaths.AddRange(
                new string[]{
                    Path.Combine(GetModuleDirectory("Renderer"), "Public"),
                   Path.Combine(GetModuleDirectory("ANFSDK"), "Public"),
                }
               );

		PrivateIncludePaths.AddRange(
			new string[]{
                Path.Combine(GetModuleDirectory("Renderer"), "Private"),
			   Path.Combine(GetModuleDirectory("Renderer"), "Internal"),
			   Path.Combine(GetModuleDirectory("VulkanRHI"), "Private"),
			}
			);
        }
        else
        {
            PublicIncludePaths.AddRange(
                new string[]{
                    Path.Combine(ModuleDirectory, "../ANFSDK/Public"),
                   Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Public"),
                }
               );

		PrivateIncludePaths.AddRange(
			new string[]{
				Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Private"),
				Path.Combine(EngineDirectory, "Source/Runtime/VulkanRHI/Private"),
			}
			);
        }

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"RenderCore",
				"Core",
				"CoreUObject",
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
				"EngineSettings",
				"DeveloperSettings",
				"Vulkan",
				"VulkanRHI",
				"RHICore",
				"ANFSDK"
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);

		PrecompileForTargets = PrecompileTargetsType.Any;
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Content/..."), StagedFileType.NonUFS);

		bEnableExceptions = true;
		PublicIncludePathModuleNames.Add("Vulkan");
		AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Source/Runtime/VulkanRHI/Private/Android"));
            if (Target.Version.MinorVersion == 0)
            {
                PrivateDefinitions.Add("VK_USE_PLATFORM_ANDROID_KHR=1");
            }
		}
		if (Target.Platform == UnrealTargetPlatform.Win64 && Target.bCompileAgainstEditor)
		{
			PublicDependencyModuleNames.Add("WindowsTargetPlatform");
            if (Target.Version.MinorVersion == 0)
            {
                PrivateDefinitions.Add("VK_USE_PLATFORM_WIN32_KHR=1");
                PrivateDefinitions.Add("VK_USE_PLATFORM_WIN32_KHX=1");
            }
        }
	}
}