# Adrenoâ„¢ Neural Fusion (ANF) SDK integration guide

This guide covers ANF integration into an Android Vulkan renderer.

The public SDK supports two temporal techniques:

- Super Resolution (SR), which reconstructs a higher-resolution scene image from a lower-resolution jittered render.
- Frame Generation (FG), which produces an interpolated scene frame between rendered frames.

For Unreal installation, engine patches, and plugin-specific restrictions, see the [plugin guide](../../../../README.md).

## Contents

- [Rendering contract](#rendering-contract)
- [Android project configuration](#android-project-configuration)
- [Native API integration](#native-api-integration)
- [Technique creation](#technique-creation)
- [Technique dispatch](#technique-dispatch)
- [Temporal state and lifecycle](#temporal-state-and-lifecycle)
- [Input validation](#input-validation)
- [Debug visualization overlay](#debug-visualization-overlay)
- [Troubleshooting](#troubleshooting)
- [Release checklist](#release-checklist)
- [Related resources](#related-resources)

## Rendering contract

ANF is a temporal system. It combines the current frame's scene data with internal history. Resource contents and frame-to-frame continuity matter as much as Vulkan formats and usage flags.

### Super Resolution resource flow

Render the 3D scene at the SR input resolution with projection jitter. Dispatch SR after the scene inputs are ready. Compose UI after SR, and schedule other output-resolution post-processing according to the renderer's pipeline.

| Resource | Resolution | Jitter state | Purpose |
|---|---:|---|---|
| Input color | SR input size | Jittered with the scene | Current low-resolution scene color |
| Depth | SR input size | Jittered with the scene | Current scene depth |
| Motion vectors | SR input size | Exclude projection jitter | Pixel correspondence between frames, including camera and object motion |
| Jitter offset | Subpixel value | Current frame | Offset applied to the projection matrix |
| Output color | SR output size | Not applicable | Reconstructed high-resolution scene color |

`AnfSRCreateInfo` declares separate input and output dimensions. The application must use those same dimensions when it creates and describes the dispatch resources.

Exclude UI, text, reticles, and other screen-space overlays from the low-resolution temporal input. Composite them after SR so they remain sharp and do not enter temporal history.

### Frame Generation resource flow

FG accepts scene data for a rendered frame and produces an interpolated scene frame. `AnfFGCreateInfo` declares one size that applies to both input and output resources.

| Resource | Resolution | Jitter state | Purpose |
|---|---:|---|---|
| Input color | Declared FG size | Not jittered | Current rendered scene color |
| Depth | Declared FG size | Not jittered | Current scene depth |
| Motion vectors | Declared FG size | Not jittered | Pixel correspondence between frames, including camera and object motion |
| Output color | Declared FG size | Not applicable | Interpolated scene frame |

Treat the FG output as scene color. Render or composite UI separately for displayed rendered frames and generated frames. This avoids interpolating text, menus, and other interface elements.

When SR output feeds FG input, FG depth and motion-vector resources must also use the dimensions declared in `AnfFGCreateInfo`. Lower-resolution SR inputs cannot yet be passed directly when those dimensions differ. Prepare matching FG resources before dispatch.

### Runtime requirements

ANF is resolution agnostic. It does not add a fixed list of supported resolutions or additional minimum, maximum, or alignment restrictions on input and output dimensions. The application selects the dimensions when creating a technique, subject to the Vulkan device's image limits and available memory.

For SR, the input and output dimensions must follow the ratio defined by the selected quality mode. `ANF_SR_QUALITY_MODE_PERFORMANCE` uses a 2.0x ratio in each dimension. For FG, all input and output resources must use the dimensions declared in `AnfFGCreateInfo`.

Technique support, Vulkan extensions, queue capabilities, resource formats, usage flags, and external-memory requirements can vary by device and technique. Query them at runtime and build resources from the returned data.

The public resource labels are:

- `ANF_RESOURCE_LABEL_INPUT_COLOR`
- `ANF_RESOURCE_LABEL_DEPTH`
- `ANF_RESOURCE_LABEL_MOTION_VECTORS`
- `ANF_RESOURCE_LABEL_OUTPUT_COLOR`

Do not assume that every technique uses every label or that one format works across all devices. `AnfTechniqueRequirements` identifies required and optional labels. `AnfResourceRequirements` defines the allowed resource type and formats for each label.

## Android project configuration

`libanf.so` is an Android ARM64 shared library. Package it for the `arm64-v8a` ABI and compile the native application for the same ABI.

The examples below use a self-contained `anf` directory under the application's native source tree. This is a common way to keep a prebuilt library and its headers together, but ANF does not require these directory names. Other layouts work when the CMake paths resolve correctly and the packaged application contains the library for the target ABI.

```text
app/
  src/
    main/
      cpp/
        CMakeLists.txt
        anf/
          include/
            ... ANF public headers ...
          lib/
            arm64-v8a/
              libanf.so
```

The imported CMake target links and packages the shared library. Confirm that the final APK or App Bundle contains `lib/arm64-v8a/libanf.so`.

The supplied library uses 16 KB-aligned load segments. All other native libraries in the application must also meet the target device's page-size requirements.

### CMake configuration

Declare an imported shared-library target and link it to the application's native target:

```cmake
if(NOT ANDROID_ABI STREQUAL "arm64-v8a")
    message(FATAL_ERROR "ANF requires the arm64-v8a ABI")
endif()

add_library(anf SHARED IMPORTED GLOBAL)

set_target_properties(anf PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_LIST_DIR}/anf/lib/${ANDROID_ABI}/libanf.so"
    INTERFACE_INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_LIST_DIR}/anf/include"
)

target_link_libraries(your_native_target PRIVATE anf)
```

Include the common API, Vulkan types, and the headers for the techniques used by the application:

```cpp
#include <anf.h>
#include <anf_types_vk.h>
#include <anf_sr.h>  // When using Super Resolution
#include <anf_fg.h>  // When using Frame Generation
```

## Native API integration

Zero-initialize every ANF structure and set its matching `header.type`. Use `header.pNext` only for extension structures defined for that call.

### 1. Function table

`GetAnfFunctions` populates the public function table. Use the returned pointers rather than calling the individual API declarations directly.

```cpp
AnfFunctions anf = {};
GetAnfFunctions(&anf);
```

The application must validate every required function pointer before use.

`QueryAnfVersion` is optional and is intended only for diagnostics.

### 2. Instance creation

The public client API is Vulkan. Enable logging during bring-up so invalid parameters and unsupported paths are visible in Logcat.

```cpp
static void AnfLogCallback(AnfLogLevel level, const char* message)
{
    // Route the message to the engine logger or Android log.
}

AnfInstanceCreateInfo instanceInfo = {};
instanceInfo.header.type           = ANF_STYPE_INSTANCE_CREATE_INFO;
instanceInfo.clientApi             = ANF_CLIENT_API_VULKAN;
instanceInfo.logLevel              = ANF_LOG_LEVEL_WARNING;
instanceInfo.logMessageCallback    = AnfLogCallback;

AnfInstance anfInstance = {};
AnfResult result = anf.CreateInstance(&instanceInfo, &anfInstance);
if (result != ANF_RESULT_SUCCESS)
{
    // Stop ANF initialization and use the application's fallback path.
}
```

For a shipping build, select the log level that matches the application's logging policy. `ANF_LOG_LEVEL_NONE` disables ANF messages.

### 3. Technique and Vulkan requirements

Select `ANF_TECHNIQUE_ID_SR_TEMPORAL` or `ANF_TECHNIQUE_ID_FG_TEMPORAL`. Call `QueryTechniqueRequirements` before creating the Vulkan instance and logical device. Query every technique that the application may enable during the same Vulkan-device lifetime. Enable the union of their required extensions and choose queue families that satisfy their queue flags.

```cpp
const AnfTechniqueRequirements* techniqueRequirements = nullptr;

result = anf.QueryTechniqueRequirements(
    anfInstance,
    techniqueId,
    &techniqueRequirements);

if ((result != ANF_RESULT_SUCCESS) ||
    (techniqueRequirements == nullptr) ||
    (techniqueRequirements->header.type != ANF_STYPE_TECHNIQUE_REQUIREMENTS))
{
    // Stop initialization before using techniqueRequirements.
    return false;
}

const AnfStructHeader* clientApiRequirements =
    techniqueRequirements->pClientApiRequirements;

if ((clientApiRequirements == nullptr) ||
    (clientApiRequirements->type !=
        ANF_STYPE_TECHNIQUE_VULKAN_API_REQUIREMENTS))
{
    // The returned client API requirements are not valid for Vulkan.
    return false;
}

const auto* vulkanRequirements =
    reinterpret_cast<const AnfTechniqueVulkanApiRequirements*>(
        clientApiRequirements);

// Enable every name in ppInstanceExtensions and ppDeviceExtensions.
// Select a queue family that supports all techniqueQueueFlags.
```

Confirm that every required instance extension is available. Confirm separately that the selected physical device supports every required device extension.

Inspect `pTechniqueGroupRequirements` when it is non-null and verify its `type` before casting it. For SR, `ANF_STYPE_SR_REQUIREMENTS` identifies an `AnfSRRequirements` structure containing the supported quality-mode bitfield.

### 4. Hardware support

After creating the Vulkan instance and device, pass their handles to `IsTechniqueSupported`.

```cpp
AnfAdapterInfoVulkan adapterInfo = {};
adapterInfo.header.type          = ANF_STYPE_ADAPTER_INFO_VULKAN;
adapterInfo.instance             = vkInstance;
adapterInfo.physicalDevice       = vkPhysicalDevice;
adapterInfo.device               = vkDevice;

result = anf.IsTechniqueSupported(
    anfInstance,
    techniqueId,
    &adapterInfo.header);
```

Only `ANF_RESULT_SUCCESS` confirms support. Handle `ANF_RESULT_NOT_SUPPORTED` by selecting the non-ANF rendering path. Treat every other result as an initialization error and stop ANF setup or use the fallback path. An extension-capable Vulkan device does not necessarily support every ANF technique.

### 5. Vulkan device and queue registration

Call `SetInstanceClientAPIAdapterInfo` before creating, dispatching, or destroying a technique. One ANF instance can bind to one Vulkan device.

Immediate dispatch also requires one or more queues. Each reported queue-capability bit must be supported by at least one supplied queue.

```cpp
AnfQueueInfoVulkan queueInfo = {};
queueInfo.header.type         = ANF_STYPE_QUEUE_INFO_VULKAN;
queueInfo.numQueues           = 1;
queueInfo.pQueues             = &graphicsQueue;
queueInfo.pQueueFamilyIndices = &graphicsQueueFamilyIndex;

queueInfo.header.pNext   = adapterInfo.header.pNext;
adapterInfo.header.pNext = &queueInfo.header;

result = anf.SetInstanceClientAPIAdapterInfo(
    anfInstance,
    &adapterInfo.header);

if (result != ANF_RESULT_SUCCESS)
{
    // Do not create the technique.
}
```

Keep the Vulkan instance, device, and supplied queues valid until all techniques are destroyed and the ANF instance no longer uses them.

### 6. Resource requirements

Walk the required labels returned by `AnfTechniqueRequirements`. Query optional labels only when the application plans to supply them.

```cpp
for (uint32_t index = 0;
     index < techniqueRequirements->numRequiredResources;
     ++index)
{
    const AnfResourceLabel label =
        techniqueRequirements->pRequiredResources[index];

    const AnfResourceRequirements* resourceRequirements = nullptr;
    result = anf.QueryTechniqueResourceRequirements(
        anfInstance,
        techniqueId,
        label,
        &resourceRequirements);

    if (result != ANF_RESULT_SUCCESS)
    {
        // Abort technique setup.
        return false;
    }

    if ((resourceRequirements == nullptr) ||
        (resourceRequirements->header.type != ANF_STYPE_RESOURCE_REQUIREMENTS))
    {
        // Stop before using the returned requirements.
        return false;
    }

    const AnfStructHeader* clientApiRequirements =
        resourceRequirements->pClientApiRequirements;

    if ((clientApiRequirements == nullptr) ||
        (clientApiRequirements->type !=
            ANF_STYPE_RESOURCE_VULKAN_API_REQUIREMENTS))
    {
        // The returned resource requirements are not valid for Vulkan.
        return false;
    }

    const auto* vulkanResourceRequirements =
        reinterpret_cast<const AnfResourceVulkanApiRequirements*>(
            clientApiRequirements);

}
```

For each resource:

- Use the returned `resourceType`.
- Select a format from `pSupportedFormats`.
- Include all returned Vulkan image or buffer usage flags.
- Honor `externalMemoryType` when it is nonzero.
- Use the dimensions declared by the technique create info.
- Keep the resource alive until every dispatch that references it has completed.

If one Vulkan image is used for more than one ANF technique or resource role, select a format supported by every applicable requirement and include the union of their required usage flags. If no common format exists, use separate images and copy or convert between them.

Do not infer an `AnfFormat` from a Vulkan format by numeric value. Use an explicit conversion table in the renderer.

## Technique creation

### Super Resolution

The example below creates SR for immediate dispatch.

```cpp
AnfTechniqueVulkanCreateInfo vulkanCreateInfo = {};
vulkanCreateInfo.header.type = ANF_STYPE_TECHNIQUE_VULKAN_CREATE_INFO;

AnfSRCreateInfo srCreateInfo = {};
srCreateInfo.header.type       = ANF_STYPE_SR_CREATE_INFO;
srCreateInfo.inputSize.width   = inputWidth;
srCreateInfo.inputSize.height  = inputHeight;
srCreateInfo.outputSize.width  = outputWidth;
srCreateInfo.outputSize.height = outputHeight;

AnfTechniqueCreateInfo createInfo = {};
createInfo.header.type               = ANF_STYPE_TECHNIQUE_CREATE_INFO;
createInfo.techniqueId               = ANF_TECHNIQUE_ID_SR_TEMPORAL;
createInfo.flags                     =
    ANF_TECHNIQUE_CREATE_FLAG_DISPATCH_IMMEDIATE;
createInfo.maxInFlight               = framesInFlight;
createInfo.pClientApiCreateInfo      = &vulkanCreateInfo.header;
createInfo.pTechniqueGroupCreateInfo = &srCreateInfo.header;

AnfTechnique srTechnique = {};
result = anf.CreateTechnique(anfInstance, &createInfo, &srTechnique);
```

Continue only when `CreateTechnique` returns `ANF_RESULT_SUCCESS`.

For recorded dispatch, omit `ANF_TECHNIQUE_CREATE_FLAG_DISPATCH_IMMEDIATE` and follow [Recorded mode](#recorded-mode).

#### SR dispatch jitter

`AnfSRDispatch::jitterOffset` is the subpixel offset applied to the current frame's projection matrix. Each component must remain in `[-0.5, 0.5]`.

```cpp
AnfSRDispatch srDispatch = {};
srDispatch.header.type    = ANF_STYPE_SR_DISPATCH;
srDispatch.jitterOffset   = currentJitter;
```

The motion-vector input should describe scene and camera motion without the projection-jitter offset. Use the debug overlay to confirm this convention during bring-up.

### Frame Generation

FG uses immediate dispatch. Its input and output resources have the same width and height.

```cpp
AnfTechniqueVulkanCreateInfo vulkanCreateInfo = {};
vulkanCreateInfo.header.type = ANF_STYPE_TECHNIQUE_VULKAN_CREATE_INFO;

AnfFGCreateInfo fgCreateInfo = {};
fgCreateInfo.header.type      = ANF_STYPE_FG_CREATE_INFO;
fgCreateInfo.inputSize.width  = frameWidth;
fgCreateInfo.inputSize.height = frameHeight;

AnfTechniqueCreateInfo createInfo = {};
createInfo.header.type               = ANF_STYPE_TECHNIQUE_CREATE_INFO;
createInfo.techniqueId               = ANF_TECHNIQUE_ID_FG_TEMPORAL;
createInfo.flags                     =
    ANF_TECHNIQUE_CREATE_FLAG_DISPATCH_IMMEDIATE;
createInfo.maxInFlight               = framesInFlight;
createInfo.pClientApiCreateInfo      = &vulkanCreateInfo.header;
createInfo.pTechniqueGroupCreateInfo = &fgCreateInfo.header;

AnfTechnique fgTechnique = {};
result = anf.CreateTechnique(anfInstance, &createInfo, &fgTechnique);
```

Continue only when `CreateTechnique` returns `ANF_RESULT_SUCCESS`.

## Technique dispatch

### Resource descriptors

Build one `AnfResourceParamDesc` for each resource supplied to the dispatch. Its label, type, format, dimensions, Vulkan image layout, and handle must describe the actual resource at the point of dispatch.

```cpp
AnfResourceParamDesc inputColor = {};
inputColor.header.type                       = ANF_STYPE_RESOURCE_PARAM_DESC;
inputColor.resourceLabel                     = ANF_RESOURCE_LABEL_INPUT_COLOR;
inputColor.resourceDesc.header.type          = ANF_STYPE_RESOURCE_DESC;
inputColor.resourceDesc.type                 = ANF_RESOURCE_TYPE_TEXTURE2D;
inputColor.resourceDesc.format               = inputColorAnfFormat;
inputColor.resourceDesc.layout               =
    static_cast<uint32_t>(inputColorLayout);
inputColor.resourceDesc.textureDims.width    = inputWidth;
inputColor.resourceDesc.textureDims.height   = inputHeight;
inputColor.resourceDesc.textureDims.numArrayLayers = 1;
inputColor.resourceDesc.textureDims.mipCount = 1;
inputColor.resourceDesc.resource             =
    reinterpret_cast<AnfClientResource>(inputColorImage);
```

Repeat this for depth, motion vectors, and output color as required by the selected technique. Provide every required resource label using the descriptors expected by that technique. List resources in `AnfTechniqueRequirements::pRequiredResources` order to simplify diagnostics.

The value in `resourceDesc.layout` is the image's current `VkImageLayout`. Before dispatch, transition each image to the layout passed in its descriptor and make earlier writes to the input resources visible to ANF. Synchronize ANF's output writes before later commands consume the output.

### Immediate mode

In immediate mode, ANF submits work to the queues supplied through `AnfQueueInfoVulkan`. Set `commandList` to `nullptr` and provide Vulkan semaphores through `AnfTechniqueDispatchImmediateInfo`.

```cpp
AnfClientSyncObject waitObjects[] = {
    reinterpret_cast<AnfClientSyncObject>(anfReadySemaphore)
};

AnfClientSyncObject signalObjects[] = {
    reinterpret_cast<AnfClientSyncObject>(anfDoneSemaphore)
};

AnfTechniqueDispatchImmediateInfo immediateInfo = {};
immediateInfo.header.type          =
    ANF_STYPE_TECHNIQUE_DISPATCH_IMMEDIATE_INFO;
immediateInfo.pWaitSyncObjects     = waitObjects;
immediateInfo.numWaitSyncObjects   = 1;
immediateInfo.pSignalSyncObjects   = signalObjects;
immediateInfo.numSignalSyncObjects = 1;

AnfTechniqueDispatchInfo dispatchInfo = {};
dispatchInfo.header.type                 = ANF_STYPE_TECHNIQUE_DISPATCH_INFO;
dispatchInfo.header.pNext                = &immediateInfo.header;
dispatchInfo.commandList                 = nullptr;
dispatchInfo.pResources                  = resources.data();
dispatchInfo.numResources                =
    static_cast<uint32_t>(resources.size());
dispatchInfo.reset                       =
    shouldResetHistory ? ANF_TRUE : ANF_FALSE;
dispatchInfo.pTechniqueGroupDispatchInfo = &srDispatch.header;

result = anf.DispatchTechnique(srTechnique, &dispatchInfo);
```

Continue only when `DispatchTechnique` returns `ANF_RESULT_SUCCESS`.

For FG, populate `dispatchInfo.pResources` and `dispatchInfo.numResources` with the FG resource descriptors, initialize the technique-specific dispatch structure, and use the FG technique handle:

```cpp
AnfFGDispatch fgDispatch = {};
fgDispatch.header.type   = ANF_STYPE_FG_DISPATCH;

dispatchInfo.pResources                  = fgResources.data();
dispatchInfo.numResources                =
    static_cast<uint32_t>(fgResources.size());
dispatchInfo.pTechniqueGroupDispatchInfo = &fgDispatch.header;
result = anf.DispatchTechnique(fgTechnique, &dispatchInfo);
```

Continue only when the FG dispatch returns `ANF_RESULT_SUCCESS`.

The wait semaphore must become signaled only after the input resources and their required transitions are complete. Wait on the signal semaphore before consuming the output. ANF semaphores do not replace image memory dependencies in the application's own submissions.

Prefer an ANF queue from the same queue family that owns the images. If ANF and the renderer use different queue families in immediate mode, create shared images with `VK_SHARING_MODE_CONCURRENT` for those families.

### Recorded mode

Recorded dispatch lets ANF add commands to an application-owned `VkCommandBuffer`.

- Create the technique without `ANF_TECHNIQUE_CREATE_FLAG_DISPATCH_IMMEDIATE`.
- Use a primary command buffer.
- Begin the command buffer before the ANF call.
- Record the ANF dispatch outside a render pass or dynamic-rendering scope.
- Allocate the command buffer from a queue family that satisfies `techniqueQueueFlags`.
- Set `commandList` to the `VkCommandBuffer` and leave `dispatchInfo.header.pNext` null.

Configure the recorded-mode dispatch as follows:

```cpp
dispatchInfo.header.pNext = nullptr;
dispatchInfo.commandList =
    reinterpret_cast<AnfClientCommandList>(commandBuffer);
```

After the call, treat command-buffer binding state as undefined. Rebind pipelines, descriptor sets, vertex and index buffers, dynamic state, and any other state required by later commands.

The application still owns command-buffer submission, inter-queue dependencies, image transitions, and resource lifetime.

### In-flight dispatch limit

`AnfTechniqueCreateInfo::maxInFlight` is the maximum number of dispatches that may still be executing for one technique instance.

Match it to the number of ANF submissions that can overlap in the application's N-buffering model, not automatically to the swapchain image count. A dispatch must finish before the application reuses or destroys its input, output, synchronization objects, or technique-specific data.

## Temporal state and lifecycle

### History reset

Set `AnfTechniqueDispatchInfo::reset` to `ANF_TRUE` when previous temporal data no longer describes the current frame. Common reset events include:

- Camera cuts or teleports
- Loading a new scene or level
- Large field-of-view or projection changes
- Resolution changes that require technique recreation
- Replacing or clearing temporal input resources
- Resuming after a long pause or skipped frame sequence
- Enabling ANF after it has not received continuous frames

Use `ANF_FALSE` during normal continuous rendering. Repeated resets prevent temporal accumulation and reduce quality.

### Technique recreation and teardown

The technique create info defines dimensions, dispatch mode, and `maxInFlight`. Destroy and recreate the technique when those values change, then query resource requirements again before creating replacement resources.

ANF receives Vulkan handles, not copies of image contents. Keep referenced images, memory, semaphores, command buffers, and Vulkan objects valid until the dispatch completes. Before shutdown or recreation, drain outstanding ANF work and destroy objects in dependency order:

```cpp
anf.DestroyTechnique(anfInstance, technique);
anf.DestroyInstance(anfInstance);
```

## Input validation

### Projection jitter

Apply jitter to the projection matrix in clip space. Keep each component in `[-0.5, 0.5]` pixels and use the same logical offset for rendering and `AnfSRDispatch::jitterOffset`. Matrix storage, clip-space conventions, viewport orientation, and Y inversion vary by renderer, so validate both axes with the debug overlay.

Use a low-discrepancy sequence such as Halton or Sobol to cover the pixel area without clustering.

### Motion vectors

Motion vectors must cover static geometry affected by camera movement as well as animated and moving objects. A reference conversion from current and previous clip positions to UV displacement is:

```glsl
vec2 CalculateMotionVector(vec4 currentClip, vec4 previousClip)
{
    vec2 currentNdc  = currentClip.xy / currentClip.w;
    vec2 previousNdc = previousClip.xy / previousClip.w;
    return 0.5 * (currentNdc - previousNdc);
}
```

The renderer may need to flip Y or reverse direction to match its texture coordinates and ANF input convention. Check the full frame with camera-only motion, object-only motion, and disocclusion. Remove projection jitter from the motion-vector signal used by SR.

### Depth

Supply depth from the same camera, projection, viewport, and scene render as the color and motion-vector inputs. Do not pass a stale depth buffer or one resolved from a different view.

Depth-format support is technique and device dependent. Query the depth resource requirements. Use `ANF_FORMAT_D24S8_UNORM` only when the returned format list contains it. ANF uses the depth aspect of a combined depth-stencil resource. Stencil data is not a technique input.

### Color pipeline and composition

The input color should contain the 3D scene at the stage expected by the application's ANF pipeline. Keep the color space, exposure state, alpha treatment, and format consistent across frames.

For SR, apply operations that require output-resolution detail after reconstruction. UI composition normally follows SR. If tone mapping, bloom, sharpening, film grain, or color grading runs after ANF, those passes also affect the debug overlay.

For FG, use matching scene-color processing for the rendered frames that provide temporal input. Composite current UI after generation.

## Debug visualization overlay

The debug overlay is an SR integration tool. It visualizes motion vectors, depth, jitter, resource validity, and other data used by ANF.

Overlay setup requires:

- Add `ANF_INSTANCE_CREATE_FLAG_ENABLE_DEBUG_OVERLAY` to the instance flags.
- After creating the instance with the overlay flag, query the SR resource requirements and create resources with the returned usage flags.
- Chain a zero-initialized `AnfDebugOverlayConfig` with `ANF_STYPE_DEBUG_OVERLAY_CONFIG` to `AnfSRDispatch::header.pNext`.
- Select one of the modes defined by `AnfDebugOverlayMode`.
- Send `ANF_DEBUG_OVERLAY_MODE_NONE` once to stop drawing.

Overlay settings persist. Chain a new configuration only when a value changes.

One debug overlay can be owned by one technique at a time. Destroy the existing technique before creating a replacement that uses the overlay.

The overlay is written into the SR output. Later post-processing can change its colors or hide parts of it. For mode descriptions, controls, validation steps, and artifact diagnosis, use the [ANF debug overlay guide](./ANF_Debug_Overlay_User_Guide_External.md).

## Troubleshooting

### Initialization and dispatch failures

Inspect the ANF log and verify:

- The selected device passed `IsTechniqueSupported`.
- Every required Vulkan extension was enabled at instance or device creation.
- `SetInstanceClientAPIAdapterInfo` received the live Vulkan handles.
- Immediate mode received queues whose families satisfy `techniqueQueueFlags`.
- Every structure has the correct `header.type`.
- The technique-specific create structure matches the selected technique ID.
- `maxInFlight` matches the application's buffering plan.
- Input and output dimensions are valid for the selected technique and device.
- Every required resource label is present.
- Resource formats, types, dimensions, and layouts match the queried requirements and technique create info.
- The dispatch mode matches the technique creation flags.
- `pTechniqueGroupDispatchInfo` points to the correct SR or FG structure.
- The number of unfinished dispatches does not exceed `maxInFlight`.

### OpenCL loading failures

Some Android system images expose OpenCL as a device-provided native library. If ANF reports an OpenCL loading error, declare the library as optional inside the application element:

```xml
<application>
    <uses-native-library
        android:name="libOpenCL.so"
        android:required="false" />
</application>
```

Keeping `android:required="false"` prevents the manifest declaration from filtering installation to devices that advertise that library. Runtime ANF support checks still decide whether the technique can run.

### Invalid or corrupted output

Check resource handles, layouts, barriers, and semaphore ordering. Verify that the output image has the returned usage flags and that the consumer waits for ANF completion.

For SR, confirm that input color and depth were rendered with the dispatched jitter. Confirm that motion vectors do not include jitter. For FG, confirm that all inputs and the output use the dimensions declared in `AnfFGCreateInfo`.

### Temporal artifacts

Inspect motion vectors, depth, jitter, and reset behavior. Common causes are missing motion on static geometry during camera movement, reversed motion-vector direction, Y-axis mismatch, jitter included in motion vectors, stale depth, or failure to reset after a camera cut.

Use the debug overlay to isolate the failing input before tuning unrelated post-processing.

### Tile-memory invalidation

During recorded dispatch, ANF may use [`VK_QCOM_tile_memory_heap`](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_QCOM_tile_memory_heap.html) for internal work. Treat application data in memory bound to that heap as undefined after the dispatch. Do not preserve application data in tile memory across recorded ANF execution.

## Release checklist

Before enabling ANF in a release build, confirm the following items.

- Package the matching public headers and `libanf.so` for `arm64-v8a`.
- Include the QTI No-Login Binary License from [`LICENSE.txt`](../libs/LICENSE.txt) with every redistribution of `libanf.so`.
- Query intended techniques before Vulkan instance and device creation.
- Enable all returned extensions and use queues with the required flags.
- Run `IsTechniqueSupported` and keep a non-ANF fallback path.
- Create every resource from its returned format, usage, and memory requirements.
- Set `maxInFlight` to the number of overlapping ANF submissions in the application's N-buffering model.
- Keep resources and synchronization objects alive until dispatch completion.
- For SR, jitter color and depth but exclude projection jitter from motion vectors.
- Reset history after discontinuities.
- Synchronize and transition every resource around ANF access.

## Related resources

- [Adrenoâ„¢ Neural Fusion SDK repository](https://github.com/SnapdragonGameStudios/adreno-neural-fusion)
- [Adrenoâ„¢ Neural Fusion debug overlay guide](./ANF_Debug_Overlay_User_Guide_External.md)
- [Adrenoâ„¢ Neural Fusion Vulkan sample](https://github.com/SnapdragonGameStudios/adreno-gpu-vulkan-code-sample-framework/tree/main/samples/anf)
- [Adrenoâ„¢ Neural Fusion plugin for Unreal Engine](https://github.com/SnapdragonGameStudios/snapdragon-game-plugins-for-unreal-engine#adreno-neural-fusion)
- [Adrenoâ„¢ Neural Fusion plugin for Unity](https://github.com/SnapdragonGameStudios/com.qualcomm.snapdragon.adreno.neural.fusion)
