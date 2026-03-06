# Snapdragon™ Game AI

Snapdragon™ Game AI SDK empowers game developers to put advanced on-device AI capabilities directly into their mobile and PC games.

![Snapdragon™ Game AI](media/sgai.png)

## 🎮 Overview

The Snapdragon Game AI SDK provides a comprehensive suite of on-device AI features optimized for real-time game scenarios. These features are currently enabled through Unreal Engine 5 plugins:

- [Speech Recognizer](#sgaispeechrecognizer)
- [LLM Pipelines](#sgaillmpipelines)
- [Text to Speech](#sgaitexttospeech)


## 📦 Plugins

### **SGAISpeechRecognizer**
*Available Engine Versions:*
| [5.6](https://github.com/quic/snapdragon-game-plugins-for-unreal-engine/tree/engine/5.6/Plugins/SGAI/sgai-speechrecognizer) |
|---|


Transform player voice input into text with real-time speech recognition. 
- Real-time voice-to-text conversion
- NPU accelerated inference using Qualcomm's Voice AI SDK

### **SGAILLMPipelines**
*Available Engine Versions:* 
| [5.6](https://github.com/quic/snapdragon-game-plugins-for-unreal-engine/tree/engine/5.6/Plugins/SGAI/sgai-llmpipelines) |
|---|

Integrate large language models into your game for dynamic, context-aware AI interactions. 
- Seamless LLM integration in Unreal Engine
- NPU accelerated LLM inference using Qualcomm's GenIE SDK.

### **SGAITextToSpeech**
*Available Engine Versions:*
| [5.6](https://github.com/quic/snapdragon-game-plugins-for-unreal-engine/tree/engine/5.6/Plugins/SGAI/sgai-texttospeech) |
|---|

Bring your game characters to life with natural-sounding, AI-generated speech.
- Real-time audio generation
- NPU accelerated inference using Qualcomm's Voice AI SDK

## 🚀 Getting Started

### Prerequisites

- Unreal Engine 5.6 or later
- Qualcomm Snapdragon-powered device

### Installation

1. Download the Snapdragon Game AI plugin package
````
git clone --branch engine/5.6 --recurse-submodules https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine.git


````
2. Copy the plugins to your Unreal Engine project's `Plugins` folder
3. Enable the desired plugins in your project settings

## 📄 License
Check out the [LICENSE](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine/blob/main/LICENSE) for more details.
