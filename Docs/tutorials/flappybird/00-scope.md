---
status: draft
---

# Flappy Bird — Build Scope

Pre-tutorial planning doc. We build the game first to validate the human + Claude workflow, then refine this into the actual tutorial docs (`01-…md`, `02-…md`, etc.) in this same folder. Living doc — edit as we go.

## Where the code lives

Reference implementation: `Source/Agility/FlappyBird/` (inside the plugin), following the `Procedural/` precedent. The tutorials will later walk a new reader through building their own version under `Source/<YourProjectModule>/MyScripts/FlappyBird/` in their host project.

## Input strategy

Use **legacy axis/action mappings** rather than Enhanced Input. Reason: Enhanced Input requires `Input Action` / `Input Mapping Context` `.uasset` files, which only the human can author in-editor. Legacy keeps input fully in Claude's lane. If we want Enhanced Input later, it becomes a polish-pass tutorial.

The plugin ships its own action mapping at `Plugins/Agility/Config/Input.ini` — UE merges plugin config additively into the host project's `[/Script/Engine.InputSettings]`, so a fresh host project that enables Agility gets the `Agility.FlappyBird.Flap` action (SpaceBar + LMB) for free. Action names are namespaced (`Agility.<Feature>.<Action>`) so they group under an "Agility" subtree in Project Settings → Input and can't collide with the host project's own mappings.

## Parts

### Part 1 — Bird, gravity, flap input ✅

**Deliverable:** A procedural bird falls under gravity in an empty scene; pressing Space gives it an upward impulse. Side-on camera view.

- **Claude (C++):** `AFlappyBird` pawn with a procedural mesh; gravity + velocity in `Tick`; `Agility.FlappyBird.Flap` action bound to Space + LMB (mapping shipped by the plugin via `Plugins/Agility/Config/Input.ini`); `AFlappyBirdGameMode` setting the default pawn; camera component on the pawn (side-on).
- **Human (editor):** Open / create the Flappy Bird level; set the GameMode override in World Settings; press Play and confirm the bird falls + flaps.

**What we learned, refined in the build:**
- `WorldGridMaterial` does *not* render per-vertex colors in this engine version, so the "vertex colors + engine default material" pattern used by `Source/Agility/Procedural/` actors is broken in practice. Working pattern instead: one mesh *section* per colored part, each section assigned a `UMaterialInstanceDynamic` over `/Engine/BasicShapes/BasicShapeMaterial` with its `Color` vector parameter set in C++. Adopt this for all future procedural actors in the plugin.
- Live Coding (`Ctrl+Alt+F11`) works for in-place edits but cannot bootstrap newly-added `.h` / `.cpp` files — those need a full IDE build (and possibly *Tools → Unreal → Refresh Project*) first. Tutorials should call this out explicitly.

### Part 2 — Procedural pipes + spawner ✅

**Deliverable:** Pipes scroll leftward across the screen at intervals with randomized gap heights; despawn off-screen.

- **Claude (C++):** `APipePair` actor (two procedural mesh halves + configurable gap, tick-based leftward motion); `APipeSpawner` actor (spawns `APipePair`s at intervals with randomized gap Y, despawns off-screen ones).
- **Human (editor):** Drop one `APipeSpawner` into the level; press Play; confirm pipes scroll past.

### Part 3 — Collision + score ✅ (with open polish items, see "Pick up next session")

**Deliverable:** Bird colliding with a pipe triggers a hit event (log it this part). Passing through a gap increments a score, displayed on a HUD.

- **Claude (C++):** Collision components on bird + pipes; overlap/hit events; score counter on `GameMode` (or a dedicated `GameState`); minimal HUD via `AHUD::DrawHUD` (procedural text — no UMG `.uasset`s needed).
- **Human (editor):** Press Play; confirm score increments and collision is detected.

