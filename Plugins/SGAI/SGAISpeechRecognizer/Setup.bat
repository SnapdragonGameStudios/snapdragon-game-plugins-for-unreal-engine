:: Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
:: SPDX-License-Identifier: BSD-3-Clause

@echo off
setlocal enabledelayedexpansion

:: ============================================================================
:: VoiceAI ASR SDK Setup Script
:: Locates and installs the VoiceAI ASR Community SDK for Unreal Engine plugin
::
:: Location: {repo-root}/SetupVoiceAI.bat
:: Target:   {repo-root}/plugin/SpeechRecognizer/Source/VoiceAI/ThirdParty/VoiceAIASRLib/
:: ============================================================================

set SDK_VERSION=2.3.0.0
set "SDK_DOWNLOAD_URL=https://qpm.qualcomm.com/#/main/tools/details/VoiceAI_ASR_Community"
set "SDK_FOLDER_PREFIX=VoiceAI_ASR_Community_v"

:: Get the directory where this script is located
set "SCRIPT_DIR=%~dp0"
set "THIRDPARTY_DIR=%SCRIPT_DIR%Source\VoiceAIASR\ThirdParty\VoiceAIASRLib"

:: User's Downloads folder
set "DOWNLOADS_DIR=%USERPROFILE%\Downloads"

:: Remove any trailing spaces or carriage returns from version
for /f "tokens=* delims= " %%a in ("%SDK_VERSION%") do set "SDK_VERSION=%%a"

:: ============================================================================
:: Parse command-line parameters
:: ============================================================================

if "%~1"=="" goto :no_params

set "USER_PARAM=%~1"

if /i "%USER_PARAM%"=="/h" goto :show_menu
if /i "%USER_PARAM%"=="/help" goto :show_menu
if /i "%USER_PARAM%"=="/?" goto :show_menu

:: Check if it's a zip file path
if /i "%USER_PARAM:~-4%"==".zip" (
    if not exist "%USER_PARAM%" (
        echo [ERROR] ZIP file not found: %USER_PARAM%
        pause
        exit /b 1
    )
    set "SETUP_MODE=2"
    set "USER_ZIP_PATH=%USER_PARAM%"
    goto :start_setup
)

:: Check if it's a directory path
if exist "%USER_PARAM%\*" (
    set "SETUP_MODE=3"
    set "USER_FOLDER_PATH=%USER_PARAM%"
    goto :start_setup
)

echo [ERROR] Invalid parameter: %~1
echo.
echo Usage:
echo   SetupVoiceAI.bat                            - Auto-detect from Downloads (default)
echo   SetupVoiceAI.bat /h                         - Show menu with all options
echo   SetupVoiceAI.bat "path\to\sdk.zip"          - Use existing ZIP
echo   SetupVoiceAI.bat "path\to\extracted\folder" - Use extracted folder
pause
exit /b 1

:no_params
set "SETUP_MODE=1"
goto :start_setup

:: ============================================================================
:: Interactive Menu (only via /h)
:: ============================================================================

:show_menu
cls
echo.
echo ============================================
echo    VoiceAI ASR SDK Setup Script
echo ============================================
echo.
echo Choose an option:
echo   1. Auto-detect SDK from Downloads folder (default)
echo   2. Use existing ZIP file
echo   3. Use already-extracted folder
echo.
set /p "SETUP_MODE=Enter your choice (1-3): "

if "%SETUP_MODE%"=="1" goto :start_setup
if "%SETUP_MODE%"=="2" goto :prompt_zip
if "%SETUP_MODE%"=="3" goto :prompt_folder

echo [ERROR] Invalid choice. Please enter 1, 2, or 3.
pause
goto :show_menu

:prompt_zip
set /p "USER_ZIP_PATH=Enter the full path to the ZIP file: "
set "USER_ZIP_PATH=%USER_ZIP_PATH:"=%"
if not exist "%USER_ZIP_PATH%" (
    echo [ERROR] ZIP file not found: %USER_ZIP_PATH%
    pause
    goto :show_menu
)
if /i not "%USER_ZIP_PATH:~-4%"==".zip" (
    echo [ERROR] File must have .zip extension
    pause
    goto :show_menu
)
goto :start_setup

