// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

using UnrealBuildTool;

public class TextToSpeechSample : ModuleRules
{
    public TextToSpeechSample(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

        PrivateDependencyModuleNames.AddRange(new string[] {});

        PrivateDependencyModuleNames.AddRange(new[] { "SGAITextToSpeech" });
    }
}
