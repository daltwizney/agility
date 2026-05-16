# Development Setup

This guide walks you through adding Agility to an existing Unreal Engine project so you can build with the plugin enabled and follow the tutorials in [`tutorials/`](./tutorials/).

Agility is **source-only** (no binary `.uasset` content) and ships as a standalone git repo. The recommended integration is to clone the plugin once on your machine and **symlink** it into each UE project that wants to use it — that way a single plugin checkout serves any number of projects, and your projects stay clean of plugin source.

## Prerequisites

1. **A UE C++ project** — Agility is a C++ plugin, so you need a project that already builds C++ (i.e. created with a "C++" template, or already has a `Source/` directory and a working `.uproject` with at least one module). A Blueprints-only project will need to be converted to C++ first (UE editor → Tools → "New C++ Class…" creates the Source tree).
2. **Unreal Engine** — installed via the Epic Games Launcher. The plugin has been developed against **UE 5.7** and should work on adjacent versions. Confirm your project's engine version via the `EngineAssociation` field in your project's `.uproject`.
3. **A C++ IDE for UE development** — on macOS we recommend [**JetBrains Rider**](https://www.jetbrains.com/rider/) (built-in UE support, no Epic-Launcher project-file generation needed). On Windows, Visual Studio (2022+) works the same way through its UE integration. Pick whichever is comfortable on your platform.
4. **Platform toolchain** — on macOS, the full Xcode SDK plus accepting the license is required for the underlying toolchain even if you don't open Xcode (`xcode-select --install` + `sudo xcodebuild -license accept`). On Windows, the Visual Studio "Game development with C++" workload covers it.

## Step 1 — Clone the plugin

Pick a location for your plugin checkouts. Anywhere outside your UE project is fine — a `~/UE-plugins/` folder works well:

```bash
mkdir -p ~/UE-plugins
cd ~/UE-plugins
git clone <agility-repo-url> agility
```

You now have the plugin at `~/UE-plugins/agility/`.

## Step 2 — Symlink it into your project

Each UE project that wants to use Agility gets a symlink under its `Plugins/` directory pointing at the cloned plugin. From your project's root:

```bash
mkdir -p Plugins
# macOS / Linux:
ln -s ~/UE-plugins/agility Plugins/Agility
# Windows (run as Administrator, or use Developer Mode for non-elevated symlinks):
mklink /D Plugins\Agility %USERPROFILE%\UE-plugins\agility
```

`Plugins/Agility` now resolves to the cloned plugin. Updates you pull in `~/UE-plugins/agility/` are picked up by the project automatically.

> **Why symlink instead of copy?** A single plugin checkout serves any number of UE projects, and your projects don't accumulate plugin source. Updates are one `git pull` in the plugin dir.

## Step 3 — Build with the plugin enabled

The plugin ships with `EnabledByDefault: true` in `Agility.uplugin`, so once your project finds it under `Plugins/`, it'll be enabled automatically the next time the project is loaded.

1. **Regenerate project files** so your IDE sees the new plugin module. In Rider: *Tools → Unreal → Refresh Project*. In Visual Studio: right-click the `.uproject` → *Generate Visual Studio project files*.
2. **Build the editor target** from your IDE.
3. **Open the editor** and verify the plugin is loaded: *Edit → Plugins → search "Agility"*.

If the build fails complaining about missing modules, double-check that `Plugins/Agility/Agility.uplugin` resolves through the symlink (it should `cat` cleanly from a terminal).

## Step 4 — Wire `claude-code` to the plugin's conventions (optional but recommended)

To get the full tutorial workflow, point your project's `CLAUDE.md` at the plugin's. Create or edit `CLAUDE.md` in your project root:

```markdown
# <Your Project Name>

@./Plugins/Agility/CLAUDE.md
```

The `@./Plugins/Agility/CLAUDE.md` line tells `claude-code` to load Agility's conventions (the human/AI split, where your work goes, public-repo safety rules) automatically when you start a session in your project root. Add any project-specific instructions above or below it.

## Day-to-day workflow notes

- **Live Coding** is available from inside the running editor for fast iteration on small C++ changes — the keyboard shortcut is `Ctrl+Alt+F11` by default. Live Coding cannot bootstrap a module that has zero compiled classes, and it can't pick up `Build.cs` changes, so those still require a full IDE build.
- **Adding new C++ files** — when you create new `.h` / `.cpp` files (either in the plugin tree or in your project's source tree), your IDE should pick them up automatically. If a new file isn't recognized, refresh the project model (Rider: *Tools → Unreal → Refresh Project*).
- **Don't commit `.xcodeproj` / `.xcworkspace` / `.sln` files.** They embed absolute paths from the developer's machine, are listed in the plugin's `.gitignore`, and are not needed when working through Rider or VS directly.

## Platform notes

Agility is developed primarily on **macOS + Rider**. The plugin source is standard cross-platform UE C++ and should compile on Windows + Visual Studio without changes — that path just isn't part of the maintainers' day-to-day CI loop. If something breaks on Windows, an issue or PR is welcome.
