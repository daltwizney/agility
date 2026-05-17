---
status: ready
---

# Flappy Bird (neon edition)

Build a fully procedural neon-styled Flappy Bird in C++ with `claude-code`: a primitive-mesh bird with flap input, scrolling cyan/magenta pipes, a procedural background (mountains / stars / moon), an emissive-material visual identity, a state-machine GameMode (Ready → Playing → Dead), a `DrawHUD` score / BEST / banner, and audio. Around ~1500 lines of C++ across 8 files, broken into three parts.

By the end you'll have:

- A playable neon Flappy Bird with BGM and SFX.
- A working feel for building a complete game loop in C++ — pawn, GameMode, HUD, custom actors, state machine, audio — without authoring a single Blueprint.
- More fluency with claude-code-led C++ work: bigger prompts, multi-file systems, deliberate "build it, then play it" handoffs to the editor.

Plan ~2–3 hours, split across three sessions if you like — each part ends with a playable build.

---

## Prerequisites

- The Agility plugin integrated into a UE C++ project. See [`../../development-setup.md`](../../development-setup.md) if you haven't.
- Recommended: walk through [`../01-hello-unreal.md`](../01-hello-unreal.md) first — it's the smaller warm-up tutorial and grounds you in the human + claude-code split before tackling a chunkier build.

That's it. The plugin already ships the neon base material (`M_AgilityNeonEmissive`) and audio assets (`/Agility/Audio/...`) — no editor authoring or asset import needed before you start.

---

## Where your work goes

| Your work goes in...                              | What lives there                          |
|---------------------------------------------------|-------------------------------------------|
| `Content/MyContent/Maps/FlappyBird.umap`          | Your map (you'll create it in Part 1)     |
| `Source/<YourProjectModule>/MyScripts/FlappyBird/`| Your C++ classes                          |

Replace `<YourProjectModule>` with your project's primary game module folder. The plugin's reference implementation lives at `Plugins/Agility/Source/Agility/FlappyBird/` — read it freely, but write your own version in `MyScripts/FlappyBird/`.

---

## The three parts

1. **[`01-bird-and-flap.md`](./01-bird-and-flap.md)** — A bird that falls under gravity and flaps on Space / LMB, with a world-locked camera framing the playfield.
2. **[`02-pipes-and-background.md`](./02-pipes-and-background.md)** — A neon world: mountains, stars, moon, ground, plus alternating cyan/magenta pipes scrolling past the bird.
3. **[`03-score-and-state.md`](./03-score-and-state.md)** — Score gates, a Ready → Playing → Dead state machine, a `DrawHUD` score / BEST / banner, BGM, and SFX. The full game.

Don't skip the in-editor steps — they're where you'll actually feel what you just wrote.
