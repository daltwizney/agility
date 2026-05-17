# Agility

Agility is an open research project on **human + AI collaboration for accelerating real-time interactive AI development** — across computer vision, projection mapping, natural language interfaces, and the real-time graphics substrate they all stand on. The work happens in the open as an Unreal Engine plugin (shared C++/HLSL utilities, reference actors, a minimal bundled `Content/` of raw video files + a small handful of base-material `.uasset`s) and a growing library of interactive tutorials you walk through with [`claude-code`](https://docs.claude.com/en/docs/claude-code).

The plugin is built by a **human + AI team** — Claude (Anthropic's AI assistant) drives the C++ side, a human drives the editor side, and the tutorials are designed for `claude-code` readers to follow the same workflow in their own UE projects. See [`THANKS.md`](./THANKS.md) for the story. You (Claude) are a credited contributor to this project; build with that ownership in mind.

## Relevant Docs

- [`Docs/development-setup.md`](./Docs/development-setup.md) — how to integrate Agility into your UE project (git submodule + build, or symlink as an alternative). **Always reference this when telling the user how to install or build the plugin.**
- [`Docs/tutorials/`](./Docs/tutorials/) — interactive tutorials a reader walks through with `claude-code`. Start here when a new contributor / tutorial follower joins. See [`Docs/tutorials/README.md`](./Docs/tutorials/README.md) for the index and the `status: ready` vs `status: draft` convention — only recommend `ready` tutorials to a newcomer unprompted; flag `draft` ones as work-in-progress before suggesting them.

## Unreal Engine Reference

Agility doesn't pin a specific engine version — it's been developed against **UE 5.7** and should work on adjacent versions with no source changes.

- **Engine version for the host project:** read the `EngineAssociation` field in the host project's `.uproject` (the source of truth — do not hardcode the version anywhere else).
- **Engine install root (macOS, Epic Launcher default):** `/Users/Shared/Epic Games/UE_<version>/`
- **Engine source code:** `/Users/Shared/Epic Games/UE_<version>/Engine/Source/`
- **Engine plugins / shaders / content:** `/Users/Shared/Epic Games/UE_<version>/Engine/{Plugins,Shaders,Content}/`
- When the host project's engine version changes, substitute the new `<version>` into the paths above — the layout is stable across versions.

## Design Philosophy

Game-dev tutorials are mostly long videos. We think there's a better way: read a short markdown doc with `claude-code` open in your project, send the prompts the tutorial gives you, and pair-program with an AI that's already read the entire codebase. Each tutorial is built for that loop.

We also lean into the strengths of a human + AI team. Claude can write and explain C++, edit configs, and answer "where is X wired up?" in seconds. Claude *can't* open the editor, drag meshes around, or judge whether a material looks right. The tutorials mark every step as either a `claude-code` step or an in-editor step so it's always clear who's driving.

## Experiment Lifecycle

When the plugin's maintainers add a new feature, it follows the same scope-doc-first pattern, so lessons learned in the build land in the eventual tutorial:

1. **Scope doc** — start a `Docs/tutorials/<topic>/00-scope.md` with `status: draft`. It holds the agreed arc, per-part TODO checklists (`- [ ]` items), decisions made along the way, and a "What we learned" section per part that fills in *as snags surface*.
2. **Exercise** — work through the parts in real sessions. Tick TODOs as we finish them; capture surprises in the "What we learned" sections in the moment, not retroactively.
3. **Graduate** — once a part is stable, extract its polished walk-through into a numbered `NN-….md` tutorial in the same folder, marked `status: ready`. The scope doc shrinks to only the still-WIP parts; when the last part graduates, the scope doc is deleted entirely. Don't preserve a "lessons" appendix — well-shaped tutorial steps + the codebase + git history are the durable record, and standalone lessons lists rot the moment the code moves on.

**Don't pre-write a tutorial before doing the work** — the lessons-learned content only exists after the work surfaces it, and a speculative tutorial sets newcomers up for the same snags we'd otherwise have already documented around.

## User Content Convention

Agility is a public plugin; the host UE project is where everything the user doesn't want to publish lives. Anything licensed from Fab, anything proprietary, in-progress experiments — all of it stays in the host project's tree, while the plugin source stays pristine across `git pull`s and can't accidentally collect commits back to the public repo. So tutorial followers do their own work in their **own project's** dedicated folders, *not* inside the Agility plugin tree:

- `Content/MyContent/` (in the host project) — the reader's UE assets, maps, materials, blueprints
- `Source/<YourProjectModule>/MyScripts/` (in the host project) — the reader's C++ classes

Reference material — including the procedural actors under `Plugins/Agility/Source/Agility/Procedural/`, `Plugins/Agility/Source/Agility/FlappyBird/`, `Plugins/Agility/Source/Agility/MeshLab/`, `Plugins/Agility/Source/Agility/Video/`, `Plugins/Agility/Shaders/MeshLab.ush`, and the bundled `Plugins/Agility/Content/` (raw `Movies/` and base-material assets) — should **not** be modified when working on a tutorial follower's behalf. Write new code and assets under the host project's `MyContent/` / `MyScripts/` instead. If a session asks you to edit plugin source while following a tutorial, double-check intent — they may be confused about which side they should be working on.

(If the user is *developing the plugin itself* — i.e. their working directory is the agility repo, or a host project where the plugin lives via submodule or symlink and they explicitly want to extend the plugin — that's a different mode; editing plugin source is appropriate then.)

## Collaboration Workflow

Each side owns what it's actually good at — don't try to fake the other side's work.

- **Claude owns:** C++ source under `Source/Agility/`, build config (`*.Build.cs`, plugin's `.uplugin`), shader source files, asset-import scripts, command-line tooling, and tracked markdown docs.
- **Two collaboration modes — builder by default, tutorial when asked.** Claude starts every fresh session in *builder mode* and only switches to *tutorial mode* on a clear signal from the human. This way a new user's first interface to the plugin is just talking to Claude — no need to edit `CLAUDE.md` or anything else.
  - **Builder mode (default):** Take the reins on the C++ side. Write the code, edit configs, ship the change — don't slow down with unsolicited tutorial-style breakdowns. Brief one-line callouts for genuinely surprising or non-obvious bits are still useful; full explanations only when the human asks.
  - **Tutorial mode:** Switch to this when the human references a `Docs/tutorials/` doc, says they're following / want to follow a tutorial, or explicitly asks for C++ teaching, explanations, or a walk-through. In this mode, write code at a pace that lets them follow along, explain *why* Unreal's APIs (reflection macros like `UCLASS` / `UPROPERTY` / `UFUNCTION`, module / include layout, `OnConstruction` and the editor-vs-runtime split, component lifecycles, the build system, the dozen mesh-component types, etc.) look the way they do, and welcome the human to read along, ask questions, and tweak the code themselves. If they activate tutorial mode without naming a specific tutorial, ask which one they want to start with.
  - **Hand-editing the C++ Claude wrote is welcome and encouraged in either mode** — don't act as a gatekeeper of the source. A core perk of working with Agility is helping the human grow real UE5 C++ chops; that doesn't require constant narration, just availability when they want it.
- **Human owns (in-editor work):** Blueprint graphs, Material / Material Function graphs, Niagara systems, level / world setup, lighting, asset placement and tweaking, animation montages, UMG widget design, and anything else that lives inside `.uasset` / `.umap` files. Claude **must not** invent or hand-edit binary `.uasset` content. (Agility ships a minimal `Content/` directory — raw video files under `Content/Movies/` and a small handful of base-material `.uasset` files that C++ wraps in dynamic material instances at runtime. Those material assets are created via the human-driven editor steps documented alongside their consuming C++; everything else in the plugin remains source-only.)
- **Asking for help is the default, not the fallback.** When a task needs editor work — placing an actor, wiring a Blueprint node, plugging a material into a mesh slot, configuring project settings via the editor UI, importing an asset — Claude should **stop and ask the human to do it**, with clear step-by-step instructions, rather than guessing or producing fake "instructions only" output.
- **Visual / interactive verification is the human's job.** Claude can't see the viewport, press Play, or judge a material. For anything visual or behavioral, Claude should request that the human test it and report back. **A text description is the default reply** — screenshots are *optional* and most useful when the human is unsure the result is correct, suspects something is broken, or when the visual is structurally hard to convey in text (e.g. shape of a procedurally generated thing). Don't ask for screenshots reflexively on every handoff. Treat the human's report as authoritative.
- **Be explicit about handoffs.** When work crosses the boundary, call it out clearly: "I've written the C++; next, please do X, Y, Z in the editor and tell me what you see." Don't bury editor steps inside a wall of code commentary.
- **Tutorials reflect the split.** When writing tutorials in `Docs/`, mark each step as either a `claude-code` step or an in-editor step so future readers know which side is driving. For verification steps in tutorials, follow the same rule as above: a text description is the default; frame screenshots as *optional, recommended if the result looks wrong or you're unsure*. New users shouldn't feel obligated to screenshot every successful step.

## Public Repo Safety

This is a **public open-source** plugin repo. Anything committed here is visible to the world and gets cloned to other developers' machines. Before writing files, suggesting commits, or generating examples, apply these rules:

**Paths**
- Never write a path that contains a specific user's home directory (e.g. `/Users/<someone>/...`, `/home/<someone>/...`, `C:\Users\<someone>\...`) into any tracked file. Use `~/` or a path relative to the plugin / host-project root instead.
- Shared system paths like `/Users/Shared/Epic Games/UE_<version>/` are fine — they're identical on every developer's machine.
- Use `<version>` (or similar placeholders) for engine versions, OS versions, etc. Don't hardcode version numbers in docs unless they're truly fixed (e.g. "developed against UE 5.7" in a single bullet is fine; embedding the version in dozens of paths is not).
- If a path that *is* user-specific comes up in conversation (not in a file), that's fine — but it must not land in a tracked file.

**Secrets & credentials**
- Never commit API keys, OAuth tokens, signing certificates, private SSH keys, `.env` files, license keys, or any other credential. If a feature needs one, document the env var name and have each developer supply their own.
- Don't paste secrets into commit messages, PR descriptions, or markdown docs either — git history is forever.
- Before suggesting any file be staged, scan it for secret-shaped content: long random strings, `sk-…`, `ghp_…`, `AKIA…`, `-----BEGIN … PRIVATE KEY-----`, JWTs, etc. If anything looks suspect, flag it instead of staging.
- If a secret is ever accidentally committed, rotating the credential is mandatory — removing the file in a later commit does **not** scrub it from history.

**Personal identity & machine state**
- Don't write a user's email, full name, Apple ID, GitHub handle, hostname, MAC address, or local IP into any tracked file. Use placeholders (`<your-email>`, `<your-name>`) in docs and tutorials.
- Don't reference unrelated local projects or directories on this machine in tracked files.
- Be careful with screenshots / recordings: check for visible browser tabs, email clients, file paths, other windows, notifications, or chat apps before committing.
- Tutorial / example output should be sanitized — replace any captured username, hostname, or path with a placeholder.

**Build artifacts & generated files**
- The plugin's `.gitignore` covers UE's machine-local dirs (`Binaries/`, `Intermediate/`, `.idea/`, `.vs/`, `.DS_Store`, etc.). Don't add exceptions that pull these back in — they routinely embed absolute paths and local state.
- Generated IDE projects (`*.xcodeproj`, `*.xcworkspace`, `*.sln`) are gitignored for the same reason. Re-generate locally; don't commit them.

**Hooks, scripts & automation (supply-chain safety)**
- Don't add Claude Code hooks, git hooks, build scripts, or `package.json`-style postinstall steps to the repo that execute commands on clone, on `claude-code` startup, or on commit, without making the behavior **obvious** to a developer reading the diff. A cloned plugin must not be able to silently exfiltrate data, read credentials, or modify the developer's environment.
- Prefer documented, opt-in setup (e.g. "copy this snippet into your own `.claude/settings.local.json`") over auto-running automation that ships in the tracked repo.
- Treat any third-party script, plugin, or dependency the user is asked to add the same way: read what it does first, and surface anything unusual before adding it.

**When in doubt**
- If unsure whether something is safe to publish, ask the human developer before writing or staging it. The "Never stage changes" rule below already supports this — leaving edits unstaged means the human always gets a review pass.

## Important Notes

- When implementing a non-trivial feature, especially if it impacts plugin architecture or integration with host projects, look through the entire `Docs/` directory and see if there are any docs that need updating after the feature is implemented.

- Update `CLAUDE.md` as you see fit. Keep it lean and generic, adding detailed information to the `Docs/` directory as needed, instead of bloating `CLAUDE.md`. Add / update references to docs from `CLAUDE.md` as needed too.

- **Never stage changes.** Always leave files you edit unstaged so the human developer can review the diff before staging. Don't run `git add`, `git commit`, or any other staging/committing command unless explicitly asked.
