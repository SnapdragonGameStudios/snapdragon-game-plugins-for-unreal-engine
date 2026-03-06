// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

public class VoiceAIASRLib : ModuleRules
{
    public VoiceAIASRLib(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        bUseRTTI = true;
        bEnableExceptions = true;
        bUseUnity = false;

        PrivateDependencyModuleNames.AddRange(new[] { "QAIRT" });

        // Add any include paths for the plugin
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "inc"));

        var libs = new List<string> { "WhisperComponent.lib" };

        var libsToPackage =
            new List<string> { "dnnvad_wrapper.dll",   "eai_float_nonmeta.dll",  "nnvad_wrapper_lib.dll",
                               "WhisperComponent.dll", "WhisperComponent.winmd", "WhisperLib.dll",
                               "WhisperProjection.dll" };

        var modelsToPackage = new List<string> { "models/speech_float.eai", "models/vocab.bin",
                                                 "models/decoder_model_htp.bin", "models/encoder_model_htp.bin" };

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string archString = "arm64ec-windows-msvc";

            if (Target.WindowsPlatform.Architecture == UnrealArch.Arm64)
            {
                archString = "aarch64-windows-msvc";
            }

            PublicDelayLoadDLLs.Add("WhisperComponent.dll");
            LinkLibs(libs, archString);
            PackageLibs(libsToPackage, archString);
            PackageFiles(modelsToPackage);
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            System.IO.DirectoryInfo di =
                new System.IO.DirectoryInfo(Path.Combine(ModuleDirectory, "lib", "android", "arm64-v8a"));

            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "AndroidPackaging.xml"));
        }
    }

    private void PackageFiles(List<string> libs)
    {
        foreach (var item in libs)
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, item), StagedFileType.NonUFS);
    }

    private void PackageLibs(List<string> libs, string platformId)
    {
        foreach (var item in libs)
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", platformId, item));
    }

    private void PackageLibs(List<string> libs)
    {
        foreach (var item in libs)
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "lib", item));
    }

    private void LinkLibs(List<string> libs, string platformId)
    {
        foreach (var item in libs)
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", platformId, item));
    }
}
