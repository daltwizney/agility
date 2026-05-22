"""Play Content/Movies/collie_agility.mp4 in an OpenCV window.

Press 'q' to quit early; otherwise the window closes when the video ends.
"""
from pathlib import Path

import cv2

# This script lives at python/experiments/video-playback/main.py — three
# directories below the repo root, where Content/Movies/ also lives.
REPO_ROOT = Path(__file__).resolve().parents[3]
VIDEO_PATH = REPO_ROOT / "Content" / "Movies" / "collie_agility.mp4"

WINDOW_NAME = "Agility — video-playback"


def main() -> None:
    if not VIDEO_PATH.exists():
        raise SystemExit(f"Video not found: {VIDEO_PATH}")

    capture = cv2.VideoCapture(str(VIDEO_PATH))
    if not capture.isOpened():
        raise SystemExit(f"OpenCV could not open: {VIDEO_PATH}")

    fps = capture.get(cv2.CAP_PROP_FPS) or 30.0
    delay_ms = max(1, int(1000 / fps))

    while True:
        ok, frame = capture.read()
        if not ok:
            break

        cv2.imshow(WINDOW_NAME, frame)
        if cv2.waitKey(delay_ms) & 0xFF == ord("q"):
            break

    capture.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
