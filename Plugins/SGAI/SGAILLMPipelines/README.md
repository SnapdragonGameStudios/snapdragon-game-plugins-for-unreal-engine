# Snapdragon Game AI LLM pipelines

This plugin provides Genie conversations, streaming responses, tool calls, embeddings, and vector retrieval through Unreal C++ and Blueprint interfaces. It depends on QAIRT and model artifacts prepared for the target device.

## Requirements

Use the matching Unreal Engine branch and enable the QAIRT and SGAILLMPipelines plugins. CPU and HTP execution depend on the installed runtime and model. HTP execution requires supported Snapdragon hardware.

## Install

1. Clone this repository with its submodules. For an existing clone, run `git submodule update --init --recursive`.
2. Copy `SGAILLMPipelines/` and the sibling `qairt/` plugin into the project's `Plugins/` directory. Preserve their subdirectories.
3. Follow the [QAIRT setup guide](../qairt/README.md), regenerate project files, and build the project.
4. Prepare model artifacts using the [Genie deployment guide](https://github.com/qualcomm/ai-hub-apps/tree/main/tutorials/llm_on_genie). Match the model, runtime, and accelerator to the target device.

## Model files

Keep `llm.json`, the model binaries it references, and `tokenizer_config.json` in the model directory. The provider's `config` setting is a filesystem directory containing `llm.json`. Use paths accessible on the target device; Unreal asset identifiers such as `/Game/Models` are not filesystem paths.

An embedding model uses `embedder.json` and the binary and tokenizer files referenced by that configuration. See the model's redistribution terms before packaging its artifacts.

## Create a conversation

The API separates model initialization from conversation initialization. Retain the model and conversation objects in `UPROPERTY` members so garbage collection cannot reclaim them during asynchronous work.

1. Create a `ULLMModel` with `NewObject<ULLMModel>(Owner)` on the game thread.
2. Call `ULLMModel::Initialize` with a `TMap<FString, FString>` containing `config`, the model directory. Wait for `FOnOperationComplete` to report success.
3. Create a `UConversationEngine` and fill `FConversationEngineConfig`. Set `Model` to the initialized model, then set `SystemPrompt`, `ModelContextConfig`, and `Tools` as required.
4. Call `UConversationEngine::Initialize` with the configuration and the completion, tool execution, and token delegates. Wait for initialization to succeed before sending a message.
5. Call `SendMessage(UserMessage)`. The delegates supplied during initialization receive tool requests and streaming output.

These are dynamic Unreal delegates. In C++, bind them to matching `UFUNCTION` handlers with `BindDynamic`; they do not support `CreateLambda`. Blueprint callers can bind matching events.

| Delegate | Handler parameters | Purpose |
|---|---|---|
| `FOnOperationComplete` | `bool bSuccess` | Initialization result |
| `FOnExecuteTool` | `FToolCall ToolCall, FString& OutResponse` | Execute the requested tool and return its response |
| `FOnLLMTokenGenerated` | `ELLMSentenceCode SentenceCode, FString Token` | Receive stream state and output |

Stream states include `Begin`, `Continue`, `End`, `Complete`, and `Abort`. Call `Reset()` to reset a conversation and `Abort()` to stop current processing. The current wrapper does not expose conversation-history or last-error accessors.

See the declarations for [the model](Source/LLMPipelines/LLMPipelines/Public/Framework/LLMModel.h), [conversation configuration and methods](Source/LLMPipelines/LLMPipelines/Public/Framework/ConversationEngine.h), and [delegates and data types](Source/LLMPipelines/LLMPipelines/Public/Framework/LLMPipelinesTypes.h).

## Embeddings and retrieval

1. Initialize a `UEmbedder` with its configuration map and wait for success. Set `config` to the directory containing `embedder.json`.
2. Call `Generate(Text, bNormalize, OnComplete)`. `FOnEmbeddingComplete` receives the embedding array and a success flag.
3. Initialize a `UVectorDB` with the provider's index configuration and wait for success.
4. Pass an embedding to `Query`. `FOnVectorDBQueryComplete` receives `FVectorDBQueryResult`, including `bSuccess`, `RetrievedText`, and `SimilarityScore`.
5. Check `bSuccess` before incorporating retrieved text into the conversation's user message.

Keep the embedding model and vector index dimensions compatible. The provider requires prepared index data; initialization does not build an index from arbitrary documents.

See [UEmbedder](Source/LLMPipelines/LLMPipelines/Public/Framework/Embedder.h), [UVectorDB](Source/LLMPipelines/LLMPipelines/Public/Framework/VectorDB.h), and the [sample project](../../../Samples/SGAI/LLMPipelinesSample/README.md) for integration details.

## Modules

| Module | Purpose |
|---|---|
| LLMPipelines | Blueprint wrappers, native interfaces, and conversation management |
| GenieLLMProvider | Genie model execution and embeddings |
| PromptFormatter | Prompt formatting and model templates |
| VectorDBProvider | Annoy vector retrieval |

## Troubleshooting

If initialization fails, check model paths, runtime versions, target architecture, and Unreal logs. Confirm that QAIRT binaries and required model files reach the packaged application. Test loading, streaming, cancellation, and tool responses on the target device before relying on editor results.

## License

Plugin source uses the repository [BSD 3-Clause license](../../../LICENSE). QAIRT, model artifacts, and bundled submodules have separate terms; retain their license and attribution files.
