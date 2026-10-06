//TODO move into Core !!!

#pragma once
#ifndef XTM_BITMAP_H
#define XTM_BITMAP_H



#include <Core/Str.h>
#include <Core/DataFormats.h>
#include <Core/Vec4.h>

namespace xtm
{
	class Color;
}

namespace xtm
{
	#define BITMAP_ALIGNMENT 1 
	//Some platform may need to align bitmap memory (e.g. XBox360)

	enum MipMapFilterType
	{
		MMFT_Point,
		MMFT_BoxAvg,
		MMFT_BoxAdd,
	};

	typedef		void (*PixelProcessorFn)(Color& c);
	
	class Bitmap
	{
	public:
		Bitmap();
		~Bitmap();

		void Clear();

		bool Create(	u32 _width, 
						u32 _height,
						u32 _depth, 
						u32 _mipMapLevels,
						DataFormat	_format);


		bool Load( const char* _filename );
		bool Save( const char* _filename ) const;

		bool LoadBMP( const char* _filename );
		bool SaveBMP( const char* _filename ) const;

		bool LoadDDS( const char* _filename );
		bool SaveDDS( const char* _filename ) const;

		bool LoadRAW_L32F( const char* _filename );
		bool SaveRAW( const char* _filename ) const;

		bool SaveHDR( const char* _filename ) const;
		
		bool LoadJPG( const char* _filename );
		bool LoadTGA( const char* _filename );
		bool LoadPNG( const char* _filename );
		bool LoadTIF( const char* _filename );
		bool LoadPVR( const char* _filename );

		inline DataFormat GetFormat() const { return m_Format; }
		
		inline u32 GetWidth() const { return m_Width; }
		inline u32 GetHeight() const { return m_Height; }
		inline u32 GetDepth() const { return m_Depth; }
		inline u32 GetArraySize() const { return m_ArraySize; }
		
		inline bool IsCubeMap() const { return m_bIsCubeMap; }
		
		inline u32 GetDataSize() const { return m_DataSize; }

		inline u32 GetNumMipMapLevels() const { return m_MipMapLevels; }

		void GetMipMapData( u32 _arrayIndex, u32 _cubeFace, u32 _mipMap,       byte** _ppData, u32* _pSize = NULL, u32* _pRowPitch = NULL );
		void GetMipMapData( u32 _arrayIndex, u32 _cubeFace, u32 _mipMap, const byte** _ppData, u32* _pSize = NULL, u32* _pRowPitch = NULL ) const;

		inline const byte* GetRawData() const	{ return m_pData; }
		inline		 byte* GetRawData()			{ return m_pData; }
		
		bool SwapRedAndGreenChannels();
		bool ConvertToFormat( DataFormat _format );
		bool ConvertToCubeMap();
		bool ConvertToNormalMap( float _bumpScale );

		//Note: This method remove all mipmaps
		bool Resize( u32 _sx, u32 _sy, u32 _sz ); //TODO filter box/bicubic etc

		bool Fill( const Color& _c );

		//_numMipMaps:	0 means whole mip-map chain down to 1x1
		//				1 means only the base mip-map and so on ...
		
		bool GenerateMipMaps( u32 _numMipMaps=0, MipMapFilterType _mipFilter=MMFT_BoxAvg );
		
		bool Blit( const Bitmap& _srcBitmap, s32 _destOffsetX, s32 _destOffsetY );
		bool StretchBlit( const Bitmap& _srcBitmap, s32 _destOffsetX, s32 _destOffsetY, s32 _destWidth, s32 _destHeight, PixelProcessorFn _proc=nullptr );

		//Sampling

		Color PointSample( const Vec2i & _coord, bool _bRepeat = false ) const;
		Color PointSample( const Vec2 & _uv ) const;
		Color BilinearSample( const Vec2 & _uv ) const;


		//Filters

		bool ApplyFilter_CustomKernel( const float* _weights, s32 _kernelSize );
		bool ApplyFilter_NoiseRemoval();
		bool ApplyFilter_3x3Median();

		//Misc

		void FlipVertically();
		Vec4 SumPixels() const;

		//Static methods

		static u32 GetMipMapRowSize( DataFormat _format, u32 _width );
		static u32 GetMipMapSize( DataFormat _format, u32 _width, u32 _height, u32 _depth = 1 );
		static u32 GetMipMapCountForFullMipMapChain( u32 _width, u32 _height, u32 _depth );

		static bool GetBitmapFileInfo(	const char* _filename, 
										DataFormat& _format, 
										u32& _width, 
										u32& _height, 
										u32& _depth, 
										u32& _mipmaps );

		static bool GetBitmapFileInfoBMP(	const char* _filename, 
											DataFormat& _format, 
											u32& _width, 
											u32& _height, 
											u32& _depth, 
											u32& _mipmaps );

		static bool GetBitmapFileInfoTIF(	const char* _filename, 
											DataFormat& _format, 
											u32& _width, 
											u32& _height, 
											u32& _depth, 
											u32& _mipmaps );

		static bool GetBitmapFileInfoRawFP32(	const char* _filename,
												DataFormat& _format,
												u32& _width,
												u32& _height,
												u32& _depth,
												u32& _mipmaps );		

		static bool GetBitmapGridInfo(	const Str&	_firstBitmapFilename,
										const char* _filenamePostfix,
										Str&		_filenameFormatingString,
										bool&		_bSwapXAndY,
										u32&		_tileResX,
										u32&		_tileResY,
										u32&		_minTileX, 
										u32&		_maxTileX,
										u32&		_minTileY, 
										u32&		_maxTileY );

	private:
		bool Fill_Float( const Color& _c );


	private:
		u32				m_Width, 
						m_Height,
						m_Depth,
						m_ArraySize,
						m_MipMapLevels;
		
		DataFormat		m_Format;
		
		byte*			m_pData;
		u32				m_DataSize;
		
		bool			m_bIsCubeMap;
	};
}

#endif
