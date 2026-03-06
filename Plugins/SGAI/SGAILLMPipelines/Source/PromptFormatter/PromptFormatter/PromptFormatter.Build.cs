// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using UnrealBuildTool;
using System.IO;

public class PromptFormatter : ModuleRules
{
    public PromptFormatter(ReadOnlyTargetRules Target) : base(Target)
    {
        bUseRTTI = true;
        bEnableExceptions = true;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] { Path.Combine(ModuleDirectory, "../../ThirdParty/json/single_include"),
                           Path.Combine(ModuleDirectory, "../ThirdParty/minja/include"),
                           Path.Combine(ModuleDirectory, "../../LLMPipelines/LLMPipelines/Public") });

        ShadowVariableWarningLevel = WarningLevel.Off;
        CppCompileWarningSettings.UnusedParameterWarningLevel = WarningLevel.Off;
        CppCompileWarningSettings.UnusedWarningLevel = WarningLevel.Off;
        CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
        PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS");
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject" });
    }
}
