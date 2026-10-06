#include "DataFormats.h"

#include <Core/Debug.h>

namespace xtm
{
	u32 DataFormat::GetBitsPerPixel( Layout _layout )
	{
		static const u32 bpp[] =
		{
			0, //Unknown,
						
			8, //Layout_8,
			16, //Layout_16,
			32, //Layout_32,
			
			16, //Layout_8_8,
			32, //Layout_16_16,
			64, //Layout_32_32,
			
			24, //Layout_8_8_8,
			48, //Layout_16_16_16,
			96, //Layout_32_32_32,
			
			32, //Layout_8_8_8_8,
			64, //Layout_16_16_16_16,
			128, //Layout_32_32_32_32,
			
			32, //Layout_24_8,
			16, //Layout_5_6_5,
			16, //Layout_4_4_4_4,
			16, //Layout_5_5_5_1,
			32, //Layout_10_10_10_2,
			32, //Layout_11_11_10,

			//BCn compressed formats
			4, //BC1,
			8, //BC2,
			8, //BC3,
			4, //BC4,
			8, //BC5,
			8, //BC6H,
			8, //BC7,
			
			//PowerVR formats
			0, //PVRTC2,
			0  //PVRTC4		
		};

		return bpp[ (u32)_layout ];
	}

	#define DEFINE_DATA_FORMAT( _name, _layout, _swizzle, _type, _flags ) \
		const DataFormat DataFormat::_name( DataFormat::Layout::_layout, DataFormat::Swizzle::_swizzle, DataFormat::Type::_type, _flags );

	DEFINE_DATA_FORMAT( Unknown,					Unknown,			RGBA,	Typeless, 0 )

