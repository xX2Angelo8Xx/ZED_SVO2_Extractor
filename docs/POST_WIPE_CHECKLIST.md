# Post‑wipe Development Environment Checklist (Windows)

Date: 2025-11-09

This is the minimal, ordered list to get back to building and running `ZED_SVO2_Extractor` after reinstalling Windows.

For detailed steps, see `docs/DEPENDENCY_SETUP.md`. This file focuses on order, what’s manual vs. automated, and quick verification.

---

## 1) NVIDIA GPU Driver (Manual)
- Download and install the latest driver for your GPU:
  - https://www.nvidia.com/Download/index.aspx
- Reboot after installation.

## 2) CUDA Toolkit (Semi‑auto)
- You can install via our bootstrap script (recommended) or the NVIDIA site.
- Website: https://developer.nvidia.com/cuda-downloads
- Recommended: CUDA 12.x (ensure compatibility with your chosen ZED SDK release).
- Verify after install:
  - `nvcc --version` shows a valid version.

## 3) Visual Studio 2022 Community (Auto)
- Required workload: Desktop development with C++.
- Our script installs VS + workload silently.
- Verify after install:
  - Open “Developer PowerShell for VS 2022”, run `cl` to see MSVC version.

## 4) CMake (Auto)
- Required for configuring and building the project.
- Verify: `cmake --version`.

## 5) Git (Auto)
- Verify: `git --version`.

## 6) ZED SDK (Manual)
- Install after CUDA is installed.
- Download: https://www.stereolabs.com/developers/release/
- Choose Windows 10/11 installer, install Tools & Samples.
- The Windows installer includes a compatible OpenCV. Separate OpenCV install is optional on Windows.
- Verify:
  - Environment variable `ZED_SDK_ROOT_DIR` exists (new shell):
    - PowerShell: `echo $env:ZED_SDK_ROOT_DIR`
  - Launch “ZED Explorer” from Start Menu to confirm the SDK works.

## 7) (Optional) OpenCV (Manual)
- Only needed if you prefer a standalone OpenCV instead of the one bundled with ZED SDK.
- Follow `docs/DEPENDENCY_SETUP.md` if you choose to install it; otherwise skip.

---

## Automation Script (Winget)

We provide a PowerShell script you can run in an elevated shell to automate the install of VS, CMake, Git, and CUDA:

Run as Administrator:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
./scripts/bootstrap-dev.ps1
```

Switches:
- `-SkipVS` to skip Visual Studio install
- `-SkipCUDA` to skip CUDA
- `-SkipCMake` to skip CMake
- `-SkipGit` to skip Git

Manual steps (driver + ZED SDK) will still be required.

---

## After Everything Is Installed

1) Reboot once after installing the ZED SDK.
2) Verify toolchain in a fresh PowerShell:

```powershell
cmake --version
git --version
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"; cl
nvcc --version
echo "ZED_SDK_ROOT_DIR=$env:ZED_SDK_ROOT_DIR"
```

3) Clone and build the project:

```powershell
cd "C:\Users\Angelo Amon\OneDrive\Privat\Turbulence Solutions\Aero Lock\ZED2i\Programming"
git clone https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor.git
cd ZED_SVO2_Extractor
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

4) Run the GUI (`build\bin\Release\gui_extractor.exe`) and verify depth preview works.

---

## Quick Reference: What You Need (Summary)

- NVIDIA GPU driver (manual)
- CUDA Toolkit 12.x (auto via script)
- Visual Studio 2022 + Desktop C++ workload (auto via script)
- CMake (auto via script)
- Git (auto via script)
- ZED SDK for Windows (manual)
- (Optional) OpenCV standalone (manual; not necessary if using ZED SDK’s OpenCV)

That’s it—after these are in place, the repo should configure and build as before.
