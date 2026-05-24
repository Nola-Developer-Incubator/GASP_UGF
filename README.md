# GASP

**Version:** 2.0
**Engine:** Unreal Engine 5.7.4
**Workspace Root:** `C:\Unreal_Projects\GASP`
**Target IDE:** Visual Studio Code + MCP Agent

## Overview

`GASP` is a modular Unreal project built around a clean separation between:

- **Brain** — high-level Gameplay Ability System (GAS) logic and tag-driven state transitions
- **Combat** — gameplay attributes, combat state, effects, and ability-driven response
- **Body** — physical movement and prediction via Mover2
- **Mind** — runtime tactical decision/data flow via State Trees and AI asset execution
- **Skins** — visual/pose customization via Mutable / MetaHuman / PoseSearch

The repository is optimized for source control without Git LFS by treating UE asset binaries as binary and keeping generated folders out of source control.

## System Laws & Governance

The following structural rules are mandatory for all code and asset changes:

1. **Tag-First Law**
   - All state changes, animation overrides, and cosmetic shifts must be expressed through Gameplay Tag mutations.
   - Direct boolean state flags such as `bIsAttacking` or `bIsStunned` are prohibited.

2. **Interface Contract**
   - The `Brain` module must not depend on `Body` or `Skins` directly.
   - Communication must occur through `I_GASPBodyInterface` or event delegates.

3. **Mutable Isolation Rule**
   - `UCustomizableObjectInstance` and Mutable mesh data belong exclusively to the Skins layer.
   - C++ logic only broadcasts tag parameters that trigger Mutable changes.

4. **Preview Fallback Protection**
   - Experimental UE 5.7.4 subsystems must be guarded with explicit preprocessor checks like `#if UE_ENABLE_FEATURE_POSESEARCH_V1`.

5. **MCP Isolation Policy**
   - The IDE Agent may only read/write inside `Source/`, `Plugins/`, and `Content/Temp/AI/`.
   - Engine source code must remain untouched.

## VS Code Toolchain & Workspace Configuration

The workspace uses the following VS Code settings:

```json
{
  "unrealEngine.projectRoot": "C:\\Unreal_Projects\\GASP",
  "unrealEngine.engineRoot": "C:\\Program Files\\Epic Games\\UE_5.7",
  "mcp.server.port": 30010,
  "files.watcherExclude": {
    "**/Intermediate/**": true,
    "**/Binaries/**": true,
    "**/.vs/**": true
  }
}
```

### Existing workspace files

- `GASP.code-workspace` — VS Code folders, settings, tasks, and launch configurations
- `.vscode/settings.json` — project root / engine root / MCP server port
- `.gitignore` — Unreal-generated directories and IDE artifacts ignored
- `.gitattributes` — binary asset handling and text normalization for project metadata

## Repository Structure

- `Source/` — C++ game module source
- `Plugins/` — local plugins, including `UE_MCP_AGENT_PLUGIN` and `AzureUE_MCP`
- `Content/` — Unreal content and assets
- `Content/AI/agents/` — local MCP agent manifest location
- `Config/` — project config files and gameplay tag definitions
- `GASP.uproject` — Unreal project manifest with enabled plugins

## Phase-Based Execution Blueprint

### Phase I: Foundation, Plugins & Toolchain Configuration

- Standardize C++ project structure and VS Code task workspace
- Enable mandatory plugins in `GASP.uproject`
- Create local MCP manifest for agent execution
- Boot editor, confirm MCP server on port `30010`, and trigger initial build

### Phase II: Brain + Combat Implementation (GAS & Sovereign Tags)

- Add gameplay tags like `Skin.Cosmetic.MutableUpdate` and `Skin.Skeletal.MetaHumanRig`
- Build `GASP_CombatComponent` derived from `UActorComponent`
- Implement health, stamina, momentum, and poise attributes
- Use GAS 2.0 `FGameplayEffectSpec` patterns
- Attach the combat component to the core character blueprint

### Phase III: Body Implementation (Mover2 & Input Prediction)

- Write `I_GASPBodyInterface.h` with pure virtual hooks for positional updates
- Derive `GASP_MoverCharacter` for Mover2 integration
- Pass prediction tokens via `FCombatMoverInputCmd`
- Configure step heights, physics limits, and replication boundaries

### Phase IV: Skins (Mutable, MetaHuman Rig, & Motion Matching)

