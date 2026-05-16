---
status: ready
---

# Hello Unreal

Build three procedural Unreal Engine 5 actors — a Perlin-noise voxel island, a spiral galaxy, and a recursive fractal tree — using `claude-code` as your pair-programming partner.

By the end you'll have:

- The Agility plugin integrated into your UE project and building cleanly.
- Three procedural actors visible in the editor, each parameter-tweakable in real time.
- A working feel for the human/`claude-code` split: Claude drafts and explains the C++ (and you're encouraged to read it, ask questions, and tweak it); you drive the editor and judge what you see.

Plan ~1–2 hours; most of it is downloads and the first compile.

---

## 1. Prerequisites

You'll need:

- A UE C++ project with the Agility plugin integrated. If you haven't done that yet, follow [`../development-setup.md`](../development-setup.md) first — it walks through cloning the plugin, symlinking it into your project, and building the editor with it enabled.
- `claude-code` installed and ready to run from your project root.

Once your editor opens with the Agility plugin loaded (Edit → Plugins → search "Agility"), you're ready to start.

---

## 2. Where your work goes

The C++ classes inside the Agility plugin — `Plugins/Agility/Source/Agility/Procedural/` and similar — are **reference material**. Don't edit them. Your own work goes in your project's source tree so a future `git pull` on the plugin won't conflict with your practice:

| Your work goes in...                       | What lives there                       |
|--------------------------------------------|----------------------------------------|
| `Content/MyContent/`                       | Your maps, materials, blueprints       |
| `Source/<YourProjectModule>/MyScripts/`    | Your C++ classes                       |

(Replace `<YourProjectModule>` with whatever your project's primary game module is called — it's the folder under `Source/` that holds your existing `.cpp` / `.h` files.)

Organize inside those however you like (`MyContent/Maps/`, `MyContent/Materials/`, etc.).

> **One UE quirk worth knowing about C++ includes.** If your project module uses a flat layout (no `Public/` / `Private/` split), then when a `.cpp` in `MyScripts/` includes its sibling `.h` from the same folder, write `#include "MyHeader.h"` — *not* `#include "MyScripts/MyHeader.h"`. Only the source file's own directory is on the implicit include path with this layout.

---

## 3. The walkthrough

Agility's design philosophy is **don't watch videos, talk to `claude-code`**. So this section is a sequence of prompts you give your own `claude-code` session — Claude writes the C++ and walks you through it, you do the editor work. If anything Claude produces looks unfamiliar (a macro, an include, a component type, a build-system line), stop and ask "what does this do and why?" — getting comfortable reading UE5 C++ is half the point of this tutorial.

Open a terminal in your project root and start `claude` (or your `claude-code` equivalent).

### Prime the session

> I'm following the Hello Unreal tutorial from the Agility plugin. Read `Plugins/Agility/CLAUDE.md` and `Plugins/Agility/Docs/tutorials/01-hello-unreal.md`. Place all my new C++ in `Source/<YourProjectModule>/MyScripts/` and any new maps/assets in `Content/MyContent/`. Don't modify anything under `Plugins/Agility/` — that's reference material.

(Substitute `<YourProjectModule>` with your actual project's primary game module folder name.)

This anchors the conversation in the tutorial's conventions.

### Part 1 — Procedural voxel terrain

> Build me a procedural voxel-style terrain actor in C++. Use a `UHierarchicalInstancedStaticMeshComponent`, the engine's built-in cube mesh (`/Engine/BasicShapes/Cube.Cube`), and Perlin noise to drive per-cell height. Expose grid size, cell size, noise scale, height scale, octaves, persistence, lacunarity, seed, and a "stack vertically" toggle as editable properties. Rebuild the geometry whenever I change a property in the editor (override `OnConstruction`). If you want a reference, look at `Plugins/Agility/Source/Agility/Procedural/ProceduralVoxelTerrain.{h,cpp}` — but write a fresh version in `MyScripts/`, don't copy.

When the C++ compiles: in the editor, create a new map under `Content/MyContent/Maps/` (e.g. `MySandbox.umap`), drag the actor in at world origin, and tweak parameters in the Details panel. The terrain should regenerate live as you change values.

### Part 2 — Procedural spiral galaxy

> Now build me a procedural spiral galaxy actor in `MyScripts/`. Same scaffold — HISMC + the engine cube + `OnConstruction`. Place stars along N spiral arms by walking a parameter `t` from 0 to 1 with `radius = t * DiskRadius` and `theta = arm_base + t * SpiralTurns * 2π`. Apply a power-distribution `CoreBias` to bias stars toward the center, add perpendicular jitter to soften the arms, and lerp star size from `StarSizeMax` near the core to `StarSizeMin` at the rim. Reference: `Plugins/Agility/Source/Agility/Procedural/ProceduralSpiralGalaxy.{h,cpp}`.

Place it floating above the terrain (e.g. `Z = 1500`). The spiral structure reads best from a top-down ortho view (*Perspective dropdown → Top*).

### Part 3 — Recursive fractal tree

> Now a procedural fractal tree, also in `MyScripts/`. This one's structurally different — use `UProceduralMeshComponent` and build the mesh geometry yourself (vertices, triangles, normals, UVs, vertex colors). A recursive `Branch(start, dir, length, radius, depth)` emits a tapered cylinder for each segment, then spawns N children rotated `BranchAngle` off the parent direction with random azimuth jitter. Vertex-color the cylinders depth-lerped from brown (trunk) to green (tips). You'll need to add `ProceduralMeshComponent` to your project module's `Build.cs`, and rebuild from your IDE afterward — Live Coding can't pick up Build.cs changes. Reference: `Plugins/Agility/Source/Agility/Procedural/ProceduralFractalTree.{h,cpp}`.

Drop the tree in your sandbox map. Try `MaxDepth = 3, 6, 8`; vary `BranchesPerNode`, `BranchAngle`, `AngleJitter`. The default `WorldGridMaterial` won't display the brown→green vertex colors the mesh writes; if you want to see them, your editor task is to make a one-node material (`VertexColor` → `BaseColor`) and assign it to the mesh's material slot.

---

## 4. What's next

You've now seen:

- C++ procedural mesh generation (HISMC + `UProceduralMeshComponent`).
- The tweak-and-rebuild loop driven by `OnConstruction`.
- The human/`claude-code` handoff pattern: Claude writes and walks you through the C++ (which you're encouraged to read and tweak); you drive the editor and judge what you see.

More tutorials live in [`../`](../) — start with the index in [`README.md`](../README.md). And the reference actors under `Plugins/Agility/Source/Agility/` are fair game to read, dissect, and ask Claude questions about as you go.
