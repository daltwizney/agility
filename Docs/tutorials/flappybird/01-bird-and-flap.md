---
status: ready
---

# Part 1 — A bird that flaps

By the end: a procedural bird falls under gravity in an otherwise empty scene, and pressing Space (or clicking LMB) makes it flap upward. A world-locked side-on camera frames the playfield so the bird moves up/down within the frame.

---

## 1. Create the map (editor step)

Create an empty map under `Content/MyContent/Maps/FlappyBird.umap`. *File → New Level → Empty Level → Save As...*. We'll set its GameMode override once the C++ exists at the end of this part.

## 2. Prime claude-code

In a terminal at your project root, start `claude` and prime the session:

> I'm following the FlappyBird tutorial from the Agility plugin (`Plugins/Agility/Docs/tutorials/flappybird/`). Read `Plugins/Agility/CLAUDE.md` and the tutorial's `README.md` for context. Place all my new C++ in `Source/<YourProjectModule>/MyScripts/FlappyBird/`. Don't modify anything under `Plugins/Agility/` — that's reference material.

(Substitute `<YourProjectModule>` with your project's primary game module folder.)

## 3. Constants

> Build me `FlappyBirdConstants.h` in `MyScripts/FlappyBird/`. Just a header with a `FlappyBirdConstants` namespace holding world bounds, bird physics (gravity, flap velocity, max fall speed, pitch up/down), pipe layout, and camera parameters as `constexpr float`s. Mirror the plugin's `Plugins/Agility/Source/Agility/FlappyBird/FlappyBirdConstants.h` — same names and values. World units are Unreal cm; the kollie → UE axis mapping is documented at the top of that file.

## 4. The bird pawn

> Now build me `FlappyBird.{h,cpp}` — an `APawn` named `AFlappyBird` in `MyScripts/FlappyBird/`. Procedural bird from `/Engine/BasicShapes` primitives (sphere body / cone beak / cone crest / cone tail / sphere eyes + pupils / cube wings on rotating pivots). Gravity each `Tick` (clamp fall to `MaxFallSpeed`), flap impulse on input. World-locked `UCameraComponent` (use `SetUsingAbsoluteLocation(true)` + `SetUsingAbsoluteRotation(true)` so the camera stays put while the bird moves vertically within the frame). `AutoPossessPlayer = EAutoReceiveInput::Player0`. Reference: `Plugins/Agility/Source/Agility/FlappyBird/FlappyBird.{h,cpp}`.
>
> For input, bind the legacy action name `"Agility.FlappyBird.Flap"` in `SetupPlayerInputComponent`. **Don't add anything to my project's `Config/DefaultInput.ini`** — the plugin ships the action mapping (SpaceBar + LMB) in `Plugins/Agility/Config/Input.ini`, which UE merges into the project's input settings automatically.

Three callouts worth knowing once and forever, that Claude will hit in this step:

- **`OnConstruction` vs constructor.** Component allocation (`CreateDefaultSubobject`) happens in the constructor and runs once per class. Per-instance setup that should re-run on editor-time tweaks (mesh assignment, transforms, MID creation) goes in `OnConstruction`.
- **Loading `/Engine/BasicShapes` meshes.** Use `LoadObject<UStaticMesh>(...)` from `OnConstruction`, not `ConstructorHelpers::FObjectFinder` (which asserts it's only used in constructors).
- **Why the plugin's input file is `Input.ini`, not `DefaultInput.ini`.** UE's plugin-modification config layer pulls `{PLUGIN}/Config/{TYPE}.ini` into the project's existing config branches. A `Default` prefix would route to a plugin-private namespace that `UInputSettings` never reads — the action mapping would silently never bind. (Ask Claude to walk you through `Engine/Source/Runtime/Core/Public/Misc/ConfigHierarchy.h` if you want to see the layer table.)

## 5. Minimal GameMode

> Build me a stub `FlappyBirdGameMode.{h,cpp}` subclassing `AGameModeBase`. For now just set `DefaultPawnClass = AFlappyBird::StaticClass()` in the constructor — we'll grow this in Part 3.

Heads-up: **Live Coding (Ctrl+Alt+F11) cannot bootstrap newly-added `.h`/`.cpp` files.** A full IDE build is needed first; after that, Live Coding handles incremental edits fine.

## 6. Build + play (editor step)

Build the editor target from your IDE. Then in the editor:

1. Open `Content/MyContent/Maps/FlappyBird.umap`.
2. *Window → World Settings*. Set **GameMode Override** to your `FlappyBirdGameMode`.
3. Hit Play. The bird should fall under gravity, and pressing Space (or clicking inside the viewport) should give it an upward flap.

If something's off, tell Claude what you see — a one-line description ("the bird falls but Space does nothing") is plenty to start debugging from.

---

## What's next

[`02-pipes-and-background.md`](./02-pipes-and-background.md) — give the bird a world to fly through.
