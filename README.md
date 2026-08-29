# StegoScope

A modern, high-performance forensic toolkit & crypto visualizer built with C++20, Dear ImGui, and OpenGL.

Designed for security researchers, CTF players, and low-level enthusiasts. Fully cross-platform (Linux & Windows).

## Features

- Bit-Plane Slicing (8x3 Matrix): Isolate individual bits (7 MSB down to 0 LSB) across RGB channels to uncover hidden DCT quantization steps, watermarks, and payload streams.
- Shannon Entropy Heatmap: Spatial information density estimator to instantly pinpoint compressed or encrypted binary containers.
- LSB Steganography: Embed and extract raw payloads into image bitstreams in real-time.
- AES-128 (Rijndael) Interactive Engine: Step-by-step 10-round state matrix machine visualizing SubBytes (S-Box), ShiftRows, MixColumns (GF(2^8) Galois Field), and AddRoundKey.
- Pixel Telemetry Inspector: Live mouse hover reporting exact coordinates, HEX color, and raw channel vectors.
- Modern Liquid Glass UI: Responsive frosted dark design with native Drag & Drop support.

## Build Instructions

Linux (Arch / Debian / Fedora):
  sudo pacman -S --needed cmake gcc git glfw pkgconf zenity
  cmake -B build -S .
  cmake --build build -j$(nproc)
  ./build/stegoscope

Windows (MSVC / MinGW):
  cmake -B build -S .
  cmake --build build --config Release
  ./build/Release/stegoscope.exe

## License
MIT License. Created by Dictor (https://github.com/Dictor457).
