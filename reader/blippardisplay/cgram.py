#!/usr/bin/env python
from PIL import Image
import sys

rows = 6
cols = 16
width = 6
height = 8

im = Image.open(sys.argv[1])
pixels = im.load()
col = 0
out = sys.stdout
for row in range(rows):
  for col in range(cols):
    out.write("{")
    for x in range(width):
      colData = 0
      bitMask = 1
      for y in range(height):
        (_, pixelGreen, _) = pixels[(col * width) + x, (row * height) + y]
        if pixelGreen != 0:
          pixelBit = 1
        else:
          pixelBit = 0
        colData = colData | (bitMask * pixelBit)
        bitMask = bitMask << 1
      out.write("0x%02x" % colData)
      if x < width - 1:
        out.write(", ")
      else:
        out.write("},\n")
  out.write("\n")