:prompt_folder
set /p "USER_FOLDER_PATH=Enter the full path to the extracted folder: "
set "USER_FOLDER_PATH=%USER_FOLDER_PATH:"=%"
if not exist "%USER_FOLDER_PATH%\*" (
    echo [ERROR] Folder not found: %USER_FOLDER_PATH%
    pause
    goto :show_menu
)
goto :start_setup

:: ============================================================================
:: Common Setup
:: ============================================================================

:start_setup
echo.
echo ============================================
echo    VoiceAI ASR SDK Setup - v%SDK_VERSION%
echo ============================================
echo.
echo [INFO] Target: %THIRDPARTY_DIR%
echo.

:: Create target directory if missing
if not exist "%THIRDPARTY_DIR%" mkdir "%THIRDPARTY_DIR%" 2>nul
if not exist "%THIRDPARTY_DIR%" (
    echo [ERROR] Failed to create target directory.
    goto :error
)

:: Check if SDK already installed
set "SDK_EXISTS=0"
if exist "%THIRDPARTY_DIR%\inc" set "SDK_EXISTS=1"
if exist "%THIRDPARTY_DIR%\lib" set "SDK_EXISTS=1"
if exist "%THIRDPARTY_DIR%\assets" set "SDK_EXISTS=1"

if "%SDK_EXISTS%"=="1" (
    echo [WARNING] Existing SDK installation detected.
    echo.
    set /p OVERWRITE="Overwrite? (y/n): "
    if /i not "!OVERWRITE!"=="y" (
        echo [INFO] Setup cancelled.
        goto :end
    )
    echo [INFO] Removing existing installation...
    if exist "%THIRDPARTY_DIR%\inc" rmdir /s /q "%THIRDPARTY_DIR%\inc" 2>nul
    if exist "%THIRDPARTY_DIR%\lib" rmdir /s /q "%THIRDPARTY_DIR%\lib" 2>nul
    if exist "%THIRDPARTY_DIR%\assets" rmdir /s /q "%THIRDPARTY_DIR%\assets" 2>nul
    echo.
)

:: ============================================================================
:: Branch by mode
:: Modes 2 and 3 jump ahead; mode 1 falls through linearly.
:: ============================================================================

if "%SETUP_MODE%"=="2" goto :mode_zip
if "%SETUP_MODE%"=="3" goto :mode_folder

:: ============================================================================
:: MODE 1: Auto-detect SDK from Downloads folder (falls through to :copy_files)
:: ============================================================================

echo [INFO] Searching Downloads folder for SDK...

set "SDK_BASE_PATH="

:: Look for extracted folder first (exact recommended version)
if exist "%DOWNLOADS_DIR%\%SDK_FOLDER_PREFIX%%SDK_VERSION%\*" (
    set "SDK_BASE_PATH=%DOWNLOADS_DIR%\%SDK_FOLDER_PREFIX%%SDK_VERSION%"
    goto :mode1_found
)

:: Look for any extracted folder matching prefix
for /d %%D in ("%DOWNLOADS_DIR%\%SDK_FOLDER_PREFIX%*") do (
    set "SDK_BASE_PATH=%%D"
    goto :mode1_found
)

:: Look for zip file (exact recommended version)
if exist "%DOWNLOADS_DIR%\%SDK_FOLDER_PREFIX%%SDK_VERSION%.zip" (
    set "USER_ZIP_PATH=%DOWNLOADS_DIR%\%SDK_FOLDER_PREFIX%%SDK_VERSION%.zip"
    goto :mode1_found_zip
)

:: Look for any zip matching prefix
for %%F in ("%DOWNLOADS_DIR%\%SDK_FOLDER_PREFIX%*.zip") do (
    set "USER_ZIP_PATH=%%F"
    goto :mode1_found_zip
)

