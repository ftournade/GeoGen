cmake_minimum_required(VERSION 3.21)

project(GeoGen LANGUAGES CXX RC)

if(NOT MSVC)
	message(FATAL_ERROR "GeoGen is an MFC/Direct3D 11 application and requires MSVC on Windows.")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------------------------
# Third-party packages
# ---------------------------------------------------------------------------------------------

include(FetchContent)

set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(tinyxml2
	GIT_REPOSITORY	https://github.com/leethomason/tinyxml2.git
	GIT_TAG			9148bdf719e997d1f474be6bcc7943881046dba1	# 11.0.0
	GIT_SHALLOW		FALSE
)
FetchContent_MakeAvailable(tinyxml2)

# The sources include <TinyXml2/tinyxml2.h> (original repo layout); forward it to the package header
set(GEOGEN_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
file(WRITE "${GEOGEN_GENERATED_INCLUDE_DIR}/TinyXml2/tinyxml2.h"
	"#pragma once\n#include \"${tinyxml2_SOURCE_DIR}/tinyxml2.h\"\n")

# MFC as a shared DLL (Visual Studio generators read this flag; _AFXDLL is also defined below
# so that non-VS generators such as Ninja behave the same way)
set(CMAKE_MFC_FLAG 2)

# ---------------------------------------------------------------------------------------------
# Sources (grouped like the original Visual Studio filters)
# ---------------------------------------------------------------------------------------------

set(SRC_ROOT
	CPUFloatMap.cpp				CPUFloatMap.h
	HTTPConnection.cpp			HTTPConnection.h
	stdafx.cpp					stdafx.h
	targetver.h
)

set(SRC_COMPUTE_NODES
	BasicComputeNodes.cpp		BasicComputeNodes.h
	BlurNode.cpp				BlurNode.h
	ColorGradientNode.cpp		ColorGradientNode.h
	ComputeNode.cpp				ComputeNode.h
	CurveNode.cpp				CurveNode.h
	CustomComputeNode.cpp		CustomComputeNode.h
	ErosionNode.cpp				ErosionNode.h
	ExtractDetailNode.cpp		ExtractDetailNode.h
	InputBitmapNode.cpp			InputBitmapNode.h
	MinstrelNoiseNode.cpp		MinstrelNoiseNode.h
	MonteCarloErosionNode.cpp	MonteCarloErosionNode.h
	MonteCarloErosionNodeCPU.cpp	MonteCarloErosionNodeCPU.h
	MountainNode.cpp			MountainNode.h
	NodeFactory.cpp				NodeFactory.h
								NormalNode.h
	OutputBitmapNode.cpp		OutputBitmapNode.h
	PreviewNode.cpp				PreviewNode.h
								RandomPick.h
	ScatterMapNode.cpp			ScatterMapNode.h
	SnowNode.cpp				SnowNode.h
)

# Shaders are compiled at runtime by the application, never at build time
set(SRC_SHADERS
	Shaders/Blur.hlsl
	Shaders/Erosion.hlsl
	Shaders/ErosionConstants.h
	Shaders/MinstrelNoise.hlsl
	Shaders/MonteCarloErosion.hlsl
	Shaders/MonteCarloErosionConstants.h
	Shaders/Mountain.hlsl
	Shaders/Noise.h
	Shaders/Terrain.hlsl
	Shaders/Utils.h
)

set(SRC_UI
	OutputWnd.cpp				OutputWnd.h
	Resource.h
)

set(SRC_UI_MAIN
	GeoGen.cpp					GeoGen.h
	MainFrm.cpp					MainFrm.h
	SettingsDlg.cpp				SettingsDlg.h
)

set(SRC_UI_NODE_EDITOR
	NodeEditor.cpp				NodeEditor.h
	NodeEditorView.cpp			NodeEditorView.h
)

set(SRC_UI_PREVIEW
	PreviewWnd.cpp				PreviewWnd.h
	GridMesh.cpp				GridMesh.h
)

set(SRC_UI_PROPERTIES
	BoolProperty.cpp			BoolProperty.h
	ColorGradientDlg.cpp		ColorGradientDlg.h
	ColorPicker.cpp				ColorPicker.h
	ColorPickerDlg.cpp			ColorPickerDlg.h
	CurveEditorDlg.cpp			CurveEditorDlg.h
	CustomSlider.cpp			CustomSlider.h
	FloatSliderProperty.cpp		FloatSliderProperty.h
	PropertiesWnd.cpp			PropertiesWnd.h
	RiverEditorDlg.cpp			RiverEditorDlg.h
)

set(SRC_UI_DEMGRAB
	DEMGrabFrm.cpp				DEMGrabFrm.h
	DEMGrabView.cpp				DEMGrabView.h
	DownloadDEMDlg.cpp			DownloadDEMDlg.h
	TerrainTileProviderSetupDlg.cpp	TerrainTileProviderSetupDlg.h
)

# Subset of xtm/Core (github.com/ftournade/xtm @ 81e8a648ad5d).
# Local changes: Bitmap.cpp has JPG/DXT support disabled (libjpeg/squish not vendored),
# Bitmap_PNG_Unsupported.cpp replaces Bitmap_PNG.cpp (LightZPNG not vendored),
# xtm_prelude.h replaces Core/stdafx.h, DLLMain.h/CORE_API removed, and Win32 calls in
# Log.cpp/UIHelper.h/WinUIHelper.cpp use explicit ANSI (...A) variants since GeoGen builds as Unicode.
set(SRC_CORE
	Core/AABBox.cpp				Core/AABBox.h				Core/AABBox.inl
								Core/BasicTypes.h
	Core/Bitmap.cpp				Core/Bitmap.h
	Core/Bitmap_BMP.cpp
	Core/Bitmap_DDS.cpp
	Core/Bitmap_PNG_Unsupported.cpp
	Core/BSphere.cpp			Core/BSphere.h				Core/BSphere.inl
	Core/Camera.cpp				Core/Camera.h
	Core/Color.cpp				Core/Color.h				Core/Color.inl
	Core/ConvexHull.cpp			Core/ConvexHull.h
	Core/Curve.cpp				Core/Curve.h
	Core/DataFormats.cpp		Core/DataFormats.h
	Core/Debug.cpp				Core/Debug.h
	Core/FileStream.cpp			Core/FileStream.h
								Core/FileSystem.h
	Core/float16.cpp			Core/float16.h
								Core/IByteStream.h
	Core/IFindFile.cpp			Core/IFindFile.h
	Core/Keyframer.cpp			Core/Keyframer.h
	Core/Log.cpp				Core/Log.h
	Core/Mat33.cpp				Core/Mat33.h				Core/Mat33.inl
	Core/Mat44.cpp				Core/Mat44.h				Core/Mat44.inl
								Core/MathConstants.h
								Core/MemoryManager.h
	Core/OBBox.cpp				Core/OBBox.h				Core/OBBox.inl
	Core/Plane.cpp				Core/Plane.h
	Core/Ray.cpp				Core/Ray.h					Core/Ray.inl
	Core/StandardLib.cpp		Core/StandardLib.h
	Core/StandardMath.cpp		Core/StandardMath.h
	Core/Str.cpp				Core/Str.h					Core/Str.inl
								Core/Threading.h
								Core/TList.h
	Core/TVector.cpp			Core/TVector.h
								Core/UIHelper.h
	Core/Utility.cpp			Core/Utility.h
	Core/Vec2.cpp				Core/Vec2.h					Core/Vec2.inl
								Core/Vec2i.h
	Core/Vec3.cpp				Core/Vec3.h					Core/Vec3.inl
	Core/Vec4.cpp				Core/Vec4.h					Core/Vec4.inl
	Core/Win32BackBuffer.cpp	Core/Win32BackBuffer.h
	Core/WinUIHelper.cpp
								Core/xtm_prelude.h
)

set(SRC_RESOURCES
	GeoGen.rc
	res/GeoGen.rc2
	res/GeoGen.ico
	res/OnOff.bmp
	res/output_wnd.ico
	res/output_wnd_hc.ico
	res/properties.bmp
	res/properties_hc.bmp
	res/properties_wnd.ico
	res/properties_wnd_hc.ico
	res/Toolbar.bmp
	res/Toolbar256.bmp
	res/Toolbar_DEMGrabber.bmp
	res/Toolbar_DEMGrabber_256.bmp
	UserImages.bmp
)

source_group("" 					FILES ${SRC_ROOT} TODO.txt)
source_group("Compute nodes"		FILES ${SRC_COMPUTE_NODES})
source_group("Compute nodes\\Shaders"	FILES ${SRC_SHADERS})
source_group("UI"					FILES ${SRC_UI})
source_group("UI\\Main"				FILES ${SRC_UI_MAIN})
source_group("UI\\Node editor"		FILES ${SRC_UI_NODE_EDITOR})
source_group("UI\\Preview"			FILES ${SRC_UI_PREVIEW})
source_group("UI\\Properties"		FILES ${SRC_UI_PROPERTIES})
source_group("UI\\DEMGrab"			FILES ${SRC_UI_DEMGRAB})
source_group("UI\\Resource Files"	FILES ${SRC_RESOURCES})
source_group("Core"					FILES ${SRC_CORE})

set_source_files_properties(${SRC_SHADERS} TODO.txt PROPERTIES HEADER_FILE_ONLY ON)

add_executable(GeoGen WIN32
	${SRC_ROOT}
	${SRC_COMPUTE_NODES}
	${SRC_SHADERS}
	${SRC_UI}
	${SRC_UI_MAIN}
	${SRC_UI_NODE_EDITOR}
	${SRC_UI_PREVIEW}
	${SRC_UI_PROPERTIES}
	${SRC_UI_DEMGRAB}
	${SRC_CORE}
	${SRC_RESOURCES}
	TODO.txt
)

# ---------------------------------------------------------------------------------------------
# Compilation
# ---------------------------------------------------------------------------------------------

target_include_directories(GeoGen PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${GEOGEN_GENERATED_INCLUDE_DIR}
)

target_compile_definitions(GeoGen PRIVATE
	_WINDOWS
	UNICODE
	_UNICODE
	_AFXDLL
	XTM_WIN32
	XTM_D3D
	XTM_USE_DOUBLE_FLOAT_PRECISION
	_CRT_SECURE_NO_WARNINGS
	$<$<CONFIG:Debug>:_DEBUG>
)

target_precompile_headers(GeoGen PRIVATE stdafx.h)

target_compile_options(GeoGen PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:
		/W3
		/MP
		/fp:fast
		/arch:AVX
		$<$<NOT:$<CONFIG:Debug>>:/openmp /GL>
	>
)

# ---------------------------------------------------------------------------------------------
# Linking (Windows SDK system libraries only)
# ---------------------------------------------------------------------------------------------

target_link_libraries(GeoGen PRIVATE
	tinyxml2::tinyxml2
	d3d11
	dxgi
	dxguid
	d3dcompiler
	ws2_32
)

target_link_options(GeoGen PRIVATE
	/ENTRY:wWinMainCRTStartup		# Unicode MFC application
	$<$<NOT:$<CONFIG:Debug>>:/LTCG /OPT:REF /OPT:ICF>
)

set_target_properties(GeoGen PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"	# Shaders/ are loaded from the working directory
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GeoGen)

# Copy the runtime-compiled shaders next to the executable so it can run standalone
add_custom_command(TARGET GeoGen POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Shaders" "$<TARGET_FILE_DIR:GeoGen>/Shaders"
	COMMENT "Copying shaders next to GeoGen executable"
)
