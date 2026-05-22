"""Agility's Python-side web server. The UE plugin talks to this as a client.

Run from python/:
    uv run uvicorn server.main:app --host 127.0.0.1 --port 8000 --app-dir .

`--app-dir .` is required — uvicorn runs as a binary script so its sys.path[0]
is .venv/bin/ rather than cwd, and `import server.main` fails without it.
"""
from fastapi import FastAPI

app = FastAPI(title="Agility Server", version="0.0.1")


@app.get("/hello")
def hello() -> dict[str, str]:
    return {"message": "hello agility"}
