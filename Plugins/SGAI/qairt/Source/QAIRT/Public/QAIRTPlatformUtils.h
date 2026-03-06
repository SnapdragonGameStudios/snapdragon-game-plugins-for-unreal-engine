// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QAIRTPlatformUtils.generated.h"

/**
 *
 */
UCLASS()
class QAIRT_API UQAIRTPlatformUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

  public:
    UFUNCTION(BlueprintCallable, Category = "QAIRT Platform Utils")
    static bool IsARM64Host();
    UFUNCTION(BlueprintCallable, Category = "QAIRT Platform Utils")
    static FString GetBinariesPath();
};