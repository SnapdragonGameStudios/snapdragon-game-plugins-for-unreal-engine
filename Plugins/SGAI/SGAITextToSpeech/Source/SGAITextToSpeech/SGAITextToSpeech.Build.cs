// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

using UnrealBuildTool;
using System.IO;

public class SGAITextToSpeech : ModuleRules
{
    public SGAITextToSpeech(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });

            // string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            // AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath,
            // "SGAITextToSpeech_UPL.xml"));
        }
    }
}
