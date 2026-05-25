# GASP Project Backlog

## Overview
This backlog captures the advanced implementation only, focusing on the high-performance combat framework and editor/runtime verification.

Basic Level Designer implementation phases are intentionally excluded from this advanced roadmap.

## Completed Work
- Project workspace and Unreal project generation verified.
- Enabled required modules in `GASP.Build.cs` and cleaned unavailable plugins.
- Implemented core combat system in `GASP_CombatComponent`.
- Added body/mover integration in `GASP_MoverCharacter` and `IGASPBodyInterface`.
- Added skin update flow in `GASP_CosmeticsComponent`, procedural rig support, and motion matching hook foundations.
- Added tactical helper functions in `GASP_TacticalLibrary`.
- Added validation utilities in `GASP_ValidationLibrary`.
- Added mind/AI logic in `GASP_StateTreeComponent` with intent tag evaluation.
- Added new gameplay tags for skin and combat states, plus AI intent tags.
- Added motion matching skeleton validation helpers in `GASP_StateTreeComponent` for AnimBP and editor asset checks.
- Added surface-aware audio subsystem scaffolding in `GASP_SurfaceAudioSubsystem` and `GASP_SurfaceAudioSettings`.
- Verified full project build success after each major integration step.

## Current Status
- `GASP` build: **Succeeded**.
- Core gameplay systems: **Implemented**.
- Mind/AI phase: **Implemented**.
- Motion matching trajectory API: **Added**.
- Motion matching schema profile: **Added**.
- AnimBP binding spec: **Defined**.
- Documentation and design plan: **Written**.
- IDE Agent implementation directives: **Completed**.
- Surface audio subsystem: **Runtime wiring started**.
- Build/tooling updates: **UMG dependency added, VS Code include paths updated**.
- Repository GitHub remote configured and pushed: **Yes**.
- Remaining focus: **advanced AnimBP hookup, HIL integration, surface audio runtime wiring, and polish**.

## Backlog
| Task | Area | Status | Notes |
|---|---|---|---|
| Validate mover pawn body simulation and replicated input | Body | Done | `AGASP_MoverCharacter` handles client-server input and replicated state.
| Implement combat attribute lifecycle | Combat | Done | Health, stamina, momentum, poise, and combat tags are functional.
| Add skin tag and update flow | Skin | Done | `GASP_CosmeticsComponent` raises skin update events and updates combat tags.
| Add motion matching pose search hooks | Skin | Done | `AnimNotifyState_DamageWindow` and pose search notifications were added for damage-driven matching.
| Implement predicted trajectory API and AnimBP history support | Motion Matching | Done | `UGASP_StateTreeComponent` now exposes predicted trajectory arrays for AnimBP use.
| Add C++ bone tracking schema and motion matching type definitions | Motion Matching | Done | `Source/GASP/Public/GASP_MotionMatchingTypes.h` and `GetLocomotionSchemaProfile()` were added.
| Document AnimBP trajectory mapping and Blueprint binding spec | Docs | Done | `README.md` now contains the Event Graph and Anim Graph mapping spec for `SandboxCharacter_Mover_ABP`.
| Publish implementation plan and phase 2 design doc | Docs | Done | `Source/GASP/PDD_UnifiedGameplayFramework.md` published and referenced in README.
| Add tactical evaluation helpers | Tactical | Done | `GASP_TacticalLibrary` evaluates distance/intent and ranks candidates.
| Add validation library and pawn checks | Validation | Done | `GASP_ValidationLibrary` validates combat component and mover pawn.
| Add mind/AI decision component | Mind | Done | `GASP_StateTreeComponent` evaluates intent and applies movement input.
| Add gameplay tag definitions | Tags | Done | Combat, skin, and intent tags were added.
| Create advanced GameMode and player pawn setup | Blueprint | In progress | Configure `GM_Sandbox`/`PC_Sandbox` and derive `BP_GASP_PlayerCharacter` from `AGASP_MoverCharacter`. Added automation script `Scripts/auto_setup_gasp_blueprints.py` and validator `Scripts/auto_validate_gasp_blueprints.py`.
| Wire pawn HUD and runtime UI | UI | In progress | `AGASP_MoverCharacter` now creates and updates a HUD widget on local control.
| Add surface-aware audio subsystem and developer settings | Audio | In progress | Added subsystem files and engine config; runtime surface impact hookup is implemented; AnimBP binding remains.
| Implement footstep audio AnimNotify and pawn surface trace hookup | Audio | In progress | Added `AnimNotify_GASPFootstep` and `UGASP_SurfaceAudioSubsystem` runtime integration scaffolding.
| Create HITL workflow and verification document | Docs | Done | Created `Docs/HITL-Workflow.md` for in-editor verification and signoff.
| Finalize AnimBP binding and editor pawn setup | HIL | Pending | Bind `UGASP_StateTreeComponent` trajectory output and surface audio events in the editor, then run HITL validation.
| Fix engine plugin VS Code include paths and build configuration | Tooling | Done | `c_cpp_properties.json` updated for Mover plugin; `GASP.Build.cs` added UMG dependency.
| Wire HIL / higher-level integration | HIL | In progress | Final game logic and editor asset hookup remain; HITL workflow is ready.
| Asset/blueprint setup for pawn and AI | Misc | Pending | Need in-editor binding and actor blueprint readiness.

