---
status: draft
---

# HLSL Shader Playground + Procedural Geometry — Build Scope

Pre-tutorial planning doc. We exercise the full workflow end-to-end first, then refine each completed part into a numbered tutorial (`NN-….md`) in this folder. Living doc — edit, tick off, and add lessons as we go.

## Goal

End goal: **port a curated set of unlit shaders from the retiring kollie project** (a Kotlin OpenGL + Jetpack Compose renderer) into UE 5.7 as Custom-node-driven materials, and along the way set the plugin up with the procedural-mesh and asset-import workflows we'll use across future experiments.

The kollie shaders use Google Filament's `.mat` format — but fragment bodies are essentially GLSL with thin Filament wrappers (`MaterialInputs`, `materialParams`, `getUV0()`, etc.), so porting is mainly mechanical: `vec*` → `float*`, `mix` → `lerp`, replace Filament accessors with Custom-node inputs. The unlit subset (plasma, kaleidoscope, neon_*, fire, liquid, fractal_glow, voronoi_glow, hypnotic_spiral, …) maps cleanly to a UE unlit material where the Custom node feeds Emissive Color.

The HLSL pipeline only needs trivial geometry (a quad, a cube) to do its job, so the procedural-mesh story splits in two: a small `UProceduralMeshComponent` actor serves as the shader canvas in Part A, and the heavyweight procedural-geometry API — `UDynamicMeshComponent` + Geometry Scripting — lands in Part C as the tool we actually use for future procedural-mesh experiments in the plugin.

## Where the code lives

Reference implementation (inside the plugin): `Source/Agility/MeshLab/`. Sibling to `Procedural/` and `FlappyBird/`. The eventual tutorials will walk a new reader through building their own version under `Source/<YourProjectModule>/MyScripts/MeshLab/` in their host project.

