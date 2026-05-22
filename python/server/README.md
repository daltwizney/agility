# server

The long-running web server the UE plugin talks to as a client. FastAPI app served by uvicorn.

## Run

From `python/`:

```bash
uv run uvicorn server.main:app --host 127.0.0.1 --port 8000 --reload --app-dir .
```

`--app-dir .` is load-bearing: uvicorn runs as a binary script, so `sys.path[0]` is `.venv/bin/` rather than your cwd — without it, `import server.main` fails with `ModuleNotFoundError`. The flag prepends the given directory to `sys.path` before app resolution.

`--reload` watches the source files and restarts on save — comfortable during development. Drop it if you don't want auto-restart.

Verify with `curl`:

```bash
curl http://127.0.0.1:8000/hello
# {"message":"hello agility"}
```

## Bind address

Always bind to `127.0.0.1`, never `0.0.0.0`. The server is meant to be reachable only from the same machine UE5 is running on; binding to all interfaces would expose it to the local network. If a future feature genuinely needs external reach, gate that behind an explicit config flag rather than flipping the default.

## UE5 side

The plugin ships an `AAgilityHelloProbe` actor under `Source/Agility/Server/` that fires `GET /hello` on `BeginPlay` and posts the response to UE's on-screen debug overlay. Drag it into any map, hit Play with the server running, and you should see "Server says: hello agility" overlaid in cyan.

## Endpoints

- `GET /hello` — sanity-check endpoint. Returns `{"message": "hello agility"}`.

## Adding more endpoints

Add route handlers to `main.py` while the file is small. Once it grows past one screen, split by feature into `server/<feature>.py` modules and include them as FastAPI routers (`app.include_router(...)`).
