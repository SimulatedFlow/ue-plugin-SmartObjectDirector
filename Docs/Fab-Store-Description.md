# Smart Object Director — Fab Store Listing

## Headline

**Smart Object Director — Runtime Smart Objects for Players & AI, done right.**
Dynamic registration, server-safe claims, Motion Warping & native StateTree — the missing runtime layer for Epic's Smart Object framework.

---

## Pitch (1 paragraph)

Epic's Smart Object system is powerful for AAA AI environments — but it's painful to spawn, modify, or use at runtime, and it doesn't hand you a player-facing or multiplayer-safe interaction layer out of the box. **Smart Object Director** closes that gap with a turnkey C++ runtime layer: drop one component on any actor to register it as a dynamic Smart Object, drop another on any character to find, claim and use nearby objects, and let AI do the same through a native StateTree task. Slot reservations are fully server-authoritative (no claim race-conditions), characters align to entry points via Motion Warping with an automatic fallback, and a color-coded editor visualizer makes debugging instant. No persistent collections, no Blueprint spaghetti, no engine forking — just Smart Objects that finally work at runtime.

---

## Feature Bullets

- **⚡ Dynamic Runtime Registration** — turn any runtime-spawned actor (barricades, crafting stations, vehicles) into a Smart Object with a single component; enable/disable individual slots by Gameplay Tag on the fly. No Smart Object Collection required.
- **🎮 Player & AI Interaction Bridge** — one component finds the best nearby Smart Object, claims a slot, and drives the interaction — for player-controlled *and* AI-controlled characters through the same path.
- **🛡️ Multiplayer Guardrails** — slot reservation is `Server, Reliable, WithValidation` and race-condition safe: two clients grabbing the same slot in the same frame resolve to exactly one winner. Claims auto-release on cancel or destroy.
- **🎯 Motion Warping Alignment + Fallback** — frame-accurate warp to slot entry points via a named warp target, with graceful interpolation/teleport fallback when no Motion Warping Component is present.
- **🌳 Native StateTree Task** — "Use Smart Object (Director)" lets AI agents find, reserve, navigate to and use objects; completes on a duration timer or an optional GAS completion tag.
- **🔍 Editor Visualizer** — real-time color-coded slot states in the viewport (green = free, red = claimed, orange = disabled).
- **🧩 Blueprint-Friendly** — every entry point is BlueprintCallable/Pure; use it entirely from Blueprint or extend it in C++.
- **📦 Clean & Submission-Ready** — two well-separated modules (Runtime + Editor), full source, no third-party binaries, no absolute paths, no external Marketplace dependencies.

---

## Technical Specs

| | |
|---|---|
| **Engine version** | Unreal Engine 5.8 |
| **Type** | C++ Code Plugin (full source included) |
| **Modules** | SmartObjectDirector (Runtime) · SmartObjectDirectorEditor (Editor) |
| **Runtime platforms** | Win64, Mac, Linux |
| **Build targets** | Development & Shipping |
| **Engine plugin deps** | SmartObjects, GameplayAbilities, MotionWarping, StateTree (auto-enabled) |
| **Network** | Server-authoritative, replication-ready |
| **Content** | Blueprint-exposed API; no mandatory content |
| **Third-party libs** | None |

---

## Key Classes

- `USODirectorComponent` — register & manage a dynamic Smart Object.
- `USOInteractionComponent` — find, claim, warp, interact (player & AI).
- `FSOTask_UseSmartObject` — StateTree task for autonomous AI use.
- `FSODirectorComponentVisualizer` — editor slot debugging.

---

## Target Audience

- **Multiplayer & co-op developers** who need collision-free, server-safe object interactions.
- **Open-world / immersive-sim & survival teams** using dynamically spawned interactables (crafting, resting, repairing, mounting).
- **AI/gameplay programmers** already invested in StateTree, GAS and Motion Warping who want a Smart Object runtime layer without building one.
- **Solo devs & small studios** wanting a turnkey interaction system instead of rolling their own line-trace framework.

---

## Suggested Price

**€149** (self-serve tier, well under the $1,500 cap).

- Positioned as a professional, multiplayer-ready gameplay framework — comparable to premium GAS/interaction toolkits on the store.
- Optional launch discount: **€119 (−20%)** for the first two weeks to seed reviews.

---

## Suggested Tags / Keywords

Smart Objects · Interaction System · AI · StateTree · GAS · Motion Warping · Multiplayer · Networking · Gameplay Framework · Runtime · C++
