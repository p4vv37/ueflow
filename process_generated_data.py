import pandas as pd
import numpy as np
import os
import pickle
import math

from collections import defaultdict

samples = os.listdir(os.path.join("samplesGeneration", "data"))

loaded_samples = defaultdict(list)
for sample in samples:
    name = sample.rsplit("_", 1)[0]
    loaded_samples[name].append(sample)

np_samples = dict()
first_frame_description = ""
for num, (name, frames) in enumerate(loaded_samples.items()):
    if num % 100:
        print "{}%".format(100.0 * num / len(loaded_samples))
    frames = sorted(frames)
    np_frames = list()
    for frame in frames:
        with open(os.path.join("samplesGeneration", "data", frame), "r") as f:
            data = pickle.load(f)
        if not np_frames and not np_samples:
            num_frames = 3
            num_blocks = len(data) - 1  # excluding graoundplane
            first_frame_description = "{};{}\n".format(num_frames, num_blocks)
            for block in data[:-1]:
                first_frame_description += "{name};{x};{y};{z};{rot_xa};{rot_xb};{rot_ya};{rot_yb};{rot_za};{rot_zb}\n" \
                                           "".format(name=block[-1],
                                                     x=block[0][0], y=block[0][1], z=block[0][2],
                                                     rot_xa=math.sin(block[1][0]), rot_xb=math.cos(block[1][0]),
                                                     rot_ya=math.sin(block[1][1]), rot_yb=math.cos(block[1][1]),
                                                     rot_za=math.sin(block[1][2]), rot_zb=math.cos(block[1][2]),
                                                     )

        np_frames.append(data)
    np_samples[name] = np_frames
print(first_frame_description)
with open("first_frame.cfg", "w") as f:
    f.write(first_frame_description)