:: Not found
echo.
echo [ERROR] SDK not found in Downloads folder.
echo.
echo         Please download the VoiceAI ASR Community SDK from:
echo         %SDK_DOWNLOAD_URL%
echo.
echo         Then either:
echo           - Extract it to your Downloads folder, or
echo           - Run this script with the path to the ZIP or folder:
echo             SetupVoiceAI.bat "path\to\sdk.zip"
echo             SetupVoiceAI.bat "path\to\extracted\folder"
echo.
goto :error

:mode1_found_zip
echo [INFO] Found SDK ZIP: %USER_ZIP_PATH%
call :check_sdk_version "%USER_ZIP_PATH%"

:: Use a short temp path for extraction
set "TEMP_DIR=%TEMP%\VoiceAI_SDK"
set "TEMP_ZIP=%TEMP_DIR%\sdk.zip"

if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%" 2>nul
mkdir "%TEMP_DIR%"
if not exist "%TEMP_DIR%" (
    echo [ERROR] Failed to create temp directory.
    goto :error
)

echo [INFO] Preparing ZIP for extraction...
copy "%USER_ZIP_PATH%" "%TEMP_ZIP%" >nul 2>&1
if not exist "%TEMP_ZIP%" (
    echo [ERROR] Failed to stage ZIP file.
    goto :error
)

set "NEED_CLEANUP=1"
goto :do_extract

:mode1_found
echo [INFO] Found SDK folder: %SDK_BASE_PATH%
call :check_sdk_version "%SDK_BASE_PATH%"
call :locate_whisper_sdk "%SDK_BASE_PATH%"
if not defined WHISPER_SDK_PATH goto :error
goto :copy_files

:: ============================================================================
:: Extract ZIP (reached by mode 1 zip fallthrough, mode 2 goto)
:: ============================================================================

:do_extract
echo [INFO] Extracting SDK...

powershell -ExecutionPolicy Bypass -Command "Expand-Archive -Path '%TEMP_ZIP%' -DestinationPath '%TEMP_DIR%\ex' -Force"

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Extraction failed. ZIP may be corrupted.
    goto :error
)

if not exist "%TEMP_DIR%\ex" (
    echo [ERROR] Extraction produced no output.
    goto :error
)

echo [INFO] Extraction complete.

set "SDK_BASE_PATH=%TEMP_DIR%\ex"

:: Check if the zip extracted into a subfolder
for /d %%D in ("%TEMP_DIR%\ex\%SDK_FOLDER_PREFIX%*") do (
    set "SDK_BASE_PATH=%%D"
)

call :locate_whisper_sdk "%SDK_BASE_PATH%"
if not defined WHISPER_SDK_PATH goto :error
goto :copy_files

:: ============================================================================
:: MODE 2: Use Existing ZIP (jumps to :do_extract)
:: ============================================================================

:mode_zip
echo [INFO] Using provided ZIP: %USER_ZIP_PATH%
echo.

if not exist "%USER_ZIP_PATH%" (
    echo [ERROR] ZIP file not found.
    goto :error
)

for %%A in ("%USER_ZIP_PATH%") do set "FILE_SIZE=%%~zA"
if "%FILE_SIZE%"=="0" (
    echo [ERROR] ZIP file is empty.
    goto :error
)

call :check_sdk_version "%USER_ZIP_PATH%"

set "TEMP_DIR=%TEMP%\VoiceAI_SDK"
set "TEMP_ZIP=%TEMP_DIR%\sdk.zip"

if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%" 2>nul
mkdir "%TEMP_DIR%"
if not exist "%TEMP_DIR%" (
    echo [ERROR] Failed to create temp directory.
    goto :error
)

echo [INFO] Preparing ZIP for extraction...
copy "%USER_ZIP_PATH%" "%TEMP_ZIP%" >nul 2>&1
if not exist "%TEMP_ZIP%" (
    echo [ERROR] Failed to stage ZIP file.
    goto :error
)