- Expose C++ bindings to trigger `UCustomizableObjectInstance` parameter changes
- Guard experimental systems behind `#if UE_ENABLE_FEATURE_POSESEARCH_V1`
- Import MetaHuman skeleton and establish visual hierarchy
- Configure Mutable customization graphs for live parameter updates

### Phase V: Mind (State Tree 1.0 & Tactical Intent)

- Keep AI asset data under `Content/Temp/AI/`
- Build evaluation tasks for distance and tactical vector calculations
- Use EQS to rank navigation coordinates
- Link behavioral state transitions through node-based execution

### Phase VI: Framework Integration & Combat Loop

- Author validation tooling to verify framework health
- Connect trace routines to GAS damage/resource impacts
- Build UI widgets for attribute display
- Segregate collision channels for weapons vs environment

### Phase VI.b: Motion Matching Integration Plan

- Keep `SandboxCharacter_Mover` as-is and avoid duplicating or repathing the pawn blueprint.
- Refer to `Source/GASP/PDD_UnifiedGameplayFramework.md` for the official Tier 1 / Tier 2 implementation plan.
- Basic implementation should be a Blueprint-only drop-in for Level Designers: add a trajectory component, point the existing AnimBP to a pose search database, and keep the current pawn type unchanged.
- Advanced implementation should build the underlying tracking and prediction pipeline in `Source/GASP` and expose clean Blueprint access through `UGASP_StateTreeComponent`.
- Track a ring buffer of trajectory samples in C++ and provide BlueprintCallable readouts for history and predicted future samples.
- Use the existing `GASP_StateTreeComponent` as the bridge from the mover body to the animation-driven decision system.

### Motion Matching Blueprint Mapping Spec

#### Event Graph Cache Layer

1. On `Blueprint Update Animation`, call `Try Get Pawn Owner`.
2. From the pawn return, call `Get Component by Class` and search for `GASP_StateTreeComponent`.
3. Promote the resulting component reference to a local variable named `GASP_StateTree_Ref`.
4. Use an `Is Valid` check before any further array operations.
5. Create a new AnimBP variable named `LivePoseSearchTrajectory` of type `TransformTrajectory`.
6. Call `Get Motion Matching Trajectory` on `GASP_StateTree_Ref` and assign the returned `TransformTrajectory` directly into `LivePoseSearchTrajectory`.
7. Do not use a `For Each Loop` or manual `Add Sample` nodes in the Event Graph.
8. Use this cached `LivePoseSearchTrajectory` directly in the Anim Graph.

#### Anim Graph Injection Layer

- Open the main locomotion output graph in `SandboxCharacter_Mover_ABP`.
- Replace legacy test state output with a native `Motion Matching` node.
- Configure the node to use the database `PSD_Locomotion`.
- Connect `LivePoseSearchTrajectory` into the node's `Trajectory` input pin.
- Route the node's pose output into the `Output Pose` result.

#### Visual Wiring Diagram

```
                          ┌─────────────────────────────────────────┐
                          │         Motion Matching Node            │
                          ├─────────────────────────────────────────┤
                          │ Database: PSD_Locomotion                │
                          │ Trajectory: [Get LivePoseSearchTrajectory]──► (Pin Entry)
                          │                                         │
                          │ (Out Pose) ─────────────────────────────┼──► [Output Pose]
                          └─────────────────────────────────────────┘
```

#### Notes

- Do not modify the base `SandboxCharacter_Mover` pawn blueprint.
- Keep all trajectory translation and array wiring inside `SandboxCharacter_Mover_ABP`.
- Use `UGASP_StateTreeComponent::GetLocomotionSchemaProfile` to keep the C++ and designer bone schema consistent.
- Use `UGASP_StateTreeComponent::GetLocomotionSchemaBoneNames` to verify the expected bone list before building the pose database.
- Use `ValidateMotionMatchingSkeleton` in the AnimBP or editor utility to verify the target skeleton before building `PSD_Locomotion`.
- Use `UGASP_StateTreeComponent::GetActivePoseSearchDatabaseName` or `IsCombatMotionMatchingActive` to optionally drive combat vs locomotion database selection in the AnimBP.
- For combat flow, build a second database named `PSD_Combat` and use the state tree to switch the AnimBP to combat-specific pose matching only when combat state is active.

### Phase VI.a: Current GAS Implementation Status

