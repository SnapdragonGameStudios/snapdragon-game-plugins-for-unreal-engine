# Snapdragon Game AI - Text-To-Speech

## Overview

This plugin wraps Qualcomm's Voice AI TTS SDK for speech synthesis on Android. It exposes C++ and Blueprint interfaces and requires the QAIRT dependency and compatible model files.

## Features
### Voice settings
- **Configurable voice parameters**:
  - Language (English, Chinese, Spanish)
  - Speech rate (0.25 to 4.0)
  - Pitch (-20.0 to 20.0)
  - Volume gain (-96.0 to 16.0 dB)
  - Sample rate
  - Audio encoding (LINEAR16, MP3, OGG_OPUS, MULAW, ALAW)
- **Raw audio data access** - Get audio data for custom processing

### Supported models
|Model|
|-----|
|[**mello-tts**](https://aihub.qualcomm.com/mobile/models/melotts_en?domain=Audio&useCase=Audio+Generation)|

### Supported platforms
| Platform | Status |
|----------|-------------|
| **Android** |  Supported |

## Prerequisites
#### Voice AI SDK
[Download](https://qpm.qualcomm.com/#/main/tools/details/VoiceAI_TTS) the Voice AI TTS SDK from Qualcomm Package manager
#### Unreal Engine
- Developed with Unreal Engine 5.6
- Supported Platforms: Android

#### Required plugins
The following Unreal Engine plugins must be enabled:
| Plugin | Description|
|-----|----|
|**QAIRT**| Qualcomm AI Runtime for NPU acceleration|

## Installation

### Copy the plugins
1. **Download or Clone** this Text-To-Speech Plugin and the dependent QAIRT plugin to your project's `Plugins` directory.
### Install the Voice AI SDK
1. Download and install the **Voice AI TTS SDK** from Qualcomm Package Manager:
[Voice AI SDK](https://qpm.qualcomm.com/#/main/tools/details/VoiceAI_TTS)
2. Extract The required binaries
Copy the following files into the corresponding plugin folders

|file|target|
|----|-----|
|tts-sdk.jar|Source/VoiceAITTS/ThirdParty/VoiceAITTSLib/libs/android/|
|libtts.so |Source/VoiceAITTS/ThirdParty/VoiceAITTSLib/libs/android/arm64-v8a/|
|libtts_jni.so|Source/VoiceAITTS/ThirdParty/VoiceAITTSLib/libs/android/arm64-v8a/|

### Prepare model files
#### Download model artifacts
Model context binaries can be downloaded from AIHub [Download](https://aihub.qualcomm.com/mobile/models/melotts_en?domain=Audio&useCase=Audio+Generation)
#### Generate model artifacts
Alternatively, following the instructions from Voice AI TTS SDK, model files can be generated.

#### Combine model artifacts
Follow the instructions from Voice AI TTS SDK's documentation to generate the single combined .qnn file

After completing all installation steps, your `VoiceAITTSLib` directory should look like this:
```text
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

### VoiceAITTSLib module
External module that manages the Qualcomm TTS SDK dependencies.

## Use the engine

1. Create a `UTextToSpeechEngine` with `NewObject<UTextToSpeechEngine>(Owner)` and retain it in a `UPROPERTY`.
2. Fill `FTTSConfigurationSettings`. `Model` identifies the prepared model file; set language, speech rate, pitch, volume gain, sample rate, and audio encoding as required by the SDK and model.
3. Call `Initialize(Config, OnInitialized)` and wait for `FOnTTSInitialized` to report success.
4. Call `Speak(Text, OnAudioAvailable, OnEvent, OnError)`. Check the returned `ETTSErrorCode` and handle the supplied callbacks.
5. Consume the audio byte buffers from `FOnTTSAudioAvailable` using the configured audio format. Event and error delegates receive integer SDK identifiers.
6. Call `Stop()` to stop synthesis and `Uninitialize()` when finished.

The callbacks are dynamic delegates. Bind them to matching `UFUNCTION` handlers with `BindDynamic`, or use Blueprint events. Keep the object alive while asynchronous work is pending.

The current wrapper accepts voice settings at initialization and exposes `GetConfig()`. See [TextToSpeechEngine.h](Source/SGAITextToSpeech/Public/TextToSpeechEngine.h) and [TextToSpeechTypes.h](Source/SGAITextToSpeech/Public/TextToSpeechTypes.h) for the current API and configuration fields.

## Validation

Use the [sample project](../../../Samples/SGAI/TextToSpeechSample/) to check model loading, synthesis, callback handling, and cleanup. Verify that SDK libraries and model files reach the packaged Android application. Inspect SDK error codes and Unreal logs when initialization or synthesis fails.

## License

Plugin source uses the repository [BSD 3-Clause license](../../../LICENSE). Voice AI, QAIRT, and model artifacts have separate terms and required notices.
