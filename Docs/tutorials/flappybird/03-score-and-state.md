---
status: ready
---

# Part 3 — Score, state, and a real game loop

By the end: a full game. A Ready → Playing → Dead state machine drives the experience; the score increments on each pipe-gap pass; pipe / floor / ceiling collisions kill the bird; tapping again restarts. A custom HUD shows the score, a best-ever pill, and TAP TO FLAP / GAME OVER banners. BGM plays throughout; flap / score / hit SFX punctuate.

---

## 1. State machine on the GameMode

> Extend `AFlappyBirdGameMode`. Add an `EFlappyPhase { Ready, Playing, Dead }` enum and a `Phase` member starting at `Ready`. Add `Score` and `Best` int members. Add public methods: `GetPhase()`, `GetScore()`, `GetBest()`, `BeginPlayPhase()` (Ready → Playing, resets Score), `NotifyBirdDied()` (Playing → Dead, updates Best), `NotifyBirdScored()` (++Score), `NotifyBirdHit(APipePair*)` (routes back to the bird's death path — declare `AFlappyBird` a friend so you can call its private `Die()`), and `RequestRestart()` (Dead → Ready, clears pipes via the spawner, calls `Bird->ResetForReady()`). Reference: `Plugins/Agility/Source/Agility/FlappyBird/FlappyBirdGameMode.{h,cpp}`.

## 2. Wire the bird to the phase machine

> Update `AFlappyBird::OnFlapPressed` to switch on `GM->GetPhase()`: Ready → `GM->BeginPlayPhase()` then `StartFlying()`; Playing → `Flap()`; Dead → `GM->RequestRestart()`. Add the bird's own `EBirdState { Idle, Flying, Dead }` machine for the visual animation: idle bob, flying tick (gravity + pitch lerp + wing-flap rate boost on recent flap), dead-fall (ragdoll spin + pinned wings + clamped to just above the floor). The bird's own kill-collider overlap with the background's floor/ceiling triggers `Die()`. Reference the same plugin file.

## 3. The HUD

> Build me `FlappyBirdHUD.{h,cpp}` — an `AHUD` subclass named `AFlappyBirdHUD`. Override `DrawHUD()` to draw via `Canvas`: top-center yellow score number, a pink "BEST n" pill below it on a translucent black background, and a center-screen banner (cyan "TAP TO FLAP" + subtitle in Ready, red "GAME OVER" + subtitle in Dead, hidden in Playing). No UMG assets — purely `FCanvasTextItem` / `FCanvasTileItem`. Use `GEngine->GetLargeFont()`. Reference: `Plugins/Agility/Source/Agility/FlappyBird/FlappyBirdHUD.{h,cpp}`.

> Wire it up by setting `HUDClass = AFlappyBirdHUD::StaticClass()` in the GameMode's constructor.

## 4. Audio

> In the GameMode's `BeginPlay`, start the BGM with `UGameplayStatics::SpawnSound2D` loading `/Agility/Audio/Music/Herd_The_Stars_v2.Herd_The_Stars_v2` at `VolumeMultiplier=0.35`, `bPersistAcrossLevelTransition=true`, `bAutoDestroy=false`. Stop it in `EndPlay`. The BGM's `Looping` is set on the imported SoundWave asset itself (the plugin's shipped `.uasset` has it on) — no per-spawn API call needed.
>
> In the bird's `Flap`, play `/Agility/Audio/SFX/space-shooter/sfx_shieldUp.sfx_shieldUp` at 0.45. In its `Die`, play `sfx_lose.sfx_lose` at 0.65. In the GameMode's `NotifyBirdScored`, play `sfx_twoTone.sfx_twoTone` at 0.55. All via `UGameplayStatics::PlaySound2D`.

## 5. Play (editor step)

Build, hit Play, and you should now have:

- A TAP TO FLAP banner over a falling-but-idle bird, BGM playing.
- First tap kicks off the game; pipes start scrolling; score increments as you pass each gap.
- Hit a pipe or the floor / ceiling → ragdoll fall + GAME OVER banner + best-score updated.
- Tap again → reset to Ready.

That's the full game. Tweak constants in `FlappyBirdConstants.h` to make it harder/easier/weirder; ask Claude to swap the bird's primitives, add particle bursts on death, persist Best across sessions via `USaveGame` — anything you want from here is yours.

---

## What's next

You've built a complete game loop. Browse [`../`](../) for other tutorials, or:

- Read `Plugins/Agility/Source/Agility/MeshLab/` for a Custom-HLSL shader playground.
- Read `Plugins/Agility/Source/Agility/Video/` for runtime video playback.
- Ask `claude-code` to extend the FlappyBird game in any direction — it's read everything you wrote and can riff with you.
