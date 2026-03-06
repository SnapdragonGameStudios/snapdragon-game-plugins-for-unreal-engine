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
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			string ThirdPartyPath = Path.Combine(PluginPath, "..", "..", "ThirdParty", "VoiceAILibTTS", "Android");

			// Add the JAR file
			string JarPath = Path.Combine(ThirdPartyPath, "libs", "tts-sdk.jar");
			PublicAdditionalLibraries.Add(JarPath);

			// Add native libraries for arm64-v8a
			string LibPath = Path.Combine(ThirdPartyPath, "libs", "arm64-v8a");
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libtts.so"));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libtts_jni.so"));

			// Add the UPL file for Android packaging
			string UPLPath = Path.Combine(ModuleDirectory, "AndroidPackaging.xml");
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", UPLPath);
		}
	}
}
