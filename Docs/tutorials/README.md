# Tutorials

The tutorials are the **public face of Agility's research** — they let any curious reader experience the human + AI build loop firsthand, on their own machine, in their own UE project. Each one is an end-to-end build you walk through with `claude-code`: open the markdown, hand it to Claude, and start asking questions.

Tutorials assume you've integrated the Agility plugin into a UE project — see [`../development-setup.md`](../development-setup.md) if you haven't yet.

## Status convention

Every tutorial doc starts with YAML frontmatter declaring its readiness:

```markdown
---
status: ready
---
```

- **`status: ready`** — Reviewed, tested end-to-end, safe for a newcomer to follow. Claude should treat these as authoritative.
- **`status: draft`** — Work-in-progress. Scope notes, partial walk-throughs, or experiments that aren't ready to be followed yet. Claude should warn you before recommending one as a starting point.

If a doc has no frontmatter, treat it as `draft` until proven otherwise.

## Index

| Tutorial | Status | What you'll build |
|---|---|---|
| [`01-hello-unreal.md`](./01-hello-unreal.md) | ready | Three procedural actors (voxel island, spiral galaxy, fractal tree) — your first end-to-end loop with the human + Claude workflow. |
| [`proceduralmesh/`](./proceduralmesh/) | draft | Procedural mesh fundamentals + a Custom-HLSL shader pipeline so Claude can author per-vertex data and shading from C++ / `.ush` text. In active development — see [`proceduralmesh/00-scope.md`](./proceduralmesh/00-scope.md). |
| [`flappybird/`](./flappybird/) | draft | Scope notes for a Flappy Bird build. Paused while we focus on procedural mesh + material workflow. |
