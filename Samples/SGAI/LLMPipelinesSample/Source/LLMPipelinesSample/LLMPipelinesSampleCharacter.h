// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"

#include "LLMPipelinesSampleCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ALLMPipelinesSampleCharacter : public ACharacter
{
    GENERATED_BODY()

    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent *CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent *FollowCamera;

  protected:
    /** Jump Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction *JumpAction;

    /** Move Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction *MoveAction;

    /** Look Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction *LookAction;

    /** Mouse Look Input Action */
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction *MouseLookAction;

  public:
    /** Constructor */
    ALLMPipelinesSampleCharacter();

  protected:
    /** Initialize input action bindings */
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

  protected:
    /** Called for movement input */
    void Move(const FInputActionValue &Value);

    /** Called for looking input */
    void Look(const FInputActionValue &Value);

  public:
    /** Handles move inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoMove(float Right, float Forward);

    /** Handles look inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoLook(float Yaw, float Pitch);

    /** Handles jump pressed inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpStart();

    /** Handles jump pressed inputs from either controls or UI interfaces */
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpEnd();

  public:
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent *GetCameraBoom() const
    {
        return CameraBoom;
    }

    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent *GetFollowCamera() const
    {
        return FollowCamera;
    }
};
