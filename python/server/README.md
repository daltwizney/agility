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
- `POST /video/restart` — seeks the UDP video stream to frame 0 and (re)starts it. Returns `{"status": "restarted", "width": W, "height": H, "udp_host": ..., "udp_port": ...}` so clients can size their display target before the first frame lands.
- `POST /video/stop` — pauses the UDP stream. Call `/video/restart` to resume.

## Video stream

A background `VideoStreamer` thread reads frames from `server/data/Wizney_Labs_Logo_Animation_Generated.mp4` with OpenCV, resizes to fit `max_dim=640`, JPEG-encodes (quality 70), and pushes each frame as a single UDP datagram to `127.0.0.1:8001`. One datagram per frame keeps the wire format trivial — the receiver just decodes the bytes as JPEG. This relies on localhost's large MTU; over a real network you'd need chunking.

The streamer auto-starts when uvicorn boots and shuts down cleanly on exit. The UE5 side is `AAgilityServerVideoQuad` — drag it into a map, hit Play with the server running, and it should display the streamed video on a procedurally generated quad sized to the source frame's aspect ratio.

## Adding more endpoints

Add route handlers to `main.py` while the file is small. Once it grows past one screen, split by feature into `server/<feature>.py` modules and include them as FastAPI routers (`app.include_router(...)`).