Shader source: `Shaders/MeshLab.ush` at the plugin root (Claude-owned text file; mounted under virtual path `/Plugin/Agility/` by the plugin's `StartupModule()` — see Decision 3). Material `.uasset`s live in the host project's `Content/` (typically `Content/Materials/M_MeshLab_*.uasset`); showcase maps under `Content/MyContent/Maps/` per the user-content convention.

## The arc

Three parts, A → C. We work them in order and only graduate a part to a numbered tutorial once we've exercised it end-to-end and captured lessons.

### Part A — HLSL Shader Playground

**Deliverable:** A single tutorial that introduces the HLSL Custom-node pipeline using two minimal PMC-built canvases (quad and cube). By the end, the reader has authored their own `.ush` shader, wired it into a material, seen per-vertex attributes flow from C++ into HLSL, and ported at least one kollie shader running on a rotating cube. PMC is the canvas — the lesson is the shader pipeline, not mesh topology.

The canvas actor is `AMeshLabSurface` (single evolving class, not one-class-per-stage; matches the flappy-bird "evolve one pawn across parts" precedent). Lives at `Source/Agility/MeshLab/MeshLabSurface.{h,cpp}`. PMC component as root, `OnConstruction` → `Regenerate()`, `EditAnywhere` `Size` parameter, `EditAnywhere` `Material` (`TObjectPtr<UMaterialInterface>`) applied via `Mesh->SetMaterial(0, Material)`.

**Canvas geometry:**

- [x] Single triangle — exercised and verified; visible only from below (back-facing). Taught us UE 5.7's PMC winding convention — see "What we learned" below. Code has since been replaced by the quad in the same actor.
- [x] Parameterized quad with corrected +Z winding and per-vertex RGBA corner colors.
- [ ] Cube — extend the actor to emit 6 quads as a unit cube (24 verts, 12 tris) so we have a 3D demo platform for shaders.
- [ ] `URotatorComponent` — small `UActorComponent` with a tick-based rotation around a configurable axis at a configurable rate. Attach to the cube to spin it for shader demos.

**HLSL pipeline:**

- [x] `Shaders/MeshLab.ush` lives at the plugin root and is mounted via the plugin's `StartupModule()` — `AddShaderSourceDirectoryMapping(TEXT("/Plugin/Agility"), ...)` — so material Custom nodes can reference `/Plugin/Agility/MeshLab.ush`.
- [x] Write a minimal `Shaders/MeshLab.ush` — chose a **pass-through-vertex-color** function (`MeshLab_PassThroughColor(InColor)`) instead of a hardcoded "hello color", so the first test validates the include path *and* the per-vertex color round-trip from C++ → vertex factory → material Custom node → HLSL in one shot.
- [x] Human: create `M_MeshLab_PassThroughHLSL.uasset` in the host project's `Content/Materials/` — Unlit, Custom node with `IncludeFilePaths` = `/Plugin/Agility/MeshLab.ush`, Code = `return MeshLab_PassThroughColor(InColor);`, `VertexColor` wired to `InColor`, output → Emissive Color.
- [x] Per-vertex color round-trip validated: `FLinearColor` from C++ reaches HLSL at full fidelity through the bilinear-decomposable RGB+black quad palette.
- [x] Color-space gamma correction (`MeshLab_DisplayToLinear`) — required for any kollie port that authors in display space.
- [x] **Port kollie's `plasma.mat`** — running animated on the quad as `M_MeshLab_Plasma`.
- [ ] Verify the **liquid** + **fractal-glow** ports (HLSL written, awaiting editor-side material creation — see "Pick up next session").
- [ ] Once the cube + rotator land: re-host the kollie shaders on the rotating cube to confirm the same Custom-node setup works on a non-trivial 3D surface.
- [ ] Pick a small demo that combines per-vertex color + UVs (e.g. vertex-color base hue × kollie pattern as luminance) so the tutorial has one example showing both attribute streams cooperating in HLSL.
- [ ] Document the editor steps for creating an unlit Custom-node material — graduates this part to the tutorial's setup section.

**What we learned:**

- **UE 5.7 PMC winding is CW-from-the-normal-direction**, i.e. opposite of right-hand-rule math intuition. The Part A triangle was written with vertices listed CCW viewed from +Z and triangle indices `(0,1,2)`, expecting a +Z-facing normal — the result was visibly back-faced (only renders from below). **Rule we'll encode in every future PMC actor:** to get a normal facing direction N, list triangle indices such that the math cross product `(V1-V0) × (V2-V0)` points in `-N`. If a face appears invisible from the side you expected, swap two indices in each of its triangles. The quad uses `(0,3,2, 0,2,1)` for this reason.

- **Custom node `.ush` includes belong in `IncludeFilePaths`, not in the `Code` field.** UE's material compiler wraps the contents of the `Code` field inside an auto-generated function (`CustomExpression0(FMaterialPixelParameters Parameters, ...)`), so an `#include` written there expands *inside* a function body — making any function definitions in the `.ush` illegal (`error: function definition is not allowed here`). The right pattern: add the virtual path (e.g. `/Plugin/Agility/MeshLab.ush`) to the Custom node's **Include File Paths** array (a UPROPERTY on `UMaterialExpressionCustom`, verified in 5.7 source at `MaterialExpressionCustom.h:86`); these are emitted at file scope. The Code field then contains only the call site (e.g. `return MeshLab_PassThroughColor(InColor);`). This is *the* idiomatic pattern for the entire experiment going forward — every kollie shader port follows it.

- **`.ush` iteration loop in 5.7 needs an editor restart, not `recompileshaders changed`.** We tried `r.ShaderDevelopmentMode 1` plus `recompileshaders changed`; UE's change-detector did not register `.ush` edits referenced via Custom-node `IncludeFilePaths` ("No Shader changes found"). Apply on the host material from the Material Editor also did not pick up the change in this setup. The only reliable propagation path we found was **fully restarting the editor** — UE re-reads every shader file from disk during launch. This is a real friction tax on iterating kollie ports; if it gets painful, worth investigating: (a) `recompileshaders /Plugin/Agility/MeshLab.ush` explicitly, or (b) a console alias / shortcut to streamline the restart cycle.

- **Linear vs. perceptual color space at the Emissive output.** UE 5 is a linear-light pipeline: vertex factories feed shader code linear-space floats, Emissive Color is treated as scene-linear radiance, and the sRGB framebuffer applies a perceptual gamma curve on output. Classic non-color-managed renderers (kollie's Filament backend, older OpenGL) instead interpolate in display-encoded space directly. Symptom: a 4-corner gradient quad rendered in UE looks "washed out / over-saturated in the middle" compared to the reference, because linear midtone `0.5` displays as `0.735` after the framebuffer's sRGB encode. Fix: `pow(InColor, 2.2)` in HLSL inverse-gammas the value so the framebuffer's forward gamma cancels it back to face value. Captured as `MeshLab_DisplayToLinear(InColor)` in `MeshLab.ush` so every ported shader can use it uniformly — kollie shaders author in display space, so wrap their output in this helper.

### Part B — Scene / lighting / asset import workflow

**Deliverable:** A small showcase scene with our procedural actors, a directional + sky light setup, and at least one human-imported asset (static mesh, texture, or material) referenced from C++. Establishes the import-and-reference workflow newcomers will need before any tutorial that pulls in non-procedural content.

- [x] Asset-access for the "default a UPROPERTY to a known asset" case settled on `ConstructorHelpers::FObjectFinder` (see What we learned). `TSoftObjectPtr` (lazy load) and `AssetRegistry` (discovery / kitbash) trade-offs still open.
- [x] Showcase scene partial — `AMeshLabScene` director with `UDirectionalLightComponent` + `USkyLightComponent` + lit witness static meshes (`/Engine/BasicShapes/Cylinder` and `/Engine/BasicShapes/Cone`) flanking the shader-canvas cube. SkyLight currently renders dark in an empty level; post-process volume not yet wired.
- [ ] Resolve the SkyLight-empty-level issue (`ASkyAtmosphere` actor vs HDRI cubemap source) so the scene has visible ambient.
- [ ] Add a post-process volume to the scene director (or decide it stays human-placed).
- [ ] Human: import one test asset (e.g. a textured static mesh) under the host project's `Content/MyContent/Imported/`.
- [ ] Claude: reference it from C++ via the `FObjectFinder` pattern; spawn it next to the cube + witnesses on `AMeshLabScene`.
- [ ] Spike `AssetRegistry`-based discovery (e.g. `AMeshLabKitbashScene` that scans `/Game/MyContent/Imported/Meshes/` and lays discovered meshes out in a grid) — pressure-tests the director pattern beyond hand-picked content.
- [ ] Document the import → reference workflow (the "newcomers won't trip over assets" goal).
- [ ] Decide whether the `FObjectFinder` + scene-director conventions belong in `CLAUDE.md` once we've also tried `TSoftObjectPtr` and registry discovery.

**What we learned:**

- **Scene-director pattern: direct components on a single actor, not `UChildActorComponent`.** First spike used `UChildActorComponent` to compose a cube actor + light actors under a director — `GetChildActor()` returned null in the director's `OnConstruction` (a known editor-construction-script timing pitfall), so we never set the child cube's `Shape` / `Material` and it rendered as the default quad with no material. The fix that worked is hosting everything as direct components on `AMeshLabScene`: `UProceduralMeshComponent` for procedural geometry, `UStaticMeshComponent` for asset-loaded geometry, `UDirectionalLightComponent` / `USkyLightComponent` for lighting. Side-benefits beyond reliability: every piece is selectable in the Details panel's Components tree (huge debug-ergonomics win when verifying that geometry actually generated), the level outliner stays uncluttered, and there's no inter-actor lifecycle to coordinate. Cost: director-owned components are reset on every reconstruction — fine for Claude-driven scenes; if a piece needs to be permanently human-tweakable in the viewport, place it as a sibling actor instead. (Captured as Decision 6.)

- **`ConstructorHelpers::FObjectFinder` for "default a UPROPERTY to a known asset".** Standard zero-cost-per-instance pattern: `static ConstructorHelpers::FObjectFinder<T> Finder(TEXT("/Path/Asset.Asset"))` in the actor constructor, then `if (Finder.Succeeded()) { Property = Finder.Object; }`. Works uniformly for any `UObject`-derived asset class — proven for `UMaterialInterface` and `UStaticMesh` (engine basic shapes defaulted on the lit witnesses). Path syntax: `/Game/<ContentRelPath>/<Name>.<Name>` for project content, `/Engine/<RelPath>/<Name>.<Name>` for engine-shipped content. Renaming or moving the asset breaks the path silently — mitigation pattern still TBD when we hit imported assets. Open trade-off vs. `TSoftObjectPtr` (lazy load — better when the asset is optional or rarely loaded) and `IAssetRegistry` queries (better for discovery / kitbash where the asset list isn't known at compile time). Note: the plugin itself ships no `/Game/` content references (since `CanContainContent: false`), so any default that points at `/Game/...` content lives on the host-project side, not in plugin source.

- **Engine basic shapes (`/Engine/BasicShapes/{Cube,Cylinder,Cone,Sphere,Plane}.uasset`) are free lighting witnesses.** They ship with default lit materials assigned to their slots, so loading one into a `UStaticMeshComponent` via `FObjectFinder` + `SetStaticMesh` gives instant directional shading with zero material setup. Useful any time a scene includes unlit content (like our shader-canvas cube), because the unlit material can't tell you whether the lighting setup is actually working — drop a witness and the dirlight's effect is obvious at a glance.

- **`USkyLightComponent` in an empty level renders dark.** Default `SourceType = CapturedScene` captures the sky from the component's location at construction; with no sky atmosphere or HDRI in the scene there's nothing to capture, so the ambient contribution stays black. Doesn't matter for unlit shaders (full-bright regardless) but matters as soon as lit content lands. Resolutions to try when we get there: add an `ASkyAtmosphere` actor, or set `SourceType = SpecifiedCubemap` with a default HDRI.

### Part C — Geometry Scripting (the real procedural-mesh API)

**Deliverable:** Get comfortable enough with `UDynamicMeshComponent` + Geometry Scripting to adopt it as **the default procedural-mesh API for future experiments in the plugin**. The reader leaves understanding the `UDynamicMesh` / `FDynamicMesh3` representation, knows how to enable the plugins and wire them into a Build.cs, can generate primitives from C++, can perform at least one topology op that would be painful in raw PMC, and knows how to convert to/from a StaticMesh asset when they want to bake a result.

- [ ] Enable required plugins in the host project's `.uproject` (`GeometryScripting`, `ModelingComponents`, `GeometryProcessing` — verify exact set in 5.7)
- [ ] Add module deps in `Build.cs` (`GeometryScriptingCore`, `GeometryFramework`, `DynamicMesh`, `GeometryCore`, …)
- [ ] Author a small `ADynamicMeshSurface` actor that owns a `UDynamicMeshComponent` and re-creates one of Part A's canvases via a Geometry Scripting primitive (e.g. `AppendBox`) — so the reader can directly compare PMC-author vs. GS-author for the same shape.
- [ ] Demonstrate one high-level op that PMC can't do trivially — pick **one** of: boolean (cube minus sphere), uniform remesh, displace-from-noise, or extrude-along-normals.
- [ ] Show the `CopyMeshToStaticMesh` (or equivalent `MeshAssetFunctions`) round-trip — generate at runtime, bake into a `UStaticMesh` asset for cases where we don't need it dynamic.
- [ ] Capture the trade-offs (when PMC is still the right call, when GS is) so the eventual tutorial gives the reader a clear "use GS unless …" rule.

**What we learned:** _(fill in)_

## Decisions made so far

1. **PMC as shader canvas, GS as the procedural-mesh tool.** Originally framed as "PMC first to teach mesh fundamentals, GS later as power tools." Reframed 2026-05-15: GS (`UDynamicMeshComponent` + Geometry Scripting) is the API we'll actually use for procedural-mesh work in future experiments, so PMC's role narrows to a minimal canvas (quad + cube) for the HLSL Shader Playground. The flat-array mental model still gets a brief moment in Part A, but only as much as the canvas geometry requires — we don't grow PMC coverage beyond that.
2. **HLSL pipeline = Custom node + `.ush` `#include`.** Standard pro UE pattern. Material `.uasset` is human-authored once per effect (in the host project's content); Claude owns the `.ush` shader source as plain text under `Shaders/` in the plugin. For per-vertex effects, route the Custom node into the appropriate material slot (e.g. World Position Offset for per-vertex displacement, since that slot evaluates in the vertex shader).
3. **Shader mounting via the plugin's `StartupModule()`.** Plugin shaders aren't auto-discovered the way a project-root `Shaders/` folder used to be. The plugin module's `StartupModule()` calls `AddShaderSourceDirectoryMapping(TEXT("/Plugin/Agility"), <PluginDir>/Shaders)` so material Custom nodes can reference `/Plugin/Agility/MeshLab.ush`. The Build.cs needs `Projects` (for `IPluginManager` to locate the plugin's base directory) + `RenderCore` (for `AddShaderSourceDirectoryMapping`) in private deps. See `Source/Agility/Agility.cpp`.
4. **Reference code lives in `Source/Agility/MeshLab/`** (inside the plugin). Sibling to `Procedural/` and `FlappyBird/`. Tutorial followers will mirror to `Source/<YourProjectModule>/MyScripts/MeshLab/` in their host project.
5. **Per-vertex color shading needs a vertex-color-reading material.** The flappy-bird workaround (one section per color with a MID) sidesteps per-vertex colors entirely. Part A deliberately validates the proper path: a human-authored material with the `VertexColor` node wired in, then HLSL post-processing on top.
6. **Scene composition uses direct components on a single director actor, not `UChildActorComponent`.** Drop one `AMeshLabScene` (or future analog) in the level; everything in the scene — procedural meshes, asset-loaded meshes, lights, eventually post-process volumes / terrain — hangs off it as direct UPROPERTY components. Validated 2026-05-15 after a `UChildActorComponent`-based first attempt failed in editor construction-script context (`GetChildActor()` returned null right when the director tried to configure spawned children, leaving them at default values). Side-benefits beyond reliability: each piece is selectable in the Details panel's Components tree, the level outliner stays clean, and there's no inter-actor lifecycle to coordinate. Trade-off: director-owned components are reset on every reconstruction — perfect for Claude-driven scenes, less great for things the human wants to permanently tweak in the viewport (those go as sibling actors instead). Likely applies to Part C as well (`UDynamicMeshComponent` is just another component, fits this pattern naturally).

## Open questions

- For Part B: is the chosen asset-access pattern worth promoting to `CLAUDE.md` as a plugin-wide convention? Probably yes, since it's cross-cutting — decide once we've used it.
- Part C plugin set: which exact engine plugins to enable in 5.7. Verify against the engine install when we get there; don't commit to a list now.
- Which subset of kollie shaders is worth porting beyond plasma? Decide once the plasma port lands and we know how mechanical the translation really is. Probably 4–6 favorites, not all 20+.
- Filament `getAspectRatio()` substitute in UE — likely a material parameter we set per-instance, or `ResolvedView.ViewSizeAndInvSize` for fullscreen-style effects. Decide during the plasma port.
- For Part C: which single topology op best showcases GS's power without ballooning the tutorial — boolean, remesh, displace-from-noise, or extrude. Decide when we get there based on which one is most visually striking with the least setup.

## Workflow validation checklist

Things we're explicitly trying to surface while building, so the eventual tutorials don't paper over them:

- Shader iteration loop — how reliable is `recompileshaders changed`, how often does a full editor restart sneak in.
- C++ ↔ HLSL handoff smoothness — can Claude really stay in text-only mode after the one-time material setup, or does each new effect drag the human back into the editor.
- Per-vertex data round-trip — does `FLinearColor` from C++ actually reach HLSL at full fidelity, or does it lose precision somewhere.
- Asset-import → C++-reference workflow — does the chosen pattern stay clean when assets get renamed or moved, or does it bite us.
- PMC → GS switching cost — when we get to Part C, how much of the Part A canvas-actor mental model carries over vs. has to be re-learned.

## Pick up next session

As of 2026-05-15 session end — `AMeshLabScene` director shipped and validated; the HLSL pipeline (Part A) and the scene-composition pipeline (Part B) are both proven end-to-end on a single drop-one-actor scene.

**Validated this session:**

- `AMeshLabScene` scene-director actor: single placement, direct components, drop-one-and-go UX. Hosts `CubeMesh` (PMC), `CylinderWitness` + `ConeWitness` (engine basic-shape static meshes with default lit materials, used to verify dirlight is actually doing something), `SunLight` (`UDirectionalLightComponent`), `SkyLight` (`USkyLightComponent`). `CubeMaterial` is left unset by default — the host project's user wires it up in the editor.
- `AMeshLabSurface` gained an `EMeshLabShape::Cube` mode (24 verts, 12 tris, per-face normals + 0..1 UVs) sharing the quad's CW-from-the-normal-direction winding rule. `BuildQuad` / `BuildCube` are now static helpers reusable from any actor with its own PMC (used by `AMeshLabScene` to build the cube without instantiating an `AMeshLabSurface`).
- `ConstructorHelpers::FObjectFinder` defaults proven for `UStaticMesh` (engine-shipped basic shape). Pattern extends to `UMaterialInterface` for any host-project-side defaulting.
- Lit-witness pattern works: rotating `SunLight` slides the shading on the cylinder + cone visibly, confirming the directional light is wired correctly.

**Known follow-ups (any can be picked up first):**

- SkyLight renders dark in empty level — add an `ASkyAtmosphere` to the scene director, or switch SkyLight's `SourceType` to a default HDRI cubemap.
- Add a post-process volume to the director (or decide it stays human-placed).
- Cube + `URotatorComponent` for spinning the canvas — Part A's last unticked item.
- Continue porting kollie shaders into `MeshLab.ush` (next favorites: kaleidoscope, neon_*, fire — see Part A).
- Spike `AMeshLabKitbashScene` that uses `IAssetRegistry` to scan `/Game/MyContent/Imported/Meshes/` and lay discovered meshes out in a grid — pressure-tests the director pattern beyond hand-picked content and validates the discovery side of the asset-access matrix.
- Document the editor steps for creating a Custom-node material — graduates Part A's HLSL pipeline section to a numbered tutorial.
- Begin Part C: enable Geometry Scripting plugins, port the cube to `UDynamicMeshComponent`, demo one topology op.

**Canonical recipe for adding the next kollie shader port:**

1. Drop the ported HLSL function into `Shaders/MeshLab.ush` (Claude).
2. Duplicate the host project's `M_MeshLab_Plasma` → rename `M_MeshLab_<Name>` → update Custom node's Code field to `return MeshLab_DisplayToLinear(MeshLab_<Name>(UV, Time));` → Apply, Save (human, editor side).
3. Switch `AMeshLabScene`'s `CubeMaterial` (or a standalone `AMeshLabSurface`'s `Material`) to the new asset.

**State of the code (all unstaged):**

- `Shaders/MeshLab.ush` — pass-through, plasma, liquid, fractal-glow functions + `MeshLab_DisplayToLinear` helper.
- `Source/Agility/MeshLab/MeshLabSurface.{h,cpp}` — quad / cube modes, static `BuildQuad` / `BuildCube` helpers, `Material` UPROPERTY, public `Regenerate()`.
- `Source/Agility/MeshLab/MeshLabScene.{h,cpp}` — scene director with cube + lit witnesses + dirlight + skylight; defaults the witnesses to engine basic shapes via `FObjectFinder`. `CubeMaterial` left unset (the host project wires it up in the editor).
- Host project's `Content/Materials/M_MeshLab_{PassThroughHLSL,Plasma,Liquid,FractalGlow}.uasset` — four shader materials authored against the plugin's `/Plugin/Agility/MeshLab.ush` include path.
