# Contributing to Snapdragon™ Game Plugins for Unreal Engine

Read the [code of conduct](CODE-OF-CONDUCT.md) and [license](LICENSE) before contributing.

## Report a problem

Include the plugin, branch, Unreal Engine version, platform, device, SDK versions, reproduction steps, expected behavior, and observed behavior. Remove private information from logs and screenshots.

## Propose a change

1. Start from the branch containing the affected plugin and engine integration. Use `main` for repository-wide catalog changes.
2. Keep the change focused. Describe compatibility effects and required engine patches.
3. Build the affected plugin with the matching engine version. Test the target device when changing rendering or runtime behavior.
4. Update installation instructions, settings, and examples when they change.
5. Record commands, results, and untested configurations in the pull request.

The root catalog README is shared by `main` and the `engine/*` branches. Keep catalog updates consistent across them. Specialized branches such as `ANF_UE5` and `SGSR_UE5` have separate installation guides.

## Submit a pull request

Use a [Developer Certificate of Origin](https://developercertificate.org/) sign-off on every commit, for example with `git commit --signoff`. Target the affected branch and explain the problem, resulting behavior, and validation.

Preserve copyright, license, and dependency notices. Do not include engine source, generated build output, SDK binaries, or models without the required redistribution rights.

## Documentation

Use sentence-case headings, direct instructions, and short paragraphs. Format paths, API names, console variables, and commands with backticks. Keep examples consistent with the selected engine branch and link to detailed guides instead of duplicating them.
