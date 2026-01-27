import http.client
import hashlib
import time
import shutil
from PIL import Image
from pathlib import Path

WIDTH = 800
HEIGHT = 480
FRAME_SIZE = (WIDTH * HEIGHT) // 8
FRAME_INTERVAL = 0.5  # 200 ms
FRAMES_DIR = Path("frames")

# Clear and recreate frames folder
if FRAMES_DIR.exists():
    shutil.rmtree(FRAMES_DIR)
FRAMES_DIR.mkdir()
print(f"Cleared {FRAMES_DIR}/")

conn = http.client.HTTPConnection("192.168.110.130", timeout=5)

last_hash = None
frame_count = 0

try:
    while True:
        start_time = time.time()

        conn.request("GET", "/frame")
        res = conn.getresponse()
        data = res.read()
        # Get CRC from header
        expected_crc = res.getheader("X-Frame-CRC")

        md5_hash = hashlib.md5(data).hexdigest()

        if md5_hash != last_hash:
            frame_count += 1
            last_hash = md5_hash

            img = Image.frombytes("1", (WIDTH, HEIGHT), data)
            img = img.rotate(-90, expand=True)

            img.save(FRAMES_DIR / f"frame_{frame_count:05d}.png")

            print(f"Saved {frame_count:5d} - {expected_crc}")

        elapsed = time.time() - start_time
        sleep_time = FRAME_INTERVAL - elapsed
        if sleep_time > 0:
            time.sleep(sleep_time)

except KeyboardInterrupt:
    print("Stopped by user")
finally:
    conn.close()