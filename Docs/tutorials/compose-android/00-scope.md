---
status: draft
---

# Jetpack Compose UI over Unreal on Android — Build Scope

Pre-tutorial planning doc. We exercise the workflow end-to-end first, then refine each completed phase into a numbered tutorial (`NN-….md`) in this folder. Living doc — edit, tick off, and add lessons as we go.

## Goal

End goal: **let an Android dev build app UI in Jetpack Compose, overlaid on top of Unreal's rendered surface inside the same `GameActivity`.** Unreal owns the 3D scene; Compose owns all 2D UI. The dev writes `@Composable` functions (a skill they already have) instead of UMG.

This is **Android-only by design.** It is not expected to work in PIE or on macOS builds — the C++ side compiles to no-ops off Android, and the gradle/Kotlin/Compose machinery only exists in the Android packaging path. macOS UI is explicitly out of scope for now; Compose Multiplatform on macOS is a possible far-future direction, not a current goal.

The reusable artifact is the **bridge + overlay plumbing**: gradle wiring that adds Kotlin + Compose to Unreal's Android build, a Compose host that satisfies Compose's view-tree requirements inside a `NativeActivity`, and a C++ ↔ Kotlin JNI bridge so UE logic and Compose UI can talk. The dummy counter is just the first thing we render through it.

## Why this is non-trivial (the load-bearing facts)

- **UE Android runs in `GameActivity`, a `NativeActivity` subclass**, rendering to the window surface via a native `ANativeWindow`. There is no Android view hierarchy we author — UE generates `GameActivity.java` and the gradle project at package time.
- **UPL (Unreal Plugin Language)** is the only injection point. An XML file attached from `Agility.Build.cs` lets us add gradle dependencies/plugins, copy Kotlin sources into the generated project, and inject Java into `GameActivity`'s lifecycle methods. Reference examples in the engine: `Engine/Plugins/Runtime/Firebase/Source/Firebase.upl.xml` (uses `buildscriptGradleAdditions` + copies), and the many `*_UPL.xml` under `Engine/Plugins` for `gameActivityOn*Additions`.
- **`ComposeView` requires `ViewTreeLifecycleOwner`, `ViewTreeViewModelStoreOwner`, and `ViewTreeSavedStateRegistryOwner` on its view tree** or it throws on attach. `NativeActivity` / `GameActivity` implement **none** of these. This is the single biggest gotcha — it's why a dedicated Kotlin host class (carrying its own owner implementations) is worth it instead of inlining a `ComposeView` into the generated Java.
- **Engine toolchain (UE 5.7, verified against this install):** Gradle 8.7, AGP 8.6.1, Java 17, no Kotlin present by default. We add Kotlin 2.0.21 + the `org.jetbrains.kotlin.plugin.compose` gradle plugin (Compose compiler is a standalone plugin since Kotlin 2.0 — no `kotlinCompilerExtensionVersion` needed). All version numbers below are first-build candidates and may need bumping; validate on the first device package.

## Where the code lives

Reference implementation (inside the plugin), sibling to `MeshLab/`, `FlappyBird/`, `Procedural/`:

- `Source/Agility/Compose/AgilityComposeOverlay.{h,cpp}` — C++ JNI bridge. All Android code behind `#if PLATFORM_ANDROID`; compiles to no-ops elsewhere so macOS/editor builds are unaffected.
- `Source/Agility/Compose/AgilityComposeCounterActor.{h,cpp}` — the actor that *owns* the counter state (a `UPROPERTY int32 Counter`). Rotates a cube mesh by the counter so the state change is visible in the 3D scene.
- `Source/Agility/Compose/AgilityComposeWorldSubsystem.{h,cpp}` — spawns the counter actor on world begin-play, so no editor placement or game-mode wiring is needed.
- `Source/Agility/Android/Agility_UPL_Android.xml` — UPL: gradle additions + `GameActivity` lifecycle injections + copies the Kotlin sources into the build.
- `Source/Agility/Android/java/com/wizneylabs/agility/AgilityComposeHost.kt` — the Compose host: owner machinery, transparent `ComposeView` overlay, lifecycle forwarding, `native` fun + `pushCounter` for the bridge.
- `Source/Agility/Android/java/com/wizneylabs/agility/AgilityComposeUI.kt` — the `@Composable` dummy counter.

`Agility.Build.cs` attaches the UPL only when `Target.Platform == UnrealTargetPlatform.Android`.

