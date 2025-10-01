//============================================================================================================
//
//
//                  Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "QcomShadowDenoiser.h"
#include "QcomShadowDenoiserImpl.h"
#include "QcomShadowDenoiserPluginSettings.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Runtime/Renderer/Private/ScreenSpaceDenoise.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif // WITH_EDITOR

#define LOCTEXT_NAMESPACE "FQcomShadowDenoiserModule"


void FQcomShadowDenoiserModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FQcomShadowDenoiserModule::PostEngineInit);
	FCoreDelegates::OnPreExit.AddRaw(this, &FQcomShadowDenoiserModule::PreExit);

	// Add the shader source paths
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("QcomShadowDenoiser"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/QcomShadowDenoiser"), PluginShaderDir);
	// Use /ThirdParty/QcomShadowDenoiser/Shared virtual path for the shared header files (the Unreal Build Tool insists on .usf/.ush for all but a few virtual paths, /ThirdParty/ gets around this).  Allows us to share implementation files with non Unreal projects.
	FString PluginExternalShaderDir = FPaths::Combine(PluginShaderDir, TEXT("Shared"));
	AddShaderSourceDirectoryMapping(TEXT("/ThirdParty/QcomShadowDenoiser/Shared"), PluginExternalShaderDir);

	m_DenoiserImplementation = nullptr;
	GScreenSpaceDenoiser = nullptr;

	m_DenoiserImplementation = MakeUnique<FQcomShadowDenoiserImplementation>();
	GScreenSpaceDenoiser = m_DenoiserImplementation.Get();
}

void FQcomShadowDenoiserModule::PostEngineInit()
{
	m_DenoiserImplementation->Initialize();
	RegisterSettings();
}

void FQcomShadowDenoiserModule::PreExit()
{
	UnregisterSettings();
	m_DenoiserImplementation->Shutdown();
}

void FQcomShadowDenoiserModule::ShutdownModule()
{
	GScreenSpaceDenoiser = nullptr;
	m_DenoiserImplementation = nullptr;
}

void FQcomShadowDenoiserModule::RegisterSettings()
{
#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "QcomShadowDenoiser",
																			   LOCTEXT("QcomPluginSettingsName", "Qcom Shadow Denoiser"),
																			   LOCTEXT("QcomPluginSettingsDescription", "Configure the Qualcomm Shadow Denoiser plug-in."),
																			   GetMutableDefault<UQcomShadowDenoiserPluginSettings>()
		);
	}
#endif // WITH_EDITOR
}

void FQcomShadowDenoiserModule::UnregisterSettings()
{
#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "QcomShadowDenoiser");
	}
#endif // WITH_EDITOR
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQcomShadowDenoiserModule, QcomShadowDenoiser)
