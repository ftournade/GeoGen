//#include "stdafx.h"
#include "Bitmap.h"

#include <Core/FileStream.h>
#include <Core/TVector.h>

#include <d3d11.h> //TODO this is for DXGIFORMAT, but if we want to be cross-platform I'll need to duplicate DXGI_FORMAT enum

namespace xtm
{
	#ifndef MAKEFOURCC
		#define MAKEFOURCC(ch0, ch1, ch2, ch3)                 \
			((u32)(byte)(ch0) | ((u32)(byte)(ch1) << 8) |       \
			((u32)(byte)(ch2) << 16) | ((u32)(byte)(ch3) << 24))
	#endif /* defined(MAKEFOURCC) */

		//--------------------------------------------------------------------------------------
		// DDS file structure definitions
		//
		// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
		//--------------------------------------------------------------------------------------
	#pragma pack(push, 1)

	#define DDS_MAGIC 0x20534444 // "DDS "

	struct DDS_PIXELFORMAT
	{
		u32  size;
		u32  flags;
		u32  fourCC;
		u32  RGBBitCount;
		u32  RBitMask;
		u32  GBitMask;
		u32  BBitMask;
		u32  ABitMask;
	};

	#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
	#define DDS_RGB         0x00000040  // DDPF_RGB
	#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
	#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
	#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
	#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
	#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8

	#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
	#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
	#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
	#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
	#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

	#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
	#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

	#define DDS_CAPS_TEXTURE 0x00001000
	#define DDS_CAPS_MIPMAP  0x00400000
	#define DDS_CAPS_COMPLEX 0x00000008

	#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
	#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
	#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
	#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
	#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
	#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

