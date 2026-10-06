# GeoGen

GeoGen is a node-based procedural terrain generator for Windows. You build a graph of
nodes (noise generators, modifiers, masks, erosion simulations, etc.); each node runs as a
Direct3D 11 compute shader on the GPU, and the resulting heightfield is previewed live in 3D.

It also includes a **DEM Grabber** that downloads real-world elevation tiles (AWS Terrain
Tiles) and satellite imagery (MapTiler) so real terrain can be used as an input.

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
  | Natural   | Fake Erosion (Minstrel pseudo-erosion), Fake Erosion V2 (runevision's erosion filter), Mountain Coloring, Erosion (Monte-Carlo, GPU), Erosion (grid-based, GPU), Erosion (CPU), Snow |
  | I/O       | Preview, Input Bitmap, Output Bitmap, Normal |
- **Fake Erosion V2** applies runevision's stacked-gully erosion filter to any heightmap
  (Scale in km, optional strength mask) and outputs the eroded height plus Erosion, Ridges and
  Drainage masks. **Mountain Coloring** turns them into the filter's color map, as separate
  Albedo and Lighting maps (Combine → Mul of the two gives the lit color) plus a Trees mask.
- **Iterative simulations** (erosion, snow) that can run step-by-step with a live preview.
- **3D preview** window with configurable camera, altitude range, sea level and terrain extent.
- **Project files** saved as XML with the `.geo` extension.

## Tech stack

- C++ / **MFC** (Visual Studio "MFC Application" with docking panes)
- **Direct3D 11** compute/vertex/pixel shaders (HLSL in `Shaders/`, compiled at runtime)
- **TinyXml2** for project files (fetched by CMake)
- **WinHTTP** for DEM tile downloads (HTTPS) and **stb_image** for PNG/JPEG loading (fetched by CMake)
- Yuka's in-house **xtm/Core** (subset vendored in `Core/`) and **SimpleD3DFramework** (recreated in `SimpleD3DFramework/`)

## Building

The project is built with **CMake** and the **Visual Studio 2022 (v143)** toolset.

Dependencies are the Windows SDK, TinyXml2 and stb_image (fetched by CMake), a subset of
xtm/Core vendored in `Core/`, and `SimpleD3DFramework/`, a recreation of the original (lost)
renderer layer.

> ⚠️ RenderDoc capture hooks are no-ops in the recreated renderer. Known bugs are listed in
> `bugs.md`.

### Requirements

- Windows 10+
- Visual Studio 2022 with the **Desktop development with C++** workload **and** the
  *C++ MFC for latest v143 build tools (x86 & x64)* component (Individual components tab)
- Windows 10/11 SDK (`d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`, `dxguid.lib`)
- CMake 3.21+

### Steps

```bat
cmake --preset vs2022
cmake --build --preset release
```

The first command generates `build/GeoGen.sln`. Use `--preset debug` for a debug build.
The executable is written to `build/<Config>/GeoGen.exe`, and `Shaders/` is copied next
to it after each build. You can also open `build/GeoGen.sln`, or open the folder directly
in Visual Studio, which picks up `CMakePresets.json`.

### Running

Shaders are compiled at runtime from the relative path `Shaders/`, so the **working
directory must contain `Shaders/`**. The generated Visual Studio project sets the debugger
working directory to the repo root. A standalone run works from `build/<Config>/`.

Logs are written to `log.html` in the working directory.

## DEM Grabber setup

The DEM Grabber (File menu) downloads Web Mercator tiles and caches them in
`Documents\GeoGen\`:

- **Elevation:** [Terrain Tiles on AWS](https://registry.opendata.aws/terrain-tiles/)
  (Mapzen/Tilezen "terrarium" tiles, zoom 0–15). Free and needs no key. Attribution is required;
  the data sources are listed in the
  [Tilezen documentation](https://github.com/tilezen/joerd/blob/master/docs/attribution.md).
- **Satellite imagery:** [MapTiler](https://www.maptiler.com/cloud/) Satellite. It needs an API
  key: the free plan is for non-commercial use and has a monthly request quota. Create a key at
  <https://cloud.maptiler.com/account/keys/> and enter it in Settings > GeoData provider, or when
  the grabber first asks for it. Without a key, only elevations are captured. Attribution:
  "© MapTiler © OpenStreetMap contributors".

The grabber view shows both attributions.

## Third-party code

- **Fake Erosion V2** and **Mountain Coloring** are ported from runevision's
  [Advanced Terrain Erosion Filter](https://www.shadertoy.com/view/wXcfWn) Shadertoy
  (see the [blog post](https://blog.runevision.com/2026/03/fast-and-gorgeous-erosion-filter.html)),
  copyright (c) 2025 Rune Skovbo Johansen, under the
  [Mozilla Public License 2.0](https://mozilla.org/MPL/2.0/). The ported files keep that license:
  `Shaders/FakeErosionCommon.h`, `Shaders/FakeErosionV2.hlsl` and `Shaders/MountainColoring.hlsl`.
  The Shadertoy's coloring and lighting derive from [Fewes' terrain Shadertoy](https://www.shadertoy.com/view/7ljcRW),
  and its gradient noise from [Inigo Quilez](https://www.shadertoy.com/view/XdXBRH).

## Status

This is a personal R&D project (2017–2021) with no tests or CI. Known bugs are listed in
`bugs.md`, and ideas in `TODO.txt`.
