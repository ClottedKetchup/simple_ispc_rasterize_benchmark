# simple ISPC rasterization benchmark

A simple rasterization benchmark written with Intel ISPC.

## Tested environment

- Windows
- AMD Ryzen 7 9850X3D 8-Core Processor
- MSVC 19.44.35225.0 (VS2022)
- ISPC v1.30.0
- AVX2

## Build

### CMake GUI

1. Open the CMake GUI, click Configure, and set the ISPC compiler path (`ISPC_EXECUTABLE`), `ISPC_ISA`, and `ISPC_TARGET_ARCH`.
2. Click Generate, then open the generated solution in Visual Studio and build it.

### Command line

```bash
cmake -S . -B build -DISPC_EXECUTABLE="C:/path/to/ispc.exe" -DISPC_ISA=avx2 -DISPC_TARGET_ARCH=x86-64
cmake --build build --config Release
```
