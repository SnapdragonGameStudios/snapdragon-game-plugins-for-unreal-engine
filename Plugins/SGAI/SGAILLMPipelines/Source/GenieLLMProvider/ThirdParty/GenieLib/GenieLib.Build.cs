// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

using System.IO;
using UnrealBuildTool;

public class GenieLib : ModuleRules
{
    public GenieLib(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        bUseRTTI = true;
        bEnableExceptions = true;

        // Add any macros that need to be set
        PublicDefinitions.Add("WITH_GENIE=1");

        string QAIRTDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "..", "..", "..", "qairt"));
        System.Console.WriteLine(QAIRTDir);

        // Add any include paths for the plugin
        PublicIncludePaths.Add(Path.Combine(QAIRTDir, "Source/ThirdParty/qairt/inc/Genie"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            if (Target.WindowsPlatform.Architecture == UnrealArch.Arm64)
            {
                // Add the import library
                PublicAdditionalLibraries.Add(
                    Path.Combine(QAIRTDir, "Source/ThirdParty/qairt/lib/aarch64-windows-msvc", "Genie.lib"));

                // Delay-load the DLL, so we can load it from the right place first
                PublicDelayLoadDLLs.Add("Genie.dll");

                // Ensure that the DLL is staged along with the executable
                RuntimeDependencies.Add(
                    "$(PluginDir)/Source/GeniePlugin/Private/ThirdParty/Genie/lib/aarch64-windows-msvc/Genie.dll");
            }
            else
            {
                // // Add the import library
                PublicAdditionalLibraries.Add(
                    Path.Combine(QAIRTDir, "Source/ThirdParty/qairt/lib/x86_64-windows-msvc", "Genie.lib"));

                // Delay-load the DLL, so we can load it from the right place first
                PublicDelayLoadDLLs.Add("Genie.dll");
                // Ensure that the DLL is staged along with the executable
                RuntimeDependencies.Add(
                    Path.Combine(QAIRTDir, "Source/ThirdParty/qairt/lib/x86_64-windows-msvc", "Genie.dll"));
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string[] LibPaths = new[] {
                Path.Combine(QAIRTDir, "Source/ThirdParty/qairt/lib/aarch64-android", "libGenie.so"),
            };

            PublicAdditionalLibraries.AddRange(LibPaths);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "AndroidPackaging.xml"));
        }
    }
}
