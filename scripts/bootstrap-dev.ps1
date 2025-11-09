<#
Bootstrap Windows dev environment for ZED_SVO2_Extractor

What it does (automated):
- Installs Visual Studio 2022 Community with Native Desktop workload
- Installs CMake and Git
- Installs NVIDIA CUDA Toolkit (Toolkit only; keep your GPU driver up to date)

What you still need to do manually:
- Install latest NVIDIA GPU driver (Game Ready/Studio)
- Install Stereolabs ZED SDK for Windows (after CUDA)
- Optionally install OpenCV separately (ZED SDK includes a compatible OpenCV on Windows)

Run:
  Right-click PowerShell → Run as Administrator, then:
  Set-ExecutionPolicy Bypass -Scope Process -Force; .\scripts\bootstrap-dev.ps1
#>

param(
  [switch]$SkipVS,
  [switch]$SkipCUDA,
  [switch]$SkipCMake,
  [switch]$SkipGit
)

function Assert-Admin {
  $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
  $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
  if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Error "Please run this script in an elevated PowerShell (Run as Administrator)."
    exit 1
  }
}

function Install-PackageWinget {
  param(
    [Parameter(Mandatory=$true)][string]$Id,
    [string]$Name,
    [string]$OverrideArgs,
    [switch]$Exact
  )
  $exactFlag = $Exact.IsPresent ? "-e" : ""
  $ovr = [string]::IsNullOrEmpty($OverrideArgs) ? "" : " --override `"$OverrideArgs`""
  $cmd = "winget install --id $Id $exactFlag --source winget --accept-package-agreements --accept-source-agreements --silent$ovr"
  Write-Host "Installing $Id via winget..." -ForegroundColor Cyan
  Write-Host "> $cmd" -ForegroundColor DarkGray
  $proc = Start-Process -FilePath powershell -ArgumentList "-NoProfile","-Command", $cmd -Wait -PassThru
  if ($proc.ExitCode -ne 0) {
    Write-Warning "winget install for $Id returned exit code $($proc.ExitCode). You may need to retry manually."
  }
}

function Test-Command($cmd) { return (Get-Command $cmd -ErrorAction SilentlyContinue) -ne $null }

Assert-Admin

Write-Host "=== Bootstrap: Developer tools ===" -ForegroundColor Green

if (-not $SkipVS) {
  if (-not (Test-Path "$env:ProgramFiles(x86)\Microsoft Visual Studio\2022\Community")) {
    # Install VS 2022 Community with Native Desktop workload
    Install-PackageWinget -Id "Microsoft.VisualStudio.2022.Community" -Exact `
      -OverrideArgs "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended"
  } else {
    Write-Host "Visual Studio 2022 Community already present; skipping." -ForegroundColor Yellow
  }
}

if (-not $SkipCMake) {
  if (-not (Test-Command cmake)) {
    Install-PackageWinget -Id "Kitware.CMake" -Exact
  } else { Write-Host "CMake already present; skipping." -ForegroundColor Yellow }
}

if (-not $SkipGit) {
  if (-not (Test-Command git)) {
    Install-PackageWinget -Id "Git.Git" -Exact
  } else { Write-Host "Git already present; skipping." -ForegroundColor Yellow }
}

Write-Host "=== Bootstrap: NVIDIA CUDA Toolkit ===" -ForegroundColor Green
if (-not $SkipCUDA) {
  if (-not (Test-Command nvcc)) {
    # Note: This installs the CUDA Toolkit. It does not necessarily update the display driver.
    Install-PackageWinget -Id "NVIDIA.CUDA" -Exact
  } else { Write-Host "CUDA Toolkit already present (nvcc found); skipping." -ForegroundColor Yellow }
}

Write-Host "=== Manual Steps Remaining ===" -ForegroundColor Green
Write-Host "1) Install/Update NVIDIA Display Driver: https://www.nvidia.com/Download/index.aspx" -ForegroundColor Cyan
Write-Host "2) Install Stereolabs ZED SDK for Windows: https://www.stereolabs.com/developers/release/ (reboot after)" -ForegroundColor Cyan
Write-Host "   - Ensure CUDA Toolkit is already installed before ZED SDK" -ForegroundColor DarkGray
Write-Host "3) (Optional) Install OpenCV separately, or rely on OpenCV bundled with ZED SDK on Windows" -ForegroundColor Cyan

Write-Host "=== Verification ===" -ForegroundColor Green
Write-Host "After completing manual steps and rebooting, run these in a new PowerShell:" -ForegroundColor Gray
Write-Host "  cmake --version" -ForegroundColor Gray
Write-Host "  git --version" -ForegroundColor Gray
Write-Host "  & 'C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat'; cl" -ForegroundColor Gray
Write-Host "  nvcc --version" -ForegroundColor Gray
Write-Host "  echo \"ZED_SDK_ROOT_DIR=$env:ZED_SDK_ROOT_DIR\"" -ForegroundColor Gray

Write-Host "=== Done. Reboot recommended. ===" -ForegroundColor Green
