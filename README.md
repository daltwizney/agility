# Agility - UE5 plugin for an Agentic Future

> **Accelerating real-time interactive AI — by building it, with agentic AI.**

We're at the start of something genuinely new: a human and an AI, side-by-side, shipping interactive systems at a pace neither could reach alone. **Agility** is the open research project where we test that thesis — across **computer vision, projection mapping, and natural language interfaces**, and the real-time graphics substrate they all stand on.

What you'll find here is the experiment in progress: an **open-source Unreal Engine plugin** co-developed with Claude (Anthropic's AI assistant), plus a growing library of **interactive tutorials** you walk through with [`claude-code`](https://docs.claude.com/en/docs/claude-code) and rebuild the same way — human + AI, in conversation, in real time.

If that sounds like fun, you're in the right place.

---

## Why this exists

**The research bet.** Real-time interactive AI — systems that see, project, talk, respond — has historically demanded long, manual engineering cycles. We think a human + AI team can collapse that loop dramatically: the AI drafts and explains; the human directs, judges, and steers; together they iterate at conversational speed. Agility is where we test that thesis, in the open, with all the work-in-progress visible.

**The methodology.** Game-dev tutorials are mostly long videos. You watch someone click around the editor for forty minutes, you scrub backwards to find the one menu they opened at 23:14, you pause, you copy, you hope your version of the engine still has that button in the same place. We think there's a better way now.

`claude-code` can read the whole project, write and explain C++, edit configs, narrate what a system does, and answer "where is X wired up?" in seconds. What it *can't* do is open the Unreal editor, drag a mesh into a viewport, judge whether a material looks right, or press Play and tell you if a build feels good. That part is still — wonderfully — yours.

**Learning UE5 C++ along the way.** UE5's C++ side is famously a wall for newcomers — `UObject`s, reflection macros, the build system, module / include layout, the dozen subtly different mesh components, the editor / runtime split. With `claude-code` writing the first draft, explaining *why* each piece looks the way it does, and answering follow-up questions on the spot, you get a pair-programmer who can take you from "I've never touched Unreal C++" to confidently reading and modifying engine-flavored code yourself. The AI doesn't replace the learning — it accelerates it, and the tutorials are written to encourage you to read the code Claude produces, ask questions about it, and tweak it.

So Agility leans into the split. **Claude drafts and explains the C++ and configs; you drive the editor and bring visual judgment; the tutorial tells you which side is doing what at every step.** No videos. No scrubbing. Just you, a terminal, the editor, and an AI partner that's read the entire plugin before you said hello.

---

## If you're unsure where to start

Install [`claude-code`](https://docs.claude.com/en/docs/claude-code), clone this repo, and start asking questions. Claude has read everything in here and is happy to recommend a tutorial, explain any UE5 concept you haven't seen before, or walk you through setup if you get stuck.

```bash
git clone https://github.com/daltwizney/agility.git
cd agility
claude
```

Then ask something like *"I'm new to Agility — what should I look at first?"*. That's the whole on-ramp.

---

## Prerequisites

Two things to have in place before working through the tutorials in earnest:

- **A passing familiarity with the Unreal Engine editor** — opening a project, navigating the viewport, the Content Browser, the Details panel. If you're coming from Unity, Epic's [Unreal Engine for Unity Developers](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-for-unity-developers?lang=en-US) is a fast, focused crash-course in the editor concepts you'll need; if you've used UE before, skip it.
- **The plugin integrated into a UE C++ project** with the editor building and Agility enabled — [`Docs/development-setup.md`](./Docs/development-setup.md) walks through the full flow (git submodule, build, wire `claude-code` to the plugin's `CLAUDE.md`).

And if either of those feels intimidating: run `claude-code` at the root of this repository and ask. Claude will read the relevant doc and walk you through it step by step.

---

## What's in the repo today

The current artifacts are the foundational graphics + media substrate the research arc builds on — co-developed with Claude, in the open:

- **Procedural reference actors** under [`Source/Agility/Procedural/`](./Source/Agility/Procedural/): a Perlin-noise voxel terrain, a parametric spiral galaxy, and a recursive fractal tree — all parameter-tweakable in real time from the editor's Details panel.
- **MeshLab shader playground** under [`Source/Agility/MeshLab/`](./Source/Agility/MeshLab/) + [`Shaders/MeshLab.ush`](./Shaders/MeshLab.ush): a procedural cube canvas wired into an HLSL Custom-node pipeline so Claude can author per-vertex data and shading from C++ / `.ush` text — the low-level material control projection-mapping workflows depend on.
- **Video module** under [`Source/Agility/Video/`](./Source/Agility/Video/): a runtime video player wired into the plugin's bundled `Content/Movies/` — the media-input scaffolding for upcoming computer-vision experiments.
- **Flappy Bird sandbox** under [`Source/Agility/FlappyBird/`](./Source/Agility/FlappyBird/): a fully procedural bird + pipes + spawner + scoring HUD, built end-to-end in C++ — a tight interactive loop to stress-test the workflow on.
- **Interactive tutorials** in [`Docs/tutorials/`](./Docs/tutorials/) — the first is [`01-hello-unreal.md`](./Docs/tutorials/01-hello-unreal.md), which walks you through rebuilding the three procedural actors yourself with `claude-code` doing the C++ heavy lifting.
- **Setup docs** for integrating the plugin into your UE project in [`Docs/development-setup.md`](./Docs/development-setup.md).
- **[`CLAUDE.md`](./CLAUDE.md)** — the file `claude-code` reads automatically. Encodes the project's conventions (the human/AI split, where your work goes vs. reference material, public-repo safety rules) so the AI shows up oriented from turn one.

The computer-vision, projection-mapping, and natural-language-interface experiments are next; expect this list to grow.

---

## Why a plugin (and not a standalone UE project)?

Agility ships as a **plugin** rather than a self-contained UE project on purpose: this repo holds the public, shareable source + a minimal bundled `Content/`, and *your* UE project — separate from this repo, never committed here — is where you keep everything you don't want to publish. Anything you've licensed from Fab, anything proprietary, anything experimental: it all lives in your project's `Content/` and `Source/`, while you still get the full Agility toolset by dropping the plugin into your project's `Plugins/Agility/` folder (as a git submodule). The split keeps the public plugin repo clean and lets you build whatever you want on top of it without worrying about asset licensing or NDA leakage.

---

## Getting started

1. **Set up your dev environment** — follow [`Docs/development-setup.md`](./Docs/development-setup.md) to add the plugin to your UE project's `Plugins/` directory (as a git submodule, recommended; or a symlink) and build the editor target with the plugin enabled.
2. **Open your project in your IDE**, build the editor, and confirm the Agility plugin loads (Edit → Plugins → search "Agility").
3. **Start `claude-code` in your project root**, then open [`Docs/tutorials/01-hello-unreal.md`](./Docs/tutorials/01-hello-unreal.md) (via the plugin at `Plugins/Agility/Docs/tutorials/01-hello-unreal.md`) and walk through it. The tutorial gives you the exact prompts to send your AI partner.
4. **Riff.** Tweak parameters. Break things. Ask Claude why something behaves the way it does. That's the whole point.

---

## The collaboration philosophy

Each side leans into what it's actually good at. Don't ask the AI to fake the editor work, and let the AI shoulder the boilerplate so you can spend your attention on understanding the interesting parts.

- **Claude writes the C++ — and walks you through it.** All source under [`Source/`](./Source/), build config, shader source, asset-import scripts, command-line tooling, and tracked markdown docs are Claude's to draft. Claude is *also* expected to explain what the code does, why Unreal's APIs are shaped that way, and answer follow-up questions — so you can read along, modify the result yourself, and build real UE5 C++ chops instead of treating the source as a black box. Hand-editing the C++ Claude wrote is absolutely welcome and encouraged.
- **You own the editor:** Blueprint graphs, Material / Material Function graphs, Niagara systems, level / world setup, lighting, asset placement, animation montages, UMG widget design — anything that lives inside `.uasset` / `.umap` files. (Agility itself ships only a minimal bundled `Content/` — raw video files and a handful of base-material `.uasset`s that C++ wraps in dynamic material instances at runtime — so the rest of the `.uasset` work happens in *your* project's content.)
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
