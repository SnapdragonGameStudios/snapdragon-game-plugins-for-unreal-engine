# SGSR UE Plugin
UE Plugin for Snapdragon Game Super Resolution, supporting UE 5.0-5.6

## Build SGSR in UE
This release contains 5 methods of SGSR:<br/>
- Spatial Upscaler
- Temporal Upscaler 
  - 2 pass no alpha
  - 2 pass fragment shader
  - 3 pass
  - 3 pass pixel lock
1) Push folder "SGSR" into folder "Plugins" of UE5 engine source code(Engine\Plugins\Runtime\Qualcomm), or project plugin folder.
2) Apply the patch under folder "Patches" to UE5 engine source code if needed. Patches are needed for:
   - UE5.3
   - UE5.4
   - UE5.5
3) Build the engine or project.


## Enable SGSR in UE
In UE5 Editor:
```
In "Plugins" tab, enable SGSR:
	"Installed" -> "Rendering" -> "SGSR": Enabled
In "Project Settings" tab, open "Plugins - Snapdragon Game Super Resolution",
    General Settings:
        r.SGSR.Enabled(refer to SGSRTUViewExtension.cpp): enabled by default.
```
By command line(on fly):
```
r.SGSR.Enabled 1
```
Enable SGSR Spatial Upscaling: <br/>
```
Editor:
In "Project Settings" tab, open "Plugins - Snapdragon Game Super Resolution",
    General Settings:
        Upscaling Method: "Spatial Upscaling".

Command line:
	r.SGSR.Method 0
```
Enable SGSR Temporal Upscaling: <br/>

```
Editor:
In "Project Settings" tab, apply following configs
	"Engine - Rendering" -> "Mobile" -> "Mobile Anti-Aliasing Method"-> "Mobile Anti-Aliasing Method(TAA)"
	"Engine - Rendering" -> "Mobile" -> "Supports desktop Gen4 TAA on mobile": enable
	"Engine - Rendering" -> "Default Settings" -> "Temporal Upsampling": enable
	"Engine - Rendering" -> "Default Settings" -> "Anti-Aliasing Method": TemporalAA

Command line:
	2 pass no alpha: 		r.SGSR.Method 1
	2 pass fragment shader: r.SGSR.Method 2
	3 pass: 				r.SGSR.Method 3
	3 pass pixel lock: 		r.SGSR.Method 4
```
In Engine\Config\BaseDeviceProfiles.ini:
	set CVars=r.MobileContentScaleFactor=0.0 to [Android_Mid DeviceProfile] and [Android_High DeviceProfile]
	

In SGSR Settings Panel:
```
In "Project Settings" tab, open "Plugins - Snapdragon Game Super Resolution",
	SU Setting:
		r.SGSR.Target(refer to SGSRSubpassScaler.cpp): Spatial Upscale target, each target is a different shader.
			Available:
				0 - Mobile
				1 - High Quality
				2 - VR
    TU Setting:
        r.SGSR.Quality(refer to SGSRTU.cpp): Temporal Upscale quality mode(TU only). Default is 1(Quality: 1.5x).
            Available:
                0 - Ultra Quality 		1.25x
                1 - Quality 			1.5x 
                2 - Balanced 			1.7x 
                3 - Performance 		2.0x
            For example: device resoltion is 2400x1080, r.SGSR.Quality=1, will upscale from 1600x720 to 2400x1080.
	
```

## Enable SGSR on mobile
Run project on mobile:
```
In "Project Settings" tab, apply following configs
	"Platforms - Android" -> "APK Packaging" -> "Package game data inside .apk": enable
	"Platforms - Android" -> "Build" -> "Support arm64": check
	"Platforms - Android" -> "Build" -> "Support OpenGL ES3.1": check
	"Platforms - Android" -> "Build" -> "Support Vulkan": check
	"Platforms - Android" -> "Build" -> "Advanced APK Packaging" -> "Extra Permissions": add two items:
		android.permission.READ_EXTERNAL_STORAGE
		android.permission.WRITE_EXTERNAL_STORAGE
```

Make sure SGSR is enabled. If not, use the following commandline to enable.
```
r.SGSR.Enabled=1       
```
Select desired upscaling method(switching bewteen SU and TU will automatically set corresponding AA methods, no need to set it mannually):
```
r.SGSR.Method=1
```
If TAA is not set correctly for TU, then use:
```
r.AntiAliasingMethod=2,r.Mobile.AntiAliasing=2
```
Push UECommandLine.txt to `/sdcard/Android/data/com.YourCompany.[PROJECT]/files/UnrealGame/[PROJECT]/` before app starts.

For instance, using TU 3pass upscale from 720p to 1080p:
```
r.MobileContentScaleFactor=0,r.SGSR.Enabled=1,r.SGSR.Method=3,r.SGSR.Quality=1
```

If storage permissions required, intall .apk through Install_[PROJECT]-arm64.bat or enter the following code:
```
adb shell pm grant com.YourCompany.[PROJECT] android.permission.READ_EXTERNAL_STORAGE
adb shell pm grant com.YourCompany.[PROJECT] android.permission.WRITE_EXTERNAL_STORAGE
```

## Build Android for UE(workable solution)
- Apply patch to engine source code (if needed)
- SDK: 
  - 5.0-5.2: Android SDK 32
  - 5.3-5.5: Android SDK 33
  - 5.6+: Android SDK 34
- Android SDK Command-line Tools: 8.0
- NDK:
  - 5.0: 21.4.7075529
  - 5.1: 25.2.9519653
  - 5.2-5.6: 25.1.8937393
- JRE:
  - 5.0-5.2: Java 1.8.0_242
  - 5.3-5.6: Java 17