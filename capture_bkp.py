import serial
import struct
import time
import shutil
from pathlib import Path
from threading import Thread
from queue import Queue
from PIL import Image

WIDTH = 800
HEIGHT = 480
FRAME_SIZE = (WIDTH * HEIGHT) // 8
FRAMES_DIR = Path("frames")

# Clear and recreate frames folder
if FRAMES_DIR.exists():
    shutil.rmtree(FRAMES_DIR)
FRAMES_DIR.mkdir()
print(f"Cleared {FRAMES_DIR}/")

save_queue = Queue()


def saver_thread():
    while True:
        frame_count, img = save_queue.get()
        if img is None:
            break
        img.save(FRAMES_DIR / f"frame_{frame_count:05d}.bmp")


saver = Thread(target=saver_thread, daemon=True)
saver.start()

ser = serial.Serial("/dev/cu.usbmodem21301", 2000000, timeout=1)  # Higher baud

frame_count = 0
start_time = time.time()

print(f"Listening for frames ({WIDTH}x{HEIGHT}, {FRAME_SIZE} bytes)...")

try:
    while True:
        if ser.read(1) != b"\xAA":
            continue

        raw_len = ser.read(4)
        if len(raw_len) < 4:
            print("Timeout reading length")
            continue

        length = struct.unpack("<I", raw_len)[0]

        if length != FRAME_SIZE:
            print(f"Unexpected length: {length} (expected {FRAME_SIZE})")
            continue

        data = ser.read(length)
        if len(data) < length:
            print(f"Incomplete frame: {len(data)}/{length} bytes")
            continue

        if ser.read(1) != b"\x55":
            print("Invalid end marker")
            continue

        frame_count += 1
        elapsed = time.time() - start_time
        fps = frame_count / elapsed if elapsed > 0 else 0

        img = Image.frombytes("1", (WIDTH, HEIGHT), data)
        img = img.rotate(-90, expand=True)
        save_queue.put((frame_count, img))

        print(f"Frame {frame_count:5d} | Queue: {save_queue.qsize():3d} | {fps:.2f} FPS")

except KeyboardInterrupt:
    print("\nStopping...")
    save_queue.put((0, None))
    saver.join()