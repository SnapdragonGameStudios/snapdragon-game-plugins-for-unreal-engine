// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

using UnrealBuildTool;

public class SpeechRecognizerSample : ModuleRules
{
	public SpeechRecognizerSample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"AudioCapture",
				"AudioCaptureCore",
				"AudioMixer",
				"Voice",
				"SGAISpeechRecognizer"
			}
		);
	}
}