# https://stackoverflow.com/questions/27606079/method-to-uniformly-randomly-populate-a-disk-with-points-in-python

import math
import random

n = 128

alpha = math.pi * (3 - math.sqrt(5))    # the "golden angle"
phase = random.random() * 2 * math.pi
points = []
offsetScale = 0.2;
for k in range(n):
  theta = k * alpha + phase
  r = math.sqrt(float(k)/n)
  dx = (random.random() - 0.5) * offsetScale
  dy = (random.random() - 0.5) * offsetScale
  points.append((r * math.cos(theta) + dx, r * math.sin(theta) + dy))

import numpy as np

samples = np.array(points)

# print(samples.shape)

print("    static const float2 diskSamples",n,"[",n,"] =")
print("    {")
for i, p in enumerate(points):
	print("    float2(", p[0], ", ", p[1], "), ", end="")
	if i % 2 == 0:
		print("")
print("    };")

import matplotlib.pyplot as plt

fig, ax = plt.subplots(figsize=(4, 4))
ax.set_aspect("equal")

ax.scatter(samples[:,0], samples[:,1], c="red", s=0.1)

ax.set_xlabel("x")
ax.set_ylabel("y")
ax.set_title("Monte Carlo Pi")

plt.show()
