// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using UnrealBuildTool;
using System.IO;

public class LLMPipelines : ModuleRules
{
    public LLMPipelines(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicSystemIncludePaths.AddRange(
            new string[] { Path.Combine(ModuleDirectory, "../../ThirdParty/json/single_include") });

        PublicDependencyModuleNames.AddRange(
            new string[] { "Core", "CoreUObject", "Engine", "Json", "JsonUtilities", "PromptFormatter" });
    }
}
