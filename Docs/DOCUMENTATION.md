# Smart Object Director — Documentation

**Version:** 1.0.0  •  **Engine:** Unreal Engine 5.8  •  **Author:** Simulated Flow
**Support:** simulatedflow@gmail.com  •  **Category:** Gameplay / Code Plugin

Smart Object Director is a C++ runtime-extension framework for Epic's Smart Object system. It makes Smart Objects easy to **spawn, register and modify at runtime**, exposes a **server-authoritative claim/interaction bridge** for both players and AI, aligns characters to slot entry points with **Motion Warping** (with a graceful fallback), and ships a native **StateTree task** plus an **editor visualizer** for fast debugging.

---

## Table of Contents

1. [Requirements](#1-requirements)
2. [Installation](#2-installation)
3. [Quick Start](#3-quick-start)
4. [Architecture Overview](#4-architecture-overview)
5. [API / Class Reference](#5-api--class-reference)
6. [Code Examples](#6-code-examples)
7. [Networking Model](#7-networking-model)
8. [Motion Warping & Fallback](#8-motion-warping--fallback)
9. [Editor Visualizer](#9-editor-visualizer)
10. [Supported Platforms & Engine](#10-supported-platforms--engine)
11. [Troubleshooting](#11-troubleshooting)
12. [FAQ](#12-faq)
13. [Support](#13-support)

---

## 1. Requirements

| Item | Requirement |
|------|-------------|
| Engine | Unreal Engine **5.8** |
| Project type | C++ **or** Blueprint (a C++ toolchain is required to build the plugin from source) |
| Enabled engine plugins | `SmartObjects`, `GameplayAbilities`, `MotionWarping`, `StateTree` (auto-enabled as plugin dependencies) |
| Platforms | Win64, Mac, Linux |

The four dependency plugins are declared in `SmartObjectDirector.uplugin` and are enabled automatically when Smart Object Director is enabled.

---

## 2. Installation

### Option A — Project plugin (recommended)

1. Close the Unreal Editor.
2. Copy the `SmartObjectDirector` folder into your project's `Plugins/` directory:
   ```
   <YourProject>/Plugins/SmartObjectDirector/
   ```
3. Re-open the project. If prompted to rebuild missing modules, confirm — the editor compiles the two modules (`SmartObjectDirector`, `SmartObjectDirectorEditor`).
4. Verify under **Edit → Plugins → Gameplay → Smart Object Director** that the plugin is enabled.

### Option B — Engine plugin

Copy the folder into `<Engine>/Engine/Plugins/Marketplace/SmartObjectDirector/` to make it available to every project on that engine install.

### Building from source (CLI)

```bat
"<Engine>\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin ^
  -Plugin="<YourProject>\Plugins\SmartObjectDirector\SmartObjectDirector.uplugin" ^
  -Package="<OutputDir>\SmartObjectDirector" ^
  -TargetPlatforms=Win64
```

> The plugin has no third-party binaries and no absolute include paths — it builds cleanly for Development and Shipping targets.

---

## 3. Quick Start

A working interaction needs three ingredients: a **Smart Object Definition asset**, a **director** on the object, and an **interaction bridge** on the character.

1. **Create a Smart Object Definition** — `Content Browser → Add → Artificial Intelligence → Smart Object Definition`. Add one or more slots and tag each slot's *Activity Tags* (e.g. `SmartObject.Activity.Craft`).

2. **Make an actor a dynamic Smart Object**
   - Add a **SO Director Component** to the actor Blueprint.
   - Assign your definition to **Smart Object Definition Asset**.
   - Leave **Auto Register On Begin Play** enabled. The actor now registers itself with the `USmartObjectSubsystem` when it spawns — no persistent Smart Object collection required.

3. **Give a character the ability to interact**
   - Add a **SO Interaction Component** to your Character/Pawn.
   - (Optional) Add a **Motion Warping Component** for precise entry-point alignment. Without it the plugin falls back to interpolation/teleport.

4. **Trigger an interaction** (Blueprint or C++):
   - Call `Find Best Interactable(Radius, FilterTags)` to locate the nearest matching object.
   - Call `Server Claim And Interact(Target, SlotTag)` to reserve the slot on the server and start the interaction.
   - Call `Cancel Current Interaction()` to abort and free the slot.

5. **AI via StateTree** — in a StateTree asset, add the **Use Smart Object (Director)** task, set the *Activity Tag*, *Search Radius*, *Interaction Duration* and optional *Completion Tag*. The AI agent will find, claim, move to, and use the object autonomously.

---

## 4. Architecture Overview

```
┌─────────────────────────── Runtime Module: SmartObjectDirector ───────────────────────────┐
│                                                                                            │
│  USODirectorComponent            USOInteractionComponent           FSOTask_UseSmartObject  │
│  (on the Smart Object actor)     (on the player / AI pawn)         (StateTree task)        │
│        │                                │                                 │                │
│        │ registers instance             │ FindBestInteractable            │ find + claim   │
│        ▼                                 ▼ ServerClaimAndInteract          ▼ + warp + wait  │
│                          USmartObjectSubsystem  (engine)                                    │
│                          MotionWarping (align)  •  GAS (completion tags)                    │
└────────────────────────────────────────────────────────────────────────────────────────────┘
┌────────────────── Editor Module: SmartObjectDirectorEditor ──────────────────┐
│  FSODirectorComponentVisualizer — draws slots in the viewport                 │
│  (green = free, red = claimed, orange = disabled)                             │
└──────────────────────────────────────────────────────────────────────────────┘
```

- **Runtime module** (`Type: Runtime`, `LoadingPhase: Default`) — all gameplay classes; ships in packaged builds.
- **Editor module** (`Type: Editor`, `LoadingPhase: PostEngineInit`) — visualizer only; excluded from packaged builds.
- Log category: `LogSmartObjectDirector`.

---

## 5. API / Class Reference

### `USODirectorComponent` — Dynamic Registrator
`ClassGroup=(SmartObjectDirector)`, BlueprintSpawnable. Attach to any actor that should behave as a runtime Smart Object.

| Member | Signature | Description |
|--------|-----------|-------------|
| `RegisterWithSubsystem` | `bool RegisterWithSubsystem()` | Registers a runtime Smart Object instance with the subsystem. Idempotent — safe to call again. |
| `UnregisterFromSubsystem` | `void UnregisterFromSubsystem()` | Removes the instance from the subsystem. |
| `SetSlotEnabled` | `bool SetSlotEnabled(FGameplayTag SlotTag, bool bEnabled)` | Enables/disables every slot whose activity tags contain `SlotTag`. Returns `true` if at least one slot matched. |
| `IsSlotClaimed` | `bool IsSlotClaimed(FGameplayTag SlotTag) const` | `true` if any matching slot is currently claimed/occupied. |
| `IsRegistered` | `bool IsRegistered() const` | `true` while a valid runtime handle is held. |
| `GetRegisteredHandle` | `FSmartObjectHandle GetRegisteredHandle() const` | The runtime instance handle (invalid until registered). |
| `GetDefinition` | `const USmartObjectDefinition* GetDefinition() const` | Synchronously loads and returns the configured definition. |

**Properties**

| Property | Type | Default | Meaning |
|----------|------|---------|---------|
| `SmartObjectDefinitionAsset` | `TSoftObjectPtr<USmartObjectDefinition>` | — | Definition describing this object's slots/behaviors. |
| `bAutoRegisterOnBeginPlay` | `bool` | `true` | Auto-registers on `BeginPlay`. |

---

### `USOInteractionComponent` — Player & AI Bridge
`ClassGroup=(SmartObjectDirector)`, BlueprintSpawnable. Attach to a pawn/character.

| Member | Signature | Description |
|--------|-----------|-------------|
| `FindBestInteractable` | `AActor* FindBestInteractable(float QueryRadius, FGameplayTagContainer FilterTags)` | Returns the best matching Smart Object owner actor within `QueryRadius` satisfying `FilterTags`. |
| `ServerClaimAndInteract` | `void ServerClaimAndInteract(AActor* TargetObject, FGameplayTag SlotTag)` | `Server, Reliable, WithValidation`. Authoritatively reserves the slot, then warps and starts the interaction. |
| `CancelCurrentInteraction` | `void CancelCurrentInteraction()` | Cleanly aborts and frees the reserved slot. |
| `IsInteracting` | `bool IsInteracting() const` | `true` while a slot is reserved and an interaction is active. |
| `SyncWarpTranslationAndRotation` | `void SyncWarpTranslationAndRotation(FVector Loc, FRotator Rot)` | Applies Motion Warping alignment to the slot entry point (fallback: teleport/rotate). |

**Properties**

| Property | Type | Default | Meaning |
|----------|------|---------|---------|
| `ActiveSmartObject` | `TObjectPtr<AActor>` (Replicated) | `nullptr` | Object currently being interacted with. |
| `MotionWarpTargetName` | `FName` | `SmartObjectAlign` | Warp target name your interaction montage drives. |

---

### `FSOTask_UseSmartObject` — StateTree Task
`FStateTreeTaskCommonBase`. Display name **"Use Smart Object (Director)"**, category `SmartObjectDirector`. Add it to a StateTree so an AI agent can find, claim, move to and use a Smart Object autonomously; the reservation is always released on exit.

**Instance data (`FSOTask_UseSmartObjectInstanceData`)**

| Field | Type | Default | Meaning |
|-------|------|---------|---------|
| `Interactor` | `TObjectPtr<AActor>` | `nullptr` | Agent that uses the object. Unset → StateTree owner/pawn. |
| `ActivityTag` | `FGameplayTag` | — | Only slots whose activity tags contain this are considered. Unset = any slot. |
| `SearchRadius` | `float` | `1500.0` | Search radius (cm) around the interactor. |
| `InteractionDuration` | `float` | `3.0` | Max seconds to hold the interaction before auto-success. |
| `CompletionTag` | `FGameplayTag` | — | Optional tag that, once present on the interactor, completes early. |

---

## 6. Code Examples

### 6.1 Register a dynamically spawned Smart Object (C++)

```cpp
#include "SODirectorComponent.h"

// Spawn a crafting station and register it as a Smart Object at runtime.
AActor* Station = GetWorld()->SpawnActor<AActor>(StationClass, SpawnTransform);
if (USODirectorComponent* Director = Station->FindComponentByClass<USODirectorComponent>())
{
    // Only needed if bAutoRegisterOnBeginPlay is false; otherwise it is already registered.
    Director->RegisterWithSubsystem();

    // Temporarily close the "repair" slot until the player has the required item.
    Director->SetSlotEnabled(
        FGameplayTag::RequestGameplayTag("SmartObject.Activity.Repair"),
        /*bEnabled=*/ false);
}
```

### 6.2 Player interaction (C++)

```cpp
#include "SOInteractionComponent.h"

void AMyCharacter::TryInteract()
{
    USOInteractionComponent* Interaction = FindComponentByClass<USOInteractionComponent>();
    if (!Interaction) return;

    FGameplayTagContainer Filter;
    Filter.AddTag(FGameplayTag::RequestGameplayTag("SmartObject.Activity.Craft"));

    if (AActor* Target = Interaction->FindBestInteractable(/*Radius=*/ 400.f, Filter))
    {
        // Reservation is validated and applied on the server (safe in multiplayer).
        Interaction->ServerClaimAndInteract(
            Target,
            FGameplayTag::RequestGameplayTag("SmartObject.Activity.Craft"));
    }
}
```

### 6.3 Blueprint flow

```
Event (Interact key)
  → Find Best Interactable (Query Radius = 400, Filter Tags = SmartObject.Activity.Craft)
  → Branch (Is Valid?)
      True → Server Claim And Interact (Target Object, Slot Tag = SmartObject.Activity.Craft)
  ...
Event (Cancel key)
  → Cancel Current Interaction
```

### 6.4 AI via StateTree

Add **Use Smart Object (Director)** to a State and configure:

- **Activity Tag:** `SmartObject.Activity.Rest`
- **Search Radius:** `2000`
- **Interaction Duration:** `5.0`
- **Completion Tag:** `SmartObject.State.Finished` *(optional — completes early when granted on the agent's ability system)*

The task claims a slot, moves the agent to the entry point, triggers the interaction, and succeeds when the duration elapses or the completion tag is granted — whichever comes first.

---

## 7. Networking Model

- **Server authority:** slot reservation happens exclusively through `ServerClaimAndInteract` (`Server, Reliable, WithValidation`). Clients never claim slots directly.
- **Race-condition guardrail:** if two clients target the same slot in the same frame, only the first server-side claim succeeds; the second request fails cleanly and does not start an interaction.
- **Replication:** the `ActiveSmartObject` reference is replicated to the owning client so UI/animation can react. The reservation (`FSmartObjectClaimHandle`) lives only on the authority.
- **Release:** `CancelCurrentInteraction` and `EndPlay` both release any held claim, so slots are never leaked when a pawn is destroyed mid-interaction.

---

## 8. Motion Warping & Fallback

`SyncWarpTranslationAndRotation` drives the warp target named by `MotionWarpTargetName` (default `SmartObjectAlign`). Author a warp target of that name in your interaction montage for frame-accurate alignment to the slot entry point.

If the character has **no Motion Warping Component**, the plugin degrades gracefully to standard location interpolation / direct rotation so the interaction still lines up. No hard dependency on a warping setup is required to ship.

---

## 9. Editor Visualizer

The editor module registers `FSODirectorComponentVisualizer` for `USODirectorComponent`. In the level viewport it draws each slot's entry point color-coded for real-time debugging of dynamic Smart Objects:

| Color | Meaning |
|-------|---------|
| 🟢 Green | Slot free |
| 🔴 Red | Slot claimed |
| 🟠 Orange | Slot disabled |

The visualizer is part of the editor-only module and has no effect on packaged/shipping builds.

---

## 10. Supported Platforms & Engine

| | |
|---|---|
| **Engine version** | Unreal Engine 5.8 (`EngineVersion: 5.8.0`) |
| **Runtime platforms** | Win64, Mac, Linux (`PlatformAllowList`) |
| **Editor platforms** | Win64, Mac, Linux |
| **Build targets** | Development & Shipping |
| **Module types** | `SmartObjectDirector` (Runtime) · `SmartObjectDirectorEditor` (Editor) |
| **Content** | `CanContainContent: true` |

---

## 11. Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| Object never found by `FindBestInteractable` | Not registered / wrong tags | Confirm the director is registered (`IsRegistered`) and the slot's activity tags contain the filter tag. |
| Interaction never starts in multiplayer | Called on client without authority | Use `ServerClaimAndInteract` (already a Server RPC) — do not attempt to claim on the client. |
| Character does not align precisely | No Motion Warping Component / missing warp target | Add a Motion Warping Component and a warp target named `SmartObjectAlign` (or change `MotionWarpTargetName`). Fallback alignment is used otherwise. |
| Slot stays claimed after a pawn dies | External code bypassed the component | Reservations are released on `CancelCurrentInteraction` and `EndPlay`; route all claims through `USOInteractionComponent`. |
| Missing-module prompt on project open | Modules not compiled | Let the editor rebuild, or run `RunUAT BuildPlugin` (see Installation). |

Enable verbose logging: `log LogSmartObjectDirector Verbose` in the console.

---

## 12. FAQ

**Do I need a Smart Object Collection in the level?** No — the director registers a runtime instance directly, which is the whole point of the plugin.

**Does it work for both players and AI?** Yes. `USOInteractionComponent` serves player pawns; `FSOTask_UseSmartObject` serves AI via StateTree. Both go through the same server-authoritative claim path.

**Is GAS mandatory?** No. GAS is only used for the optional `CompletionTag` early-out in the StateTree task; you can leave it unset and rely on `InteractionDuration`.

**Blueprint-only project?** Supported — every gameplay entry point is `BlueprintCallable`/`BlueprintPure`, but a C++ toolchain is needed to compile the plugin.

---

## 13. Support

- **Email:** simulatedflow@gmail.com
- **Author:** Simulated Flow
- When reporting an issue, include your engine version, platform, a `LogSmartObjectDirector` excerpt and repro steps.