Kotlin package is `com.wizneylabs.agility` (independent of the app's `com.wizneylabs.collie` namespace — the JNI symbol names are mangled from this package, so it must match the C++ `extern "C"` function names exactly).

Tutorial followers will eventually mirror this into their host project under `Source/<YourProjectModule>/MyScripts/` — but note the UPL + gradle wiring is plugin-level infrastructure they mostly inherit, not re-author.

## The arc

### Phase 1 — Overlay + round-trip bridge (the dummy counter)

**Deliverable:** On an Android device, a Compose card overlaid on the live UE scene showing a number with `+` / `−` buttons. Pressing a button calls into UE C++ over JNI; the delta is marshalled onto the game thread and applied to a **real UE actor** (`AAgilityComposeCounterActor`) that owns the counter, rotates a cube to make the change visible, and pushes the new value back into Compose, which recomposes. This proves *all* the hard parts at once: gradle wiring, the lifecycle-owner host, transparent z-order over the 3D scene, both directions of the bridge, **and** that Kotlin can mutate genuine UObject/actor state (not just a C++ global).

- [ ] `Agility.Build.cs` — attach `Agility_UPL_Android.xml` via `AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", …))`, Android-only.
- [ ] `Agility_UPL_Android.xml`:
  - [ ] `buildscriptGradleAdditions` — Kotlin + compose-compiler gradle plugin classpaths.
  - [ ] `buildGradleAdditions` — apply `kotlin.android` + `kotlin.plugin.compose`, `buildFeatures { compose true }`, `kotlinOptions { jvmTarget = '17' }`, Compose BOM + `ui` + `material3` + `activity-compose` + lifecycle/savedstate deps.
  - [ ] `prebuildCopies` — `copyDir` the Kotlin tree into `$S(BuildDir)/src`.
  - [ ] `gameActivityImportAdditions` + `gameActivityOnCreateAdditions` (`AgilityComposeHost.attach(this)`) + on-resume/pause/destroy forwarding.
- [ ] `AgilityComposeHost.kt` — owners (Lifecycle/ViewModelStore/SavedStateRegistry), transparent full-screen `ComposeView` via `addContentView`, set the three `ViewTree*Owner`s on it, `@JvmStatic external fun nativeOnCounterDelta(Int)`, `@JvmStatic fun pushCounter(Int)` (posts to main looper → updates `mutableStateOf`), lifecycle forwarders.
- [ ] `AgilityComposeUI.kt` — `@Composable` counter card (Text + `+`/`−` Buttons), reads the host's counter state, calls `nativeOnCounterDelta` on click.
- [ ] `AgilityComposeOverlay.{h,cpp}` — `extern "C"` `Java_com_wizneylabs_agility_AgilityComposeHost_nativeOnCounterDelta`, owns the counter int, logs via `UE_LOG`, calls back `pushCounter` (static method ID resolved from the passed `jclass`); also cache a global ref so a future game-thread `SetCounter` path works.
- [ ] First device package — resolve whatever gradle/version friction surfaces (this is where the version candidates get validated).
- [ ] Confirm (human, on device): overlay renders on top of the 3D scene, buttons round-trip through C++, value updates.

### Phase 2 — Make it a real feature surface (post-test)

Once Phase 1 is green, the open design work:

- [ ] **Touch pass-through.** A full-screen `ComposeView` consumes touches in its bounds, so the 3D scene below stops receiving input. Decide the model: size the overlay to just the UI region, use a transparent/`pointerInteropFilter` pass-through outside interactive composables, or a hit-testing strategy. (Phase 1 deliberately ignores this — the counter card eating the whole screen's touches is acceptable for the test.)
- [ ] **Spontaneous (non-button) game-thread → Compose pushes.** Phase 1 already pushes from the game thread (the actor calls `PushCounter` after applying a delta), but always in response to a button. Validate a fully UE-initiated push (e.g. from tick or gameplay) so UE can drive the UI without a user tap kicking it off — the binding is currently cached lazily on the first tap, so a tap-free path needs the binding resolved another way.
- [ ] **A clean C++ API surface.** The actor is a start. Decide what general UE-side code calls to drive arbitrary overlay state — a `UGameInstanceSubsystem`? a Blueprint function library? — rather than the bridge's counter-specific functions.
- [ ] **Compose state ownership model.** Where does UI state live — Compose-side, UE-side, or a shared model — and how do we keep it from getting gnarly as real screens appear.
- [ ] **Rotation / config-change / app-background behavior.** Confirm the overlay survives (or is correctly recreated through) the lifecycle we're forwarding.

## Decisions made so far

1. **No separate UE module, no committed AAR.** Android code stays in the existing `Agility` runtime module behind `#if PLATFORM_ANDROID`; Kotlin ships as source copied by UPL. Iterating Compose UI is just editing `.kt` files, and nothing binary lands in the public repo (fits the public-repo safety rules — no committed `.bin`/AAR blobs).
2. **A dedicated Kotlin host class owns the view-tree owners**, rather than making `GameActivity` implement the AndroidX owner interfaces via UPL class injection. The host is self-contained, testable, and keeps the generated-Java injection to a few one-line calls.
3. **Phase 1 includes the full JNI round-trip** (Compose → C++ → Compose), not just a pure-Compose counter. The bridge is the actually-novel reusable piece, so we prove it in the first milestone.
4. **Kotlin package `com.wizneylabs.agility`**, decoupled from the app namespace, because JNI symbol mangling ties the C++ `extern "C"` names to the Kotlin class FQN.
5. **The counter lives on a UE actor, not a C++ global** (decided 2026-05-30, mid-Phase-1). The bridge holds a `TWeakObjectPtr` to the active `AAgilityComposeCounterActor`; the JNI callback routes deltas to it. This makes the demo prove the real goal — Kotlin mutating live actor state — and gives a natural home for the C++-side API (vs. the free-function/global stopgap the first cut used).
6. **The actor is spawned by a `UWorldSubsystem`, not placed in the editor or wired via a custom GameMode.** A world subsystem auto-runs in every game world (PIE + packaged Android) with zero config; a GameMode would need to be set as the default via project settings / World Settings. Keeps the demo drop-in.

## Open questions

- Exact dependency versions (Kotlin 2.0.21, Compose BOM 2024.09.00, activity-compose 1.9.x, lifecycle 2.8.x) — candidates only; the first device build is the arbiter.
- Does Compose's lib loading conflict with UE's `.so` loading order, or does lazy JNI symbol resolution at first `native` call (UE lib already loaded before `onCreate`) just work? Validate on first build.
- Whether `prebuildCopies` → `$S(BuildDir)/src` is the right copy target (matches Firebase's Java copy) vs. a dedicated kotlin srcDir — confirm the generated `app/build.gradle` actually compiles `.kt` from there.
- Touch pass-through approach (Phase 2) — biggest UX unknown for turning this into a real surface.
- Long-term: is Compose Multiplatform on macOS worth the abstraction, or do we keep UI Android-only and use something else (UMG / Slate) on desktop?

## What we learned

- **A Kotlin-only change does NOT propagate into the APK on an incremental package — UE keeps reusing the stale copy.** UE's Android packaging only regenerates the gradle project (and re-runs the `prebuildCopies` that copy our `.kt` into it) when it detects a tracked change — a UPL edit, a manifest change, etc. A plain content edit to a UPL-copied source file is *not* in that input set, so the intermediate keeps the old `.kt` while C++ still rebuilds normally (UBT tracks `.cpp`/`.h` independently). Symptom: C++ behavior updates but the Compose UI stays on the previous version. Confirmed by the stale copies under `Intermediate/Android/arm64/{src,gradle/app/src/main/java}/com/wizneylabs/agility/` keeping an old timestamp after a redeploy. `copyDir` already force-copies (`force` defaults true) — the issue is the *step* not running, not the copy skipping, so `force` doesn't help. **Reliable fix when only Kotlin changed: `rm -rf Intermediate/Android` before packaging** (cheap — that dir is just gradle staging; C++ objects live under `Intermediate/Build/Android/` and are untouched, so no full native rebuild). Editing the UPL also forces it (`Active UPL files changed, forcing clean`). This is the Compose-side analogue of the `.ush`-needs-an-editor-restart friction in the proceduralmesh experiment — UE's change detection for non-primary files is unreliable.

- **The JNI callback arrives on the Android UI thread, so touching a UObject from it is a data race.** `nativeOnCounterDelta` runs on whatever thread fired the Compose `onClick` (the Android main/UI thread), *not* UE's game thread. UObjects/actors are game-thread-only, so the delta is marshalled with `AsyncTask(ENamedThreads::GameThread, …)` before the actor is touched. The first cut mutated a free `int32` global directly from the callback — it appeared to work but was a latent race (UI-thread write vs. game-thread read). Moving the counter onto an actor forced doing this correctly. The reverse direction is fine the other way around: `pushCounter` is called from the game thread and the Kotlin side re-posts to the UI looper before updating Compose state.

- **`$S(PluginDir)` in UPL is the directory that contains the UPL file, not the plugin root.** Our UPL lives at `Source/Agility/Android/Agility_UPL_Android.xml`, so `$S(PluginDir)` = `.../Plugins/Agility/Source/Agility/Android`. The first `prebuildCopies` used `$S(PluginDir)/Source/Agility/Android/java`, which doubled the path (`.../Android/Source/Agility/Android/java`) — the copy logged `(True)` but silently copied nothing because the source didn't exist, so `GameActivity.java`'s injected `import com.wizneylabs.agility.AgilityComposeHost;` failed with *"package com.wizneylabs.agility does not exist"* at `compileDebugJavaWithJavac` (and `compileDebugKotlin` reported `NO-SOURCE`). Fix: reference the `java` dir relative to the UPL's own location (`$S(PluginDir)/java`). Confirm the resolved value in the build log — every plugin dumps its `PluginDir` during `UPL Init`. Engine examples like Firebase look like they use the plugin root only because their `.upl.xml` sits directly in `Source/`.
