# Qualcomm AI Runtime plugin

This plugin packages Qualcomm AI Runtime binaries for Unreal plugins that use the Qualcomm AI Engine Direct SDK or Genie. Qualcomm® AI accelerators include the Qualcomm® Kryo™ CPU, Qualcomm® Adreno™ GPU, and Qualcomm® Hexagon™ NPU. Accelerator support depends on the selected runtime, model, and target hardware.

## Set up

1. Copy `qairt/` into the project's `Plugins/` directory or the engine's plugin directory.
2. From the plugin directory, run `QAIRTSetup.bat`. The script downloads and extracts the SDK version named by `SDK_VERSION` in that script.
3. To use a downloaded SDK archive, run `QAIRTSetup.bat "path-to-sdk.zip"`. Run `QAIRTSetup.bat /help` for available options.
4. Regenerate project files and build the project. Enable this plugin and the SGAI plugins that depend on it.

The setup places SDK files under `Source/ThirdParty/qairt/`. Verify the binaries for the target platform before packaging, and test the packaged application on the target hardware.

## License

The setup and integration source use the repository [BSD 3-Clause license](../../../LICENSE). The downloaded Qualcomm AI Runtime SDK has its own terms and notices; preserve them when distributing its binaries.
