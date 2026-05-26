"""Agility's Python-side web server. The UE plugin talks to this as a client.

Run from python/:
    uv run uvicorn server.main:app --host 127.0.0.1 --port 8000 --app-dir .

`--app-dir .` is required — uvicorn runs as a binary script so its sys.path[0]
is .venv/bin/ rather than cwd, and `import server.main` fails without it.
"""
from contextlib import asynccontextmanager
from pathlib import Path

from fastapi import FastAPI

from server.video_stream import VideoStreamer

_DATA_DIR = Path(__file__).parent / "data"
_VIDEO_FILE = _DATA_DIR / "Wizney_Labs_Logo_Animation_Generated.mp4"

# UDP target the streamer pushes JPEG-encoded frames at. The UE5
# AAgilityServerVideoQuad actor listens on this port by default.
_UDP_HOST = "127.0.0.1"
_UDP_PORT = 8001

streamer: VideoStreamer


@asynccontextmanager
async def lifespan(app: FastAPI):
    global streamer
    # Construct only — the worker thread doesn't spin up until the UE5 client
    # calls /video/restart on BeginPlay. Keeps the server quiet (no UDP traffic
    # to nothing) until a client actually connects.
    streamer = VideoStreamer(_VIDEO_FILE, dest_host=_UDP_HOST, dest_port=_UDP_PORT)
    try:
        yield
    finally:
        streamer.shutdown()


app = FastAPI(title="Agility Server", version="0.0.1", lifespan=lifespan)


@app.get("/hello")
def hello() -> dict[str, str]:
    return {"message": "hello agility"}


@app.post("/video/restart")
def video_restart() -> dict[str, object]:
    """Seek to frame 0 and (re)start the UDP stream.

    Returns the post-resize frame dimensions so the client can size its
    display target up-front instead of waiting for the first UDP packet.
    """
    streamer.restart()
    return {
        "status": "restarted",
        "width": streamer.frame_width,
        "height": streamer.frame_height,
        "udp_host": _UDP_HOST,
        "udp_port": _UDP_PORT,
    }


@app.post("/video/stop")
def video_stop() -> dict[str, str]:
    """Pause the UDP stream. Call /video/restart to resume from frame 0."""
    streamer.stop()
    return {"status": "stopped"}
