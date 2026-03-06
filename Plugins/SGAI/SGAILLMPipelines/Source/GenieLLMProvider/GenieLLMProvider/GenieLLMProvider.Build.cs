// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using UnrealBuildTool;
using System.IO;

public class GenieLLMProvider : ModuleRules
{
    public GenieLLMProvider(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });

        PrivateDependencyModuleNames.AddRange(new[] { "Projects", "QAIRT", "LLMPipelines", "GenieLib" });

        PublicSystemIncludePaths.AddRange(new string[] {
            Path.Combine(ModuleDirectory, "../../ThirdParty/json/single_include"),
        });

        PrivateDefinitions.Add("BUILD_FOR_UE");
    }
}