- `AGASP_MoverCharacter` now owns a replicated `UAbilitySystemComponent` and implements `IAbilitySystemInterface`.
- The pawn supports `DefaultAbilities` grants, with ability replication mode set to `Minimal`.
- Movement remains separated: `CharacterMoverComponent` / `UMoverComponent` handle body simulation, while GAS handles gameplay state, tags, and effects.
- `GASP_CombatComponent` remains the combat bridge and can dispatch gameplay tag-based state changes to GAS-driven systems.

### Known Workflow Issues and Fixes

- **Issue:** `CBP_SBCM_GASP` may contain an engine `CharacterMoverComponent`, while runtime pawn code also creates a fallback `UMoverComponent`.
  - **Fix:** `AGASP_MoverCharacter::EnsureCharacterMoverComponent()` now detects the Blueprint `UCharacterMoverComponent`, replaces the fallback mover at runtime, and preserves the existing input producer wiring.

- **Issue:** `GameplayAbilities` was declared as a module dependency, but the project had no local `AbilitySystemInterface` implementation in pawn code.
  - **Fix:** The pawn now includes `AbilitySystemComponent.h` and `GameplayAbility.h`, exposes `AbilitySystemComponent`, and implements `GetAbilitySystemComponent()`.

- **Issue:** Abilities and effects should not be hard-coded into movement or body logic.
  - **Fix:** The architecture now keeps `Body` movement separate from `Brain`/`GAS` gameplay logic, preserving the Tag-First Law and enabling the combat/mind layers to trigger abilities through tags and delegates.

- **Issue:** Experimental engine-only subsystems can leak into gameplay logic and cause unstable builds.
  - **Fix:** Continued guardrails remain in place for optional systems, and the product design document should not permit direct engine source edits.

### Recommended Next Fixes

- Add a dedicated `UGASP_AttributeSet` or equivalent to expose health/stamina/momentum/poise as GAS attributes.
- Implement `InitializeGameplayAbilities()` in `PossessedBy` / `OnRep_PlayerState` to grant default abilities only when authority is available.
- Add blueprint-facing documentation for `CBP_SBCM_GASP` to ensure the `CharacterMoverComponent` is not duplicated and that the `AbilitySystemComponent` is configured in the pawn Blueprint.

### Phase VII: Performance Engineering & Optimization

- Add device profiles and startup scaling overrides
- Capture Unreal Insights traces for combat loops
- Audit VRAM and preserve a safe performance margin

## Agent Execution Prompt

Use this exact prompt with your VS Code MCP/AI agent:

```text
You are an expert automated AI Developer Agent operating in this Visual Studio Code workspace located at C:\Unreal_Projects\GASP.
Your assignment is to parse the 'GASP.uproject' JSON file in the root directory and update its plugin configurations.

1. Locate the "Plugins" array and ensure the following 14 modules are explicitly declared and set to "Enabled": true. If they do not exist, append them structurally:
   - GameplayAbilities
   - Mover
   - PoseSearch
   - Chooser
   - ControlRig
   - NetworkPrediction
   - CustomizableObject
   - MetaHuman
   - ModularGameplayActors
   - ModularGameplay
   - GameFeatures
   - GameSubtitles
   - UE_MCP_AGENT_PLUGIN
   - AzureUE_MCP

2. Generate or update the workspace MCP agent manifest file named 'gasp-ai-combat-v1.yaml' and save it directly to the relative path: Content/AI/agents/gasp-ai-combat-v1.yaml
3. The manifest must specify:
   safety_level: medium
   allowed_directories: ["Source/", "Plugins/", "Content/Temp/AI/"]

4. Once the files are updated, do not touch any .cpp or .h code yet. Run your internal file-save verification and confirm when the descriptor JSON is structurally valid.
```

## Human-In-The-Loop Validation Gate

Run these commands to verify the workspace after the agent work:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RocketGenerateProjectFiles.bat" -project="C:\Unreal_Projects\GASP\GASP.uproject" -game -engine -vscode
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" GASPEditor Win64 Development "C:\Unreal_Projects\GASP\GASP.uproject" -WaitMutex -FromMsBuild
```

## Notes

- The agent must not modify raw engine source.
- The local MCP manifest should remain under `Content/AI/agents/` only.
- The root `README.md` documents current project goals, governance, and the agent execution blueprint.