set "NEED_CLEANUP=1"
goto :do_extract

:: ============================================================================
:: MODE 3: Use Already-Extracted Folder (jumps to :copy_files)
:: ============================================================================

:mode_folder
echo [INFO] Using provided folder: %USER_FOLDER_PATH%
echo.

if not exist "%USER_FOLDER_PATH%\*" (
    echo [ERROR] Folder not found.
    goto :error
)

call :check_sdk_version "%USER_FOLDER_PATH%"

set "SDK_BASE_PATH=%USER_FOLDER_PATH%"

call :locate_whisper_sdk "%SDK_BASE_PATH%"
if not defined WHISPER_SDK_PATH goto :error
goto :copy_files

:: ============================================================================
:: Copy Files to Target Directory
:: ============================================================================

:copy_files

if not defined WHISPER_SDK_PATH (
    echo [ERROR] Internal error: SDK path not resolved.
    goto :error
)

:: Check destination path length (Windows MAX_PATH = 260)
:: THIRDPARTY_DIR + longest subpath (~50 chars) must be under 260
set "PATH_CHECK=%THIRDPARTY_DIR%\lib\android\whisper_all_quantized\arm64-v8a\placeholder"
set "PATH_LEN=0"
for /l %%i in (0,1,259) do if not "!PATH_CHECK:~%%i,1!"=="" set /a PATH_LEN+=1
if %PATH_LEN% GEQ 250 (
    echo [ERROR] Destination path is too long (%PATH_LEN% chars^). Risk of MAX_PATH failures.
    echo         Move the project to a shorter path and try again.
    goto :error
)

echo.
echo [INFO] Installing SDK components...

