// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using UnrealBuildTool;

public class SGAISpeechRecognizer : ModuleRules
{
    public SGAISpeechRecognizer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });

        PrivateDependencyModuleNames.AddRange(
            new[] { "Projects", "AudioCapture", "AudioCaptureCore", "AudioMixer", "Voice" });

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("GoogleOboe");
            PrivateIncludePathModuleNames.Add("GoogleOboe");
            AddEngineThirdPartyPrivateStaticDependencies(Target, "GoogleOboe");
        }
    }
}
