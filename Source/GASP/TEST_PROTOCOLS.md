# HITL Verification Protocol

## Purpose
This file captures the Human-in-the-Loop (HITL) testing protocol for the high-performance native motion matching integration in `SandboxCharacter_Mover_ABP`.

## Verification Sequence

### Step 1: Compilation & Live-Link Registry Check

* Close the Unreal Editor.
* Execute a clean build from your IDE/compiler.
* Relaunch the editor.
* Open the Output Log and verify there are no initialization errors for the `PoseSearch` module or `UGASP_StateTreeComponent`.
* Open `SandboxCharacter_Mover_ABP` and verify the `Get Motion Matching Trajectory` node is visible in the context menu and compiles cleanly in the Event Graph.

### Step 2: The Rewind Debugger Setup

* In the Editor toolbar, navigate to `Tools > Debug > Rewind Debugger`.
* Drop an instance of `SandboxCharacter_Mover` into an empty test level.
* Press Play In Editor (PIE).
* Open the Rewind Debugger window and select the live character instance from the actor trace list.
* Record telemetry frames for the running character.

### Step 3: Trajectory & Debug Visualizer Validation

* Open `SandboxCharacter_Mover_ABP`.
* In the AnimGraph, select the `Motion Matching` node.
* In the Details Panel, under the **Misc** section, set `bDebug` = true.
* While playing, observe the floor around the character in the viewport.
* Expect a sequence of color-coded debug points tracing the character's trajectory ahead and behind.
* The Test:
  * **Slope Test:** Walk up a ramp. The forward trajectory dots must not penetrate the floor.
  * **Snap Test:** Rapidly stop and turn. The dots must follow the velocity vector smoothly.
* Pass criteria:
  * Colored trajectory dots are visible and project forward smoothly.
  * Trajectory points do not snap or jitter.
  * Dots are not collapsed to `0,0,0`.

### Step 3.5: Thread Profiler Validation

* Open `Window > Developer Tools > Unreal Insights`.
* Click `Start Capture` and enter the test level.
* Move the character for 10 seconds, then stop the capture.
* In the Insights view selector, filter for `AnimInstance`.
* Inspect the `UpdateAnimation` thread.
* Pass criteria:
  * The custom motion matching code stays below `0.05ms` per frame.
  * No sustained spikes appear in the animation thread.

### Step 4: Skeletal Mapping & Bone Integrity

* In `SandboxCharacter_Mover_ABP`, go to the Utilities panel and drag out `Check Skeleton`.
* Wire the node into the Update thread.
* The Test: verify the bone names `foot_l`, `foot_r`, and `pelvis` return **Green** (True).
* Pass criteria:
  * `Check Skeleton` returns true for all three bone names.
  * No case-sensitive mismatch exists (e.g. `Foot_R` vs `foot_r`).

### Step 5: Pose Search Database Index Check

* Open the locomotion database asset `PSD_Locomotion`.
* Click through the individual animation sequences: Walk, Run, Slide.
* Verify that the tracking lines mapped to `pelvis`, `foot_l`, and `foot_r` align cleanly with the bone structure of the `SKM_UEFN_Mannequin`.
* Pass criteria:
  * Tracking indicators are attached to the skeleton.
  * They do not float away from the feet.
  * No case-sensitivity mismatches exist in bone name lookup.

## HITL Evaluation Matrix

| Verification Phase | Check Type | Target Metric / Expected Behavior | Status |
| --- | --- | --- | --- |
| **Visual Debug** | `bDebug` Pin | Trajectory dots must project forward smoothly along velocity vectors without jitter. | [ ] |
| **Profiler** | Unreal Insights | `AnimInstance` thread time < 0.05ms per frame. | [ ] |
| **Skeletal** | `Check Skeleton` Utility | `foot_l`, `foot_r`, `pelvis` must all return True (Green). | [ ] |
| **Slope Handling** | Physics Trace | Trajectory points must not clip below floor geometry during slopes > 45°. | [ ] |

## Active Block Targets

* `AnimBP Variable Connection`
* `HITL Editor Validation`

## Notes

* Keep the validation artifacts limited to `Source/GASP` and `SandboxCharacter_Mover_ABP` serialization updates.
* Do not create any untracked workspace files outside the `Source/GASP` folder except for approved editor asset metadata changes.
* For complete HITL coverage, also follow `Docs/HITL-Workflow.md` and record outcomes in `BACKLOG.md`.
