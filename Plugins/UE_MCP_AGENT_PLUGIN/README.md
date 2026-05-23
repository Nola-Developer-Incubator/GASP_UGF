UE MCP Plugin (MCPServerPlugin)

Overview
- This plugin provides an enterprise‑grade MCP Server + Connector for Unreal Engine 5.7.1.
- This folder is the primary maintained plugin source in the repository.
- It initializes a server, connector client, token manager and a simple tool registry.
- It also ships an in‑editor dashboard, telemetry helpers, and onboarding tools to
  ease new‑developer adoption.

Quick start
1. Enable the plugin (`Edit > Plugins > MCP Agent`).
2. Open the dashboard (`Window > Developer Tools > MCP Agent Dashboard`).
3. Configure `BaseUrl`/`ApiKey` via `Project Settings > MCP Agent` or edit
   `DefaultEngine.ini`.
4. Use the dashboard buttons or run `tool.perform` commands to exercise tools.
5. Run `tool.perform name=onboard.createExample` to generate a sample blueprint.
Files of interest
- Source: `Plugins/UE_MCP_AGENT_PLUGIN/Source/` (module sources and Build.cs files)
- Resources: `Plugins/UE_MCP_AGENT_PLUGIN/Resources/` (icons and UI assets)
- Repository status: `docs/REPOSITORY_STATUS.md`

What I changed
- Added a polished 128x128 plugin icon to silence startup warnings and provide a thumbnail for the plugin browser:
  - `Plugins/UE_MCP_AGENT_PLUGIN/Resources/Icon128.png` (128x128 PNG, gradient background with white "UE MCP AGENT" text)
  - SHA256: 

How to build and run (Windows PowerShell)
- Launch the Editor for this project (adjust UE path if needed):
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Unreal_Projects\Azure_UE_MCP_AGENT\Azure_UE_MCP_AGENT.uproject"
```

- Full rebuild (requires Visual Studio + UE toolchain):
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" Azure_UE_MCP_AGENTEditor Win64 Development "C:\Unreal_Projects\Azure_UE_MCP_AGENT\Azure_UE_MCP_AGENT.uproject" -waitmutex
```

- Generate project files (if you need a Visual Studio solution):
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="C:\Unreal_Projects\Azure_UE_MCP_AGENT\Azure_UE_MCP_AGENT.uproject" -game -engine
```

Notes & troubleshooting
- If the editor logs show missing profiler DLLs (aqProf, Vtune, WinPix), those are optional profiling libs and can be ignored unless you need that profiler.
- If you see PSO creation hitches (LogPSOHitching) and runtime stutter, consider enabling PSO precaching and shader warm-up in project settings (`r.PSOPrecache.*`).
- Replace the placeholder `Icon128.png` with a proper 128x128 PNG if you want a production icon.

How to replace the plugin icon
- A helper PowerShell script is included at `scripts\generate_icon.ps1` that will decode a bundled base64 PNG into `Plugins\UE_MCP_AGENT_PLUGIN\Resources\Icon128.png` (128x128 colored placeholder). To run it locally:
```powershell
# Run from the repository root in PowerShell
Set-Location -Path "C:\Unreal_Projects\Azure_UE_MCP_AGENT"
.\scripts\generate_icon.ps1
# Verify file exists
Test-Path "Plugins\UE_MCP_AGENT_PLUGIN\Resources\Icon128.png"
```
- You can replace the generated icon with your own 128x128 PNG by copying it to `Plugins\UE_MCP_AGENT_PLUGIN\Resources\Icon128.png`.

How to verify locally
- Confirm the icon exists and check its fingerprint:
```powershell
Test-Path "Plugins\UE_MCP_AGENT_PLUGIN\Resources\Icon128.png"
Get-FileHash "Plugins\UE_MCP_AGENT_PLUGIN\Resources\Icon128.png" -Algorithm SHA256
```

CI (GitHub Actions - lightweight)
- A minimal workflow was added at `.github/workflows/ci.yml` which:
  - Runs on push and pull_request
  - Checks out the repo on a Windows runner
  - Runs `scripts\generate_icon.ps1` to ensure the plugin icon exists
  - Uploads `Intermediate/logs_report.json` (if present) as an artifact for quick log inspection

Contact
- If you want, I can customize the CI to run actual Unreal builds on a self-hosted runner with UE installed, or add more plugin docs and developer notes.
