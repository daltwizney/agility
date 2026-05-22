# Experiments

This is Agility's local-only Python playground. Every folder under here is one self-contained computer-vision experiment, runnable directly from the command line with no web API and no UE5 involvement.

Most experiments should live and die here. The point of this tier is fast iteration on the *algorithm*, in the language with the best CV/ML library coverage. Once an experiment is genuinely useful, it gets promoted up the stack:

1. **`experiments/<name>/`** — you are here. Runs locally on macOS, scripts only, no API surface.
2. **`server/`** — promote when it's worth UE5 calling over HTTP / WebSocket (heavy model, Python-only library, useful as a network service).
3. **UE5 C++ port** under `Source/Agility/` — promote when it needs to run real-time on-device (Mac or Android).

Most experiments never reach steps 2 or 3, and that's a feature — the local playground is the cheapest place to find out what doesn't work.

## How to add a new experiment

1. Make a new folder: `python/experiments/<your-experiment-name>/`
2. Add `main.py` (entry point) and a short `README.md` explaining what it does and how to run it.
3. If you need a new Python dependency, add it to `python/pyproject.toml` (or run `uv add <package>` from `python/`) — deps are shared across all experiments and the eventual server, not per-folder.
4. Run with `uv run experiments/<your-experiment-name>/main.py` from `python/`, or `uv run main.py` from inside the experiment folder.

## Conventions

- **Resolve paths relative to the script,** not the working directory, so `uv run` works from anywhere. The `video-playback` experiment shows the pattern.
- **Keep each experiment self-contained** until it has a real second caller. Don't speculatively promote utilities to `python/shared/`.
- **Comment sparingly** — clear function names + a short per-experiment `README.md` beat sprinkled inline commentary.

## Existing experiments

- [`video-playback/`](./video-playback/) — opens the bundled `Content/Movies/collie_agility.mp4` in an OpenCV window. The skeleton most future experiments will build on.
