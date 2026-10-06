# GeoGen

GeoGen is a node-based procedural terrain generator for Windows. You build a graph of
nodes (noise generators, modifiers, masks, erosion simulations, etc.); each node runs as a
Direct3D 11 compute shader on the GPU, and the resulting heightfield is previewed live in 3D.

It also includes a **DEM Grabber** that downloads real-world elevation tiles (Nextzen /
Mapzen terrain tiles) and aerial imagery (HERE) so real terrain can be used as an input.

## Features

- **Node graph editor** with typed input/output slots and per-node parameters
  (sliders, color pickers, curves, gradients, combo boxes, file pickers).
- **GPU compute**: most nodes are small HLSL snippets wrapped in a generic compute node and
  compiled at runtime.
- **Node library** (registered in `NodeEditor.cpp`):
  | Category  | Nodes |
  |-----------|-------|
  | Generator | Perlin Noise, Voronoise, Voronoi, Mountain (river-network driven), Radial, Gradient, Constant Color/Value, Scatter Map, Checkerboard |
  | Combiner  | Combine, Mix, Channel Split, Channel Merge |
  | Modifier  | Terrace, Re-Range, Scale & Bias, Invert, Curve, Distort, Blur, Directional Blur, Sharpen, Color Gradient, Extract Detail, Expander, Power, Absolute, Periodic, HSV |
  | Mask      | Altitude Mask, Slope Mask, Convexity Mask |
  | Natural   | Fake Erosion (Minstrel pseudo-erosion), Erosion (Monte-Carlo, GPU), Erosion (grid-based, GPU), Erosion (CPU), Snow |
  | I/O       | Preview, Input Bitmap, Output Bitmap, Normal |
- **Iterative simulations** (erosion, snow) that can run step-by-step with a live preview.
- **3D preview** window with configurable camera, altitude range, sea level and terrain extent.
- **Project files** saved as XML with the `.geo` extension.

## Tech stack

- C++ / **MFC** (Visual Studio "MFC Application" with docking panes)
- **Direct3D 11** compute/vertex/pixel shaders (HLSL in `Shaders/`, compiled at runtime)
- **TinyXml2** for project files, **OpenSSL** for HTTPS tile downloads, **NVAPI** (x64)
- Yuka's in-house libraries **xtm/Core** and **SimpleD3DFramework**, which are **not** part of this repo

## Building

> ⚠️ This repo can't be built on its own. It is one project inside a larger source tree,
> and it uses relative paths to shared libraries, third-party SDKs and MSBuild property
> sheets that live outside the repo.

### Requirements

- Windows 10+
- Visual Studio with the **Desktop development with C++** workload and **MFC**
  (the project targets the **v142** / VS2019 toolset; with only VS2022 installed, retarget to v143)
- Windows 10 SDK (`d3d11.lib`, `d3dcompiler.lib`, `dxgi.lib`)

### Expected directory layout

The `.sln`/`.vcxproj` reference everything relative to the project folder, so the project
has to sit **two levels below** a code root that contains `Shared/`, `Extern/` and `Games/`.
The original layout is:

```
<DevRoot>/
├── Build/                         # output: GeoGen-<Config>-<Platform>.exe  (OutDir = ../../../Build/)
├── Temp/Games/GeoGen/...          # intermediate files
└── Code/
    ├── D3D10 Config.props, VS2010_settings*.props
    ├── Games/                     # "<Debug|Release> D3D10 <x32|x64> Config.props" (imported by GeoGen.vcxproj)
    ├── Shared/
    │   ├── xtm/Core/              # Core.vcxproj    (math, logging, bitmaps, strings...)
    │   └── SimpleD3DFramework/    # SimpleD3DFramework.vcxproj (Renderer, D3DObject...)
    ├── Extern/
    │   ├── libjpeg/  Squish/  TinyXml2/  LightZPNG/
    │   ├── OpenSSL/               # include/, x64/lib, x64/bin (1.1.x)
    │   └── NVAPI/amd64/           # NVAPI64.lib
    └── Sandbox/
        └── GeoGen/                # <- this repository
```

### Steps

1. Clone (or link) this repo to `<DevRoot>/Code/Sandbox/GeoGen`.
2. Open `GeoGen.sln` in Visual Studio.
3. Select **Release | x64** (or Debug | x64) and build. The solution also builds
   `Core`, `SimpleD3DFramework`, `jpeg`, `squish`, `tinyxml2` and `LightZPNG`.
4. The executable is written to `<DevRoot>/Build/GeoGen-Release-x64.exe`.

Command-line equivalent (from a VS Developer prompt):

```bat
msbuild GeoGen.sln /p:Configuration=Release /p:Platform=x64 /m
```

### Running

Shaders are loaded at runtime from the relative path `Shaders/`, so the **working
directory must be the folder that contains `Shaders/`**. Visual Studio's default debugger
working directory (`$(ProjectDir)`) handles this. For a standalone build, copy the
`Shaders` folder next to the executable.

The OpenSSL 1.1 DLLs (`libssl-1_1-x64.dll`, `libcrypto-1_1-x64.dll`) must be on the
`PATH` or next to the exe for the DEM Grabber to work. Logs are written to `log.html` in
the working directory.

### Packaging

`MakeVersion.bat` asks for a version number, copies `Build/GeoGen-Release-x64.exe` and
the `Shaders/` folder into `<DevRoot>/Packaging/GeoGen/Build_<version>/`, and zips the
result with 7-Zip.

## DEM Grabber setup

Downloading elevation and imagery tiles needs your own API keys, which you enter in the
app's tile-provider setup dialog:

- Nextzen: <https://developers.nextzen.org>
- HERE: <https://developer.here.com>

## Status

This is a personal R&D project (2017–2021) with no tests or CI. Known bugs and ideas are
tracked in `TODO.txt`.
