# video-playback

The simplest possible OpenCV experiment: open the bundled `Content/Movies/collie_agility.mp4` from the repo root, display it frame-by-frame in a native macOS window, and exit when the video ends or you press `q`.

## Run

From the `python/` directory:

```bash
uv run experiments/video-playback/main.py
```

(Or from this folder directly: `uv run main.py` — `uv` walks up to find `pyproject.toml` either way.)

If this is the first command you've run in `python/`, `uv` will fetch Python 3.12 and install OpenCV automatically before the script starts. After that, runs are immediate.

## What this experiment shows

- **Resolving a repo-relative path from a script** without depending on the current working directory — `Path(__file__).resolve().parents[3]` walks up from `python/experiments/video-playback/main.py` to the repo root.
- **The canonical OpenCV per-frame loop** — `cv2.VideoCapture` → `read()` → `imshow` → `waitKey`. Anything you want to do per-frame (color conversion, edge detection, optical flow, model inference) slots in between `read()` and `imshow()`.
- **Frame pacing** via `cv2.CAP_PROP_FPS` so the video plays at its real speed instead of as fast as Python can iterate.

## Next steps

This script is the skeleton most future experiments will build on. Easy modifications to try:

- Convert each frame to grayscale (`cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)`) before `imshow`.
- Run a Canny edge detector (`cv2.Canny(frame, 100, 200)`) and display the result.
- Crop or resize each frame before display.
- Stack the original and a processed version side-by-side with `numpy.hstack`.
