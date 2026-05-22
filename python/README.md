# Agility — Python Side

This directory is the home of Agility's **Python-side research code** — computer-vision experiments, deep-learning inference scripts, and the long-running web server that the Unreal plugin talks to as a client.

It lives in the same repo as the UE plugin on purpose. Most computer-vision and deep-learning work is faster to prototype in Python — OpenCV, PyTorch, and modern inference stacks are Python-first — so the algorithm work happens here first, where iteration is quick and the focus stays on the *computer-vision side* of the problem. Once an experiment is stable, one of two things happens:

- **Port to C++** in the plugin's `Source/Agility/`, where the focus shifts to the *software-engineering side* — real-time performance, threading, the editor surface, UE module integration.
- **Keep in Python, expose over HTTP / WebSocket** — when the algorithm depends on a Python-only library or a model too heavy to inline in UE, the plugin stays a client and talks to the server here over the network.

Either way, the algorithm work happens here first. See [`experiments/README.md`](./experiments/README.md) for the full promotion ladder (local experiment → web-API endpoint → C++ port).

## What this directory is *not*

- **Not part of the UE plugin install.** UE doesn't load or watch anything under `python/`. When this repo is added to a host UE project as `Plugins/Agility/`, the `python/` folder comes along, but the editor ignores it entirely.
- **Not required to use the plugin.** If you only care about the in-engine work, you can skip this directory.

## Layout

```
python/
  README.md                  # this file
  pyproject.toml             # shared deps for experiments and the server
  uv.lock                    # pinned dep versions; committed for reproducibility
  .python-version            # interpreter version pin (3.12)
  experiments/               # local CV scripts, no API, no UE involvement
    README.md                # how to add a new experiment; promotion ladder
    video-playback/          # first example: OpenCV plays the bundled video
      main.py
      README.md
  server/                    # the web server UE5 talks to (FastAPI)
    README.md                # how to run, bind-address policy, current endpoints
    main.py                  # FastAPI app — single GET /hello endpoint for now
  shared/                    # (planned) utilities promoted once they have real reuse
```

Dependencies are shared across all experiments and (eventually) the server — one `pyproject.toml` for the whole Python workspace. If an experiment ever needs a conflicting dep (rare in CV/ML), we'll split it then.

## Setup (macOS)

We use [`uv`](https://docs.astral.sh/uv/) — Astral's modern Python tool. It installs Python *for* you (so you don't need to think about which Python you have), manages dependencies, and creates the virtualenv automatically when you run a script. No `python3 -m venv .venv`, no `source .venv/bin/activate`, no "wait, which Python is this?".

**One-time install of `uv` itself.** Pick either path:

```bash
# Homebrew:
brew install uv

# Or, no Homebrew:
curl -LsSf https://astral.sh/uv/install.sh | sh
```

**Then, from the repo root:**

```bash
cd python
uv sync                                          # downloads Python 3.12 + installs deps
uv run experiments/video-playback/main.py        # plays the bundled video
```

The first `uv sync` will take a minute or so (downloading the interpreter and OpenCV). After that, every subsequent `uv run` is instant.

> **Windows isn't currently supported.** `uv` works on Windows, but we haven't validated the experiments (or the future server) there. PRs welcome; for now, treat the Python workspace as macOS-only.

## Running an experiment

```bash
# From python/:
uv run experiments/<experiment-name>/main.py

# Or from inside the experiment folder — uv walks up to find pyproject.toml:
uv run main.py
```

Try [`experiments/video-playback/`](./experiments/video-playback/) first — it's the simplest one and confirms the whole toolchain (Python, OpenCV, the native window, frame decoding) is working on your machine.

## Adding a dependency

```bash
cd python
uv add <package-name>     # updates pyproject.toml + uv.lock, installs into the venv
```

Then commit `pyproject.toml` and `uv.lock` together — the lockfile is what makes every clone reproducible.

## Public-repo safety

This directory ships under the same public-repo rules as the rest of the plugin — no machine-local paths, no secrets, no personal identity in tracked files. Use `.env` (gitignored) for per-developer configuration; track a sanitized `.env.example` once there's something concrete to document. See the **Public Repo Safety** section of [`../CLAUDE.md`](../CLAUDE.md) for the full rules.