## Progress Chart
| Phase | Description | Completion |
|---|---|---|
| Phase 1 | Project setup, build verification, plugin/manifest cleanup | 100% ✅ |
| Phase 2 | Combat system, health/stamina/momentum/poise | 100% ✅ |
| Phase 3 | Body/mover pawn input and replication | 100% ✅ |
| Phase 4 | Skin component, procedural rig, and motion matching foundation | 100% ✅ |
| Phase 5 | Tactical and validation utilities | 100% ✅ |
| Phase 6 | Mind/AI intent decision and movement | 100% ✅ |
| Phase 7 | HIL-only integration and editor hookup | 90% 🔄 |

## Next Steps
1. Start with advanced GameMode and character blueprint setup.
   - Use the existing `GM_Sandbox` and `PC_Sandbox` blueprints as the foundation for your player setup.
   - Create `BP_GASP_PlayerCharacter` from `AGASP_MoverCharacter`.
   - Assign the skeleton/rig and hook the existing `SandboxCharacter_Mover_ABP` (or derive from it) as the AnimBP.
2. Validate in-editor input and pawn possession flow with the sandbox GameMode first.
3. Wire `SandboxCharacter_Mover_ABP` to the motion matching node by converting `GetPredictedTrajectoryHistory` output into the native `PoseSearchTrajectory` input.
4. Add HIL-level telemetry / debug logging for body/mind decision paths.
5. Confirm gameplay tags drive the intended AI behavior in runtime.
6. Wire `SandboxCharacter_Mover_ABP` to the motion matching node by converting `GetPredictedTrajectoryHistory` output into the native `PoseSearchTrajectory` input.
7. Integrate `GASP_SurfaceAudioSubsystem` with surface impact audio events and designer-facing settings.
8. Validate `FGASPLocomotionSchemaProfile` bone definitions against `SKM_UEFN_Mannequin` using `ValidateMotionMatchingSkeleton` and adjust weights as needed.
9. Keep `SandboxCharacter_Mover` unchanged; all integration should occur through AnimBP and `UGASP_StateTreeComponent`.
10. Reopen VS Code or regenerate compile commands after the engine include-path update, then verify diagnostics for the Mover plugin header.
11. Polish any remaining runtime issues or asset bindings.
12. Keep future commits focused on source/docs changes only and avoid tracking large untracked content assets.
13. Refer to `Docs/AI-Orchestrated-Development-Roadmap.md` for the governance-aligned execution plan.
14. See `Docs/AI-Orchestrated-Sprint-Plan.md` for the current execution task list and sprint status.

---

_Last updated: May 23, 2026_
