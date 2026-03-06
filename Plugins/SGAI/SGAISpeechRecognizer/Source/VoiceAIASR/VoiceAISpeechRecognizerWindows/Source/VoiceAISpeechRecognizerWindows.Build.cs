// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using UnrealBuildTool;

public class VoiceAISpeechRecognizerWindows : ModuleRules
{
	public VoiceAISpeechRecognizerWindows(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Projects",
				"QAIRT",
				"SGAISpeechRecognizer",
				"VoiceAIASRLib"
			}
		);
	}
}