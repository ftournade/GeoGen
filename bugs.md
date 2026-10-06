# Known bugs and gaps

Found by reading the code (not by running it), ordered by decreasing severity.
Not covered in depth: most shader code, `SnowNode.cpp`, `DEMGrabView.cpp`, and the MFC
dialogs beyond where they touch nodes.

## Crashes and memory corruption

1. **Use-after-free in the custom node panels.** The panels for Constant Color, Curve, Color
   Gradient and Mountain keep a raw pointer to their node (`BasicComputeNodes.cpp:15`,
   `CurveNode.cpp:46`, `ColorGradientNode.cpp:46`, `MountainNode.cpp:131`).
   `CPropertiesWnd::Clear()` (`PropertiesWnd.h:25`) only empties the property list and leaves
   the panel alive. Deleting the node (Del, File > New, File > Open) and then using the panel
   that is still on screen writes into a freed node.
2. **Cycles in the graph overflow the stack.** `NodeEditor::CreateLink` (`NodeEditor.cpp:612`)
   never checks for a cycle (see the TODO at `NodeEditorView.cpp:275`). Linking a node to itself
   or to anything upstream makes `SetDirty()` and `Compute()` recurse forever.
3. **Projects with a "Fixed" resolution node can't be reloaded.** `UpdateInternalResolution`
   never sets `m_Resolution` when the mode is `Res_Fixed` (`ComputeNode.cpp:419`). After a load it
   stays 0, texture creation fails and a null texture is dereferenced later. This affects every
   node the DEM Grabber creates. Picking "Fixed" from the context menu also leaves
   `m_FixedResolution = 0`, and that 0 is what gets saved.
4. **`NodeEditor::Load` falls over on imperfect files:**
   - an unknown node class pushes `nullptr` into `m_AllNodes`, which crashes at
     `NodeEditor.cpp:782`;
   - a missing `<NodeLinks>` element dereferences null (`NodeEditor.cpp:798`);
   - link node and slot indices are never bounds-checked;
   - in `ComputeNode::Load`, a missing `Name`, Color or String attribute crashes (constructing a
     `std::string` from null, `sscanf( NULL, ... )`);
   - a missing Float/Int/Bool param loads as 0 instead of its default, so adding a param to a
     node silently breaks older files.
5. **Editing properties after File > Open crashes.** `OnFileLoad` never clears the property
   grid, and `CPropertiesWnd::OnPropertyChanged` (`PropertiesWnd.cpp:362-370`) dereferences an
   expired `weak_ptr` (Release builds have no ASSERT to stop it).
6. **Deleting the height source of a previewed Preview node crashes.** `GetPreviewHeight` calls
   `pPreviewNode->GetHeightMap()->GetSRV()` with no null check (`NodeEditor.cpp:231`).
7. **"Relative to first input" on a node with no inputs** (Radial, Gradient, Constant Color,
   Constant Value) indexes `m_InputSlots[ 0 ]` on an empty vector (`ComputeNode.cpp:400-401`).
8. **Heap overflow in `MonteCarloErosionNodeCPU::InitSim`** (`MonteCarloErosionNodeCPU.cpp:114`).
   `Map::CopyFromGPU( float* )` doesn't know the destination size, and `InitSim` sizes it with the
   node's own resolution, so it overflows whenever the input map is larger. (The padded row pitch
   overflow in `CopyFromGPU` itself is fixed: it now copies row by row.)
9. **A failed renderer start crashes the preview, and node registration depends on it.** If
   `g_Renderer.Init` fails, `CPreviewWnd::Render()` still runs with a null device context. Nodes
   are only registered (`g_NodeEditor.Init()`, `PreviewWnd.cpp:73`) on the preview pane's first
   paint: if that pane is hidden, closed, or restored hidden from the saved layout, the Add Node
   menu is empty and loading a file pushes null nodes (see 4).

## Data loss

10. **File > New doesn't reset `m_Filename`** (`GeoGen.cpp:309`). A later Save silently
    overwrites the last file that was opened. There is also no unsaved-changes prompt on New,
    Open or exit.
11. **Mountain rivers aren't saved.** `MountainNode` has no `Load`/`Save` override, so the river
    network drawn in its editor is lost on save and reload.

## Wrong results

