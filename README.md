# Agility

**A source-only Unreal Engine plugin built for human + AI co-development — and a growing library of interactive tutorials you walk through with [`claude-code`](https://docs.claude.com/en/docs/claude-code).**

Agility is two things at once:

1. **A drop-in UE plugin** with reference C++ actors (procedural geometry, an HLSL shader playground, a Flappy Bird sandbox) you can study, extend, or use directly in your own UE project.
2. **An interactive tutorial library** designed for the human + AI team workflow. You add the plugin to your project, you start a `claude-code` session, you read a tutorial, you build alongside an AI pair-programmer who already knows the codebase.

If that sounds like fun, you're in the right place.

---

## Why this exists

Game-dev tutorials are mostly long videos. You watch someone click around the editor for forty minutes, you scrub backwards to find the one menu they opened at 23:14, you pause, you copy, you hope your version of the engine still has that button in the same place.

We think there's a better way now.

`claude-code` can read the whole project, write and explain C++, edit configs, narrate what a system does, and answer "where is X wired up?" in seconds. What it *can't* do is open the Unreal editor, drag a mesh into a viewport, judge whether a material looks right, or press Play and tell you if a build feels good. That part is still — wonderfully — yours.

**Just as importantly, Agility is a great way to actually learn C++ in Unreal Engine.** UE5's C++ side is famously a wall for newcomers — `UObject`s, reflection macros, the build system, module / include layout, the dozen subtly different mesh components, the editor / runtime split. With `claude-code` writing the first draft, explaining *why* each piece looks the way it does, and answering follow-up questions on the spot, you get a pair-programmer who can take you from "I've never touched Unreal C++" to confidently reading and modifying engine-flavored code yourself. The AI doesn't replace the learning — it accelerates it, and the tutorials are written to encourage you to read the code Claude produces, ask questions about it, and tweak it.

So Agility leans into the split. **Claude drafts and explains the C++ and configs; you drive the editor and bring visual judgment; the tutorial tells you which side is doing what at every step.** No videos. No scrubbing. Just you, a terminal, the editor, and an AI partner that's read the entire plugin before you said hello.

---

## What's in the box

- **Procedural reference actors** under [`Source/Agility/Procedural/`](./Source/Agility/Procedural/): a Perlin-noise voxel terrain, a parametric spiral galaxy, and a recursive fractal tree — all parameter-tweakable in real time from the editor's Details panel.
- **MeshLab shader playground** under [`Source/Agility/MeshLab/`](./Source/Agility/MeshLab/) + [`Shaders/MeshLab.ush`](./Shaders/MeshLab.ush): a procedural cube canvas wired into an HLSL Custom-node pipeline so Claude can author per-vertex data and shading from C++ / `.ush` text.
- **Flappy Bird sandbox** under [`Source/Agility/FlappyBird/`](./Source/Agility/FlappyBird/): a fully procedural bird + pipes + spawner + scoring HUD, built end-to-end in C++.
- **Interactive tutorials** in [`Docs/tutorials/`](./Docs/tutorials/) — the first is [`01-hello-unreal.md`](./Docs/tutorials/01-hello-unreal.md), which walks you through rebuilding the three procedural actors yourself with `claude-code` doing the C++ heavy lifting.
- **Setup docs** for integrating the plugin into your UE project in [`Docs/development-setup.md`](./Docs/development-setup.md).
- **[`CLAUDE.md`](./CLAUDE.md)** — the file `claude-code` reads automatically. Encodes the project's conventions (the human/AI split, where your work goes vs. reference material, public-repo safety rules) so the AI shows up oriented from turn one.

---

## Getting started

1. **Set up your dev environment** — follow [`Docs/development-setup.md`](./Docs/development-setup.md) to clone the plugin, symlink it into your UE project's `Plugins/` directory, and build the editor target with the plugin enabled.
2. **Open your project in your IDE**, build the editor, and confirm the Agility plugin loads (Edit → Plugins → search "Agility").
3. **Start `claude-code` in your project root**, then open [`Docs/tutorials/01-hello-unreal.md`](./Docs/tutorials/01-hello-unreal.md) (via the symlinked plugin at `Plugins/Agility/Docs/tutorials/01-hello-unreal.md`) and walk through it. The tutorial gives you the exact prompts to send your AI partner.
4. **Riff.** Tweak parameters. Break things. Ask Claude why something behaves the way it does. That's the whole point.

---

## The collaboration philosophy

Each side leans into what it's actually good at. Don't ask the AI to fake the editor work, and let the AI shoulder the boilerplate so you can spend your attention on understanding the interesting parts.

- **Claude writes the C++ — and walks you through it.** All source under [`Source/`](./Source/), build config, shader source, asset-import scripts, command-line tooling, and tracked markdown docs are Claude's to draft. Claude is *also* expected to explain what the code does, why Unreal's APIs are shaped that way, and answer follow-up questions — so you can read along, modify the result yourself, and build real UE5 C++ chops instead of treating the source as a black box. Hand-editing the C++ Claude wrote is absolutely welcome and encouraged.
- **You own the editor:** Blueprint graphs, Material / Material Function graphs, Niagara systems, level / world setup, lighting, asset placement, animation montages, UMG widget design — anything that lives inside `.uasset` / `.umap` files. (Agility itself ships no `.uasset` content — `CanContainContent: false` — so this all happens in *your* project's content.)
- **Visual judgment is always yours.** Claude can't see the viewport, press Play, or tell whether a material looks right. When the work crosses that boundary, the AI stops and asks you to verify; you report back what you see.

The full version of this contract lives in [`CLAUDE.md`](./CLAUDE.md), and tutorials follow the same convention — every step is marked as either a `claude-code` step or an in-editor step so it's always clear who's driving.

---

## Contributing

Agility is in active early-stage experimentation, but issues, ideas for tutorial topics, and pull requests are very welcome. If you build something cool on top of one of these tutorials, we'd love to hear about it.

Public-repo safety rules (no machine-local paths, no secrets, no personal identity in tracked files) are documented in [`CLAUDE.md`](./CLAUDE.md) — please skim that section before opening a PR.

---

## Credits

Agility is a human + AI collaboration. The codebase, the tutorials, and most of the docs in this repo — including this README — were written together by wizneylabs and Claude. See [`THANKS.md`](./THANKS.md) for the story.

Here's to building cool things together.
