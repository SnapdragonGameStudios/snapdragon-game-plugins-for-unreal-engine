//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "ANFViewExtension.h"
#include "ANFTU.h"
#include "ANFTUModule.h"
#include "ANFConfig.h"

#include "PostProcess/PostProcessing.h"

#include "ScenePrivate.h"
#include "EngineUtils.h"
#include "LegacyScreenPercentageDriver.h"

FANFViewExtension::FANFViewExtension(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase(AutoRegister)
{
	PreviousANFState = GEnableANF;

	FANFTUModule& ANFModule = FModuleManager::GetModuleChecked<FANFTUModule>(TEXT("ANFTUModule"));
	if (ANFModule.GetTU() == nullptr)
	{
		TSharedPtr<FANFTU, ESPMode::ThreadSafe> ANFTU = MakeShared<FANFTU, ESPMode::ThreadSafe>();
		ANFModule.SetTU(ANFTU);
	}
}

void FANFViewExtension::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	FANFTUModule& ANFModule = FModuleManager::GetModuleChecked<FANFTUModule>(TEXT("ANFTUModule"));
	check(ANFModule.GetANFU());
	int32 EnableANF = GEnableANF;
	int32 MatchANFResolution = GANFMatchResolution;

	if (EnableANF != 0 && MatchANFResolution != 0)
	{
		FVector2D anfFloatRes = ANFModule.GetANFU()->GetANFOutputResolution();
		FIntVector2 anfRes = FIntVector2(anfFloatRes.X, anfFloatRes.Y);
		float anfPerc = ANFModule.GetANFU()->GetANFScreenPercentage();

		const float resolutionPercentage = FLegacyScreenPercentageDriver::GetCVarResolutionFraction();

		if ((anfRes.X != 0 && anfRes.Y != 0) && 
			(GSystemResolution.ResX != anfRes.X || GSystemResolution.ResY != anfRes.Y ||
				resolutionPercentage != anfPerc))
		{
			if (GEngine->GetWorld())
			{
				UE_LOG(LogANFTU, Verbose, TEXT("Updating Rendering resolution from (%dx%d) %.1f%% to (%dx%d) %.1f%%"),
					GSystemResolution.ResX, GSystemResolution.ResY, 
					resolutionPercentage,
					(int32_t)anfRes.X, (int32_t)anfRes.Y,
					anfPerc);
				FString resCmd = FString(TEXT("r.SetRes ")) + FString::FromInt(anfRes.X) + FString(TEXT("x")) + FString::FromInt(anfRes.Y);
				GEngine->GetWorld()->Exec(GEngine->GetWorld(), *resCmd);

				FString percCmd = FString(TEXT("r.ScreenPercentage ")) + FString::SanitizeFloat(anfPerc);
				GEngine->GetWorld()->Exec(GEngine->GetWorld(), *percCmd);
			}
		}
	}

	if (PreviousANFState != EnableANF)
	{
		PreviousANFState = EnableANF;
	}
}

void FANFViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
	FANFTUModule& ANFModule = FModuleManager::GetModuleChecked<FANFTUModule>(TEXT("ANFTUModule"));
	FANFTU* Upscaler = ANFModule.GetANFU();
	bool isTUrequest = false;
	bool isGameview = !WITH_EDITOR;
	bool isSupported = Upscaler->IsANFSRSupported();


	for (int i = 0; i < InViewFamily.Views.Num(); i++)
	{
		const FSceneView* InView = InViewFamily.Views[i];
		if (ensure(InView))
		{
			isGameview |= InView->bIsGameView;

			isTUrequest |= (InView->PrimaryScreenPercentageMethod == EPrimaryScreenPercentageMethod::TemporalUpscale);
		}
	}

	if (isTUrequest && GEnableANF && (InViewFamily.GetTemporalUpscalerInterface() == nullptr) && isGameview)
	{
		if (isSupported)
		{
			InViewFamily.SetTemporalUpscalerInterface(new FANFTUFork(Upscaler));
		}
		else
		{
			UE_LOG(LogANFTU, Error, TEXT("ANF SR is enabled but not supported on this device; skipping upscaler registration."));
		}
	}
}

void FANFViewExtension::PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily)
{
}

void FANFViewExtension::PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily)
{
}

void FANFViewExtension::PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
}