**Mid-Part-3 fixes already applied:**
- **Camera world-lock.** Originally the `UCameraComponent` was attached to the bird and followed it vertically — there was no visible "going off-screen" signal. Fixed by calling `SetUsingAbsoluteLocation(true)` / `SetUsingAbsoluteRotation(true)` on the camera component in the bird's constructor and pinning the world transform in `BeginPlay` based on the bird's spawn X. The camera now stays fixed; the bird moves up/down within the frame.
- **Pipe see-through.** Pipes looked hollow from the side because UE renders single-sided, the gap-facing rim cap was nearly edge-on from the side camera (~9° off the view direction), and the inside cylinder walls had outward normals → back-face culled. Fixed by making `EmitTubeBand` emit double-sided geometry: after the outward-normal triangles, append duplicate verts with inward normals and reverse-wound triangles so the inside walls also render.

### Part 4 — Game over + restart

**Deliverable:** Collision ends the run; pressing a key restarts the level; score resets.

- **Claude (C++):** Game state machine (playing / dead); freeze pipes + bird on death; restart input handler.
- **Human (editor):** Press Play, die, restart, confirm the flow.

### Part 5 — Polish (optional, scope TBD)

Ideas: bird rotates based on velocity, procedural materials via `UMaterialInstanceDynamic`, simple particle effect on death, optional migration to Enhanced Input. Decide what's worth doing once parts 1–4 work.

## Decisions

