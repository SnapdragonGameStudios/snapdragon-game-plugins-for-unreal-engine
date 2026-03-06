# Snapdragon Game AI - LLM Pipelines

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6-informational.svg)
![Platform](https://img.shields.io/badge/platform-Win64%20%7C%20Android-lightgrey.svg)
![License](https://img.shields.io/badge/license-BSD--3--Clause-green.svg)
![Qualcomm](https://img.shields.io/badge/Qualcomm-Snapdragon%20GameAI-red.svg)
![NPU](https://img.shields.io/badge/NPU-Accelerated-orange.svg)

**Hardware Accelerated LLM Inference for Unreal Engine**

Powered by Qualcomm's Genie SDK with NPU-accelerated models on Snapdragon platforms

[Features](#features) • [Installation](#installation) • [Quick Start](#quick-start)

</div>

---

## Overview

This plugin provides comprehensive conversational AI capabilities for Unreal Engine. Built as a wrapper around Qualcomm's Genie SDK, it enables developers to create intelligent, context-aware AI experiences directly within their games and applications.

**Leveraging Snapdragon On-Device AI**: On Snapdragon-powered devices, the Genie SDK takes full advantage of the Neural Processing Unit (NPU) to deliver hardware-accelerated LLM inference with exceptional performance and power efficiency. This enables truly on-device AI experiences with enhanced privacy, reduced latency, and offline capability.

### Key Highlights
- ⚡ **NPU-Accelerated**: Hardware acceleration on Snapdragon platforms for optimal performance
- 🔋 **Power Efficient**: NPU offloading reduces CPU/GPU load and extends battery life
- 🌐 **Cross-Platform**: Native support for Windows (x64, ARM64) and Android
- 🧠 **Multiple Model Support**: Compatible with various LLM and embedding models
- 📱 **Blueprint & C++**: Full support for both Blueprint and C++ workflows

---

## Features

### Capabilities

- **Conversation Engine**: Multi-turn conversations with system prompts, context management, and conversation history
- **Tool Calling**: Function calling capabilities enabling LLMs to trigger actions and retrieve information
- **Embeddings**: Text-to-vector embedding generation for semantic search and similarity matching
- **Vector Database**: RAG support with vector storage for efficient similarity search
- **Streaming Responses**: Token-by-token generation with real-time callbacks for responsive UX
- **Blueprint Integration**: Full Blueprint support with delegate-based event system
- **C++ API**: Complete C++ interface for advanced integration scenarios

### Supported Models

For a full list of models, refer to the [Qualcomm LLM On-Device Deployment guide](https://github.com/qualcomm/ai-hub-apps/tree/main/tutorials/llm_on_genie).

#### Supported Backends

| Backend | Description | Platforms |
|---------|-------------|-----------|
| **CPU** | x64, ARM64 | Windows, Android |
| **HTP** | Hexagon Tensor Processor (NPU) | Snapdragon NPU enabled devices |

### Supported Platforms

| Platform | Architecture  | Status |
|----------|-------------|---------|
| **Windows** | x64, ARM64 | ✅ Supported |
| **Android** | ARM64 (aarch64) | ✅ Supported |

---

## Prerequisites

### Unreal Engine
- Developed with **Unreal Engine 5.6**
- Supported platforms: Win64, Android

### Required Plugins

The following Unreal Engine plugins must be enabled:

| Plugin | Description |
|--------|-------------|
| **QAIRT** | Qualcomm AI Runtime for NPU acceleration and model execution |

---

## Installation

### Step 1: Plugin Setup

Clone this LLMPipelines plugin **recursively** as well as the dependent QAIRT plugin to your project's `Plugins` directory:

```
YourProject/
└── Plugins/
    ├── LLMPipelines/
    └── QAIRT/
```

### Step 2: Download Qualcomm AI Runtime SDK

Follow instructions on the QAIRT Plugin's README.

### Step 3: Obtain Model Artifacts

Models need to be converted to Genie-compatible formats and quantized as required.

#### For LLM Models

**Required Files:**
- Compiled model binaries
- `llm.json` - Genie dialog configuration file (**must be named exactly this**)
- `tokenizer_config.json` - Tokenizer configuration (**must be named exactly this**)

**Sources:**

1. **Qualcomm AI Hub**: [https://aihub.qualcomm.com/](https://aihub.qualcomm.com/)
   - Browse pre-compiled models optimized for Snapdragon hardware
   - Download model binaries directly for your target platform
   - Filter by "Generative AI" or "Text Generation" categories for LLMs

2. **Generate Custom Models**: Follow AI Hub tutorials to compile your own models
   - GitHub Tutorial: [Qualcomm LLM On-Device Deployment guide](https://github.com/qualcomm/ai-hub-apps/tree/main/tutorials/llm_on_genie)
   - Compile models for specific hardware targets (CPU, HTP)
   - Optimize for your performance requirements

3. **Tokenizer Configuration**: Download from HuggingFace
   - Visit the model's HuggingFace page (e.g., [Qwen/Qwen2.5-7B-Instruct](https://huggingface.co/Qwen/Qwen2.5-7B-Instruct))
   - Download `tokenizer_config.json` from the "Files and versions" tab.

**Example**

Place the files in your project's Content directory:

```
YourProject/Content/Models/LLM/
├── llm.bin                    # From Qualcomm LLM On-Device Deployment guide
├── llm.json                   # Genie dialog config (must be named exactly this)
└── tokenizer_config.json      # From HuggingFace (must be named exactly this)
```

**Important Notes:**
- The required file names above, i.e., `llm.json` and `tokenizer_config.json`, are case-sensitive
- Both `llm.json` and `tokenizer_config.json` must be in the same directory
- Ensure the model binary matches your target platform (Windows x64/ARM64 or Android ARM64)

#### For Embedding Models

**Required Files:**
- `encoder_model.bin` - Compiled encoder binary (or equivalent model file)
- `embedder.json`- Genie configuration file (**must be named exactly this**)

**Example:**

```
YourProject/Content/Models/Embedder/
├── encoder_model.bin      # From Qualcomm AI Hub
└── embedder.json          # Genie config (must be named exactly this)

```

**Sources:**
- Download from [Qualcomm AI Hub](https://aihub.qualcomm.com/)
- Follow similar compilation process as LLMs using AI Hub tools or deployment guide

### Step 4: Verify Installation

After completing the installation steps, your plugin directory should look like this:

```
Plugins/LLMPipelines/
├── LLMPipelines.uplugin
└── Source/
    ├── LLMPipelines/           # Core framework
    ├── GenieLLMProvider/       # Genie SDK integration
    └── VectorDBProvider/       # Vector database for RAG
```

And your models should be organized in your desired directory (e.g., your project's Content directory):

```
YourProject/Content/Models/
├── LLM/
│   ├── llm.bin
│   ├── llm.json
│   └── tokenizer_config.json
└── Embedder/
    └── encoder_model.bin
    └── embedder.json
```

---

## Quick Start

### C++ Usage

#### 1. Include Headers

```cpp
#include "Framework/ConversationEngine.h"
#include "Framework/Embedder.h"
#include "Framework/VectorDB.h"
#include "Framework/LLMPipelinesTypes.h"
```

#### 2. Initialize Conversation Engine

```cpp
// Create the conversation engine
UConversationEngine* Engine = UConversationEngine::CreateConversationEngine(this);

// Define tools (optional)
TArray<FLLMToolDefinition> Tools;
// Add tool definitions if needed

// Initialize the engine
Engine->Initialize(
    TEXT("GenieLLMProvider"),
    TEXT("/Game/Models/Qwen2.5-7B-Instruct/llm.json"),
    TEXT("/Game/Models/Qwen2.5-7B-Instruct/tokenizer_config.json"),
    TEXT("You are a helpful AI assistant in a game."),
    Tools,
    FOnOperationComplete::CreateLambda([](bool bSuccess)
    {
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Conversation engine initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize conversation engine"));
        }
    })
);
```

#### 3. Send Messages

```cpp
// Send a message with callbacks
Engine->SendMessage(
    TEXT("What is the capital of France?"),
    TEXT(""), // RAG context (optional)
    
    // Tool executor callback
    FOnExecuteTool::CreateLambda([](const FToolCall& ToolCall, FString& OutResponse)
    {
        // Handle tool execution
        UE_LOG(LogTemp, Log, TEXT("Tool called: %s"), *ToolCall.ToolName);
        OutResponse = TEXT("Tool execution result");
    }),
    
    // Token streaming callback
    FOnLLMTokenGenerated::CreateLambda([](ELLMSentenceCode Code, const FString& Token)
    {
        if (Code == ELLMSentenceCode::Begin)
        {
            UE_LOG(LogTemp, Log, TEXT("Response started"));
        }
        else if (Code == ELLMSentenceCode::Continue)
        {
            UE_LOG(LogTemp, Log, TEXT("Token: %s"), *Token);
        }
        else if (Code == ELLMSentenceCode::End)
        {
            UE_LOG(LogTemp, Log, TEXT("Response complete"));
        }
    }),
    
    // Completion callback
    FOnComplete::CreateLambda([]()
    {
        UE_LOG(LogTemp, Log, TEXT("Message processing complete"));
    })
);
```

#### 4. RAG Example

```cpp
// Initialize embedder
UEmbedder* Embedder = NewObject<UEmbedder>();

TMap<FString, FString> EmbedderConfig;
EmbedderConfig.Add(TEXT("provider"), TEXT("GenieLLMProvider"));
EmbedderConfig.Add(TEXT("model_path"), TEXT("/Game/Models/BGE-Large/encoder_model.bin"));

Embedder->Initialize(EmbedderConfig, 
    FOnOperationComplete::CreateLambda([this, Embedder](bool bSuccess)
    {
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Embedder initialized, dimension: %d"), Embedder->Dimension());
            
            // Generate embedding for a query
            Embedder->Generate(TEXT("What is machine learning?"), true,
                FOnEmbeddingComplete::CreateLambda([](const TArray<float>& Embedding, bool bSuccess)
                {
                    if (bSuccess)
                    {
                        UE_LOG(LogTemp, Log, TEXT("Generated embedding with %d dimensions"), Embedding.Num());
                        // Use embedding for vector search
                    }
                })
            );
        }
    })
);

// Initialize vector database
UVectorDB* VectorDB = NewObject<UVectorDB>();

TMap<FString, FString> VectorDBConfig;
VectorDBConfig.Add(TEXT("provider"), TEXT("VectorDBProvider"));
VectorDBConfig.Add(TEXT("index_path"), TEXT("/Game/Data/VectorIndex"));

VectorDB->Initialize(VectorDBConfig,
    FOnOperationComplete::CreateLambda([VectorDB](bool bSuccess)
    {
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("VectorDB initialized"));
        }
    })
);

// Query vector database
TArray<float> QueryEmbedding; // Your query embedding
VectorDB->Query(QueryEmbedding,
    FOnRAGQueryComplete::CreateLambda([](const FRAGQueryResult& Result)
    {
        if (Result.bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Retrieved context: %s"), *Result.RetrievedText);
            UE_LOG(LogTemp, Log, TEXT("Similarity score: %f"), Result.SimilarityScore);
            // Pass Result.RetrievedText as RAG context to SendMessage
        }
    })
);
```

#### 5. Conversation Management

```cpp
// Get conversation history
TArray<FLLMMessage> History = Engine->GetConversationHistory();
for (const FLLMMessage& Message : History)
{
    UE_LOG(LogTemp, Log, TEXT("%s: %s"), *Message.Role, *Message.Content);
}

// Reset conversation
Engine->ResetConversation();

// Check if processing
if (Engine->IsProcessing())
{
    // Abort current processing
    Engine->Abort();
}

// Get last error
FString LastError = Engine->GetLastError();
if (!LastError.IsEmpty())
{
    UE_LOG(LogTemp, Error, TEXT("Last error: %s"), *LastError);
}
```

### Blueprint Usage

Refer to the included Sample Project, which demonstrates the key Blueprint features of the plugin. Its [README](../../sample/README.md) provides further details.

---

## Architecture Overview

### Module Structure

The LLMPipelines plugin consists of three main modules:

| Module | Description | Dependencies |
|--------|-------------|--------------|
| **LLMPipelines** | Core framework providing Blueprint wrappers, native interfaces, and conversation management | QAIRT |
| **GenieLLMProvider** | Genie SDK integration for LLM inference and embedding generation | LLMPipelines, Genie SDK |
| **VectorDBProvider** | Annoy-based vector storage for RAG and similarity search | LLMPipelines |

### Component Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    Blueprint Layer                      │
│  UConversationEngine │ UEmbedder │ UVectorDB            │
└────────────────┬────────────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────────────┐
│                  LLMPipelines Core                      │
│  Native Interfaces │ Conversation Logic │ Types         │
└────────┬───────────────────────┬─────────────────────┬──┘
         │                       │                     │
┌────────▼──────────┐  ┌─────────▼─────────┐  ┌────────▼───────┐
│ GenieLLMProvider  │  │ VectorDBProvider  │  │  QAIRT Plugin  │
│ • LLM Inference   │  │ • Annoy Index     │  │ • NPU Runtime  │
│ • Embeddings      │  │ • Similarity      │  │ • HTP Backend  │
└───────────────────┘  └───────────────────┘  └────────────────┘
         │
┌────────▼──────────┐
│   Genie SDK       │
│ • Model Execution │
│ • NPU Acceleration│
└───────────────────┘
```

---

## Getting Help

For additional support:
- Check the [Qualcomm Developer Network](https://developer.qualcomm.com/)
- Review [Genie SDK documentation](https://www.qualcomm.com/developer/software/gen-ai-inference-extensions)
- Explore [AI Hub tutorials](https://github.com/qualcomm/ai-hub-apps/tree/main/tutorials)

---

## License
Check out the [LICENSE](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine/blob/main/LICENSE) for more details.
