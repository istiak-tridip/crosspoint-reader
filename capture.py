import http.client
import hashlib
import time
import shutil
from PIL import Image
from pathlib import Path

FRAME_INTERVAL = 0.5
FRAMES_DIR = Path("frames")

# Clear and recreate frames folder
if FRAMES_DIR.exists():
    shutil.rmtree(FRAMES_DIR)
FRAMES_DIR.mkdir()
print(f"Cleared {FRAMES_DIR}/")

conn = http.client.HTTPConnection("192.168.110.130", timeout=5)

frame_count = 0
last_hash = None
last_frame_hash = None

try:
    while True:
        start_time = time.time()

        conn.request("GET", "/frame")
        res = conn.getresponse()
        data = res.read()

        frame_hash = res.getheader("X-Frame-Hash")
        frame_width = res.getheader("X-Frame-Width")
        frame_height = res.getheader("X-Frame-Height")

        md5_hash = hashlib.md5(data).hexdigest()

        if (md5_hash != last_hash) & (frame_hash != last_frame_hash) & (frame_width is not None) & (frame_height is not None):
            frame_count += 1
            last_hash = md5_hash
            last_frame_hash=frame_hash

            img = Image.frombytes("1", (int(frame_height), int(frame_width)), data)
            img = img.rotate(-90, expand=True)

            img.save(FRAMES_DIR / f"frame_{frame_count:05d}.png")

            print(f"Frame {frame_count:5d} | {frame_width}x{frame_height} | Hash: {frame_hash}")

        elapsed = time.time() - start_time
        sleep_time = FRAME_INTERVAL - elapsed
        if sleep_time > 0:
            time.sleep(sleep_time)

except KeyboardInterrupt:
    print("Stopped by user")
finally:
    conn.close()