1. **Bird shape:** Stylized bird built from a few `UProceduralMeshComponent`s — body (stretched sphere), beak (cone), wings (flat triangular meshes, separate components so they can rotate). Wings flap via sine-wave rotation in `Tick`.
2. **Materials / colors:** Per-section MIDs over `/Engine/BasicShapes/BasicShapeMaterial` (see Part 1's lesson). No human-authored material `.uasset`s required.
3. **Level:** Dedicated map under the host project's `Content/MyContent/Maps/FlappyBird.umap` (human creates in-editor). Convention going forward: each experiment / tutorial gets its own map under `Content/MyContent/Maps/`.
4. **2D-ish constraint:** Bird locked to a single plane (X = 0) — enforced each `Tick`. Visuals stay fully 3D.

## Pick up next session — open items

As of session end on 2026-05-12, Parts 1–3 are working in-editor, with two polish issues flagged on the pipes that we didn't get to:

### 1. Pipes appear to be "missing tops" (from the side camera)

The gap-facing rim cap is a flat disc at constant Z, and the camera at world (0, 900, 0) looking -Y views it at a very shallow angle (~9° off perpendicular). Mathematically the cap IS rendered, just with near-zero screen area, so the eye reads it as "no top". The two-sided fix made the inside walls render, which solved looking-through-the-pipe, but didn't make the cap any more visible.

**Likely fixes to try, in order of ambition:**
- **Cheapest:** give the rim cap a tiny extruded thickness so its side walls have screen area from the side. I.e. replace the flat disc cap at `Z0` with a short stub cylinder from `Z0` to `Z0 + DirZ * tiny_cap_thickness` (a cap "lid" sitting just outside the open end), with its own side band + outer disc. Reads as a Mario-pipe-style flat-but-not-paper-thin lid.
- **Alternative:** raise the camera slightly so it looks down at the pipes more (steeper view angle = more visible cap area). Trade-off: changes the overall composition.
- **Last resort:** replace the flat disc cap with a shallow dome (use the existing `EmitSphere` helper with `Rz << Rx == Ry`). More visible from any angle but slightly off the Mario-pipe aesthetic.

### 2. Harsh shadows on pipes

Pipes look severely contrast-y between lit and shadowed sides. Likely culprits, in order of likelihood:
- **Flat-shaded faceted normals.** The procedural mesh sides have normals that change discretely between each face. With a default directional light, this gives stark light/dark bands. Fix: average normals at shared vertex positions to get smooth shading. (Sphere already does this implicitly via the parametric normal; the tube band emits per-face flat normals.) Worth confirming visually first — could also just be the level's lighting.
- **Lighting setup in the level.** If `FlappyBird.umap` was created from the "Basic" template, the default directional light might be too bright or at a punishing angle. Easy to adjust in-editor: tweak the directional light's intensity or add a sky light / ambient cubemap to soften shadows.
- **Two-sided geometry interaction.** The inside-wall duplicates emit inward normals; if those triangles render slightly visible from outside (e.g. due to depth-fighting at grazing angles), they'd shade as if lit from inside = dark. Less likely but worth keeping in mind if smoothing normals doesn't fix it.

Recommendation: try smoothing the tube-band normals first, then tweak lighting in-editor if it still looks rough. Save the cap-extrusion fix for after lighting since the two interact.

### 3. Part 4 — Game over + restart (not started)

Now that the camera is fixed and the player can see when the bird is about to go off-screen, the death-feedback loop is ready to close. Per the scope doc:

- Bird hitting a pipe → die (currently just logs).
- Bird falling off the bottom of the screen (or hitting a configurable death-Z) → die.
- Optionally: bird flying off the top → die, or clamp to the ceiling. Decide in the moment.
- On death: freeze the bird + pipes; show "Game Over" + final score on the HUD.
- Press Space (or R) to restart the level — `UGameplayStatics::OpenLevel` with the current level name is the easy path; resets everything.

`PipePair::OnHitBoxOverlap` is already wired with a `Cast<AFlappyBird>(OtherActor)` filter — the death call swaps in for the current `UE_LOG` line. The score-trigger handler already guards against double-counting via `bScored`, so it'll behave through restarts.

### Files touched this session

- `Source/Agility/FlappyBird/FlappyBird.{h,cpp}` — pawn, gravity, flap input, collider, world-locked camera
- `Source/Agility/FlappyBird/FlappyBirdGameMode.{h,cpp}` — default pawn, HUD class, score state
- `Source/Agility/FlappyBird/FlappyBirdHUD.{h,cpp}` — DrawHUD score text
- `Source/Agility/FlappyBird/PipePair.{h,cpp}` — procedural pipe pair, hit boxes, score trigger, double-sided tube band
- `Source/Agility/FlappyBird/PipeSpawner.{h,cpp}` — timer-based pipe spawner
- `Plugins/Agility/Config/Input.ini` — `Agility.FlappyBird.Flap` action mappings (SpaceBar + LMB), shipped by the plugin so no host-project config is needed
- `Docs/tutorials/flappybird/00-scope.md` — this doc

All files left unstaged per project rule.

## Workflow validation checklist

Things we're explicitly trying to learn / confirm while building, so the tutorial doesn't paper over them:

- Live Coding round-trip time for additive C++ changes (new actors, new components) vs. when a full editor restart is needed.
- How smooth the C++ ↔ editor handoffs feel in practice (and whether the "request, don't fake" rule needs tightening).
- Whether procedural meshes + `DrawHUD` are enough for a feel-good prototype, or if we hit walls that need editor assets.
- Any pain points in input setup. Note: action mappings are now plugin-shipped (`Plugins/Agility/Config/Input.ini`) so a new reader shouldn't have to touch their own project's input config to play the game — but they'll still need to learn the legacy-vs-Enhanced-Input distinction if they build their own controller.

## Part 6 — kollie port (in progress, started 2026-05-17)

**Deliverable:** Replace the prototype game from Parts 1–4 with a 1:1 visual + behavioral port of the kollie OpenGL Flappy Bird (`../kollie/app/src/main/java/com/wizneylabs/kollie/demo/flappybird/`). Same neon palette, same procedurally-built bird (cube-sphere body + cone beak/crest/tail + sphere eyes + flap-pivot wings), same alternating cyan/magenta cube pipes, same procedural background (ground / mountains / stars / moon / moonlight), same Ready→Playing→Dead state machine, same audio (flap / score / hit + Herd The Stars BGM), same HUD layout (yellow score, pink BEST pill, cyan TAP TO FLAP / red GAME OVER banners).

The kollie reference is the source of truth — when in doubt, mirror the kollie file. World units are kollie meters ×100 (UE cm). Axis map: kollie `X` (scroll) → UE `X`, kollie `Y` (vertical) → UE `Z`, kollie `Z` (depth) → UE `Y`.

**Why a full rewrite instead of an incremental polish pass:** Parts 1–3 used cylinder pipes, a sphere+triangle bird with flat shading via `BasicShapeMaterial`, and the procedural-mesh-per-actor pattern. The kollie reference is structurally different — emissive materials + bloom drive the entire visual language, scale-and-paint with static cubes/spheres/cones is faster than hand-coding triangles for every part, and the state machine + alternating-color spawner is cleaner than the timer-only spawner we had. Easier to scrap and rebuild than to migrate piece by piece.

### File map

| File | Role |
| --- | --- |
| `FlappyBirdConstants.h` | All gameplay constants — kollie values ×100 (m → cm). |
| `FlappyNeonMaterials.{h,cpp}` | Loader + per-spec MID factory over `M_AgilityNeonEmissive`. Centralizes the kollie palette. |
| `FlappyBackground.{h,cpp}` | Procedural mountains, sphere stars, sphere moon, plane ground, directional moonlight, plus floor/ceiling kill colliders. Auto-spawned by the game mode. |
| `PipePair.{h,cpp}` | Cube shaft + wider cube cap per column (cyan or magenta); two kill boxes + one score-trigger AABB; scrolls in `-X` and self-destructs past `PipeDespawnX`. |
| `PipeSpawner.{h,cpp}` | Ticks while phase==Playing, spawns one pair per `PipeSpawnInterval` with alternating colors and random gap Z; owns the active-pipe list for restart cleanup. |
| `FlappyBird.{h,cpp}` | Pawn — cube-sphere body + cone beak/crest/tail + sphere eyes/pupils + flap-pivot wings. Idle bob, live flight (gravity / flap / pitch lerp / wing flap), dead-fall ragdoll. World-locked camera with bloom enabled in C++. |
| `FlappyBirdGameMode.{h,cpp}` | State machine (Ready / Playing / Dead), score + best, spawns background + spawner on BeginPlay, BGM autoplay, restart flow. |
| `FlappyBirdHUD.{h,cpp}` | DrawHUD: yellow score top-center, pink BEST pill, cyan TAP TO FLAP / red GAME OVER banner. No UMG assets. |

### Editor work the human owns (one-time, see "Pre-flight handoff" below)

- [ ] **Create `M_AgilityNeonEmissive` material asset** under `Plugins/Agility/Content/Materials/`. Three parameters: `BaseColor` (Vector4), `EmissiveColor` (Vector3), `EmissiveIntensity` (Scalar). Until this exists, every neon mesh renders as the unshaded default and `FlappyNeonMaterials` logs one warning.
- [ ] **Import the audio files** at `Plugins/Agility/Content/Audio/` (4 used by the game: `SFX/space-shooter/sfx_shieldUp.ogg`, `SFX/space-shooter/sfx_twoTone.ogg`, `SFX/space-shooter/sfx_lose.ogg`, `Music/Herd The Stars v2.mp3`). Right-click → Import inside the editor. Set `Looping` on the BGM SoundWave so it actually loops.
- [ ] **Set the level's GameMode override** to `FlappyBirdGameMode` (World Settings panel). The map can otherwise be empty — the game mode spawns the background, pipe spawner, and bird itself.

### What we learned, refined in the port (fill in as snags surface)

- _(placeholder for first PIE run)_

### Open / deferred

- **Best-score persistence** across sessions (current `Best` lives in-memory on the game mode). Easy SaveGame add-on; defer until after the port plays well.
- **Bloom tuning.** Camera-side bloom intensity is parked at `1.0` in C++; kollie used `0.32`, but UE's bloom curve responds differently. Expect to tune by eye on first PIE run.
- **Mountain shading.** Pentagonal cones with flat-shaded face normals look faceted by design (matches kollie screenshots). If they read as too sharp under the moonlight, average normals at shared vertices.
- **Android target.** Game logic is platform-agnostic; mouse-click maps to the same `Flap` action that a touch event would. Touch-input wiring + Android packaging is its own future part.
