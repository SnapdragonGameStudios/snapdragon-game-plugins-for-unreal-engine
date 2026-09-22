# Adreno Neural Fusion Unreal Engine plugin

Adreno Neural Fusion provides Super Resolution and Frame Generation for Android Vulkan games. Super Resolution reconstructs an image from a lower-resolution render. Frame Generation produces an additional scene frame for presentation.

![Adreno Neural Fusion scene](media/hero.png)

## Resources

| Resource | Use |
|---|---|
| [Native SDK integration](ANF/Source/ANFSDK/Public/README.md) | SDK API and rendering contract |
| [Debug overlay](ANF/Source/ANFSDK/Public/ANF_Debug_Overlay_User_Guide_External.md) | Inspect SR inputs and temporal artifacts |
| [ANF SDK](https://github.com/SnapdragonGameStudios/adreno-neural-fusion) | Device requirements and native package |

## Requirements

This branch contains integration patches for UE 5.0 through 5.8. Use the patches matching the engine source version.

ANF runtime techniques require an Android device with Snapdragon™ 8 Elite Gen 6 or higher, `arm64-v8a`, and Vulkan. Broader platform support is planned.

The plugin currently supports 2x scaling in each dimension for SR. Mobile Deferred rendering is the recommended SR path. Frame Generation cannot run alongside SR or another upscaler in this plugin. These restrictions describe the Unreal integration, not every native SDK use case.

## Install and enable

1. Copy `ANF/` into the project's `Plugins/` directory or `Engine/Plugins/Runtime/Qualcomm/`.
2. From a compatible Unreal Engine source tree, use `git apply --check <patch-path>` and then `git apply <patch-path>` for each patch from the matching `UE-Patches/UE5.<version>.x/` directory, in numerical order. These are plain diffs.
3. Regenerate project files and build the engine or project.
4. Enable ANF in the Editor's Plugins window.
5. Build and test the Android Vulkan application on a supported device.

Configure temporal upscaling in `DefaultEngine.ini`:

```ini
[/Script/Engine.RendererSettings]
r.TemporalAA.Upsampling=True
r.Mobile.AntiAliasing=2
r.AntiAliasingMethod=2
r.TemporalAASamples=4
r.Mobile.SupportsGen4TAA=True
r.Vulkan.Depth24Bit=1
```

Enable SR with `r.ANF.Enabled=1`. To use FG, disable SR and other upscalers, then set `r.ANF.FrameGen.Enable=1`.

## Frame pacing

Choose one pacing path and measure its output on the target device. The rendered frame rate and displayed frame rate are different when FG is active.

### SwappyVK

For a light workload, enable SwappyVK. Set `t.MaxFPS` to the rendered frame rate and `r.SetFramePace` to the display rate. For 30 rendered frames and 60 displayed frames per second:

```text
t.MaxFPS 30
r.SetFramePace 60
```

### ANF CPU frame pacer

For a heavier workload, disable competing pacers and enable ANF's pacer:

```ini
[ConsoleVariables]
a.UseSwappyForFramePacing=0
r.Vulkan.ExtensionFramePacer=0
r.Vulkan.CPURenderthreadFramePacer=0
r.Vulkan.CPURHIThreadFramePacer=0
r.ANF.FrameGen.UseFramePacer=1
r.ANF.FrameGen.FramePacerTargetFPS=60
t.MaxFPS=60
```

Set `r.SetFramePace 60` for this 60 FPS display target. The ANF target controls presentation timing. Validate frame delivery and latency before selecting a shipping configuration.

## Validate the integration

Check SR and FG separately. Verify that unsupported devices retain a working fallback, UI composition remains correct, and mode changes do not leave stale history. Inspect GPU and frame timing on the target device.

For SR diagnosis, use the [debug overlay guide](ANF/Source/ANFSDK/Public/ANF_Debug_Overlay_User_Guide_External.md). The plugin exposes `r.ANF.DebugOverlay.*` controls. Disable the overlay for production.

## License

The plugin source and public SDK headers use [BSD 3-Clause](ANF/LICENSE-BSD-3-Clause.txt). The bundled `libanf.so` uses the [QTI No-Login Binary License](ANF/Source/ANFSDK/libs/LICENSE.txt). Include that license and required notices when redistributing the binary.
