# CLAUDE.md

GeoGen is an MFC + Direct3D 11 node-based procedural terrain generator (C++, Windows only).
See README.md for features and build steps.

## Build

- CMake only (`CMakeLists.txt` + `CMakePresets.json`): `cmake --preset vs2022`, then
  `cmake --build --preset release` (or `debug`). Generator is VS 2022, toolset v143, x64, and the
  build dir is `build/` (ignored by git). There are no tests and no CI.
- **The build is intentionally incomplete.** Dependencies are limited to: Windows SDK system
  libs (`d3d11`, `dxgi`, `dxguid`, `d3dcompiler`, `ws2_32`), TinyXml2 11.0.0 via `FetchContent`,
  and the xtm/Core subset vendored in `Core/`. Don't add OpenSSL,
  NVAPI, libjpeg, squish, LightZPNG or any out-of-repo path. SimpleD3DFramework (`g_Renderer`,
  `D3DObject<T>`, `ConstantBuffer<T>`) is not in xtm and has no replacement yet.
- `Core/` is a subset of github.com/ftournade/xtm's `Core/`. It's compiled directly into the
  GeoGen executable (`SRC_CORE` list, same PCH and Unicode settings). Don't make it a separate
  library. `Core/xtm_prelude.h` replaces `Core/stdafx.h` (never include that). Local changes:
  JPG/DXT disabled in `Bitmap.cpp`, `Bitmap_PNG_Unsupported.cpp` instead of `Bitmap_PNG.cpp`,
  and `DLLMain.h`/`CORE_API` removed. GeoGen gets the Core headers through `stdafx.h`.
- The source file list in `CMakeLists.txt` is explicit (no globbing). New `.cpp/.h` files
  must be added to the matching `SRC_*` list, which also sets their IDE folder.
- `stdafx.h` is the precompiled header (`target_precompile_headers`, which also force-includes it).
  MFC is a shared DLL (`CMAKE_MFC_FLAG 2` + `_AFXDLL`), Unicode, and the entry point is
  `/ENTRY:wWinMainCRTStartup`.
- Building requires the "C++ MFC for latest v143 build tools" VS component (otherwise MSB8041).
- Shaders load from the relative path `Shaders/*.hlsl` at runtime and are never compiled
  offline (`HEADER_FILE_ONLY`). A post-build step copies `Shaders/` next to the exe, and the VS
  debugger working directory is the repo root.

## Architecture

- `GeoGen.cpp` (`CGeoGenApp theApp`) holds global settings: resolution, min/max altitude,
  sea level, terrain extent, camera. `MainFrm` hosts the panes `NodeEditorView` (graph),
  `CPreviewWnd` (3D preview + iterative sims), `CPropertiesWnd` (node params) and `COutputWnd`.
- Globals: `g_Renderer` (SimpleD3DFramework), `g_NodeEditor`, `g_NodeFactory`, `g_log`.
- `ComputeNode` (`ComputeNode.h`) is the base class: input/output/param slots, resolution
  handling, dirty propagation, XML `Load`/`Save`, `InternalCompute()`, and optional iterative
  sim hooks (`InitSim`/`StepSim`/`RenderSimPreview`). GPU data is a `Map` (texture + SRV/UAV).
- `CustomComputeNode` is the generic HLSL-snippet node. Most simple nodes live in
  `BasicComputeNodes.h` as small subclasses. The snippet sees `_pos`, `_uv`, `_wsPos`,
  `_input0..N`, `_output0..N`, the params by name, and `MinAltitude`/`MaxAltitude`/`Extent`/`Resolution`.
- Complex nodes (erosion, snow, blur, mountain, scatter, curve, gradient) subclass
  `ComputeNode` directly and load entry points via `g_Renderer.CreateShader("Shaders/X.hlsl", "Entry", ...)`.
- `DEMGrabFrm`/`DEMGrabView` + `HTTPConnection` (raw sockets + OpenSSL) download elevation
  and imagery tiles.

## Adding a node

1. Simple per-pixel op: add a `CustomComputeNode` subclass to `BasicComputeNodes.h`, following
   the existing pattern: `COMPUTE_NODE_FACTORY(Name)`, a ctor with `CustomComputeNode(nIn, nOut)`,
   then `SetUIName`, `AddParam(...)` and `SetHLSLBody(...)`.
2. Register it in `NodeEditor.cpp` with `REGISTER_COMPUTE_NODE(Class, Category, "UI Name", id)`.
   IDs are grouped by category (Generator 10+, Combiner 110+, Modifier 210+, Mask 310+,
   Natural 410+, I/O 510+) and must be unique (`DBG_CHECK`).
3. New `.cpp/.h` files must be added to `GeoGen.vcxproj` **and** `GeoGen.vcxproj.filters`.

## Compatibility gotchas

- `.geo` project files are XML. Each node is saved as an element named by its **C++ class
  name**, and links are stored as node index + **slot index**. Renaming a node class or
  reordering its inputs, outputs or params breaks existing saved files.
- `Shaders/ErosionConstants.h` is included by both HLSL and C++ (`ErosionNode.h`), so keep it
  valid in both languages. Other nodes inject `#include "Shaders/..."` into runtime-compiled
  HLSL through string prefixes (e.g. `MonteCarloErosionNode.cpp`, `SnowNode.cpp`).

## Code style

- Tabs, CRLF line endings, spaces inside parentheses: `Foo( a, b )`.
- Members `m_Name`, booleans `m_bName`, pointers `m_pName`, parameters `_name`, globals `g_Name`.
- Types from xtm: `u32`, `s32`, `Str`, `Vec2`, `Color`. COM objects are held in `D3DObject<T>`.
- Every `.cpp` starts with `#include "stdafx.h"`. MFC message maps for UI classes.
- No exceptions or RTTI. Return `bool` for failure and log with `LOG(...)`.

## Secrets

`TODO.txt` contains hard-coded API keys from the original author, and git history still
contains FTP credentials from the deleted `MakeVersion.bat`. Never copy them elsewhere, and
never add new credentials to the repo.