	DEFINE_DATA_FORMAT( R32G32B32A32_TYPELESS,		Layout_32_32_32_32, RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R32G32B32A32_FLOAT,			Layout_32_32_32_32, RGBA,	Float, IsSigned )
	DEFINE_DATA_FORMAT( R32G32B32A32_UINT,			Layout_32_32_32_32, RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R32G32B32A32_SINT,			Layout_32_32_32_32, RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( R32G32B32_TYPELESS,			Layout_32_32_32,	RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R32G32B32_FLOAT,			Layout_32_32_32,	RGBA,	Float, IsSigned )
	DEFINE_DATA_FORMAT( R32G32B32_UINT,				Layout_32_32_32,	RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R32G32B32_SINT,				Layout_32_32_32,	RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( R16G16B16A16_TYPELESS,		Layout_16_16_16_16, RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R16G16B16A16_FLOAT,			Layout_16_16_16_16, RGBA,	Float, IsSigned )
	DEFINE_DATA_FORMAT( R16G16B16A16_UNORM,			Layout_16_16_16_16, RGBA,	Norm, 0 )
	DEFINE_DATA_FORMAT( R16G16B16A16_UINT,			Layout_16_16_16_16, RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R16G16B16A16_SNORM,			Layout_16_16_16_16, RGBA,	Norm, IsSigned )
	DEFINE_DATA_FORMAT( R16G16B16A16_SINT,			Layout_16_16_16_16, RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( R32G32_TYPELESS,			Layout_32_32,		RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R32G32_FLOAT,				Layout_32_32,		RGBA,	Float, IsSigned )
	DEFINE_DATA_FORMAT( R32G32_UINT,				Layout_32_32,		RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R32G32_SINT,				Layout_32_32,		RGBA,	Int, IsSigned )
//	DEFINE_DATA_FORMAT( R32G8X24_TYPELESS,			Layout_,			RGBA,	Typeless, 0 )
//	DEFINE_DATA_FORMAT( D32_FLOAT_S8X24_UINT,		Layout_,			DS,		, 0 )
//	DEFINE_DATA_FORMAT( R32_FLOAT_X8X24_TYPELESS,	Layout_,			DS,		, 0 )
//	DEFINE_DATA_FORMAT( X32_TYPELESS_G8X24_UINT,	Layout_,			DS,		, 0 )
	DEFINE_DATA_FORMAT( R10G10B10A2_TYPELESS,		Layout_10_10_10_2,	RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R10G10B10A2_UNORM,			Layout_10_10_10_2,	RGBA,	Norm, 0 )
	DEFINE_DATA_FORMAT( R10G10B10A2_UINT,			Layout_10_10_10_2,	RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R11G11B10_FLOAT,			Layout_11_11_10,	RGBA,	Float, IsSigned )
	DEFINE_DATA_FORMAT( R8G8B8A8_TYPELESS,			Layout_8_8_8_8,		RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R8G8B8A8_UNORM,				Layout_8_8_8_8,		RGBA,	Norm, 0 )
	DEFINE_DATA_FORMAT( R8G8B8A8_UNORM_SRGB,		Layout_8_8_8_8,		RGBA,	Norm, IsSRGB )
	DEFINE_DATA_FORMAT( R8G8B8A8_UINT,				Layout_8_8_8_8,		RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R8G8B8A8_SNORM,				Layout_8_8_8_8,		RGBA,	Norm, IsSigned )
	DEFINE_DATA_FORMAT( R8G8B8A8_SINT,				Layout_8_8_8_8,		RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( R8G8B8_UNORM,				Layout_8_8_8,		RGBA,	Norm, 0 )
	DEFINE_DATA_FORMAT( R8G8B8_UNORM_SRGB,			Layout_8_8_8,		RGBA,	Norm, IsSRGB )
	DEFINE_DATA_FORMAT( R16G16_TYPELESS,			Layout_16_16,		RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R16G16_FLOAT,				Layout_16_16,		RGBA,	Float, IsSigned )
	DEFINE_DATA_FORMAT( R16G16_UNORM,				Layout_16_16,		RGBA,	Norm, 0 )
	DEFINE_DATA_FORMAT( R16G16_UINT,				Layout_16_16,		RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R16G16_SNORM,				Layout_16_16,		RGBA,	Norm, IsSigned )
	DEFINE_DATA_FORMAT( R16G16_SINT,				Layout_16_16,		RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( R32_TYPELESS,				Layout_32,			RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( D32_FLOAT,					Layout_32,			DS,		Float, IsSigned )
	DEFINE_DATA_FORMAT( R32_FLOAT,					Layout_32,			RGBA,	Float, IsSigned )
	DEFINE_DATA_FORMAT( R32_UINT,					Layout_32,			RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R32_SINT,					Layout_32,			RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( R24G8_TYPELESS,				Layout_24_8,		DS,		Typeless, 0 )
	DEFINE_DATA_FORMAT( D24_UNORM_S8_UINT,			Layout_24_8,		DS,		Norm_Int, 0 )
	DEFINE_DATA_FORMAT( R24_UNORM_X8_TYPELESS,		Layout_24_8,		DS,		Norm_Typeless, 0 )
	DEFINE_DATA_FORMAT( X24_TYPELESS_G8_UINT,		Layout_24_8,		DS,		Typeless_Int, 0 )
	DEFINE_DATA_FORMAT( R8G8_TYPELESS,				Layout_8_8,			RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R8G8_UNORM,					Layout_8_8,			RGBA,	Norm, 0 )
	DEFINE_DATA_FORMAT( R8G8_UINT,					Layout_8_8,			RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R8G8_SNORM,					Layout_8_8,			RGBA,	Norm, IsSigned )
	DEFINE_DATA_FORMAT( R8G8_SINT,					Layout_8_8,			RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( R16_TYPELESS,				Layout_16,			RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R16_FLOAT,					Layout_16,			RGBA,	Float, IsSigned )
	DEFINE_DATA_FORMAT( D16_UNORM,					Layout_16,			DS,		Norm, 0 )
	DEFINE_DATA_FORMAT( R16_UNORM,					Layout_16,			RGBA,	Norm, 0 )
	DEFINE_DATA_FORMAT( R16_UINT,					Layout_16,			RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R16_SNORM,					Layout_16,			RGBA,	Norm, IsSigned )
	DEFINE_DATA_FORMAT( R16_SINT,					Layout_16,			RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( R8_TYPELESS,				Layout_8,			RGBA,	Typeless, 0 )
	DEFINE_DATA_FORMAT( R8_UNORM,					Layout_8,			RGBA,	Norm, 0 )
	DEFINE_DATA_FORMAT( R8_UINT,					Layout_8,			RGBA,	Int, 0 )
	DEFINE_DATA_FORMAT( R8_SNORM,					Layout_8,			RGBA,	Norm, IsSigned )
	DEFINE_DATA_FORMAT( R8_SINT,					Layout_8,			RGBA,	Int, IsSigned )
	DEFINE_DATA_FORMAT( A8_UNORM,					Layout_8,			Alpha,	Norm, 0 )
//	DEFINE_DATA_FORMAT( R1_UNORM,					Layout_,			RGBG,	Norm, 0 )
//	DEFINE_DATA_FORMAT( R9G9B9E5_SHAREDEXP,			Layout_,			RGBE,	0 )
//	DEFINE_DATA_FORMAT( R8G8_B8G8_UNORM,			Layout_,			RGBG,	0 )
//	DEFINE_DATA_FORMAT( G8R8_G8B8_UNORM,			Layout_,			GRGB,	0 )
	DEFINE_DATA_FORMAT( BC1_TYPELESS,				BC1,				RGBA,	Typeless, IsCompressed )
	DEFINE_DATA_FORMAT( BC1_UNORM,					BC1,				RGBA,	Norm, IsCompressed )
	DEFINE_DATA_FORMAT( BC1_UNORM_SRGB,				BC1,				RGBA,	Norm, IsCompressed | IsSRGB )
	DEFINE_DATA_FORMAT( BC2_TYPELESS,				BC2,				RGBA,	Typeless, IsCompressed )
	DEFINE_DATA_FORMAT( BC2_UNORM,					BC2,				RGBA,	Norm, IsCompressed )
	DEFINE_DATA_FORMAT( BC2_UNORM_SRGB,				BC2,				RGBA,	Norm, IsCompressed | IsSRGB )
	DEFINE_DATA_FORMAT( BC3_TYPELESS,				BC3,				RGBA,	Typeless, IsCompressed )
	DEFINE_DATA_FORMAT( BC3_UNORM,					BC3,				RGBA,	Norm, IsCompressed )
	DEFINE_DATA_FORMAT( BC3_UNORM_SRGB,				BC3,				RGBA,	Norm, IsCompressed | IsSRGB )
	DEFINE_DATA_FORMAT( BC4_TYPELESS,				BC4,				RGBA,	Typeless, IsCompressed )
	DEFINE_DATA_FORMAT( BC4_UNORM,					BC4,				RGBA,	Norm, IsCompressed )
	DEFINE_DATA_FORMAT( BC4_SNORM,					BC4,				RGBA,	Norm, IsCompressed | IsSigned )
	DEFINE_DATA_FORMAT( BC5_TYPELESS,				BC5,				RGBA,	Typeless, IsCompressed )
	DEFINE_DATA_FORMAT( BC5_UNORM,					BC5,				RGBA,	Norm, IsCompressed )
	DEFINE_DATA_FORMAT( BC5_SNORM,					BC5,				RGBA,	Norm, IsCompressed | IsSigned )
	DEFINE_DATA_FORMAT( B5G6R5_UNORM,				Layout_5_6_5,		BGRA,	Norm, 0 )
	DEFINE_DATA_FORMAT( B5G5R5A1_UNORM,				Layout_5_5_5_1,		BGRA,	Norm, 0 )
	DEFINE_DATA_FORMAT( B8G8R8A8_UNORM,				Layout_8_8_8_8,		BGRA,	Norm, 0 )
	DEFINE_DATA_FORMAT( B8G8R8X8_UNORM,				Layout_8_8_8_8,		BGRA,	Norm, 0 )
//	DEFINE_DATA_FORMAT( R10G10B10_XR_BIAS_A2_UNORM,	Layout_10_10_10_2,	RGBA,	0 )
	DEFINE_DATA_FORMAT( B8G8R8A8_TYPELESS,			Layout_8_8_8_8,		BGRA,	Typeless, IsCompressed )
	DEFINE_DATA_FORMAT( B8G8R8A8_UNORM_SRGB,		Layout_8_8_8_8,		BGRA,	Norm, IsSRGB )
//	DEFINE_DATA_FORMAT( B8G8R8X8_TYPELESS,			Layout_8_8_8_8,		BGRA,	Typeless, IsCompressed )
//	DEFINE_DATA_FORMAT( B8G8R8X8_UNORM_SRGB,		Layout_8_8_8_8,		BGRA,	Norm, IsSRGB )
	DEFINE_DATA_FORMAT( BC6H_TYPELESS,				BC6H,				RGBA,	Typeless, IsCompressed )
	DEFINE_DATA_FORMAT( BC6H_UF16,					BC6H,				RGBA,	Float, IsCompressed )
	DEFINE_DATA_FORMAT( BC6H_SF16,					BC6H,				RGBA,	Float, IsCompressed | IsSigned )
	DEFINE_DATA_FORMAT( BC7_TYPELESS,				BC7,				RGBA,	Typeless, IsCompressed )
	DEFINE_DATA_FORMAT( BC7_UNORM,					BC7,				RGBA,	Norm, IsCompressed )
	DEFINE_DATA_FORMAT( BC7_UNORM_SRGB,				BC7,				RGBA,	Norm, IsCompressed | IsSRGB )
//	DEFINE_DATA_FORMAT( AYUV,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( Y410,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( Y416,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( NV12,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( P010,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( P016,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( 420_OPAQUE,					Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( YUY2,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( Y210,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( Y216,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( NV11,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( AI44,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( IA44,						Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( P8,							Layout_,			RGBA,	0 )
//	DEFINE_DATA_FORMAT( A8P8,						Layout_,			RGBA,	0 )
	DEFINE_DATA_FORMAT( B4G4R4A4_UNORM,				Layout_4_4_4_4,		BGRA,	Norm, 0 )

	DEFINE_DATA_FORMAT( PVRTC2,						PVRTC2,		RGBA,	Norm, IsCompressed )
	DEFINE_DATA_FORMAT( PVRTC4,						PVRTC4,		RGBA,	Norm, IsCompressed )

#ifdef XTM_WIN32
	DXGI_FORMAT ConvertToDXGIFormat( DataFormat _format )
	{
		switch( _format.m_Fields.m_Layout )
		{
			case DataFormat::Layout::Layout_8:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R8_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R8_SINT  : DXGI_FORMAT_R8_UINT;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_Signed ? DXGI_FORMAT_R8_SNORM : DXGI_FORMAT_R8_UNORM;
				}
				break;

			case DataFormat::Layout::Layout_16:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R16_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R16_SINT  : DXGI_FORMAT_R16_UINT;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_Signed ? DXGI_FORMAT_R16_SNORM : _format.m_Fields.m_Swizzle == DataFormat::Swizzle::DS ? DXGI_FORMAT_D16_UNORM :DXGI_FORMAT_R16_UNORM;
					case DataFormat::Type::Float:		return DXGI_FORMAT_R16_FLOAT;
				}
				break;

			case DataFormat::Layout::Layout_32:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R8_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R32_SINT  : DXGI_FORMAT_R32_UINT;
					case DataFormat::Type::Float:		return _format.m_Fields.m_Swizzle == DataFormat::Swizzle::DS ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_R32_FLOAT;
				}
				break;

			case DataFormat::Layout::Layout_8_8:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R8G8_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R8G8_SINT  : DXGI_FORMAT_R8G8_UINT;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_Signed ? DXGI_FORMAT_R8G8_SNORM : DXGI_FORMAT_R8G8_UNORM;
				}
				break;

			case DataFormat::Layout::Layout_16_16:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R16G16_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R16G16_SINT  : DXGI_FORMAT_R16G16_UINT;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_Signed ? DXGI_FORMAT_R16G16_SNORM : DXGI_FORMAT_R16G16_UNORM;
					case DataFormat::Type::Float:		return DXGI_FORMAT_R16G16_FLOAT;
				}
				break;

			case DataFormat::Layout::Layout_32_32:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R32G32_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R32G32_SINT  : DXGI_FORMAT_R32G32_UINT;
					case DataFormat::Type::Float:		return DXGI_FORMAT_R32G32_FLOAT;
				}
				break;

			case DataFormat::Layout::Layout_32_32_32:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R32G32B32_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R32G32B32_SINT  : DXGI_FORMAT_R32G32B32_UINT;
					case DataFormat::Type::Float:		return DXGI_FORMAT_R32G32B32_FLOAT;
				}
				break;

			case DataFormat::Layout::Layout_8_8_8_8:
				switch( _format.m_Fields.m_Swizzle )
				{
					case DataFormat::Swizzle::RGBA:

						switch( _format.m_Fields.m_Type )
						{
							case DataFormat::Type::Typeless:	return DXGI_FORMAT_R8G8B8A8_TYPELESS;
							case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R8G8B8A8_SINT : DXGI_FORMAT_R8G8B8A8_UINT;
							case DataFormat::Type::Norm:
								if( _format.m_Fields.m_SRGB )
									return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
								else
									return _format.m_Fields.m_Signed ? DXGI_FORMAT_R8G8B8A8_SNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
						}

						break;

					case DataFormat::Swizzle::BGRA:
						DBG_CHECK( _format.m_Fields.m_Type != DataFormat::Type::Int );//unsupported by directX

						switch( _format.m_Fields.m_Type )
						{
							case DataFormat::Type::Typeless:	return DXGI_FORMAT_B8G8R8A8_TYPELESS;
							case DataFormat::Type::Norm:
								if( _format.m_Fields.m_SRGB )
									return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
								else
								{
									DBG_CHECK( !_format.m_Fields.m_Signed ); //unsupported by directX
									return DXGI_FORMAT_B8G8R8A8_UNORM;
								}
						}
						break;

					default:
						DBG_CHECK( false );//unsupported by directX
				}
				break;

			case DataFormat::Layout::Layout_16_16_16_16:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R16G16B16A16_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R16G16B16A16_SINT  : DXGI_FORMAT_R16G16B16A16_UINT;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_Signed ? DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT_R16G16B16A16_UNORM;
					case DataFormat::Type::Float:		return DXGI_FORMAT_R16G16B16A16_FLOAT;
				}
				break;

			case DataFormat::Layout::Layout_32_32_32_32:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_R32G32B32A32_TYPELESS;
					case DataFormat::Type::Int:			return _format.m_Fields.m_Signed ? DXGI_FORMAT_R32G32B32A32_SINT  : DXGI_FORMAT_R32G32B32A32_UINT;
					case DataFormat::Type::Float:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
				}
				break;

			case DataFormat::Layout::Layout_24_8:
			case DataFormat::Layout::Layout_5_6_5:
			case DataFormat::Layout::Layout_4_4_4_4:
			case DataFormat::Layout::Layout_5_5_5_1:
			case DataFormat::Layout::Layout_10_10_10_2:
			case DataFormat::Layout::Layout_11_11_10:
				NOT_YET_IMPLEMENTED //Too lazy to code these now :)
				break;

			case DataFormat::Layout::BC1:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_BC1_TYPELESS;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_SRGB ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
				}
				break;

			case DataFormat::Layout::BC2:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_BC2_TYPELESS;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_SRGB ? DXGI_FORMAT_BC2_UNORM_SRGB : DXGI_FORMAT_BC2_UNORM;
				}
				break;

			case DataFormat::Layout::BC3:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_BC3_TYPELESS;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_SRGB ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;
				}
				break;

			case DataFormat::Layout::BC4:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_BC4_TYPELESS;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_Signed ? DXGI_FORMAT_BC4_SNORM : DXGI_FORMAT_BC4_UNORM;
				}
				break;

			case DataFormat::Layout::BC5:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_BC5_TYPELESS;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_Signed ? DXGI_FORMAT_BC5_SNORM : DXGI_FORMAT_BC5_UNORM;
				}
				break;

			case DataFormat::Layout::BC6H:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_BC6H_TYPELESS;
					case DataFormat::Type::Float:		return _format.m_Fields.m_Signed ? DXGI_FORMAT_BC6H_SF16 : DXGI_FORMAT_BC6H_UF16;
				}
				break;

			case DataFormat::Layout::BC7:
				switch( _format.m_Fields.m_Type )
				{
					case DataFormat::Type::Typeless:	return DXGI_FORMAT_BC7_TYPELESS;
					case DataFormat::Type::Norm:		return _format.m_Fields.m_SRGB ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
				}
				break;
		}

		return DXGI_FORMAT_UNKNOWN;
	}
#endif

}
