// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "CoreMinimal.h"
#include "Framework/LLMPipelinesTypes.h"
#include "LLMPipelinesNative/ILLM.h"

#include "LLMModel.generated.h"

UCLASS(BlueprintType)
class LLMPIPELINES_API ULLMModel : public UObject
{
    GENERATED_BODY()
  public:
    UFUNCTION(BlueprintCallable, Category = "LLM|Conversation")
    void Initialize(const TMap<FString, FString> &Config, const FOnOperationComplete &OnComplete);

    TSharedPtr<LLMPipelinesNative::ILLMModel> GetNativeObj();

  private:
    TSharedPtr<LLMPipelinesNative::ILLMModel> LLMModelNativeObj;
};
