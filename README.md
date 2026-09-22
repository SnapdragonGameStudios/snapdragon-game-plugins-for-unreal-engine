# Snapdragon Game Super Resolution Unreal Engine plugin

This plugin provides spatial and temporal upscaling for UE 5.0 through 5.8. Choose one method for the renderer:

| `r.SGSR.Method` | Method |
|---|---|
| `0` | Spatial upscaling |
| `1` | Temporal upscaling with two fragment passes |
| `2` | Temporal upscaling with three compute passes |

## Install

1. Copy `SGSR/` into the project's `Plugins/` directory or `Engine/Plugins/Runtime/Qualcomm/`.
2. Apply the matching patch under `SGSR/Patches/` when using UE 5.3, 5.4, or 5.5.
3. Regenerate project files and build the engine or project.
4. Enable SGSR in the Editor's Plugins window.

Use the Android SDK, NDK, and Java versions required by the selected engine branch. The plugin's previous build notes used these combinations; verify them against the engine installation:

| UE version | Android SDK | NDK | Java |
|---|---|---|---|
| 5.0 | 32 | 21.4.7075529 | 8 |
| 5.1 | 32 | 25.2.9519653 | 8 |
| 5.2 | 32 | 25.1.8937393 | 8 |
| 5.3 through 5.5 | 33 | 25.1.8937393 | 17 |
| 5.6 | 34 | 25.1.8937393 | 17 |
| 5.7 and 5.8 | 34 | 27.2.12479018 | 21 |

## Configure upscaling

Use the Snapdragon Game Super Resolution section in Project Settings, or add settings to `DefaultEngine.ini`:

```ini
[/Script/SGSRTUModule.GSRSettings]
r.SGSR.Enabled=1
r.SGSR.Method=2
r.SGSR.Quality=1
```

At runtime, use console syntax such as `r.SGSR.Method 2`. Method `2` selects the three-pass compute path; `3` is not a valid method.

Temporal upscaling needs TAA, temporal upsampling, and the mobile Gen4 TAA support setting. Check these values in Project Settings:

```ini
[/Script/Engine.RendererSettings]
r.AntiAliasingMethod=2
r.Mobile.AntiAliasing=2
r.TemporalAA.Upsampling=1
r.Mobile.SupportsGen4TAA=True
```

Set `r.MobileContentScaleFactor=0` in the relevant Android device profile when using the native display resolution as the output size. Verify the effective settings on the device; device profiles can override project defaults.

## Quality

| `r.SGSR.Quality` | Mode | Scale per dimension | Screen percentage |
|---|---|---|---|
| `0` | Ultra Quality | 1.25x | 80 |
| `1` | Quality | 1.5x | About 66.7 |
| `2` | Balanced | 1.7x | About 58.8 |
| `3` | Performance | 2x | 50 |
| `4` | Custom | Selected by screen percentage | `r.SGSR.CustomScreenPercentage` |

For custom quality, set `r.SGSR.CustomScreenPercentage` between 50 and 100. The default is 100. Quality mode `1` reconstructs a 2400 by 1080 output from a 1600 by 720 input.

## Settings reference

| Setting | Default | Use |
|---|---|---|
| `r.SGSR.Enabled` | `1` | Enable or disable SGSR |
| `r.SGSR.Method` | `0` | Select method 0, 1, or 2 |
| `r.SGSR.Quality` | `1` | Select quality 0 through 4 |
| `r.SGSR.HalfPrecision` | `1` | Use half precision when supported |
| `r.SGSR.Target` | `0` | Spatial shader target: 0 mobile, 1 high quality, 2 VR |
| `r.SGSR.5Sample` | `1` | Temporal filter: 0 uses nine samples, 1 uses five |
| `r.SGSR.LanczosOpt` | `0` | Two-pass temporal Lanczos option |
| `r.SGSR.ThinFeature` | `0` | Two-pass thin-feature option |
| `r.SGSR.DoSharpening` | `0` | Add sharpening to the three-pass path |
| `r.SGSR.Sharpness` | `1.12` | Three-pass sharpening strength, 0 through 1.3 |
| `r.SGSR.PixelLock` | `0` | Three-pass thin-feature option |

The half-precision path requires platform FP16 support. Check `bSupportsRealTypes` in the engine's Android platform configuration before enabling it.

## Android validation

Build for ARM64 and enable the rendering API used by the project. Package and install using the engine's generated deployment scripts. Enable only the storage permissions required by the application's deployment method.

Inspect the effective method, quality, input size, and output size on the device. Compare moving objects and camera motion when validating temporal inputs. Test the spatial path separately from both temporal variants.

## License

Applicable source uses the [BSD 3-Clause License](LICENSE). Preserve any additional notices in bundled shaders and dependencies. See the [standalone SGSR repository](https://github.com/SnapdragonGameStudios/snapdragon-gsr) for technique documentation.

AMD-derived temporal shaders also carry MIT terms. See the [third-party notices](SGSR/THIRD-PARTY-NOTICES.txt).
