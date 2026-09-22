# Snapdragon™ Game AI

Snapdragon™ Game AI provides Unreal Engine plugins for speech recognition, LLM inference, and text-to-speech. Runtime SDKs and model artifacts are obtained separately.

## Plugins

| Plugin | Purpose |
|---|---|
| [Speech recognizer](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine/tree/engine/5.6/Plugins/SGAI/SGAISpeechRecognizer) | Convert captured speech to text using the Voice AI SDK |
| [LLM pipelines](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine/tree/engine/5.6/Plugins/SGAI/SGAILLMPipelines) | Run Genie conversations, tools, embeddings, and retrieval |
| [Text-to-speech](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine/tree/engine/5.6/Plugins/SGAI/SGAITextToSpeech) | Synthesize speech on Android with the Voice AI TTS SDK |
| [QAIRT](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine/tree/engine/5.6/Plugins/SGAI/qairt) | Package the Qualcomm AI Runtime libraries |

## Getting started

1. Choose `engine/5.6` or `engine/5.7` to match the project. Clone that branch with submodules:

```powershell
git clone --branch engine/5.6 --recurse-submodules https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine.git
```

2. Copy the selected plugin and its dependencies from `Plugins/SGAI/` into the project's `Plugins/` directory.
3. Follow each plugin's guide to install the required SDK, copy runtime libraries, and prepare a compatible model.
4. Enable the plugins in Unreal Editor, regenerate project files, and build the project.
5. Test on the intended platform. NPU execution requires supported hardware and model artifacts compiled for that target.

The links above show the UE 5.6 guides. When working on UE 5.7, use the corresponding files in that branch.

## License

Applicable plugin source uses the [repository license](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine/blob/main/LICENSE). Qualcomm SDKs, runtime libraries, models, and third-party dependencies retain their own license and notice requirements.
