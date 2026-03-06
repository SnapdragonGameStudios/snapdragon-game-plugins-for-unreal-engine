
# Snapdragon Game AI - Text-To-Speech

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.x-informational.svg)
![Platform](https://img.shields.io/badge/platform-Win64%20%7C%20Android-lightgrey.svg)
![License](https://img.shields.io/badge/license-BSD--3--Clause-green.svg)
![Qualcomm](https://img.shields.io/badge/Qualcomm-Snapdragon%20GameAI-red.svg)
![NPU](https://img.shields.io/badge/NPU-Accelerated-orange.svg)

**Hardware Accelerated Text-To-Speech plugin for Unreal Engine**

Powered by Qualcomm's Voice AI SDK with NPU-accelerated models on Snapdragon platforms

[Features](#features) • [Installation](#installation) • [Quick Start](#quick-start) • [Documentation](#api-reference)

</div>

## Overview

This plugin provides a comprehensive Text-to-Speech solution for Unreal Engine projects targeting Android devices. It wraps the functionality of Qualcomm's VoiceAI TTS SDK.

**Leveraging Snapdragon NPU**: On Snapdragon-powered devices, the Voice AI TTS SDK takes full advantage of the Neural Processing Unit (NPU) to deliver hardware-accelerated speech synthesis with exceptional performance and power efficiency.

## Key Highlights

- ⚡ **NPU-Accelerated**: Hardware acceleration on Snapdragon platforms for optimal performance
- 🔋 **Power Efficient**: NPU offloading reduces CPU/GPU load and extends battery life on mobile devices
- 🌐 **Cross-Platform**: Native support for Windows and Android
- 🧠 **Multiple Models**: Choose from small, large-turbo, and quantized Whisper variants
- 📱 **Blueprint & C++**: Full support for both Blueprint and C++ workflows

- **Android-only support**

## Features
### Core Capabilities
- **Configurable voice parameters**:
  - Language (English, Chinese, Spanish)
  - Speech rate (0.25 to 4.0)
  - Pitch (-20.0 to 20.0)
  - Volume gain (-96.0 to 16.0 dB)
  - Sample rate
  - Audio encoding (LINEAR16, MP3, OGG_OPUS, MULAW, ALAW)
- **Raw audio data access** - Get audio data for custom processing

### Supported Models
|Model|
|-----|
|[**mello-tts**](https://aihub.qualcomm.com/mobile/models/melotts_en?domain=Audio&useCase=Audio+Generation)|

### Supported Snapdragon Platforms
| Platform | Status |
|----------|-------------|
| **Android** | ✅ Supported |

## Prerequisites
#### Voice AI SDK
[Download](https://qpm.qualcomm.com/#/main/tools/details/VoiceAI_TTS) the Voice AI TTS SDK from Qualcomm Package manager
#### Unreal Engine
- Developed with Unreal Engine 5.6
- Supported Platforms: Android

#### Required Plugins
The following Unreal Engine plugins must be enabled:
| Plugin | Description|
|-----|----|
|**QAIRT**| Qualcomm AI Runtime for NPU acceleration|


## Installation

### Step 1: Plugin Setup
1. **Download or Clone** this Text-To-Speech Plugin and the dependent QAIRT plugin to your project's `Plugins` directory.
### Step 2: Download Voice AI SDK
1. Download and install the **Voice AI TTS SDK** from Qualcomm Package Manager:
[Voice AI SDK]((https://qpm.qualcomm.com/#/main/tools/details/VoiceAI_TTS)
2. Extract The required binaries
Copy the following files into the corresponding plugin folders

|file|target|
|----|-----|
|tts-sdk.jar|Source/VoiceAITTS/ThirdParty/VoiceAITTSLib/libs/android/|
|libtts.so |Source/VoiceAITTS/ThirdParty/VoiceAITTSLib/libs/android/arm64-v8a/|
|libtts_jni.so|Source/VoiceAITTS/ThirdParty/VoiceAITTSLib/libs/android/arm64-v8a/|

### Step 3: Download or Generate the model files
#### Step 3a: Download
Model context binaries can be downloaded from AIHub [Download](https://aihub.qualcomm.com/mobile/models/melotts_en?domain=Audio&useCase=Audio+Generation)
#### Step 3a: Generate
Alternatively, following the instructions from Voice AI TTS SDK, model files can be generated.

#### Step 3b: Generate the combined model artifact file.
Follow the instructions from Voice AI TTS SDK's documentation to generate the single combined .qnn file

After completing all installation steps, your `VoiceAITTSLib` directory should look like this:
```
plugin/SGAITextToSpeech/Source/VoiceAITTS/ThirdParty/VoiceAITTSLib/
├── libs/
│   └── android/
│       └── tts-sdk.jar
│       └── arm64-v8a
│           └── libtts.so
│           └── libtts_jni.so
├── models/
│   ├── decoder_model_htp.bin
│   ├── encoder_model_htp.bin
│   └── vocab.bin
├── AndroidPackaging.xml
└── VoiceAITTSLib.Build.cs
```

## Modules

### SGAITextToSpeech (Runtime Module)
The main plugin module that provides the TTS engine implementation.

**Key Classes:**
- `UTextToSpeechEngine` - UObject for TTS engine
- `UTextToSpeechEngineAndroid` - Android-specific implementation

### VoiceAILibTTS (Third-party Module)
External module that manages the Qualcomm TTS SDK dependencies.

## Usage

### C++ Example

```cpp
#include "SGAITextToSpeechModule.h"
#include "TextToSpeechEngine.h"

// Create TTS engine instance
UTextToSpeechEngine* TTSEngine = FSGAITextToSpeechModule::Get().CreateTextToSpeechEngine();

// Configure TTS
FTTSConfig Config;
Config.ModelPath = TEXT("/sdcard/tts_models/model.bin");
Config.Language = ETTSLanguage::English;
Config.SpeechRate = 1.0f;
Config.Pitch = 0.0f;
Config.VolumeGain = 0.0f;
Config.SampleRate = 44100;

// Bind to events
TTSEngine->OnSpeechStarted.AddDynamic(this, &AMyActor::OnSpeechStarted);
TTSEngine->OnAudioDataAvailable.AddDynamic(this, &AMyActor::OnAudioData);
TTSEngine->OnSpeechCompleted.AddDynamic(this, &AMyActor::OnSpeechCompleted);
TTSEngine->OnSpeechError.AddDynamic(this, &AMyActor::OnSpeechError);

// Initialize
ETTSErrorCode Result = TTSEngine->Initialize(Config);
if (Result == ETTSErrorCode::Success)
{
    // Start speech synthesis
    TTSEngine->Speak(TEXT("Hello, this is a test of text to speech."));
}
```

### Blueprint Example

1. Create a TTS Engine instance using the module's `CreateTextToSpeechEngine` function
2. Configure the engine with `Initialize` node
3. Bind to events (OnSpeechStarted, OnAudioDataAvailable, OnSpeechCompleted, OnSpeechError)
4. Call `Speak` with your text
5. Handle the callbacks as needed

## API Reference

### UTextToSpeechEngine

**Methods:**
- `Initialize(FTTSConfig Config)` - Initialize the TTS engine
- `Speak(FString Text)` - Start speech synthesis
- `Stop()` - Stop current speech
- `Deinitialize()` - Clean up resources
- `SetLanguage(ETTSLanguage Language)` - Change language
- `SetSpeechRate(float Rate)` - Change speech rate
- `SetPitch(float Pitch)` - Change pitch
- `SetVolumeGain(float Gain)` - Change volume
- `SetSampleRate(int32 SampleRate)` - Change sample rate

**Events:**
- `OnSpeechStarted` - Fired when speech synthesis starts
- `OnAudioDataAvailable` - Fired when audio data is available
- `OnSpeechCompleted` - Fired when speech synthesis completes
- `OnSpeechError` - Fired when an error occurs

## Requirements

- Unreal Engine 5.6 or later
- Qualcomm Voice AI TTS SDK 


## License
Check out the [LICENSE](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine/blob/main/LICENSE) for more details.
