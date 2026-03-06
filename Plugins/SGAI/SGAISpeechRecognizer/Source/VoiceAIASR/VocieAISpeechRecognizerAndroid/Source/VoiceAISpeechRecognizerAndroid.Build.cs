// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using UnrealBuildTool;
using System.IO;

public class VoiceAISpeechRecognizerAndroid : ModuleRules
{
    public VoiceAISpeechRecognizerAndroid(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });

        PrivateDependencyModuleNames.AddRange(
            new[] { "Projects", "SGAISpeechRecognizer", "VoiceAIASRLib", "Launch" });

        AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "AndroidPackaging.xml"));
    }
}
