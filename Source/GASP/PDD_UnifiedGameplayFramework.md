# 📑 Product Development Document (PDD): Unified Gameplay Framework

**Module Context:** `Source/GASP`

**Target Engine Version:** Unreal Engine 5.7 / 5.8

**Architecture Core:** Native C++ Subsystems + Mover 2.0 Plugin Integration

---

## 🎯 Architectural Mandate

To maintain project stability, prevent data corruption, and ensure rapid modular development, **all development phases are strictly partitioned into a two-tier implementation topology (Basic and Advanced)**.

> [!IMPORTANT]
> **Core Constraint:** Under no circumstances will the base Pawn/Character C++ class type or parent class of the existing `SandboxCharacter_Mover` Blueprint be changed or swapped. No duplicate character blueprints (`_Duplicate`) are permitted. All changes must be non-destructive and preserve existing serialization structures (e.g., custom sliding states, existing component configurations).

---

## 🟢 Tier 1: Basic Implementation (Level Designer Playbook)

*Design Goal: Highly modular, plug-and-play setups that a Level Designer can execute directly inside the Unreal Editor in under 5 minutes per phase without editing C++ code or breaking the core blueprint.*

### 📍 Phase 1: Modular Component Integration & Verification

* **Editor Setup:** Open the existing `SandboxCharacter_Mover` Blueprint asset.
* **Component Drop-In:** In the Components panel, click **Add Component** and drop the newly renamed native components onto the hierarchy:
  * `GASP_AbilitySystemComponent` (Handles GAS Attributes/Effects)
  * `GASP_StateTreeComponent` (Drives logic states)
  * `GASP_CosmeticsComponent` (Manages visual/mesh overrides)

* **Validation Pass:** Compile and save the Blueprint. Ensure the `[CoreRedirects]` have cleanly mapped old references to these new components without throwing broken pin errors.

### 📍 Phase 2: Trajectory & Motion Matching Layer

* **Trajectory Addition:** Add a native engine `CharacterTrajectoryComponent` (or version-specific `PoseSearchTrajectoryComponent`) to `SandboxCharacter_Mover`.
* **Horizon Calibration:** In the Details Panel, configure the tracking time horizons to match the standard UEFN Mannequin animation set:
  * **History Horizon:** `-0.2s`
  * **Prediction Points:** `0.2s`, `0.5s`, `1.0s`

* **Anim Blueprint Hookup:**
  1. Open `SandboxCharacter_Mover_ABP`.
  2. Drop a native **Motion Matching** node inside the main locomotion pose graph.
  3. Slot a pre-built Pose Search Database (`PSD_Locomotion`) directly into the node.
  4. Connect the Anim Instance's internal trajectory data reference pin directly into the Motion Matching node's **Trajectory Input Pin**.

### 📍 Phase 3: IK Retargeting & Cosmetic Activation

* **Mesh Verification:** Select the `SkeletalMeshComponent` and verify its relative **Location Z** is adjusted to exactly `-86.000000` to align flush with the capsule base ($CapsuleHalfHeight = 86.0$).
* **Tag Enforcement:** Go to the `Component Tags` array on the mesh inside the Details panel. Add an element at **Index 0** and set it to `Mesh` (or the specific literal string key parsed by your script) to satisfy the initialization contract for `ABP_GenericRetarget`.

---

## 🔵 Tier 2: Advanced Implementation (From-Scratch Engineering)

*Design Goal: Build the high-performance underlying physics models, custom memory ring-buffers, and context filtering logic from scratch in C++ inside the `Source/GASP` module to drive the Tier 1 components with absolute precision.*

### 📐 Mathematical Kinematic Prediction Model

To feed the motion matching system with highly responsive trajectory tracking during high-velocity maneuvers (like sliding), the C++ subsystem bypasses generic frame approximations and processes position prediction using a standard kinematic calculus integration loop:

$$\vec{x}_{\text{future}} = \vec{x}_{\text{current}} + \vec{v}_{\text{current}}t + \frac{1}{2}\vec{a}t^2$$

Where:

