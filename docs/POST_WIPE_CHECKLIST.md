# Post‑wipe Setup Checklist (Windows) — ZED_SVO2_Extractor

Date: 2025-11-15

Purpose: Nach einem kompletten C:\‑Wipe alle Tools in korrekter Reihenfolge installieren, damit das Projekt sofort wieder gebaut und gestartet werden kann.

Hinweis: Details siehe auch `docs/DEPENDENCY_SETUP.md`. Diese Checkliste ist kurz, in richtiger Reihenfolge, inkl. Verifikation.

---

## 1) NVIDIA GPU‑Treiber (manuell)
- Download: https://www.nvidia.com/Download/index.aspx
- Studio/Game‑Ready wählen, installieren, neu starten.

## 2) CUDA Toolkit (halb automatisiert)
- Download: https://developer.nvidia.com/cuda-downloads
- Version zur ZED SDK Version passend wählen (CUDA 12.x wird empfohlen).
- Verifizieren: `nvcc --version` in neuer PowerShell zeigt Version an.

## 3) Visual Studio 2022 Community ODER Build Tools (automatisiert)
- Benötigter Workload: Desktopentwicklung mit C++.
- Verifizieren: In „Developer PowerShell for VS 2022“ `cl` ausführen (MSVC Version sichtbar).

## 4) CMake ≥ 3.15 (automatisiert)
- Verifizieren: `cmake --version`.

## 5) Git (automatisiert)
- Verifizieren: `git --version`.

## 6) Stereolabs ZED SDK (manuell)
- Download: https://www.stereolabs.com/developers/release/
- Windows 10/11 Installer, Tools & Samples mitinstallieren.
- Kompatibel zur installierten CUDA Version wählen (Release Notes beachten).
- Verifizieren:
  - Neue PowerShell: `echo $env:ZED_SDK_ROOT_DIR` zeigt Installationspfad (typisch: `C:\Program Files (x86)\ZED SDK`).
  - „ZED Explorer“ aus dem Startmenü starten.

## 7) OpenCV 4.10.0+ extern (erforderlich)
- Dieses Projekt nutzt eine externe OpenCV 4.10+ Installation (nicht das alte, mit ZED gebündelte 3.1.0).
- Download (Windows vorgebaute Binaries oder selbst bauen): https://opencv.org/releases/
- Empfohlene Struktur (für Auto‑Erkennung im CMake):
  - `C:\opencv\build\x64\vc16\bin` enthält DLLs (z. B. `opencv_world4100.dll`, `opencv_videoio_ffmpeg4100_64.dll`).
  - `C:\opencv\build` wird als `OpenCV_DIR` erkannt.
- Falls nicht unter `C:\opencv\build` installiert: Beim Konfigurieren setzen:
  - `cmake .. -DOpenCV_DIR="C:/Pfad/zu/opencv/build"`
- Laufzeit: Stelle sicher, dass die OpenCV DLLs neben den Executables liegen (`build\bin\Release\`).

---

## Ein‑Klick‑Automatisierung (winget‑Script)

Wir liefern ein PowerShell‑Script für VS, CMake, Git und CUDA. OpenCV und ZED SDK bleiben manuelle Schritte.

Als Administrator ausführen:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
./scripts/bootstrap-dev.ps1
```

Schalter:
- `-SkipVS`, `-SkipCUDA`, `-SkipCMake`, `-SkipGit`

Danach: NVIDIA Treiber, ZED SDK und OpenCV 4.10.0+ manuell installieren.

---

## Verifikation nach Installation

Neue PowerShell öffnen und prüfen:

```powershell
cmake --version
git --version
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"; cl
nvcc --version
echo "ZED_SDK_ROOT_DIR=$env:ZED_SDK_ROOT_DIR"
```

OpenCV‑Prüfung (Dateien vorhanden):
- `C:\opencv\build\x64\vc16\bin\opencv_world4100.dll`
- `C:\opencv\build\x64\vc16\bin\opencv_videoio_ffmpeg4100_64.dll`

---

## Projekt klonen und bauen

```powershell
cd "C:\Users\Angelo Amon\OneDrive\Privat\Turbulence Solutions\Aero Lock\ZED2i\Programming"
git clone https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor.git
cd ZED_SVO2_Extractor
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022" -A x64
# Falls OpenCV nicht unter C:\opencv\build liegt:
# cmake .. -G "Visual Studio 17 2022" -A x64 -DOpenCV_DIR="C:/Pfad/zu/opencv/build"
cmake --build . --config Release
```

Executables: `build\bin\Release\`
- `gui_extractor.exe`
- `frame_extractor_cli.exe`
- `video_extractor_cli.exe`

Beim ersten Start sicherstellen, dass die OpenCV‑DLLs im selben Ordner liegen (s. oben).

---

## Häufige Stolpersteine (Troubleshooting)

- „CMake cannot find OpenCV“: `OpenCV_DIR` korrekt setzen oder nach `C:\opencv\build` installieren.
- „opencv_world*.dll fehlt“: DLLs aus `C:\opencv\build\x64\vc16\bin` neben die `.exe` kopieren.
- „ZED SDK not found“: `ZED_SDK_ROOT_DIR` prüfen, ggf. PC neu starten, Installer erneut ausführen.
- „Video sehr klein/leer“: Sicherstellen, dass OpenCV ≥ 4.10.0 installiert ist (FFmpeg‑Videoio in den DLLs enthalten).
- „GUI startet nicht“: GPU‑Treiber aktualisieren, OpenGL ≥ 3.0 erforderlich (GLFW/ImGui).

---

## Zusammenfassung (Minimal‑Set)
- NVIDIA GPU‑Treiber
- CUDA Toolkit (passend zur ZED SDK)
- Visual Studio 2022 (C++ Desktop workload)
- CMake
- Git
- ZED SDK (Windows)
- OpenCV 4.10.0+ extern unter `C:\opencv\build` (oder `OpenCV_DIR` setzen)

Mit diesen Komponenten ist das Projekt nach einem System‑Wipe sofort wieder baubar und lauffähig.
