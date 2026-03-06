// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "LLMPipelinesSamplePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "LLMPipelinesSample.h"
#include "Widgets/Input/SVirtualJoystick.h"

void ALLMPipelinesSamplePlayerController::BeginPlay()
{
    Super::BeginPlay();

    // only spawn touch controls on local player controllers
    if (ShouldUseTouchControls() && IsLocalPlayerController())
    {
        // spawn the mobile controls widget
        MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

        if (MobileControlsWidget)
        {
            // add the controls to the player screen
            MobileControlsWidget->AddToPlayerScreen(0);
        }
        else
        {

            UE_LOG(LogLLMPipelinesSample, Error, TEXT("Could not spawn mobile controls widget."));
        }
    }
}

void ALLMPipelinesSamplePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // only add IMCs for local player controllers
    if (IsLocalPlayerController())
    {
        // Add Input Mapping Contexts
        if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            for (UInputMappingContext *CurrentContext : DefaultMappingContexts)
            {
                Subsystem->AddMappingContext(CurrentContext, 0);
            }

            // only add these IMCs if we're not using mobile touch input
            if (!ShouldUseTouchControls())
            {
                for (UInputMappingContext *CurrentContext : MobileExcludedMappingContexts)
                {
                    Subsystem->AddMappingContext(CurrentContext, 0);
                }
            }
        }
    }
}

bool ALLMPipelinesSamplePlayerController::ShouldUseTouchControls() const
{
    // are we on a mobile platform? Should we force touch?
    return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