:: ------------------------------------------------------------------
:: 1. Copy headers: whisper_sdk/include/npu/rpc/windows/*.h -> inc/
:: ------------------------------------------------------------------

set "SRC_INC=%WHISPER_SDK_PATH%\include\npu\rpc\windows"

if not exist "%SRC_INC%\*" (
    echo [ERROR] Headers not found in SDK.
    goto :error
)

if not exist "%THIRDPARTY_DIR%\inc" mkdir "%THIRDPARTY_DIR%\inc"
if not exist "%THIRDPARTY_DIR%\inc" (
    echo [ERROR] Failed to create headers directory.
    goto :error
)

xcopy "%SRC_INC%\*.h" "%THIRDPARTY_DIR%\inc\" /Y /Q >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Failed to install headers.
    goto :error
)

echo [INFO] Headers installed.

:: ------------------------------------------------------------------
:: 2. Copy libs: whisper_sdk/libs/npu/rpc_libraries/{windows,android} -> lib/
:: ------------------------------------------------------------------

set "SRC_LIBS=%WHISPER_SDK_PATH%\libs\npu\rpc_libraries"

if not exist "%SRC_LIBS%\*" (
    echo [ERROR] Libraries not found in SDK.
    goto :error
)

if not exist "%THIRDPARTY_DIR%\lib" mkdir "%THIRDPARTY_DIR%\lib"
if not exist "%THIRDPARTY_DIR%\lib" (
    echo [ERROR] Failed to create libraries directory.
    goto :error
)

set "LIB_COUNT=0"

:: Copy windows libraries (only whisper_all_quantized)
if exist "%SRC_LIBS%\windows\whisper_all_quantized\*" (
    if not exist "%THIRDPARTY_DIR%\lib\windows\whisper_all_quantized" mkdir "%THIRDPARTY_DIR%\lib\windows\whisper_all_quantized"
    xcopy "%SRC_LIBS%\windows\whisper_all_quantized\*" "%THIRDPARTY_DIR%\lib\windows\whisper_all_quantized\" /E /Y /Q >nul 2>&1
    if !ERRORLEVEL! equ 0 set /a LIB_COUNT+=1
) else if exist "%SRC_LIBS%\windows\*" (
    echo [WARNING] whisper_all_quantized not found under windows, copying all windows libraries.
    if not exist "%THIRDPARTY_DIR%\lib\windows" mkdir "%THIRDPARTY_DIR%\lib\windows"
    xcopy "%SRC_LIBS%\windows\*" "%THIRDPARTY_DIR%\lib\windows\" /E /Y /Q >nul 2>&1
    if !ERRORLEVEL! equ 0 set /a LIB_COUNT+=1
)

:: Copy android libraries (only whisper_all_quantized)
if exist "%SRC_LIBS%\android\whisper_all_quantized\*" (
    if not exist "%THIRDPARTY_DIR%\lib\android\whisper_all_quantized" mkdir "%THIRDPARTY_DIR%\lib\android\whisper_all_quantized"
    xcopy "%SRC_LIBS%\android\whisper_all_quantized\*" "%THIRDPARTY_DIR%\lib\android\whisper_all_quantized\" /E /Y /Q >nul 2>&1
    if !ERRORLEVEL! equ 0 set /a LIB_COUNT+=1
) else if exist "%SRC_LIBS%\android\*" (
    echo [WARNING] whisper_all_quantized not found under android, copying all android libraries.
    if not exist "%THIRDPARTY_DIR%\lib\android" mkdir "%THIRDPARTY_DIR%\lib\android"
    xcopy "%SRC_LIBS%\android\*" "%THIRDPARTY_DIR%\lib\android\" /E /Y /Q >nul 2>&1
    if !ERRORLEVEL! equ 0 set /a LIB_COUNT+=1
)

if "%LIB_COUNT%"=="0" (
    echo [ERROR] No platform libraries found in SDK.
    goto :error
)

echo [INFO] Platform libraries installed (%LIB_COUNT% platform(s)).

:: ------------------------------------------------------------------
:: 3. Copy assets: whisper_sdk/libs/npu/rpc_libraries/assets/ -> assets/
:: ------------------------------------------------------------------

if exist "%SRC_LIBS%\assets\*" (
    if not exist "%THIRDPARTY_DIR%\assets" mkdir "%THIRDPARTY_DIR%\assets"
    xcopy "%SRC_LIBS%\assets\*" "%THIRDPARTY_DIR%\assets\" /E /Y /Q >nul 2>&1
    if %ERRORLEVEL% equ 0 (
        echo [INFO] Assets installed.
    ) else (
        echo [WARNING] Some assets may not have copied correctly.
    )
) else (
    echo [WARNING] Assets folder not found in SDK. Skipping.
)

:: ============================================================================
:: Cleanup temp files
:: ============================================================================

if defined NEED_CLEANUP (
    echo [INFO] Cleaning up temporary files...
    if defined TEMP_DIR if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%" 2>nul
)

:: ============================================================================
:: Verify Installation
:: ============================================================================

echo.
echo [INFO] Verifying installation...

set "VERIFY_OK=1"

if not exist "%THIRDPARTY_DIR%\inc" (
    echo [FAIL] Headers directory missing.
    set "VERIFY_OK=0"
)
if not exist "%THIRDPARTY_DIR%\lib" (
    echo [FAIL] Libraries directory missing.
    set "VERIFY_OK=0"
)

set "INC_COUNT=0"
for %%F in ("%THIRDPARTY_DIR%\inc\*.h") do set /a INC_COUNT+=1
if "%INC_COUNT%"=="0" (
    echo [FAIL] No header files installed.
    set "VERIFY_OK=0"
)

set "LIB_DIR_COUNT=0"
for /d %%D in ("%THIRDPARTY_DIR%\lib\*") do set /a LIB_DIR_COUNT+=1
if "%LIB_DIR_COUNT%"=="0" (
    echo [FAIL] No platform libraries installed.
    set "VERIFY_OK=0"
)

if "%VERIFY_OK%"=="0" goto :error

echo.
echo ============================================
echo    Setup Complete
echo ============================================
echo.
echo    Location: %THIRDPARTY_DIR%
echo.
echo    Installed:
echo      - inc/ (%INC_COUNT% headers)
echo      - lib/
for /d %%D in ("%THIRDPARTY_DIR%\lib\*") do echo        - %%~nxD
if exist "%THIRDPARTY_DIR%\assets" echo      - assets/
echo.
goto :end

:: ############################################################################
:: SUBROUTINES
:: ############################################################################

:: ============================================================================
:: Subroutine: Locate whisper_sdk inside SDK base path
:: Sets WHISPER_SDK_PATH if found, clears it if not.
:: Expected structure: {base}/{version}/whisper_sdk/
:: ============================================================================

:locate_whisper_sdk
set "SEARCH_BASE=%~1"
set "WHISPER_SDK_PATH="

:: Direct: {base}/whisper_sdk/
if exist "%SEARCH_BASE%\whisper_sdk\include" if exist "%SEARCH_BASE%\whisper_sdk\libs" (
    set "WHISPER_SDK_PATH=%SEARCH_BASE%\whisper_sdk"
    goto :locate_done
)

:: One level deep: {base}/{version}/whisper_sdk/
for /d %%V in ("%SEARCH_BASE%\*") do (
    if exist "%%V\whisper_sdk\include" if exist "%%V\whisper_sdk\libs" (
        set "WHISPER_SDK_PATH=%%V\whisper_sdk"
        goto :locate_done
    )
)

:: Two levels deep: {base}/{prefix}/{version}/whisper_sdk/
for /d %%A in ("%SEARCH_BASE%\*") do (
    for /d %%V in ("%%A\*") do (
        if exist "%%V\whisper_sdk\include" if exist "%%V\whisper_sdk\libs" (
            set "WHISPER_SDK_PATH=%%V\whisper_sdk"
            goto :locate_done
        )
    )
)

:locate_done
if defined WHISPER_SDK_PATH (
    echo [INFO] SDK located successfully.
) else (
    echo [ERROR] Could not locate whisper_sdk directory with 'include' and 'libs'.
    echo         Verify the SDK folder structure is intact.
)

goto :eof

:: ============================================================================
:: Subroutine: Check SDK version from path and warn if not recommended
:: ============================================================================

:check_sdk_version
set "VPATH=%~nx1"
set "DETECTED_VER="

:: Strip known prefix to get version (e.g., VoiceAI_ASR_Community_v2.4.0.0 -> 2.4.0.0)
set "STRIPPED=!VPATH:%SDK_FOLDER_PREFIX%=!"

:: Also strip .zip extension if present
if /i "!STRIPPED:~-4!"==".zip" set "STRIPPED=!STRIPPED:~0,-4!"

:: If stripping changed the string, we got a version
if not "!STRIPPED!"=="!VPATH!" set "DETECTED_VER=!STRIPPED!"

if defined DETECTED_VER (
    if not "!DETECTED_VER!"=="%SDK_VERSION%" (
        echo.
        echo [WARNING] Detected SDK version: !DETECTED_VER!
        echo          Recommended version: %SDK_VERSION%
        echo.
        set /p "VER_OK=Continue with version !DETECTED_VER!? (y/n): "
        if /i not "!VER_OK!"=="y" (
            echo [INFO] Setup cancelled.
            goto :error
        )
        echo.
    )
)

goto :eof

:: ============================================================================
:: Error and End Handlers
:: ============================================================================

:error
echo.
echo ============================================
echo    Setup Failed
echo ============================================
echo.
if defined TEMP_DIR if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%" 2>nul
if exist "%THIRDPARTY_DIR%\inc" rmdir /s /q "%THIRDPARTY_DIR%\inc" 2>nul
if exist "%THIRDPARTY_DIR%\lib" rmdir /s /q "%THIRDPARTY_DIR%\lib" 2>nul
if exist "%THIRDPARTY_DIR%\assets" rmdir /s /q "%THIRDPARTY_DIR%\assets" 2>nul
pause
exit /b 1

:end
pause
exit /b 0