# Qcom Shadow Denoiser UE Plugin
UE Plugin for reducing noise in ray-traced shadows on both desktop and mobile renderers, with optimizations for Qualcomm® Adreno™ GPUs.

## Build in UE5.5
*1) Push "QcomShadowDenoiser" into folder "Plugins" of UE5 engine source code(Engine\Plugins\Runtime\Qualcomm), or project plugin folder. <br/>
*2) Build the engine or project.<br/>


## Enable in UE5.5
In UE5 Editor:
```
In "Plugins" tab, enable "QcomShadowDenoiser"

In "Project Settings" tab, apply following configs
	"Engine - Rendering" -> "Direct Lighting" -> "Ray Traced Shadows": check