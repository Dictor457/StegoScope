# StegoScope

A desktop forensic analysis utility and cryptographic visualizer implemented in modern C++20 using Dear ImGui and OpenGL 3.3 Core Profile.

Designed for reverse engineers, digital forensics analysts, and CTF participants to analyze image payloads, detect anomalies via information density mapping, and inspect cryptographic transformations in real time

## Core Capabilities

### 1. Bit-Plane Decomposition
* Matrix decomposition isolating individual bit-planes from bit 7 (MSB) down to bit 0 (LSB) across independent R, G, B channels.
* Direct visualization of quantization noise, DCT coefficients degradation, and hidden payload patterns.
* Dynamic bitmask manipulation applied directly via CPU memory buffer before texture streaming.

### 2. Shannon Entropy Heatmap
* Spatial entropy estimation over localized pixel kernels (8x8 and 16x16 blocks).
* Implements the discrete Shannon entropy formula:
  $$H(X) = -\sum_{i=0}^{255} P(x_i) \log_2 P(x_i)$$
* Maps high-entropy areas (compressed data, AES/ChaCha payloads, encrypted headers) as localized heat values against natural image noise.

### 3. LSB Steganography Engine
* Real-time bitstream insertion and extraction directly into the least significant bits.
* Configurable channel targeting (interleaved RGB, individual channels, variable bit depth per pixel).
* Binary payload injection with length prefix and raw byte dumping capabilities.

### 4. AES-128 State Machine Visualizer
* Interactive step-by-step breakdown of the standard Rijndael cipher state matrix across all 10 rounds.
* Visual transformation tracing for:
  * SubBytes (Non-linear substitution via S-Box)
  * ShiftRows (Cyclic byte shifting per row)
  * MixColumns (Matrix multiplication in Galois Field $GF(2^8)$)
  * AddRoundKey (Bitwise XOR against round key schedule)

### 5. Technical Specifications
* Graphics Pipeline: OpenGL 3.3 Core Profile, GLFW3 window context.
* GUI Framework: Dear ImGui (docking branch) with optimized draw lists.
* Standard: ISO C++20 (`std::span`, concepts, designated initializers).
* Supported Image Formats: PNG, BMP, TGA, JPEG (8-bit per channel via stb_image).

## Dependencies

The project relies on standard system libraries:
* C++20 compliant compiler (GCC 11+, Clang 13+, or MSVC 19.29+)
* CMake 3.20 or newer
* OpenGL 3.3+ development headers
* GLFW3
* PkgConfig

## Building and Installation

### Arch Linux
```bash
# Install dependencies
sudo pacman -S --needed cmake gcc git glfw-x11 pkgconf zenity

# Clone and configure
git clone https://github.com/Dictor457/StegoScope.git
cd StegoScope

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run
./build/stegoscope
```

### Debian / Ubuntu
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y cmake g++ git libglfw3-dev libgl1-mesa-dev pkg-config zenity

# Build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/stegoscope
```

### Fedora
```bash
# Install dependencies
sudo dnf install -y cmake gcc-c++ git glfw-devel mesa-libGL-devel pkgconf zenity

# Build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/stegoscope
```

### Windows (MSVC / Visual Studio 2022)
```powershell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\Release\stegoscope.exe
```

## Running on Wayland

If running on Linux under Wayland (Hyprland, Sway), ensure GLFW uses the appropriate platform backend:
```bash
# Force Wayland backend
export GLFW_PLATFORM=wayland
./build/stegoscope

# Or fallback to XWayland if fractional scaling issues occur
export GLFW_PLATFORM=x11
./build/stegoscope
```

## Architecture Overview

```
[ Image Source (File / Drag-and-Drop) ]
                   │
                   ▼
       [ stb_image / Memory Buffer ]
                   │
         ┌─────────┴─────────┐
         ▼                   ▼
[ Bit-Plane Extractor ]   [ Shannon Entropy Estimator ]
         │                   │
         └─────────┬─────────┘
                   ▼
     [ GL Texture Dynamic Streaming ]
                   │
                   ▼
    [ Dear ImGui Viewport Renderer ] <──> [ AES-128 State Machine Engine ]
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
