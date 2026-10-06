# CLAUDE.md

GeoGen is an MFC + Direct3D 11 node-based procedural terrain generator (C++, Windows only).
See README.md for features and the full build layout.

## Build

- Build `GeoGen.sln` with MSBuild/Visual Studio, **x64** (Debug or Release). There are no
  tests, no CI and no CMake.
- The project **only builds from inside the original source tree**. `GeoGen.vcxproj` and
  `GeoGen.sln` use `../../Shared/...`, `../../Extern/...` and `../../Games/*.props`, and the
  output goes to `../../../Build/`. On this machine that tree is `D:\Dev\Code` (the original
  working copy is `D:\Dev\Code\Sandbox\GeoGen`, and the build output is `D:\Dev\Build`). A clone
  anywhere else (e.g. `D:\Dev\YukaSoftware\GeoGen`) fails to resolve those dependencies.
  Don't "fix" this by rewriting paths unless asked.
- Include paths, `XTM_*` defines, forced include of `stdafx.h`, no exceptions and no RTTI
  (`/EHs-c-`, `/GR-`), fast FP and AVX all come from `Games/*Config*.props`, not from the vcxproj.
- Toolset is `v142`. Only VS2022 (v143) is installed here, so either pass
  `/p:PlatformToolset=v143` or retarget.
- Run with the working directory set to the project folder: shaders load from the relative
  path `Shaders/*.hlsl` at runtime. The `.hlsl` files are `ExcludedFromBuild` in the vcxproj
  and are never compiled offline.

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

`MakeVersion.bat` and `TODO.txt` contain hard-coded FTP credentials and API keys from the
original author. Never copy them elsewhere, and never add new credentials to the repo.
