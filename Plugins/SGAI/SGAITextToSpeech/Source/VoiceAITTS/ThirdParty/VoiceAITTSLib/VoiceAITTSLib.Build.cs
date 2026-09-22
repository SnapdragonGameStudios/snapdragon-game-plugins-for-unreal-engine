// SPDX-License-Identifier: BSD-3-Clause

// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.

using UnrealBuildTool;
using System.IO;

public class VoiceAITTSLib : ModuleRules
{
	public VoiceAITTSLib(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string ThirdPartyPath = Path.Combine(ModuleDirectory, "libs", "android");

			// AndroidPackaging.xml adds tts-sdk.jar to Gradle dependencies.

			// Add native libraries for arm64-v8a
			string LibPath = Path.Combine(ThirdPartyPath, "arm64-v8a");
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libtts.so"));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libtts_jni.so"));

			// Add the UPL file for Android packaging
			string UPLPath = Path.Combine(ModuleDirectory, "AndroidPackaging.xml");
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", UPLPath);
		}
	}
}
