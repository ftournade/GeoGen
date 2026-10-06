#pragma once
#ifndef XTM_DATA_FORMATS_H
#define XTM_DATA_FORMATS_H

#ifdef XTM_WIN32
	#include <d3d11.h> //for DXGI_FORMAT conversion
#endif


	struct DataFormat
	{
		enum class Layout : u32
		{
			Unknown,

			Layout_8,
			Layout_16,
			Layout_32,

			Layout_8_8,
			Layout_16_16,
			Layout_32_32,

			Layout_8_8_8,
			Layout_16_16_16,
			Layout_32_32_32,

			Layout_8_8_8_8,
			Layout_16_16_16_16,
			Layout_32_32_32_32,

			Layout_24_8,
			Layout_5_6_5,
			Layout_4_4_4_4,
			Layout_5_5_5_1,
			Layout_10_10_10_2,
			Layout_11_11_10,

			//BCn compressed formats
			BC1,
			BC2,
			BC3,
			BC4,
			BC5,
			BC6H,
			BC7,

			//PowerVR formats
			PVRTC2,
			PVRTC4
		};

		enum class Swizzle : u32
		{
			RGBA,
			BGRA,
			ABGR,
			DS, //depth/stencil
			Alpha,
			Luminance
		};

		enum class Type : u32
		{
			Typeless,

			Int,
			Norm,
			Float,

			//Z/Stencil formats
			Norm_Int,
			Typeless_Int,
			Norm_Typeless
		};

		union
		{
			struct Fields
			{
				Layout	m_Layout : 5;
				Swizzle m_Swizzle : 3;
				Type	m_Type : 3;
				u32		m_Compressed : 1;
				u32		m_Signed : 1;
				u32		m_SRGB : 1;

				u32		m_BitsPerPixel : 8;

				u32		m_Unused : 10;
			} m_Fields;
		
			u32 m_Raw;
		};
		
		enum Flag
		{
			IsCompressed	= BIT(0),
			IsSigned		= BIT(1),
			IsSRGB			= BIT(2)
		};

		inline DataFormat() { m_Fields.m_Unused = 0; }
		inline DataFormat( const DataFormat& _format ) : m_Raw( _format.m_Raw ) {}
		inline DataFormat( u32 _raw ) : m_Raw( _raw ) {}
		inline DataFormat( Layout _layout, Swizzle _swizzle, Type _type, u8 _flags )
		{
			m_Fields.m_Layout	= _layout;
			m_Fields.m_Swizzle	= _swizzle;
			m_Fields.m_Type	= _type;
			m_Fields.m_Compressed = _flags & IsCompressed ? 1 : 0;
			m_Fields.m_Signed = _flags & IsSigned ? 1 : 0;
			m_Fields.m_SRGB = _flags & IsSRGB ? 1 : 0;

			m_Fields.m_BitsPerPixel = GetBitsPerPixel( _layout );

			m_Fields.m_Unused	= 0;
		}

		//inline operator u32() const { return m_Raw; }

		inline bool operator==( const DataFormat& _rhs ) const { return m_Raw == _rhs.m_Raw; }
		inline bool operator!=( const DataFormat& _rhs ) const { return m_Raw != _rhs.m_Raw; }
		inline bool operator<( const DataFormat& _rhs ) const { return m_Raw < _rhs.m_Raw; } //for STL like containers

		inline u32 GetSizeInBytes() const { return m_Fields.m_BitsPerPixel / 8; }

		static u32 GetBitsPerPixel( Layout _layout );

		//Common formats
		static const DataFormat Unknown;

		static const DataFormat R32G32B32A32_TYPELESS;
		static const DataFormat R32G32B32A32_FLOAT;
		static const DataFormat R32G32B32A32_UINT;
		static const DataFormat R32G32B32A32_SINT;
		static const DataFormat R32G32B32_TYPELESS;
		static const DataFormat R32G32B32_FLOAT;
		static const DataFormat R32G32B32_UINT;
		static const DataFormat R32G32B32_SINT;
		static const DataFormat R16G16B16A16_TYPELESS;
		static const DataFormat R16G16B16A16_FLOAT;
		static const DataFormat R16G16B16A16_UNORM;
		static const DataFormat R16G16B16A16_UINT;
		static const DataFormat R16G16B16A16_SNORM;
		static const DataFormat R16G16B16A16_SINT;
		static const DataFormat R32G32_TYPELESS;
		static const DataFormat R32G32_FLOAT;
		static const DataFormat R32G32_UINT;
		static const DataFormat R32G32_SINT;
		//static const DataFormat R32G8X24_TYPELESS;
		//static const DataFormat D32_FLOAT_S8X24_UINT;
		//static const DataFormat R32_FLOAT_X8X24_TYPELESS;
		//static const DataFormat X32_TYPELESS_G8X24_UINT;
		static const DataFormat R10G10B10A2_TYPELESS;
		static const DataFormat R10G10B10A2_UNORM;
		static const DataFormat R10G10B10A2_UINT;
		static const DataFormat R11G11B10_FLOAT;
		static const DataFormat R8G8B8A8_TYPELESS;
		static const DataFormat R8G8B8A8_UNORM;
		static const DataFormat R8G8B8A8_UNORM_SRGB;
		static const DataFormat R8G8B8A8_UINT;
		static const DataFormat R8G8B8A8_SNORM;
		static const DataFormat R8G8B8A8_SINT;
		static const DataFormat R8G8B8_UNORM;
		static const DataFormat R8G8B8_UNORM_SRGB;
		static const DataFormat R16G16_TYPELESS;
		static const DataFormat R16G16_FLOAT;
		static const DataFormat R16G16_UNORM;
		static const DataFormat R16G16_UINT;
		static const DataFormat R16G16_SNORM;
		static const DataFormat R16G16_SINT;
		static const DataFormat R32_TYPELESS;
		static const DataFormat D32_FLOAT;
		static const DataFormat R32_FLOAT;
		static const DataFormat R32_UINT;
		static const DataFormat R32_SINT;
		static const DataFormat R24G8_TYPELESS;
		static const DataFormat D24_UNORM_S8_UINT;
		static const DataFormat R24_UNORM_X8_TYPELESS;
		static const DataFormat X24_TYPELESS_G8_UINT;
		static const DataFormat R8G8_TYPELESS;
		static const DataFormat R8G8_UNORM;
		static const DataFormat R8G8_UINT;
		static const DataFormat R8G8_SNORM;
		static const DataFormat R8G8_SINT;
		static const DataFormat R16_TYPELESS;
		static const DataFormat R16_FLOAT;
		static const DataFormat D16_UNORM;
		static const DataFormat R16_UNORM;
		static const DataFormat R16_UINT;
		static const DataFormat R16_SNORM;
		static const DataFormat R16_SINT;
		static const DataFormat R8_TYPELESS;
		static const DataFormat R8_UNORM;
		static const DataFormat R8_UINT;
		static const DataFormat R8_SNORM;
		static const DataFormat R8_SINT;
		static const DataFormat A8_UNORM;
		//static const DataFormat R1_UNORM;
		//static const DataFormat R9G9B9E5_SHAREDEXP;
		//static const DataFormat R8G8_B8G8_UNORM;
		//static const DataFormat G8R8_G8B8_UNORM;
		static const DataFormat BC1_TYPELESS;
		static const DataFormat BC1_UNORM;
		static const DataFormat BC1_UNORM_SRGB;
		static const DataFormat BC2_TYPELESS;
		static const DataFormat BC2_UNORM;
		static const DataFormat BC2_UNORM_SRGB;
		static const DataFormat BC3_TYPELESS;
		static const DataFormat BC3_UNORM;
		static const DataFormat BC3_UNORM_SRGB;
		static const DataFormat BC4_TYPELESS;
		static const DataFormat BC4_UNORM;
		static const DataFormat BC4_SNORM;
		static const DataFormat BC5_TYPELESS;
		static const DataFormat BC5_UNORM;
		static const DataFormat BC5_SNORM;
		static const DataFormat B5G6R5_UNORM;
		static const DataFormat B5G5R5A1_UNORM;
		static const DataFormat B8G8R8A8_UNORM;
		static const DataFormat B8G8R8X8_UNORM;
		//static const DataFormat R10G10B10_XR_BIAS_A2_UNORM;
		static const DataFormat B8G8R8A8_TYPELESS;
		static const DataFormat B8G8R8A8_UNORM_SRGB;
		//static const DataFormat B8G8R8X8_TYPELESS;
		//static const DataFormat B8G8R8X8_UNORM_SRGB;
		static const DataFormat BC6H_TYPELESS;
		static const DataFormat BC6H_UF16;
		static const DataFormat BC6H_SF16;
		static const DataFormat BC7_TYPELESS;
		static const DataFormat BC7_UNORM;
		static const DataFormat BC7_UNORM_SRGB;
		//static const DataFormat AYUV;
		//static const DataFormat Y410;
		//static const DataFormat Y416;
		//static const DataFormat NV12;
		//static const DataFormat P010;
		//static const DataFormat P016;
		//static const DataFormat 420_OPAQUE;
		//static const DataFormat YUY2;
		//static const DataFormat Y210;
		//static const DataFormat Y216;
		//static const DataFormat NV11;
		//static const DataFormat AI44;
		//static const DataFormat IA44;
		//static const DataFormat P8;
		//static const DataFormat A8P8;
		static const DataFormat B4G4R4A4_UNORM;

		//PowerVR formats
		static const DataFormat PVRTC2;
		static const DataFormat PVRTC4;
		
	};

#ifdef XTM_WIN32
	//TODO Move in lib 3D ?
	DXGI_FORMAT ConvertToDXGIFormat( DataFormat _format );
#endif

#endif
