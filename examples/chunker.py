#!/usr/bin/env -S GI_TYPELIB_PATH=${PWD}/_build/src:${GI_TYPELIB_PATH} LD_PRELOAD=${LD_PRELOAD}:${PWD}/_build/src/libmars.so python3

import os
import sys
import time
import gi

gi.require_versions({"Gst": "1.0", "Mars": "1.0"})
from gi.repository import Gst
from gi.repository import Mars

# To keep the logic simple, we just read `data/sample.wav` and write to `output`
# using `wavenc`. See `examples/chunker.c` for a complicated logic.
# Run the example as `./examples/chunker.py`.

Gst.init(sys.argv)

os.makedirs("output", exist_ok=True)

chunker = Mars.Chunker(
    input="data/sample.wav", output="output/%02d.wav", muxer="wavenc"
)
chunker.play()

print("Waiting for chunking to complete…")
time.sleep(1)

while chunker.is_playing():
    pass
