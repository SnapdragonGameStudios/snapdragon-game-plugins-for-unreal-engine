// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Framework/Embedder.h"
#include "Async/Async.h"
#include "Framework/FrameworkUtils.h"
#include "Misc/FileHelper.h"
#include "ProviderHelpers.h"

void UEmbedder::Initialize(const TMap<FString, FString> &Config, FOnOperationComplete OnComplete)
{
    // Call on GAME THREAD ONLY
    IProviderModuleInterface *ModuleInterface = FProviderHelpers::GetProviderModule("GenieLLMProvider");

    // Execute on background thread
    Execute([this, ModuleInterface, Config, OnComplete]() {
        std::map<std::string, std::string> config = TMapToStdMap(Config);
        std::unique_ptr<LLMPipelinesNative::IEmbedder> EmbedderPtr = ModuleInterface->CreateEmbedder(config);
        if (EmbedderPtr != nullptr)
        {
            Embedder = MakeShareable<LLMPipelinesNative::IEmbedder>(EmbedderPtr.release());

            // Test to get dimension
            LLMPipelinesNative::VectorEmbedding TestEmbed = Embedder->Generate("test", true);
            EmbeddingDimension = TestEmbed.size();

            UE_LOG(LogTemp, Verbose, TEXT("Embedder initialized with dimension: %d"), EmbeddingDimension);

            // Marshal result back to game thread
            OnGameThread([OnComplete]() {
                if (OnComplete.IsBound())
                {
                    OnComplete.Execute(true);
                }
            });
        }
    });
}

void UEmbedder::Generate(const FString &Text, bool bNormalize, FOnEmbeddingComplete OnComplete)
{
    if (Embedder.IsValid())
    {
        // Execute on background thread
        Execute([this, Text, bNormalize, OnComplete]() {
            std::string TextStr = TCHAR_TO_UTF8(*Text);
            LLMPipelinesNative::VectorEmbedding Embedding = Embedder->Generate(TextStr, bNormalize);

            TArray<float> Result = StdVectorToTArray(Embedding);

            // Marshal result back to game thread
            OnGameThread([OnComplete, Result]() {
                if (OnComplete.IsBound())
                {
                    OnComplete.Execute(Result, true);
                }
            });
        });
    }
    else
    {
        TArray<float> EmptyResult;
        UE_LOG(LogTemp, Error, TEXT("Embedder not initialized"));

        if (OnComplete.IsBound())
        {
            OnComplete.Execute(EmptyResult, false);
        }
        return;
    }
}

void UEmbedder::Destroy()
{
    if (Embedder.IsValid())
    {
        Embedder->Destroy();
        Embedder.Reset();
    }
    EmbeddingDimension = 0;
}