12. **Resolution handling:**
    - Generated HLSL hard-codes the global resolution (`CustomComputeNode.cpp:342`), and `_uv` /
      `quadLength` derive from it. Any node with a resolution modifier, a fixed resolution or a
      differently sized input computes wrong UVs (a 1/2 node only covers a quarter of the noise;
      slope and normal values are mis-scaled).
    - Changing one node's resolution reference or modifier doesn't update downstream "Relative to
      first input" nodes, which keep textures of the old size. `ErosionNode::InitSim`'s
      `CopyResource` then silently fails (`ErosionNode.cpp:111`), and the CPU erosion overflows
      (see 8).
    - `NodeEditorView::OnSetNodeResolutionReference` never calls `SetDirty()` or redraws.
    - The modifier is only saved for `Res_MainInput` (`ComputeNode.cpp:532-537`), so e.g.
      "Global x2" is lost on reload.
    - Every caller ignores the return value of `OnResolutionChanged()`. For example 16384 x4
      exceeds the D3D11 texture limit, the textures end up null, and they are dereferenced later.
13. **Changing the terrain extent doesn't recompute anything** (`GeoGen.cpp:283`). Extent is baked
    into the generated shaders, which aren't recompiled, and nodes aren't marked dirty. Nodes
    recompiled later for other reasons pick up the new value, so the graph mixes old and new
    extents.
14. **Output type is only resolved when a link is made.** Distort and Scatter Map always output
    R32, so color inputs lose G/B/A. Combine, Mix and Directional Blur re-check their output type
    only when a link is made on that node; if an upstream node's format changes later, they keep a
    shader with the wrong texture type.
15. **Input Bitmap switches to RGBA8 at compute time** (`InputBitmapNode.cpp:64-74`), after links
    were validated as Float. Nothing else handles that format (Output Bitmap asserts RGBA16F), and
    alpha comes out as 0 because it goes through `COLORREF`.
16. **Output Bitmap with an unsupported extension writes the wrong format.** `Bitmap::Save` falls
    back to xtm's internal `.tex` layout for any extension it doesn't know (png, tga, tif...)
    without reporting an error.
17. **Live-sim results don't reach downstream nodes.** After the 'S' iterative sim, downstream
    nodes aren't marked dirty, and the next normal `Compute()` overwrites the sim result.
18. **Border artifacts in Normal, Slope Mask and Convexity Mask.** They read
    `uint2( _pos.x - 1, ... )`, which wraps at 0; out-of-bounds texture reads return 0, so edge
    pixels get fake cliffs.
19. **Directional Blur can hang the GPU or divide by zero.** `numTaps = Distance * Resolution`
    (up to 2048 taps per pixel) risks a GPU timeout (TDR). When `Distance * Resolution < 1`,
    `numTaps` is 0 and the output is NaN.
20. **The curve lookup table can have holes.** `CurveNode::OnCurveChanged` only writes the table
    entries the sampled curve lands on; skipped entries stay 0 and show up as spikes in steep
    sections.
21. **`ComputeNode::Compute()` clears the dirty flag even when `InternalCompute()` bailed out**
    (shader failed to compile, missing input), so the node is never retried and keeps stale
    output.

## Broken features and gaps

22. **The Input/Output Bitmap file picker only offers `.rawfp32` and `.bmp`**
    (`PropertiesWnd.cpp`), although PNG and JPEG now load (stb_image). TGA loading is a stub
    (`Bitmap::LoadTGA` returns false).
23. **Minor gaps:**
    - min altitude >= max altitude isn't rejected, making `NormalizeTerrain` divide by zero;
    - `NodeEditor::Load` reads a `Version` attribute but `Save` writes `VersionMajor` /
      `VersionMinor`;
    - Poisson scatter distribution uses uninitialized positions.

## Minor bugs

- `LOG_R( "...%s...", pNode->GetName() )` passes a `std::string` through varargs, which is
  undefined behaviour (`NodeEditor.cpp:787`, `NodeEditorView.cpp:530`).
- `ScatterMapNode.cpp:195` treats the `CreateInputLayout` HRESULT as a bool, so success leaks the
  compiled VS blob. `ScatterMapNode.cpp:464` unbinds CS SRVs instead of PS SRVs.
- `CPreviewWnd::OnLButtonUp` calls `OnLButtonDown` (`PreviewWnd.cpp:211`). There's no `SetCapture`
  either, so releasing the mouse outside the pane leaves the view rotating.
- The CPU erosion's OpenMP loop (Release only) shares one global `rand()` state across threads.
- `MonteCarloErosionNode::GetOutput() const` dispatches a compute shader on every call, including
  every preview repaint.
