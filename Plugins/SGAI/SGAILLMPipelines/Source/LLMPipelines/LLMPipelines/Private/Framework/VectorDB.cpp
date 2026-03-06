// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Framework/VectorDB.h"
#include "Async/Async.h"
#include "Framework/FrameworkUtils.h"
#include "ProviderHelpers.h"

void UVectorDB::Initialize(const TMap<FString, FString> &Config, FOnOperationComplete OnComplete)
{
    // Call on GAME THREAD ONLY
    IProviderModuleInterface *ModuleInterface = FProviderHelpers::GetProviderModule("VectorDBProvider");

    // Execute on background thread
    Execute([this, ModuleInterface, Config, OnComplete]() {
        if (ModuleInterface != nullptr)
        {
            std::map<std::string, std::string> config = TMapToStdMap(Config);
            std::unique_ptr<LLMPipelinesNative::IVectorDB> VectorDBPtr = ModuleInterface->CreateVectorDB(config);
            if (VectorDBPtr != nullptr)
            {
                VectorDB = MakeShareable<LLMPipelinesNative::IVectorDB>(VectorDBPtr.release());
                // Marshal result back to game thread
                OnGameThread([OnComplete]() {
                    if (OnComplete.IsBound())
                    {
                        OnComplete.Execute(true);
                    }
                });
            }
        }
    });
}

void UVectorDB::Query(const TArray<float> &Embedding, FOnVectorDBQueryComplete OnComplete)
{
    if (VectorDB.IsValid())
    {
        // Execute on background thread
        Execute([this, Embedding, OnComplete]() {
            // Query vector DB directly with embedding
            LLMPipelinesNative::Chunk ResultChunk = VectorDB->Query(TArrayToStdVector(Embedding));

            FVectorDBQueryResult Result;
            Result.RetrievedText = FString(UTF8_TO_TCHAR(ResultChunk.second.c_str()));
            Result.SimilarityScore = ResultChunk.first;
            Result.bSuccess = true;

            // Marshal result back to game thread
            OnGameThread([OnComplete, Result]() {
                if (OnComplete.IsBound())
                {
                    OnComplete.Execute(Result);
                }
            });
        });
    }
    else
    {
        FVectorDBQueryResult Result;
        Result.bSuccess = false;
        // Marshal result back to game thread
        OnGameThread([OnComplete, Result]() {
            if (OnComplete.IsBound())
            {
                OnComplete.Execute(Result);
            }
        });
    }
}