	#define DDS_CUBEMAP_ALLFACES (DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
		DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
		DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ)

	#define DDS_CAPS2_CUBEMAP 0x00000200
	#define DDS_CAPS2_VOLUME 0x00200000

	typedef struct
	{
		u32          size;
		u32          flags;
		u32          m_Height;
		u32          m_Width;
		u32          pitchOrLinearSize;
		u32          depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
		u32          mipMapCount;
		u32          reserved1[11];
		DDS_PIXELFORMAT ddspf;
		u32          caps;
		u32          caps2;
		u32          caps3;
		u32          caps4;
		u32          reserved2;
	} DDS_HEADER;

	typedef struct
	{
		DXGI_FORMAT dxgiFormat;
		u32      resourceDimension;
		u32      miscFlag; // see D3D11_RESOURCE_MISC_FLAG
		u32      arraySize;
		u32      reserved;
	} DDS_HEADER_DXT10;

	#pragma pack(pop)

	#define ISBITMASK(r, g, b, a) (ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a)

	static DataFormat ConvertToXtmFormat( DXGI_FORMAT _format )
	{
		switch( _format )
		{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:             return DataFormat::R32G32B32A32_TYPELESS;
			case DXGI_FORMAT_R32G32B32A32_FLOAT:				return DataFormat::R32G32B32A32_FLOAT;
			case DXGI_FORMAT_R32G32B32A32_UINT:					return DataFormat::R32G32B32A32_UINT;
			case DXGI_FORMAT_R32G32B32A32_SINT:					return DataFormat::R32G32B32A32_SINT;
			case DXGI_FORMAT_R32G32B32_TYPELESS:				return DataFormat::R32G32B32_TYPELESS;
			case DXGI_FORMAT_R32G32B32_FLOAT:					return DataFormat::R32G32B32_FLOAT;
			case DXGI_FORMAT_R32G32B32_UINT:					return DataFormat::R32G32B32_UINT;
			case DXGI_FORMAT_R32G32B32_SINT:					return DataFormat::R32G32B32_SINT;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS:				return DataFormat::R16G16B16A16_TYPELESS;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:				return DataFormat::R16G16B16A16_FLOAT;
			case DXGI_FORMAT_R16G16B16A16_UNORM:				return DataFormat::R16G16B16A16_UNORM;
			case DXGI_FORMAT_R16G16B16A16_UINT:					return DataFormat::R16G16B16A16_UINT;
			case DXGI_FORMAT_R16G16B16A16_SNORM:				return DataFormat::R16G16B16A16_SNORM;
			case DXGI_FORMAT_R16G16B16A16_SINT:					return DataFormat::R16G16B16A16_SINT;
			case DXGI_FORMAT_R32G32_TYPELESS:					return DataFormat::R32G32_TYPELESS;
			case DXGI_FORMAT_R32G32_FLOAT:						return DataFormat::R32G32_FLOAT;
			case DXGI_FORMAT_R32G32_UINT:						return DataFormat::R32G32_UINT;
			case DXGI_FORMAT_R32G32_SINT:						return DataFormat::R32G32_SINT;
//			case DXGI_FORMAT_R32G8X24_TYPELESS:					return DataFormat::R32G8X24_TYPELESS;
//			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:				return DataFormat::D32_FLOAT_S8X24_UINT;
//			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:			return DataFormat::R32_FLOAT_X8X24_TYPELESS;
//			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:			return DataFormat::X32_TYPELESS_G8X24_UINT;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS:				return DataFormat::R10G10B10A2_TYPELESS;
			case DXGI_FORMAT_R10G10B10A2_UNORM:					return DataFormat::R10G10B10A2_UNORM;
			case DXGI_FORMAT_R10G10B10A2_UINT:					return DataFormat::R10G10B10A2_UINT;
			case DXGI_FORMAT_R11G11B10_FLOAT:					return DataFormat::R11G11B10_FLOAT;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:					return DataFormat::R8G8B8A8_TYPELESS;
			case DXGI_FORMAT_R8G8B8A8_UNORM:					return DataFormat::R8G8B8A8_UNORM;
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:				return DataFormat::R8G8B8A8_UNORM_SRGB;
			case DXGI_FORMAT_R8G8B8A8_UINT:						return DataFormat::R8G8B8A8_UINT;
			case DXGI_FORMAT_R8G8B8A8_SNORM:					return DataFormat::R8G8B8A8_SNORM;
			case DXGI_FORMAT_R8G8B8A8_SINT:						return DataFormat::R8G8B8A8_SINT;
			case DXGI_FORMAT_R16G16_TYPELESS:					return DataFormat::R16G16_TYPELESS;
			case DXGI_FORMAT_R16G16_FLOAT:						return DataFormat::R16G16_FLOAT;
			case DXGI_FORMAT_R16G16_UNORM:						return DataFormat::R16G16_UNORM;
			case DXGI_FORMAT_R16G16_UINT:						return DataFormat::R16G16_UINT;
			case DXGI_FORMAT_R16G16_SNORM:						return DataFormat::R16G16_SNORM;
			case DXGI_FORMAT_R16G16_SINT:						return DataFormat::R16G16_SINT;
			case DXGI_FORMAT_R32_TYPELESS:						return DataFormat::R32_TYPELESS;
			case DXGI_FORMAT_D32_FLOAT:							return DataFormat::D32_FLOAT;
			case DXGI_FORMAT_R32_FLOAT:							return DataFormat::R32_FLOAT;
			case DXGI_FORMAT_R32_UINT:							return DataFormat::R32_UINT;
			case DXGI_FORMAT_R32_SINT:							return DataFormat::R32_SINT;
			case DXGI_FORMAT_R24G8_TYPELESS:					return DataFormat::R24G8_TYPELESS;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:					return DataFormat::D24_UNORM_S8_UINT;
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:				return DataFormat::R24_UNORM_X8_TYPELESS;
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:				return DataFormat::X24_TYPELESS_G8_UINT;
			case DXGI_FORMAT_R8G8_TYPELESS:						return DataFormat::R8G8_TYPELESS;
			case DXGI_FORMAT_R8G8_UNORM:						return DataFormat::R8G8_UNORM;
			case DXGI_FORMAT_R8G8_UINT:							return DataFormat::R8G8_UINT;
			case DXGI_FORMAT_R8G8_SNORM:						return DataFormat::R8G8_SNORM;
			case DXGI_FORMAT_R8G8_SINT:							return DataFormat::R8G8_SINT;
			case DXGI_FORMAT_R16_TYPELESS:						return DataFormat::R16_TYPELESS;
			case DXGI_FORMAT_R16_FLOAT:							return DataFormat::R16_FLOAT;
			case DXGI_FORMAT_D16_UNORM:							return DataFormat::D16_UNORM;
			case DXGI_FORMAT_R16_UNORM:							return DataFormat::R16_UNORM;
			case DXGI_FORMAT_R16_UINT:							return DataFormat::R16_UINT;
			case DXGI_FORMAT_R16_SNORM:							return DataFormat::R16_SNORM;
			case DXGI_FORMAT_R16_SINT:							return DataFormat::R16_SINT;
			case DXGI_FORMAT_R8_TYPELESS:						return DataFormat::R8_TYPELESS;
			case DXGI_FORMAT_R8_UNORM:							return DataFormat::R8_UNORM;
			case DXGI_FORMAT_R8_UINT:							return DataFormat::R8_UINT;
			case DXGI_FORMAT_R8_SNORM:							return DataFormat::R8_SNORM;
			case DXGI_FORMAT_R8_SINT:							return DataFormat::R8_SINT;
			case DXGI_FORMAT_A8_UNORM:							return DataFormat::A8_UNORM;
//			case DXGI_FORMAT_R1_UNORM:							return DataFormat::R1_UNORM;
//			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:				return DataFormat::R9G9B9E5_SHAREDEXP;
//			case DXGI_FORMAT_R8G8_B8G8_UNORM:					return DataFormat::R8G8_B8G8_UNORM;
//			case DXGI_FORMAT_G8R8_G8B8_UNORM:					return DataFormat::G8R8_G8B8_UNORM;
			case DXGI_FORMAT_BC1_TYPELESS:						return DataFormat::BC1_TYPELESS;
			case DXGI_FORMAT_BC1_UNORM:							return DataFormat::BC1_UNORM;
			case DXGI_FORMAT_BC1_UNORM_SRGB:					return DataFormat::BC1_UNORM_SRGB;
			case DXGI_FORMAT_BC2_TYPELESS:						return DataFormat::BC2_TYPELESS;
			case DXGI_FORMAT_BC2_UNORM:							return DataFormat::BC2_UNORM;
			case DXGI_FORMAT_BC2_UNORM_SRGB:					return DataFormat::BC2_UNORM_SRGB;
			case DXGI_FORMAT_BC3_TYPELESS:						return DataFormat::BC3_TYPELESS;
			case DXGI_FORMAT_BC3_UNORM:							return DataFormat::BC3_UNORM;
			case DXGI_FORMAT_BC3_UNORM_SRGB:					return DataFormat::BC3_UNORM_SRGB;
			case DXGI_FORMAT_BC4_TYPELESS:						return DataFormat::BC4_TYPELESS;
			case DXGI_FORMAT_BC4_UNORM:							return DataFormat::BC4_UNORM;
			case DXGI_FORMAT_BC4_SNORM:							return DataFormat::BC4_SNORM;
			case DXGI_FORMAT_BC5_TYPELESS:						return DataFormat::BC5_TYPELESS;
			case DXGI_FORMAT_BC5_UNORM:							return DataFormat::BC5_UNORM;
			case DXGI_FORMAT_BC5_SNORM:							return DataFormat::BC5_SNORM;
			case DXGI_FORMAT_B5G6R5_UNORM:						return DataFormat::B5G6R5_UNORM;
			case DXGI_FORMAT_B5G5R5A1_UNORM:					return DataFormat::B5G5R5A1_UNORM;
			case DXGI_FORMAT_B8G8R8A8_UNORM:					return DataFormat::B8G8R8A8_UNORM;
			case DXGI_FORMAT_B8G8R8X8_UNORM:					return DataFormat::B8G8R8X8_UNORM;
//			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:		return DataFormat::R10G10B10_XR_BIAS_A2_UNORM;
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:					return DataFormat::B8G8R8A8_TYPELESS;
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:				return DataFormat::B8G8R8A8_UNORM_SRGB;
//			case DXGI_FORMAT_B8G8R8X8_TYPELESS:					return DataFormat::B8G8R8X8_TYPELESS;
//			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:				return DataFormat::B8G8R8X8_UNORM_SRGB;
			case DXGI_FORMAT_BC6H_TYPELESS:						return DataFormat::BC6H_TYPELESS;
			case DXGI_FORMAT_BC6H_UF16:							return DataFormat::BC6H_UF16;
			case DXGI_FORMAT_BC6H_SF16:							return DataFormat::BC6H_SF16;
			case DXGI_FORMAT_BC7_TYPELESS:						return DataFormat::BC7_TYPELESS;
			case DXGI_FORMAT_BC7_UNORM:							return DataFormat::BC7_UNORM;
			case DXGI_FORMAT_BC7_UNORM_SRGB:					return DataFormat::BC7_UNORM_SRGB;
//			case DXGI_FORMAT_AYUV:								return DataFormat::AYUV;
//			case DXGI_FORMAT_Y410:								return DataFormat::Y410;
//			case DXGI_FORMAT_Y416:								return DataFormat::Y416;
//			case DXGI_FORMAT_NV12:								return DataFormat::NV12;
//			case DXGI_FORMAT_P010:								return DataFormat::P010;
//			case DXGI_FORMAT_P016:								return DataFormat::P016;
//			case DXGI_FORMAT_420_OPAQUE:						return DataFormat::420_OPAQUE;
//			case DXGI_FORMAT_YUY2:								return DataFormat::YUY2;
//			case DXGI_FORMAT_Y210:								return DataFormat::Y210;
//			case DXGI_FORMAT_Y216:								return DataFormat::Y216;
//			case DXGI_FORMAT_NV11:								return DataFormat::NV11;
//			case DXGI_FORMAT_AI44:								return DataFormat::AI44;
//			case DXGI_FORMAT_IA44:								return DataFormat::IA44;
//			case DXGI_FORMAT_P8:								return DataFormat::P8;
//			case DXGI_FORMAT_A8P8:								return DataFormat::A8P8;
			case DXGI_FORMAT_B4G4R4A4_UNORM:					return DataFormat::B4G4R4A4_UNORM;	
		}

		return DataFormat::Unknown;
	}

	static DataFormat GetXTMFormat(const DDS_PIXELFORMAT& ddpf)
	{
		if (ddpf.flags & DDS_RGB)
		{
			// Note that sRGB formats are written using the "DX10" extended header

			switch (ddpf.RGBBitCount)
			{
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DataFormat::R8G8B8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				{
					return DataFormat::B8G8R8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
				{
					return DataFormat::B8G8R8X8_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000) aka D3DFMT_X8B8G8R8

				// Note that many common DDS reader/writers (including D3DX) swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assumme
				// below that the 'backwards' header mask is being used since it is most
				// likely written by D3DX. The more robust solution is to use the 'DX10'
				// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

				// For 'correct' writers, this should be 0x000003ff, 0x000ffc00, 0x3ff00000 for RGB data
				if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
				{
					return DataFormat::R10G10B10A2_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000) aka D3DFMT_A2R10G10B10

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
				{
					return DataFormat::R16G16_UNORM;
				}

				if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
				{
					// Only 32-bit color channel format in D3D9 was R32F
					return DataFormat::R32_FLOAT; // D3DX writes this out as a FourCC of 114
				}
				break;

			case 24:
				// No 24bpp DXGI formats aka D3DFMT_R8G8B8
				break;

			case 16:
				if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
				{
					return DataFormat::B5G5R5A1_UNORM;
				}
				if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
				{
					return DataFormat::B5G6R5_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x0000) aka D3DFMT_X1R5G5B5
				if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
				{
					return DataFormat::B4G4R4A4_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x0f00, 0x00f0, 0x000f, 0x0000) aka D3DFMT_X4R4G4B4

				// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
				break;
			}
		}
		else if (ddpf.flags & DDS_LUMINANCE)
		{
			if (8 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000) ||
					ISBITMASK(0x000000ff, 0x000000ff, 0x000000ff, 0x00000000) )
				{
					return DataFormat::R8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}

				// No DXGI format maps to ISBITMASK(0x0f, 0x00, 0x00, 0xf0) aka D3DFMT_A4L4
			}

			if (16 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
				{
					return DataFormat::R16_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
				if( ISBITMASK( 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00 ) ||
					ISBITMASK( 0x000000ff, 0x000000ff, 0x000000ff, 0x0000ff00 ) )
				{
					return DataFormat::R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
			}
		}
		else if (ddpf.flags & DDS_ALPHA)
		{
			if (8 == ddpf.RGBBitCount)
			{
				return DataFormat::A8_UNORM;
			}
		}
		else if (ddpf.flags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
			{
				return DataFormat::BC1_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
			{
				return DataFormat::BC2_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
			{
				return DataFormat::BC3_UNORM;
			}
			
			// While pre-mulitplied alpha isn't directly supported by the DXGI formats,
			// they are basically the same as these BC formats so they can be mapped
			if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
			{
				return DataFormat::BC2_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
			{
				return DataFormat::BC3_UNORM;
			}

			if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
			{
				return DataFormat::BC4_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
			{
				return DataFormat::BC4_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
			{
				return DataFormat::BC4_SNORM;
			}

			if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
			{
				return DataFormat::BC5_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
			{
				return DataFormat::BC5_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
			{
				return DataFormat::BC5_SNORM;
			}

			// BC6H and BC7 are written using the "DX10" extended header
/*
			if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
			{
				return DataFormat::R8G8_B8G8_UNORM;
			}
			if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
			{
				return DataFormat::G8R8_G8B8_UNORM;
			}
*/			
			// Check for D3DFORMAT enums being set here
			switch (ddpf.fourCC)
			{
			case 36: // D3DFMT_A16B16G16R16
				return DataFormat::R16G16B16A16_UNORM;

			case 110: // D3DFMT_Q16W16V16U16
				return DataFormat::R16G16B16A16_SNORM;

			case 111: // D3DFMT_R16F
				return DataFormat::R16_FLOAT;

			case 112: // D3DFMT_G16R16F
				return DataFormat::R16G16_FLOAT;

			case 113: // D3DFMT_A16B16G16R16F
				return DataFormat::R16G16B16A16_FLOAT;

			case 114: // D3DFMT_R32F
				return DataFormat::R32_FLOAT;

			case 115: // D3DFMT_G32R32F
				return DataFormat::R32G32_FLOAT;

			case 116: // D3DFMT_A32B32G32R32F
				return DataFormat::R32G32B32A32_FLOAT;
			}
		}

		return DataFormat::Unknown;
	}


	bool Bitmap::LoadDDS( const char* _filename )
	{
		SAFE_DELETE( m_pData );
		
		FileStream file;
        if( !file.Open( _filename, "rb" ) )
			return false;
		
		IByteStream::OffsetType fileSize = file.GetSize();

		TVector< byte > fileData( (u32)fileSize );

		if( file.ReadBytes( &fileData[0], fileSize ) != fileSize )
			return false;

		const byte* pFileData = &fileData[0];
		u32 magicNumber = *(const u32*)(pFileData);

		if( magicNumber != DDS_MAGIC )
			return false;

		const DDS_HEADER* header = reinterpret_cast<const DDS_HEADER*>(pFileData + sizeof(u32));

		// Verify header to validate DDS file
		if (header->size != sizeof(DDS_HEADER) || header->ddspf.size != sizeof(DDS_PIXELFORMAT))
		{
			return false;
		}

		// Check for DX10 extension
		bool bDXT10Header = false;

		if( ( header->ddspf.flags & DDS_FOURCC ) && ( MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC ) )
		{
			// Must be long enough for both headers and magic value
			//if( ddsDataSize < (sizeof(DDS_HEADER) + sizeof(u32) + sizeof(DDS_HEADER_DXT10) ) )
			//{
			//	return false;
			//}

			bDXT10Header = true;
		}

		ptrdiff_t offset = sizeof(u32) + sizeof(DDS_HEADER) + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);

		m_Width = header->m_Width;
		m_Height = header->m_Height;
		m_Depth = Max<u32>( header->depth, 1 );

		u32 resDim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
		m_ArraySize = 1;
		//DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		
		m_bIsCubeMap = false;

		m_MipMapLevels = Max<u32>( header->mipMapCount, 1 );

		if( bDXT10Header )
		{
			const DDS_HEADER_DXT10* d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));
					
			m_ArraySize = d3d10ext->arraySize;
			if (m_ArraySize == 0)
			{
				return false;
			}

			m_Format = ConvertToXtmFormat( d3d10ext->dxgiFormat );

			if( m_Format.m_Fields.m_BitsPerPixel == 0 )
			{
				return false;
			}

			switch (d3d10ext->resourceDimension)
			{
			case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
				// D3DX writes 1D textures with a fixed Height of 1
				if ((header->flags & DDS_HEIGHT) && m_Height != 1)
				{
					return false;
				}
				m_Height = m_Depth = 1;
				break;

			case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
				if (d3d10ext->miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE)
				{
					//m_ArraySize *= 6;
					m_bIsCubeMap = true;
				}
				m_Depth = 1;
				break;

			case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
				if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
				{
					return false;
				}

				if (m_ArraySize > 1)
				{
					return false;
				}
				break;

			default:
				return false;
			}

			resDim = d3d10ext->resourceDimension;
		
		}
		else
		{
			m_Format = GetXTMFormat( header->ddspf );

			if( m_Format == DataFormat::Unknown )
			{
				return false;
			}

			if (header->flags & DDS_HEADER_FLAGS_VOLUME)
			{
				resDim = D3D11_RESOURCE_DIMENSION_TEXTURE3D;
			}
			else
			{
				if (header->caps2 & DDS_CAPS2_CUBEMAP)
				{
					// We require all six faces to be defined
					if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
					{
						return false;
					}

					m_bIsCubeMap = true;
				}

				m_Depth = 1;
				resDim = D3D11_RESOURCE_DIMENSION_TEXTURE2D;

				// Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
			}

			DBG_CHECK( m_Format.m_Fields.m_BitsPerPixel != 0 );
		}

		// Bound sizes (for security purposes we don't trust DDS file metadata larger than the D3D 11.x hardware requirements)
		if( m_MipMapLevels > D3D11_REQ_MIP_LEVELS )
		{
			return false;
		}

		switch (resDim)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			if ((m_ArraySize > D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
				(m_Width > D3D11_REQ_TEXTURE1D_U_DIMENSION))
			{
				return false;
			}
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			if( m_bIsCubeMap )
			{
				// This is the right bound because we set arraySize to (NumCubes*6) above
				if ((m_ArraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
					(m_Width > D3D11_REQ_TEXTURECUBE_DIMENSION) ||
					(m_Height > D3D11_REQ_TEXTURECUBE_DIMENSION))
				{
					return false;
				}
			}
			else if ((m_ArraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
				(m_Width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
				(m_Height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION))
			{
				return false;
			}
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			if ((m_ArraySize > 1) ||
				(m_Width > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
				(m_Height > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
				(m_Depth > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
			{
				return false;
			}
			break;
		}

		m_DataSize = 0;
		u32 w = m_Width;
		u32 h = m_Height;
		u32 d = m_Depth;

		for( u32 i=0 ; i < m_MipMapLevels ; ++i )
		{
			m_DataSize += GetMipMapSize( m_Format, w, h, d );

			w >>= 1;
			h >>= 1;
			d >>= 1;

			w = Max<u32>( w, 1 );
			h = Max<u32>( h, 1 );
			d = Max<u32>( d, 1 );
		}

		m_DataSize *= m_ArraySize;

		if( m_bIsCubeMap )
			m_DataSize *= 6;

		m_pData = (byte*)malloc( m_DataSize );

		DBG_CHECK( offset + m_DataSize <= fileData.size() );

		memcpy( m_pData, pFileData + offset, m_DataSize );

/*
		// Create the texture
		std::unique_ptr<D3D11_SUBRESOURCE_DATA> initData(new D3D11_SUBRESOURCE_DATA[mipCount * arraySize]);

		size_t skipMip = 0;
		size_t twidth = 0;
		size_t theight = 0;
		size_t tdepth = 0;
		FillInitData(m_Width, m_Height, depth, mipCount, arraySize, format, maxsize, bitSize, bitData, twidth, theight, tdepth, skipMip, initData.get());

		hr = CreateD3DResources(d3dDevice, resDim, twidth, theight, tdepth, mipCount - skipMip, arraySize, format, isCubeMap, initData.get(), texture, textureView);
*/
/*
		if (FAILED(hr) && !maxsize && (mipCount > 1))
		{
			// Retry with a maxsize determined by feature level
			switch (d3dDevice->GetFeatureLevel())
			{
			case D3D_FEATURE_LEVEL_9_1:
			case D3D_FEATURE_LEVEL_9_2:
				if (isCubeMap)
				{
					maxsize = D3D_FL9_1_REQ_TEXTURECUBE_DIMENSION;
				}
				else
				{
					maxsize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
						? D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION
						: D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;
				}
				break;

			case D3D_FEATURE_LEVEL_9_3:
				maxsize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
					? D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION
					: D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;
				break;

			default: // D3D_FEATURE_LEVEL_10_0 & D3D_FEATURE_LEVEL_10_1
				maxsize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
					? D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION
					: D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
				break;
			}

			FillInitData(m_Width, m_Height, depth, mipCount, arraySize, format, maxsize, bitSize, bitData, twidth, theight, tdepth, skipMip, initData.get());

			hr = CreateD3DResources(d3dDevice, resDim, twidth, theight, tdepth, mipCount - skipMip, arraySize, format, isCubeMap, initData.get(), texture, textureView);
		}
*/

		return true;
	}

	bool Bitmap::SaveDDS( const char* _filename ) const
	{
		if( m_bIsCubeMap || (m_ArraySize != 1) || (m_MipMapLevels != 1) )
		{
			return false; // only simple format supported for now
		}

		FileStream file;
        if( !file.Open( _filename, "wb" ) )
			return false;

		Write( file, DDS_MAGIC );

		DDS_HEADER header;
		header.size = sizeof(DDS_HEADER);
		header.flags = DDS_HEADER_FLAGS_TEXTURE;
		
		if( m_MipMapLevels > 1 )
			header.flags |= DDS_HEADER_FLAGS_MIPMAP;

		if( m_Depth > 1 )
			header.flags |= DDS_HEADER_FLAGS_VOLUME;

		header.flags |= m_Format.m_Fields.m_Compressed ? DDS_HEADER_FLAGS_LINEARSIZE : DDS_HEADER_FLAGS_PITCH;

		header.m_Height = m_Width;
		header.m_Width = m_Height;
		header.pitchOrLinearSize = GetMipMapRowSize( m_Format, m_Width );
		header.depth = m_Depth;
		header.mipMapCount = m_MipMapLevels;
		//header.reserved1[11];
		header.ddspf.size = sizeof(DDS_PIXELFORMAT);
		header.ddspf.flags = DDS_FOURCC;
		header.ddspf.fourCC = MAKEFOURCC('D', 'X', '1', '0');
		header.ddspf.RGBBitCount = m_Format.m_Fields.m_BitsPerPixel;
		header.ddspf.RBitMask = 0;
		header.ddspf.GBitMask = 0;
		header.ddspf.BBitMask = 0;
		header.ddspf.ABitMask = 0;
		header.caps = DDS_CAPS_TEXTURE;
		
		if( (m_MipMapLevels > 1) || (m_ArraySize > 1) || m_bIsCubeMap )
			header.caps |= DDS_CAPS_COMPLEX;
		
		if( m_MipMapLevels > 1 )
			header.caps |= DDS_CAPS_MIPMAP;

		header.caps2 = 0;

		if( m_bIsCubeMap )
			header.caps2 |= DDS_CAPS2_CUBEMAP | DDS_CUBEMAP_ALLFACES;

		if( m_Depth > 1 )
			header.caps2 |= DDS_CAPS2_VOLUME;

		header.caps3 = 0; //unused
		header.caps4 = 0; //unused
		header.reserved2 = 0; //unused

		Write( file, header );

		DDS_HEADER_DXT10 dx10Header;
		dx10Header.dxgiFormat = ConvertToDXGIFormat( m_Format );
		dx10Header.resourceDimension = (m_Depth > 1) ? D3D11_RESOURCE_DIMENSION_TEXTURE3D : D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		dx10Header.miscFlag = m_bIsCubeMap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
		dx10Header.arraySize = m_ArraySize;
		dx10Header.reserved = 0;

		Write( file, dx10Header );

		const byte* pSurfaceData;
		u32 surfaceDataSize;
		GetMipMapData( 0, 0, 0, &pSurfaceData, &surfaceDataSize );

		//TODO beware of different pitch alignment between D3D11 and DDS
		file.WriteBytes( pSurfaceData, surfaceDataSize );

		return true;
	}


}
