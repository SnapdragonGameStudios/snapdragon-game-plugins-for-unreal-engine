// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using UnrealBuildTool;
using System.IO;

public class VoiceAITTS : ModuleRules
{
    public VoiceAITTS(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });

        PrivateDependencyModuleNames.AddRange(new[] { "Projects", "SGAITextToSpeech", "VoiceAITTSLib", "Launch" });

        AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "AndroidPackaging.xml"));
    }
}
