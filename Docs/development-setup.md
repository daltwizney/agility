# Development Setup

This guide walks you through adding Agility to an existing Unreal Engine project so you can build with the plugin enabled and follow the tutorials in [`tutorials/`](./tutorials/).

Agility is **mostly source** — C++, HLSL, and a minimal bundled `Content/` (raw video files plus a small handful of base-material `.uasset`s that C++ wraps at runtime) — and ships as a standalone git repo. Shipping as a plugin (rather than as a UE project) is deliberate: it lets you keep Agility cleanly separated from your own work, so anything you've licensed from Fab, anything proprietary, anything you're not ready to publish stays in *your* UE project, while the public plugin repo stays free of it.

There are two supported ways to integrate it into a UE project:

- **Git submodule (recommended).** The plugin lives inside your project at `Plugins/Agility/` as a git submodule, pinned to a specific commit. Clean separation, per-project version pinning, and plugin file changes can never accidentally get staged in your project repo — the parent only ever records a SHA pointer, never the plugin's file contents. Works equally well whether you're just consuming the plugin or actively co-developing it (you can `cd` into the submodule, commit, and push upstream like any other repo).
- **Symlink (alternative).** Clone the plugin once on your machine and symlink it into each UE project. Useful when you have several UE projects that should all track the same in-flight plugin checkout without per-project SHA pinning — see [Alternative: symlink instead of submodule](#alternative-symlink-instead-of-submodule) at the bottom of this guide.

The rest of this guide assumes the submodule path.

> **A note on the `python/` directory.** The Agility repo also contains a Python-side research workspace at [`python/`](../python/) — sibling to the plugin, home of computer-vision experiments and an optional web server UE can talk to as a client. The UE editor doesn't load anything from there, so the submodule install brings it along and the editor ignores it. If you don't plan to run the Python side, you can ignore it too. Setup for the Python side is independent of the plugin build and is documented in [`python/README.md`](../python/README.md).

## Prerequisites

1. **`claude-code`** — the CLI that drives every tutorial in this repo. Install it by following [Anthropic's instructions](https://docs.claude.com/en/docs/claude-code). The plugin's [`CLAUDE.md`](../CLAUDE.md) is auto-loaded by `claude-code` when you launch a session in a directory that references it, so the project's conventions (the human/AI split, where your own work goes, public-repo safety rules) kick in from turn one — see Step 3 below for how to wire it into your own project.
2. **A UE C++ project under git.** Agility is a C++ plugin, so you need a project that already builds C++ (i.e. created with a "C++" template, or already has a `Source/` directory and a working `.uproject` with at least one module). A Blueprints-only project will need to be converted to C++ first (UE editor → Tools → "New C++ Class…" creates the Source tree). Submodules also require the parent project to be a git repo — if you haven't run `git init` in it yet, do that first.
3. **Unreal Engine** — installed via the Epic Games Launcher. The plugin has been developed against **UE 5.7** and should work on adjacent versions. Confirm your project's engine version via the `EngineAssociation` field in your project's `.uproject`.
   - **Install the target-platform components you'll build for, up front.** In the Epic Games Launcher → *Unreal Engine → Library*, click the dropdown (⌄) on your engine version's tile → *Options*, and check the target platforms you want under **Target Platforms** (e.g. **Android**) before clicking *Apply*. These ship the per-platform build templates the engine copies into your project — without them, *Project Settings → Platforms → Android → Configure Now* fails with `Could not overwrite Project Properties file`, because the source template at `Engine/Build/Android/Java/project.properties` doesn't exist. Adding **Android** here lets you configure Android packaging and build Android targets even while developing in-editor on macOS. (The Android device toolchain — Android Studio / NDK — is a separate setup step covered in [Building for Android devices](#building-for-android-devices) below, only needed when you actually package and deploy to a device.)
4. **A C++ IDE for UE development** — on macOS we recommend [**JetBrains Rider**](https://www.jetbrains.com/rider/) (built-in UE support, no Epic-Launcher project-file generation needed). On Windows, Visual Studio (2022+) works the same way through its UE integration. Pick whichever is comfortable on your platform.
5. **Platform toolchain** — on macOS, the full Xcode SDK plus accepting the license is required for the underlying toolchain even if you don't open Xcode (`xcode-select --install` + `sudo xcodebuild -license accept`). On Windows, the Visual Studio "Game development with C++" workload covers it.

## Step 1 — Add the plugin as a submodule

From your UE project's root:

```bash
git submodule add <agility-repo-url> Plugins/Agility
```

This clones the plugin into `Plugins/Agility/`, creates a `.gitmodules` entry, and stages both the `.gitmodules` file and the gitlink pointer to the plugin's current commit. Commit the result in your project repo:

```bash
git commit -m "Add Agility plugin as submodule"
```

That's it for the install — `Plugins/Agility/` is now a real, independent checkout of the plugin's git repo, pinned to whichever commit was current when you added it.

### Cloning a project that already has the submodule

Anyone else (or you on a fresh machine) picks up the plugin with either:

```bash
git clone --recurse-submodules <your-project-url>
# or, after a plain clone:
git submodule update --init --recursive
```

Optional one-time global config so future `git pull` / `git checkout` in the parent auto-update the submodule to match the recorded SHA:

```bash
git config --global submodule.recurse true
```

This is pure git — no extra extensions or tools required beyond a standard `git` install.

### Updating the pinned plugin version

When you want your project to track a newer plugin commit:

```bash
cd Plugins/Agility
git fetch
git checkout <branch-or-commit>    # e.g. origin/main, or a specific SHA
cd ../..
git add Plugins/Agility            # bumps the recorded SHA in the parent
git commit -m "Bump Agility to <version>"
```

The parent's `git status` will show `Plugins/Agility` as `(new commits)` whenever the submodule's HEAD differs from the SHA the parent has pinned — that's how you know there's an unrecorded version change.

> **`git add .` in the parent is safe for plugin file contents.** It will never stage file-level changes from inside the submodule — those belong to the submodule's own repo. It *will* stage a SHA bump if you've moved the submodule's HEAD (via commit or checkout). If you'd rather the parent ignore submodule HEAD movement entirely until you explicitly bump it, add `ignore = all` to the submodule's entry in `.gitmodules`.

### Co-developing the plugin from inside your project

If you want to make changes to the plugin while working in your UE project, the submodule directory *is* the plugin's git repo — `cd Plugins/Agility`, edit, commit, and `git push` against the plugin remote like any other repo. The parent project will show new commits in `git status` (so you can decide when to bump the pin) but won't see your individual file changes.

While you have uncommitted edits inside the submodule, the parent `git status` / `git diff` will show the submodule pointer as `<sha>-dirty`. That's purely a status marker — `-dirty` isn't a real SHA, so don't try to commit it in the parent. Commit those edits inside the submodule first; the marker disappears, and the parent's diff becomes a clean SHA bump you can then commit as the "Bump Agility" step above.

## Step 2 — Build with the plugin enabled

The plugin ships with `EnabledByDefault: true` in `Agility.uplugin`, so once your project finds it under `Plugins/`, it'll be enabled automatically the next time the project is loaded.

1. **Regenerate project files** so your IDE sees the new plugin module. In Rider: *Tools → Unreal → Refresh Project*. In Visual Studio: right-click the `.uproject` → *Generate Visual Studio project files*.
2. **Build the editor target** from your IDE.
3. **Open the editor** and verify the plugin is loaded: *Edit → Plugins → search "Agility"*.

If the build fails complaining about missing modules, double-check that `Plugins/Agility/Agility.uplugin` is actually present — if the submodule wasn't initialized after a fresh clone, the directory will exist but be empty. Run `git submodule update --init --recursive` from the project root.

## Step 3 — Wire `claude-code` to the plugin's conventions

To get the full tutorial workflow, point your project's `CLAUDE.md` at the plugin's. Create or edit `CLAUDE.md` in your project root:

```markdown
# <Your Project Name>

@./Plugins/Agility/CLAUDE.md
```

The `@./Plugins/Agility/CLAUDE.md` line tells `claude-code` to load Agility's conventions (the human/AI split, where your work goes, public-repo safety rules) automatically when you start a session in your project root. Add any project-specific instructions above or below it.

## Next: working through your first tutorial

With the plugin building and `claude-code` wired in, head to [`tutorials/`](./tutorials/) and pick one — [`01-hello-unreal.md`](./tutorials/01-hello-unreal.md) is the suggested starting point.

One thing to internalize before you open it: **the tutorials are conversation starters, not read-throughs.** Reading the markdown alone will only ever get you a partial picture. The intended loop is: open the tutorial next to a live `claude-code` session in your project root, send the prompts the tutorial gives, and then *go off-script* — ask Claude why it wrote the code that way, ask what would change if a parameter were different, ask for an explanation of any Unreal API or reflection macro you haven't seen before, ask for a tweak and watch it land in the editor. The tutorial gives you the spine; the back-and-forth with Claude is where you actually learn UE5 C++ and get a feel for the human + AI loop the plugin is built around.

## Day-to-day workflow notes

- **Live Coding** is available from inside the running editor for fast iteration on small C++ changes — the keyboard shortcut is `Ctrl+Alt+F11` by default. Live Coding cannot bootstrap a module that has zero compiled classes, and it can't pick up `Build.cs` changes, so those still require a full IDE build.
- **Adding new C++ files** — when you create new `.h` / `.cpp` files (either in the plugin tree or in your project's source tree), your IDE should pick them up automatically. If a new file isn't recognized, refresh the project model (Rider: *Tools → Unreal → Refresh Project*).
- **Don't commit `.xcodeproj` / `.xcworkspace` / `.sln` files.** They embed absolute paths from the developer's machine, are listed in the plugin's `.gitignore`, and are not needed when working through Rider or VS directly.

## Building for Android devices

The Epic-Launcher **Android target platform** (Prerequisites step 3) is enough to *configure* Android packaging and build in-editor on macOS. Actually deploying to a device or emulator additionally needs the Android **device toolchain** — an SDK, an NDK, and a JDK — plus telling Unreal where they live.

### Install the toolchain

The simplest source for all three is **[Android Studio](https://developer.android.com/studio)**, which bundles an SDK manager and a compatible JDK (its JBR):

1. Install Android Studio.
2. In its SDK Manager, install the **Android SDK** and **command-line tools**. By default these land at `~/Library/Android/sdk` on macOS.
3. Install the **NDK version Unreal expects**. UE pins this per engine version in `Engine/Config/Android/Android_SDK.json` (`MainVersion` / `MinVersion` / `MaxVersion`); for UE 5.7 that's **NDK r27c** (`27.2.12479018`). Installing a different major version is the most common reason the Android SDK shows up as *Invalid* in the editor. The NDK installs under `~/Library/Android/sdk/ndk/<version>`.

### Point Unreal at the toolchain

Unreal needs three paths — the SDK, the NDK, and a JDK. There are two ways to supply them, and the choice matters if you launch the editor from an IDE.

**Recommended — the editor's Android SDK settings page.** Open *Project Settings → Platforms → Android SDK* and fill in:

- **Location of Android SDK** — `~/Library/Android/sdk`
- **Location of Android NDK** — `~/Library/Android/sdk/ndk/<version>`
- **Location of JAVA** — Android Studio's bundled JDK, `/Applications/Android Studio.app/Contents/jbr/Contents/Home`

(These fields don't expand `~` — browse to, or paste, the resolved absolute path.)

This is the most robust option for two reasons:

- These settings are `globaluserconfig` — Unreal stores them in your **machine-global** Epic config (`~/Library/Application Support/Epic/UnrealEngine/<version>/Saved/Config/`), **not** in the project's tracked `Config/`. So filling in absolute paths here leaks nothing into a shared repo; each teammate just sets their own once.
- Saving the page pushes the paths into the **running editor's environment** and immediately refreshes device detection — so it works regardless of how the editor was launched.

That last point matters because a **GUI-launched editor (e.g. from Rider, or the Epic Launcher) does not inherit your shell environment** — any `ANDROID_HOME` / `NDKROOT` / `JAVA_HOME` you export in `~/.zshrc` or `~/.zprofile` won't be visible to it. The settings page sidesteps that entirely.

**Alternative — environment variables.** If you leave the settings-page fields blank, Unreal falls back to the `ANDROID_HOME` (SDK), `NDKROOT` (NDK), and `JAVA_HOME` (JDK) environment variables. This keeps absolute paths out of even your local config, but only works for an editor that actually inherits those variables — one launched from a terminal where they're set, not from a GUI app. If you go this route, `JAVA_HOME` must point at a real JDK *image* (a directory containing a `release` file), not a bare keg/symlink, and the JDK must be version 17+.

Either way, you can confirm the toolchain is healthy in *Project Settings → Platforms → Android SDK* — a valid setup reports the SDK as found. Until it's valid, the **Android platform category won't appear** in the editor's *Platforms* dropdown, and no devices will be listed under it.

### Deploy and iterate

With the toolchain configured and a device connected (USB debugging enabled and authorized) — or an emulator running — the device appears under *Platforms → Android*, and **Launch** deploys to it. A few things worth knowing about iteration time:

- **Emulator vs. physical device is the same build.** The expensive work — compiling UE C++ to native `.so` libraries, cooking content, packaging the APK — is keyed to the target **architecture**, not the destination. An emulator on an Apple-Silicon Mac is arm64, the same as a modern physical device, so it receives an identical binary; deploying to an emulator does **not** save build time. (An x86_64 emulator on an Intel Mac is a *different* arch — an extra compile, not a faster one.)
- **What rebuilds depends on what you changed.** Editing UE C++ triggers the native `.so` recompile (the slow path); editing content re-cooks (iterative cooking only re-cooks changed assets); editing only the Java/Kotlin/Gradle layer (e.g. native Android UI) skips the UE recompile and cook entirely, so it's much faster.
- **Use Launch, not Package Project, for iteration.** *Launch on device* does an iterative cook and pushes only changed files; *Package Project* is for producing a distributable build.

## Platform notes

Agility is developed primarily on **macOS + Rider**. The plugin source is standard cross-platform UE C++ and should compile on Windows + Visual Studio without changes — that path just isn't part of the maintainers' day-to-day CI loop. If something breaks on Windows, an issue or PR is welcome.

## Alternative: symlink instead of submodule

Useful when you actively develop the plugin against several UE projects and want all of them to track one in-flight plugin checkout without per-project SHA pinning. Skip this section if you went with the submodule path above.

### Clone the plugin once

Pick a location for your plugin checkouts. Anywhere outside your UE projects is fine — a `~/UE-plugins/` folder works well:

```bash
mkdir -p ~/UE-plugins
cd ~/UE-plugins
git clone <agility-repo-url> agility
```

You now have the plugin at `~/UE-plugins/agility/`.

### Symlink it into your project

Each UE project that wants to use Agility gets a symlink under its `Plugins/` directory pointing at the cloned plugin. From your project's root:

```bash
mkdir -p Plugins
# macOS / Linux:
ln -s ~/UE-plugins/agility Plugins/Agility
# Windows (run as Administrator, or use Developer Mode for non-elevated symlinks):
mklink /D Plugins\Agility %USERPROFILE%\UE-plugins\agility
```

`Plugins/Agility` now resolves to the cloned plugin. Updates you pull in `~/UE-plugins/agility/` are immediately visible to every project symlinked to it.

From here, jump back up to [Step 2 — Build with the plugin enabled](#step-2--build-with-the-plugin-enabled) and continue with the rest of the guide. The `Plugins/Agility/CLAUDE.md` reference in Step 3 resolves identically through the symlink.
