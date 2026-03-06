// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using UnrealBuildTool;
using System.IO;

public class VectorDBProvider : ModuleRules
{
    public VectorDBProvider(ReadOnlyTargetRules Target) : base(Target)
    {
        bWarningsAsErrors = false;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "../ThirdParty/Annoy/src") });

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });

        PrivateDependencyModuleNames.AddRange(new[] {
            "Projects",
            "LLMPipelines",
        });

        PublicDefinitions.AddRange(new[] { "_CRT_SECURE_NO_WARNINGS" });
    }
}
