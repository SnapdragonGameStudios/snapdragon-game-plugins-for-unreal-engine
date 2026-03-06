// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Framework/LLMModel.h"
#include "FrameworkUtils.h"
#include "ProviderHelpers.h"

void ULLMModel::Initialize(const TMap<FString, FString> &Config, const FOnOperationComplete &OnComplete)
{
    // Call on GAME THREAD ONLY
    IProviderModuleInterface *ModuleInterface = FProviderHelpers::GetProviderModule("GenieLLMProvider");

    // Execute on background thread
    Execute([this, Config, ModuleInterface, OnComplete]() {
        std::unique_ptr<LLMPipelinesNative::ILLMModel> ptr = ModuleInterface->CreateLLMModel(TMapToStdMap(Config));
        const bool bResult = (ptr != nullptr);
        if (bResult)
        {
            LLMModelNativeObj = MakeShareable<LLMPipelinesNative::ILLMModel>(ptr.release());
        }
        OnGameThread([OnComplete, bResult]() {
            if (OnComplete.IsBound())
            {
                OnComplete.Execute(bResult);
            }
        });
    });
}

TSharedPtr<LLMPipelinesNative::ILLMModel> ULLMModel::GetNativeObj()
{
    return LLMModelNativeObj;
}
