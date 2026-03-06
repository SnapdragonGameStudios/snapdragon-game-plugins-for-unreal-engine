// Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Framework/ConversationEngine.h"
#include "Async/Async.h"
#include "Framework/FrameworkUtils.h"
#include "LLMPipelinesNative/ConversationEngineNative.h"
#include "ProviderHelpers.h"

namespace
{
FString SerializeTools(const TArray<FLLMToolDefinition> &Tools)
{
    FString ToolsJson;
    {
        TArray<TSharedPtr<FJsonValue>> ToolsArray;
        for (const FLLMToolDefinition &Tool : Tools)
        {
            TSharedPtr<FJsonObject> ToolObj = MakeShared<FJsonObject>();
            ToolObj->SetStringField(TEXT("name"), Tool.Name);
            ToolObj->SetStringField(TEXT("description"), Tool.Description);

            // Parse parameters JSON string into object
            TSharedPtr<FJsonObject> ParamsObj;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Tool.ParametersJson);
            if (FJsonSerializer::Deserialize(Reader, ParamsObj) && ParamsObj.IsValid())
            {
                ToolObj->SetObjectField(TEXT("parameters"), ParamsObj);
            }
            else
            {
                // Fallback to empty object if parse fails
                ToolObj->SetObjectField(TEXT("parameters"), MakeShared<FJsonObject>());
            }

            ToolsArray.Add(MakeShared<FJsonValueObject>(ToolObj));
        }

        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ToolsJson);
        FJsonSerializer::Serialize(ToolsArray, Writer);
    }
    return ToolsJson;
}

FConversationEngineNative::FConfig GetFConversationEngineNativeConfig(const FConversationEngineConfig &Config)
{
    FConversationEngineNative::FConfig config;
    config.SystemPrompt = TCHAR_TO_UTF8(*Config.SystemPrompt);
    config.ToolDefinitions = TCHAR_TO_UTF8(*SerializeTools(Config.Tools));
    config.ModelContextConfig = TMapToStdMap(Config.ModelContextConfig);
    return config;
}
} // namespace

void UConversationEngine::Initialize(const FConversationEngineConfig &EngineConfig,
                                     const FOnOperationComplete &OnComplete, const FOnExecuteTool &OnExecuteTool,
                                     const FOnLLMTokenGenerated &OnToken)
{
    this->Config = EngineConfig;
    if (Config.Model)
    {
        Execute([this, OnComplete, OnExecuteTool, OnToken]() {
            // Create conversation engine with context
            bool bResult = false;
            Engine = MakeUnique<FConversationEngineNative>();
            if (Engine.IsValid())
            {
                FConversationEngineNative::FConfig config = GetFConversationEngineNativeConfig(this->Config);

                bResult = Engine->Init(
                    Config.Model->GetNativeObj().Get(), config,
                    [this, OnExecuteTool](const LLMPipelinesNative::FToolCall &Call) {
                        return ExecuteToolInternal(Call, OnExecuteTool);
                    },
                    [OnToken](LLMPipelinesNative::ESentenceCode Code, const std::string &Token) {
                        // Marshal to game thread
                        OnGameThread([OnToken, Code, Token]() {
                            if (OnToken.IsBound())
                            {
                                OnToken.Execute(static_cast<ELLMSentenceCode>(Code),
                                                FString(UTF8_TO_TCHAR(Token.c_str())));
                            }
                        });
                    });
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create conversation engine"));
            }

            OnGameThread([OnComplete, bResult]() {
                if (OnComplete.IsBound())
                {
                    OnComplete.Execute(bResult);
                }
            });
        });
    }
}

void UConversationEngine::SendMessage(const FString &UserMessage)
{
    if (Engine.IsValid())
    {
        // Execute on background thread
        Execute([this, UserMessage]() {
            std::string UserMessageStr = TCHAR_TO_UTF8(*UserMessage);
            Engine->SendMessage(UserMessageStr);
        });
    }
}

void UConversationEngine::Reset()
{
    if (Engine.IsValid())
    {
        Engine->Reset();
    }
}

void UConversationEngine::Abort()
{
    if (Engine.IsValid())
    {
        Engine->Abort();
    }
}

std::string UConversationEngine::ExecuteToolInternal(const LLMPipelinesNative::FToolCall &ToolCall,
                                                     FOnExecuteTool Executor)
{
    FString Response;
    if (Executor.IsBound())
    {
        // Create a promise/future pair for cross-thread communication
        TPromise<FString> Promise;
        TFuture<FString> Future = Promise.GetFuture();

        // Convert to Blueprint tool call
        FToolCall BPCall(ToolCall.ToolName.c_str(), ToolCall.ArgumentsJson.c_str());

        // Marshal tool execution to game thread to avoid Slate/UI crashes
        OnGameThread([Executor, BPCall, Promise = MoveTemp(Promise)]() mutable {
            // Execute Blueprint delegate on game thread (safe for Slate/UI operations)
            FString Response;
            Executor.Execute(BPCall, Response);

            // Fulfill the promise with the result
            Promise.SetValue(Response);
        });

        // Wait for result on background thread (safe - not holding game thread resources)
        Response = Future.Get();
    }

    // Convert to std::string and return
    return TCHAR_TO_UTF8(*Response);
}