* $\vec{x}_{\text{future}}$ is the predicted location vector of the character at time offset $t$.
* $\vec{x}_{\text{current}}$ is the live world transform location of the root capsule.
* $\vec{v}_{\text{current}}$ is the immediate velocity vector obtained directly from the `CharacterMoverComponent`.
* $\vec{a}$ is the instant input acceleration vector modified by the character's active friction constants (`GroundFriction` and `BrakingFrictionFactor`).

---

### 🛠️ Step-by-Step Advanced Engineering Roadmap

### 📍 Phase 1: Native C++ Core & Interface Bindings

* **Interface Implementation:** Ensure `AGASP_MoverCharacter` natively inherits from `IAbilitySystemInterface`.
* **Authority Initialization:** Fully implement the standard GAS authority lifecycle methods inside the source file:
  * `GetAbilitySystemComponent()` -> Returns the `GASP_AbilitySystemComponent`.
  * `PossessedBy()` -> Initializes the ASC capabilities on the Server.
  * `OnRep_PlayerState()` -> Binds the ASC initialization on the Client network layer.

* **Sanitize Subobjects:** Ensure all component constructors use literal string keys (e.g., `TEXT("GASP_AbilitySystemComponent")`) to safely preserve asset serialization.

### 📍 Phase 2: Custom C++ Trajectory Processing Pipeline

* **Historical Sampling Ring-Buffer:** Define an optimized `TArray<FGASPTrajectorySample>` ring buffer within `Source/GASP` to track and cache the character's global spatial transforms over a rolling 30-frame window.
* **Mover 2.0 State Sampling:** Direct the tracking logic to pull live parameters (`MaxSpeed`, `Acceleration`, and physics modes) from the active `CharacterMoverComponent`.
* **Expose API Parameters:** Mark all calculation outputs with `BlueprintCallable` and `BlueprintReadOnly` macros inside the `Category = "GASP|Locomotion"` specifier, allowing the animation blueprint to fetch clean path telemetry seamlessly.

### 📍 Phase 3: Pose Search Schema & Database Architecture

* **Schema Engineering (`PSS_UEFNMannequin`):**
  * **Position Weight:** Set to `1.0` for rigid root tracking.
  * **Velocity Weight:** Set to `1.5` to eliminate foot-sliding artifacts during radical deceleration curves.
  * **Bone Selection Tracks:** Add explicit joint samplers for `foot_l`, `foot_r`, and `pelvis`.

* **Database Compilation (`PSD_Locomotion`):** Target the `SKM_UEFN_Mannequin` skeletal mesh. Inject clean locomotion assets (Walk, Run, Idle, and your custom `BP_MovementMode_Slide` asset profiles) and run an indexing generation pass to bake the vector cache.

### 📍 Phase 4: State Tree Orchestration & Context Filtering

* **Context Evaluation:** Configure the `GASP_StateTreeComponent` to monitor global character state criteria.
* **Dynamic Cost Injection:** Instead of letting the Anim Blueprint handle transitions blindly, the State Tree evaluates condition contexts (e.g., *"Is Character Sliding?"*). It dynamically passes gameplay tags or filtering cost-modifiers directly to the Motion Matching graph node to instantly eliminate non-sliding poses from the selection pool during high-velocity maneuvers.

---

## 🤖 IDE Agent Implementation Guidelines

When generating or modifying source code to complete tasks within this document, the IDE agent must abide by the following operational constraints:

1. **Scope Verification:** Changes must be entirely contained within the `Source/GASP/` directory or project `.ini`/`.cs` files. Do not modify raw engine code or any assets located in the `Content/` directory.
2. **Naming Convention Enforcement:** Use explicit, self-explanatory, literal naming patterns matching native Unreal naming conventions. Never use abstract metaphors.
3. **No Structural Breaking Changes:** When updating constructors or component layout patterns, ensure properties are exposed via standard UPROPERTY macros to guarantee they serialize cleanly into the existing `SandboxCharacter_Mover` Blueprint instance. Use `CoreRedirects` inside `DefaultEngine.ini` if structural renames are required.
