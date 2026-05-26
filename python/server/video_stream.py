"""Background video streamer: reads frames with OpenCV, JPEG-encodes, sends over UDP.

A single UDP datagram per frame keeps the wire format trivial — the receiver
just decodes the bytes as JPEG and gets back a full image. This relies on
localhost's large MTU; over a real network you'd need chunking.
"""
from __future__ import annotations

import logging
import socket
import struct
import threading
import time
from pathlib import Path

import cv2

logger = logging.getLogger(__name__)

# Per-chunk payload size. macOS's loopback interface MTU is 16384 bytes and
# the kernel returns EMSGSIZE for larger UDP sends instead of fragmenting at
# the IP layer (unlike Linux). 8 KiB keeps each chunk well under the MTU with
# plenty of headroom for IP + UDP + our app header.
_CHUNK_PAYLOAD_BYTES = 8192

# Per-chunk header: frame_id (u32), chunk_index (u16), chunk_count (u16).
# Big-endian for wire-format convention. The UE5 receiver parses these as
# explicit byte shuffles so this layout is the contract.
_CHUNK_HEADER = struct.Struct("!IHH")


class VideoStreamer:
    def __init__(
        self,
        video_path: Path,
        dest_host: str = "127.0.0.1",
        dest_port: int = 8001,
        max_dim: int = 640,
        jpeg_quality: int = 70,
    ) -> None:
        self.video_path = video_path
        self.dest = (dest_host, dest_port)
        self.max_dim = max_dim
        self.jpeg_quality = jpeg_quality

        # Probe the source dimensions up front so clients (e.g. the UE5 quad)
        # can size their display target before any frames arrive over UDP.
        probe = cv2.VideoCapture(str(video_path))
        if not probe.isOpened():
            raise FileNotFoundError(f"could not open video {video_path}")
        src_w = int(probe.get(cv2.CAP_PROP_FRAME_WIDTH))
        src_h = int(probe.get(cv2.CAP_PROP_FRAME_HEIGHT))
        probe.release()
        scale = max_dim / max(src_w, src_h)
        if scale < 1.0:
            self.frame_width = int(src_w * scale)
            self.frame_height = int(src_h * scale)
        else:
            self.frame_width, self.frame_height = src_w, src_h

        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._thread: threading.Thread | None = None
        self._next_frame_id = 0

        # _running: worker streams when set, idles when cleared (stop).
        # _restart: one-shot seek-to-zero signal consumed at top of next loop iter.
        # _shutdown: tells the worker to exit entirely (process shutdown).
        self._running = threading.Event()
        self._restart = threading.Event()
        self._shutdown = threading.Event()

    def start(self) -> None:
        """Spin up the worker thread and begin streaming from the current position."""
        if self._thread and self._thread.is_alive():
            self._running.set()
            return
        self._shutdown.clear()
        self._running.set()
        self._thread = threading.Thread(target=self._loop, daemon=True, name="VideoStreamer")
        self._thread.start()

    def stop(self) -> None:
        """Pause sending frames. Worker thread stays alive so restart() can resume."""
        self._running.clear()

    def restart(self) -> None:
        """Seek to frame 0 and (re)start streaming. Spins up the worker if
        it hasn't started yet — this is the normal entry point from the UE5
        client's BeginPlay so the server stays idle until a client connects.
        """
        self._restart.set()
        self.start()

    def shutdown(self) -> None:
        """Permanently stop the worker thread. Called on app shutdown."""
        self._shutdown.set()
        self._running.set()  # wake worker if it's idling
        if self._thread:
            self._thread.join(timeout=2.0)
        self._socket.close()

    def _loop(self) -> None:
        # NB: prints go through uvicorn's stdout passthrough; our logger.info
        # calls get swallowed by uvicorn's default log config, hence print().
        cap = cv2.VideoCapture(str(self.video_path))
        if not cap.isOpened():
            print(f"[VideoStreamer] ERROR: failed to open {self.video_path}", flush=True)
            return

        fps = cap.get(cv2.CAP_PROP_FPS) or 30.0
        frame_interval = 1.0 / fps
        print(
            f"[VideoStreamer] streaming {self.video_path.name} -> "
            f"udp://{self.dest[0]}:{self.dest[1]} at ~{fps:.1f} fps "
            f"(frame size {self.frame_width}x{self.frame_height})",
            flush=True,
        )

        try:
            while not self._shutdown.is_set():
                if not self._running.is_set():
                    # Idle until start/restart/shutdown wakes us.
                    self._running.wait(timeout=0.5)
                    continue

                if self._restart.is_set():
                    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                    self._restart.clear()

                t_start = time.perf_counter()
                ok, frame = cap.read()
                if not ok:
                    # End of file -> loop.
                    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                    continue

                h, w = frame.shape[:2]
                scale = self.max_dim / max(h, w)
                if scale < 1.0:
                    frame = cv2.resize(
                        frame, (int(w * scale), int(h * scale)), interpolation=cv2.INTER_AREA,
                    )

                ok, buf = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, self.jpeg_quality])
                if not ok:
                    continue
                data = buf.tobytes()

                chunk_count = (len(data) + _CHUNK_PAYLOAD_BYTES - 1) // _CHUNK_PAYLOAD_BYTES
                if chunk_count == 0 or chunk_count > 0xFFFF:
                    print(f"[VideoStreamer] frame chunk_count {chunk_count} out of range", flush=True)
                    continue

                frame_id = self._next_frame_id
                self._next_frame_id = (self._next_frame_id + 1) & 0xFFFFFFFF

                for i in range(chunk_count):
                    payload = data[i * _CHUNK_PAYLOAD_BYTES : (i + 1) * _CHUNK_PAYLOAD_BYTES]
                    header = _CHUNK_HEADER.pack(frame_id, i, chunk_count)
                    try:
                        self._socket.sendto(header + payload, self.dest)
                    except OSError as e:
                        print(f"[VideoStreamer] sendto failed on chunk {i}/{chunk_count}: {e}", flush=True)
                        break

                elapsed = time.perf_counter() - t_start
                sleep_for = frame_interval - elapsed
                if sleep_for > 0:
                    time.sleep(sleep_for)
        finally:
            cap.release()
            logger.info("VideoStreamer: worker exiting")
