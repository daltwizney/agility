# Development Setup

This guide walks you through adding Agility to an existing Unreal Engine project so you can build with the plugin enabled and follow the tutorials in [`tutorials/`](./tutorials/).

Agility is **mostly source** — C++, HLSL, and a minimal bundled `Content/` (raw video files plus a small handful of base-material `.uasset`s that C++ wraps at runtime) — and ships as a standalone git repo. There are two supported ways to integrate it into a UE project:

- **Git submodule (recommended).** The plugin lives inside your project at `Plugins/Agility/` as a git submodule, pinned to a specific commit. Clean separation, per-project version pinning, and plugin file changes can never accidentally get staged in your project repo — the parent only ever records a SHA pointer, never the plugin's file contents. Works equally well whether you're just consuming the plugin or actively co-developing it (you can `cd` into the submodule, commit, and push upstream like any other repo).
- **Symlink (alternative).** Clone the plugin once on your machine and symlink it into each UE project. Useful when you have several UE projects that should all track the same in-flight plugin checkout without per-project SHA pinning — see [Alternative: symlink instead of submodule](#alternative-symlink-instead-of-submodule) at the bottom of this guide.

The rest of this guide assumes the submodule path.

## Prerequisites

1. **`claude-code`** — the CLI that drives every tutorial in this repo. Install it by following [Anthropic's instructions](https://docs.claude.com/en/docs/claude-code). The plugin's [`CLAUDE.md`](../CLAUDE.md) is auto-loaded by `claude-code` when you launch a session in a directory that references it, so the project's conventions (the human/AI split, where your own work goes, public-repo safety rules) kick in from turn one — see Step 3 below for how to wire it into your own project.
2. **A UE C++ project under git.** Agility is a C++ plugin, so you need a project that already builds C++ (i.e. created with a "C++" template, or already has a `Source/` directory and a working `.uproject` with at least one module). A Blueprints-only project will need to be converted to C++ first (UE editor → Tools → "New C++ Class…" creates the Source tree). Submodules also require the parent project to be a git repo — if you haven't run `git init` in it yet, do that first.
3. **Unreal Engine** — installed via the Epic Games Launcher. The plugin has been developed against **UE 5.7** and should work on adjacent versions. Confirm your project's engine version via the `EngineAssociation` field in your project's `.uproject`.
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
