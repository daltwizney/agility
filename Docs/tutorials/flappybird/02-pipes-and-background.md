---
status: ready
---

# Part 2 — Pipes and a world

By the end: the bird has a neon-styled world to fly through. Procedural mountains, stars, and a moon set the scene; alternating cyan/magenta pipes scroll past from right to left. No scoring or death yet — that's Part 3.

---

## 1. Neon material helper

The plugin already ships the base material `M_AgilityNeonEmissive` at `/Agility/Materials/`. We just need a small helper to spin up dynamic material instances (MIDs) with per-mesh color + emissive specs.

> Build me `FlappyNeonMaterials.{h,cpp}` in `MyScripts/FlappyBird/`. A `FlappyNeonPalette` namespace exposing `FNeonSpec` structs (BaseColor, EmissiveColor, EmissiveIntensity) for every part of the scene — background (Ground / Mountain / Star / Moon), bird (Body / Beak / Crest / Tail / Wing / EyeWhite / Pupil), and pipes (cyan shaft + cap, magenta shaft + cap). A `FlappyNeonMaterials` namespace with `MakeMID(Outer, Spec)` that loads `/Agility/Materials/M_AgilityNeonEmissive.M_AgilityNeonEmissive`, creates a `UMaterialInstanceDynamic`, and sets the `BaseColor` / `EmissiveColor` / `EmissiveIntensity` parameters from the spec. Log once and return nullptr if the base material is missing. Reference: `Plugins/Agility/Source/Agility/FlappyBird/FlappyNeonMaterials.{h,cpp}`.

> Now update `AFlappyBird::OnConstruction` to use `FlappyNeonMaterials::MakeMID` for each of its mesh parts, applying the right palette entry (Body → `BirdBody`, Beak → `BirdBeak`, etc.). The bird should look unmistakably neon next time you play.

## 2. The background

> Build me `FlappyBackground.{h,cpp}` — an `AActor` named `AFlappyBackground` in `MyScripts/FlappyBird/`. Procedurally builds: a row of pentagonal-cone mountains across the back, sphere stars scattered across the upper sky, a sphere moon, a flat ground plane, a directional moonlight, and two invisible kill colliders at `WorldFloorZ` / `WorldCeilingZ` for death detection. Each mesh uses its `FlappyNeonPalette` spec via `FlappyNeonMaterials::MakeMID`. Reference: `Plugins/Agility/Source/Agility/FlappyBird/FlappyBackground.{h,cpp}`.

> Then update `AFlappyBirdGameMode` to spawn one `AFlappyBackground` at world origin in `BeginPlay`. Expose `BackgroundClass` as a `UPROPERTY(EditAnywhere) TSubclassOf<AFlappyBackground>` so a subclass can swap it.

## 3. The pipes

> Build me `PipePair.{h,cpp}` — an `AActor` named `APipePair`. Two cube columns (one above the gap, one below) built from `/Engine/BasicShapes/Cube`, each column = a tall shaft cube + a slightly wider cap cube at the gap-facing end. Color is set from a constructor-time `EPipeColor { Cyan, Magenta }` enum (use the matching `PipeShaft*` / `PipeCap*` palette specs). Add two `UBoxComponent` kill boxes (one per column) and one `UBoxComponent` score-trigger spanning the gap. Scrolls in `-X` at `PipeScrollSpeed` and `Destroy()`s itself past `PipeDespawnX`. The score box fires `GM->NotifyBirdScored()` on the GameMode (guarded by a `bScored` flag to avoid double-counting); kill boxes fire `GM->NotifyBirdHit(this)`. Reference: `Plugins/Agility/Source/Agility/FlappyBird/PipePair.{h,cpp}`.

> And `PipeSpawner.{h,cpp}` — an `AActor` named `APipeSpawner`. Ticks while the GameMode phase is `Playing` and spawns one `APipePair` every `PipeSpawnInterval`. Each spawn alternates color (cyan ↔ magenta) and picks a random gap Z within `±PipeGapZRange`. Tracks active pipes in a list so `ClearActivePipes()` can wipe them on restart. An `InitialGraceDelay` defers the first spawn so the player gets a moment to settle in. Reference: `Plugins/Agility/Source/Agility/FlappyBird/PipeSpawner.{h,cpp}`.

> Then have the GameMode spawn one `APipeSpawner` on `BeginPlay` (same pattern as the background; expose `PipeSpawnerClass` as a `UPROPERTY`).

## 4. Play (editor step)

Build the editor, hit Play. You should see:

- The bird against a neon-purple sky with mountains, stars, and a moon.
- Pipes scrolling in from the right after a short delay.
- The bird passing harmlessly through pipes (no collisions are wired yet — that's next).

---

## What's next

[`03-score-and-state.md`](./03-score-and-state.md) — make it a real game with score, death, restart, HUD, and BGM.
