//#include "stdafx.h"
#include "Bitmap.h"

#include <Core/StandardMath.h>
#include <Core/Color.h>

#include <Core/Utility.h>
#include <Core/Debug.h>
#include <Core/StandardLib.h>
#include <Core/FileStream.h>
#include <Core/FileSystem.h>
#include <Core/Log.h>

//#define XTM_SUPPORT_JPG_LOADING //GeoGen: disabled, libjpeg is not part of this repo

#ifdef XTM_SUPPORT_JPG_LOADING
	#include <libjpeg/jpeglib.h>
#endif

#if !defined(XTM_DISTRIBUTION) && !defined(XTM_IPHONE)
//	#define XTM_SUPPORT_DXT_CONVERSION //GeoGen: disabled, squish is not part of this repo

	#ifndef _WIN64 //PVRTexLib is only available as 32 bit
	//	#define XTM_SUPPORT_PVRTC_CONVERSION
	#endif
#endif

#ifdef XTM_SUPPORT_DXT_CONVERSION
	#include <squish.h>
#endif

#ifdef XTM_SUPPORT_PVRTC_CONVERSION
	#include <PVRTexLib/PVRTexLib.h>

	#if defined(XTM_WIN32)
		#pragma comment(lib, "../../../Extern/PVRTexLib/Win32/PVRTexLib.lib")
	#elif defined(XTM_MACOSX) 
		//#pragma comment(lib, "../../../Extern/PVRTexLib/MacOS/PVRTexLib.a") //Note: Doesn't work
	#elif defined(XTM_LINUX)
		#pragma comment(lib, "../../../Extern/PVRTexLib/Linux/PVRTexLib.a")
	#endif
#endif

	Bitmap::Bitmap() :
		m_pData(NULL),
		m_Width(0),
		m_Height(0),
		m_Depth(0),
		m_MipMapLevels(0),
		m_ArraySize(1),
		m_DataSize(0),
		m_bIsCubeMap(false)
	{
	}


	Bitmap::~Bitmap()
	{
		Clear();
	}

	void Bitmap::Clear()
	{
		SAFE_DELETE_ARRAY( m_pData )
		m_Width = 0;
		m_Height = 0;
		m_Depth = 0;
		m_MipMapLevels = 0;
		m_ArraySize = 1;
		m_DataSize = 0;

	}

	bool Bitmap::Create(	u32 _width, 
							u32 _height,
							u32 _depth, 
							u32 _mipMapLevels,
							DataFormat	_format)
	{
		DBG_CHECK( _mipMapLevels == 1 ); //Not yet supported		

		SAFE_DELETE_ARRAY( m_pData )

		m_Width = 0; 
		m_Height = 0;
		m_Depth = 0; 
		m_MipMapLevels = 0;

		u32 pixelSizeInBits = _format.m_Fields.m_BitsPerPixel;

		if( pixelSizeInBits == 0 )
			return false;
		
		m_Format = _format;
		
		u64 dataSize = ( (u64)pixelSizeInBits * (u64)_width * (u64)_height * (u64)_depth ) / 8ULL;

		m_pData = xtmNew byte[ dataSize ];

		if( !m_pData )
			return false;

		m_Width = _width; 
		m_Height = _height;
		m_Depth = _depth; 
		m_MipMapLevels = _mipMapLevels;
		m_DataSize = dataSize;
		
		m_bIsCubeMap = false;

		return true;
	}

	bool Bitmap::Load( const char* _filename )
	{
		Str ext = ToLower( GetFileExtension( _filename ) );

		if( ext == "dds" )
		{
			return LoadDDS( _filename );
		}
		else if( ext == "bmp" )
		{
			return LoadBMP( _filename );
		}
		else if( ext == "tga" )
		{
			return LoadTGA( _filename );
		}
		else if( ext == "tif" )
		{
			return LoadTIF( _filename );
		}
		else if( ext == "jpg" )
		{
			return LoadJPG( _filename );
		}
		else if( ext == "png" )
		{
			return LoadPNG( _filename );
		}
		else if( ext == "rawfp32" )
		{
			return LoadRAW_L32F( _filename );
		}

		DBG_CHECK( ext == "tex" );

		FileStream file;
		
		if( !file.Open( _filename, "rb" ) )
			return false;
		
		SAFE_DELETE_ARRAY( m_pData )
		
		//TODO file type checking and versioning 
		
		Read( file, m_Width );
		Read( file, m_Height );
		Read( file, m_Depth );
		Read( file, m_MipMapLevels );
		Read( file, m_Format );
		Read( file, m_DataSize );
		Read( file, m_bIsCubeMap );
		
		m_pData = (byte*)malloc( m_DataSize );
		
		if( !m_pData )
			return false;
		
		file.ReadBytes( m_pData, m_DataSize );

		return true;
	}

	bool Bitmap::Save( const char* _filename ) const
	{
		if( !m_pData )
			return false;
	
		Str ext = ToLower( GetFileExtension( _filename ) );

		if( ext == "dds" )
		{
			return SaveDDS( _filename );
		}
		else if( ext == "bmp" )
		{
			return SaveBMP( _filename );
		}
		else if( ext == "rawfp32" )
		{
			return SaveRAW( _filename );
		}

		FileStream file;
		
		if( !file.Open( _filename, "wb" ) )
			return false;

		//TODO file type checking and versioning 
		
		Write( file, m_Width );
		Write( file, m_Height );
		Write( file, m_Depth );
		Write( file, m_MipMapLevels );
		Write( file, m_Format );
		Write( file, m_DataSize );
		Write( file, m_bIsCubeMap );
		
		file.WriteBytes( m_pData, m_DataSize );
		
		return true;
	}

	bool Bitmap::SwapRedAndGreenChannels()
	{
		if( !m_pData || ( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8_8 ) )
			return false;

		u32 numPixels = m_Width * m_Height;
		u32 pixelSize = m_Format.GetSizeInBytes();
		
		for( u32 i=0 ; i < numPixels ; ++i )
		{
			u8* pPixel = (u8*)(m_pData + i * pixelSize);
			TSwap( pPixel[0], pPixel[2] );
		}
		
		return true;
	}

	u32 Bitmap::GetMipMapRowSize( DataFormat _format, u32 _width )
	{
		if( _format.m_Fields.m_Compressed )
		{
			CHECK( IsPow2( _width ) );
			u32 blockSizeInBytes = (_format.m_Fields.m_BitsPerPixel * 4 * 4) / 8;
			return ((_width + 3) / 4) * blockSizeInBytes;
		}
		else
		{
			return _format.GetSizeInBytes() * _width;
		}
/*						
			//PowerVR compression (iPhone)
			case DF_PVRTC2:
			{
				CHECK( IsPow2( _width ) && IsPow2( _height ) );
				u32 blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
				u32	widthBlocks = _width / 8;
				u32	heightBlocks = _height / 4;
				u32	bpp = 2;
				
				widthBlocks = Max( widthBlocks, (u32)2 );
				heightBlocks = Max( heightBlocks, (u32)2 );
				
				return widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);	
			}
				
			case DF_PVRTC4: 
			{
				CHECK( IsPow2( _width ) && IsPow2( _height ) );
				u32 blockSize = 4 * 4; // Pixel by pixel block size for 2bpp
				u32	widthBlocks = _width / 4;
				u32	heightBlocks = _height / 4;
				u32	bpp = 4;
				
				widthBlocks = Max( widthBlocks, (u32)2 );
				heightBlocks = Max( heightBlocks, (u32)2 );
				
				return widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);	
			}
		*/
	
	}


	u32 Bitmap::GetMipMapSize( DataFormat _format, u32 _width, u32 _height, u32 _depth )
	{
		if( _format.m_Fields.m_Compressed )
		{
			DBG_CHECK( _depth == 1 ); //TODO handle 3D tex

			//4x4 blocks
			CHECK( IsPow2( _width ) && IsPow2( _height ) );
				
			u32 blockSizeInBytes = (_format.m_Fields.m_BitsPerPixel * 4 * 4) / 8;
			u32 numBlocks = ((_width + 3) / 4) * ((_height + 3) / 4);
				
			return blockSizeInBytes * numBlocks;
		}
		else
		{
			return GetMipMapRowSize( _format, _width ) * _height * _depth;
		}

		/*		
			//PowerVR compression (iPhone)
			case DF_PVRTC2:
			{
				CHECK( IsPow2( _width ) && IsPow2( _height ) );
				u32 blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
				u32	widthBlocks = _width / 8;
				u32	heightBlocks = _height / 4;
				u32	bpp = 2;
				
				widthBlocks = Max( widthBlocks, (u32)2 );
				heightBlocks = Max( heightBlocks, (u32)2 );
				
				return widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);	
			}
				
			case DF_PVRTC4: 
			{
				CHECK( IsPow2( _width ) && IsPow2( _height ) );
				u32 blockSize = 4 * 4; // Pixel by pixel block size for 2bpp
				u32	widthBlocks = _width / 4;
				u32	heightBlocks = _height / 4;
				u32	bpp = 4;
				
				widthBlocks = Max( widthBlocks, (u32)2 );
				heightBlocks = Max( heightBlocks, (u32)2 );
				
				return widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);	
			}
		*/		
		
	}

	void Bitmap::GetMipMapData( u32 _arrayIndex, u32 _cubeFace, u32 _mipMap, byte** _ppData, u32* _pSize, u32* _pRowPitch )
	{
		GetMipMapData( _arrayIndex, _cubeFace, _mipMap, const_cast< const byte** >( _ppData ), _pSize, _pRowPitch );
	}

	void Bitmap::GetMipMapData( u32 _arrayIndex, u32 _cubeFace, u32 _mipMap, const byte** _ppData, u32* _pSize, u32* _pRowPitch ) const
	{
		DBG_CHECK( _arrayIndex < m_ArraySize );
		DBG_CHECK( _mipMap < m_MipMapLevels );
		DBG_CHECK( _cubeFace < 6 );
		
		const byte* pArrayElementData = m_pData + _arrayIndex * m_DataSize / m_ArraySize;
		const byte* pSubResourceData;

		if( m_bIsCubeMap )
		{
			u32 cubeMapSize = m_DataSize / (6 * m_ArraySize);

			pSubResourceData = pArrayElementData + cubeMapSize * _cubeFace;
		}
		else
		{
			DBG_CHECK( _cubeFace == 0 );
			pSubResourceData = pArrayElementData;
		}


		u32 size, rowPitch;
		
		//TODO use GetMipMapSize instead of repeating this code
		if( m_Format.m_Fields.m_Compressed )
		{
			DBG_CHECK( m_Depth == 1 ); //TODO handle 3D tex

			u32 w = m_Width;
			u32 h = m_Height;
				
			u32 blockSizeInBytes = (m_Format.m_Fields.m_BitsPerPixel * 4 * 4) / 8;

			size = GetMipMapSize( m_Format, w, h );
			u32 offset = 0;

			u32 blockCount = ((w + 3) / 4);
			rowPitch =  blockCount * blockSizeInBytes;
				
			for( u32 i=0 ; i < _mipMap ; ++i )
			{					
				w /= 2;
				h /= 2;

				offset += size;
					
				size = GetMipMapSize( m_Format, w, h );

				blockCount = ((w + 3) / 4);
				rowPitch =  blockCount * blockSizeInBytes;
			}

			*_ppData = pSubResourceData + offset;
		}
		else
		{
			u32 texelSize = m_Format.GetSizeInBytes();
			DBG_CHECK( texelSize > 0 );
				
			//u32 offset = 0;

			rowPitch = texelSize * m_Width;
				
			size = m_Width * m_Height * m_Depth * texelSize;
			*_ppData = pSubResourceData;
				
			//const byte* pMipMapData = pCubeFaceData;
				
			for( u32 i=0 ; i < _mipMap ; ++i )
			{
				*_ppData += size;
				rowPitch /= 2;
				size /= 4;
			}
		}
		/*
		//#ifdef XTM_IPHONE
		case DF_PVRTC2:
		case DF_PVRTC4:
		{
			//Compressed
				
			u32 w = m_Width;
			u32 h = m_Height;
			u32 blockSize;
			u32 widthBlocks;
			u32 heightBlocks;
			u32 bpp;
				
			const byte* pMipMapData = pCubeFaceData;
				
			for( u32 i=0 ; i <= _mipMap ; ++i )
			{

				if( m_Format == DF_PVRTC4 )
				{
					blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
					widthBlocks = w / 4;
					heightBlocks = h / 4;
					bpp = 4;
				}
				else
				{
					blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
					widthBlocks = w / 8;
					heightBlocks = h / 4;
					bpp = 2;
				}
					
				widthBlocks = Max( widthBlocks, (u32)2 );
				heightBlocks = Max( heightBlocks, (u32)2 );
					
				size = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
				*_ppData = pMipMapData;

				pMipMapData += size;
					
				rowPitch = blockSize * widthBlocks;

				w = Max( w >> 1, (u32)1 );
				h = Max( h >> 1, (u32)1 );
			}
				
			break;
		}
		*/
				
		
		if( _pSize )
			*_pSize = size;

		if( _pRowPitch )
			*_pRowPitch = rowPitch;
	}

	u32 Bitmap::GetMipMapCountForFullMipMapChain( u32 _width, u32 _height, u32 _depth )
	{
		/*	
		u32 w = _desc.Width;
		u32 h = _desc.Height;
		//u32 d = _desc.Depth;
			
		while( (w > 0) && (h > 0) )
		{
			++actualMipMapCount;
			w /= 2;
			h /= 2;
		}
		*/
		DBG_CHECK_MSG( _depth == 1, "3D Textures not yet implemented" );
		return Min( Log2( (float)_width ), Log2( (float)_height ) ) + 1;
	}

	bool Bitmap::GenerateMipMaps( u32 _numMipMaps, MipMapFilterType _mipFilter )
	{
		#define SAMPLE_MIPMAP( mipMapAddr, w, h, x, y ) mipMapAddr + ((y) * (w) + (x)) * texelSize
		//#define DBG_MIPMAPS
		
		if( !m_pData )
			return false;
		
		if( ( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8 ) &&
			( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8 ) && 
			( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8_8 ) )
			return false;
		
		if( !IsPow2( m_Width ) || !IsPow2( m_Height ) )
			return false;
		
		
		//Find number of mip maps to compute
		
		u32 maxMipMapLevels = Min( Log2( (float)m_Width ), Log2( (float)m_Height ) ) + 1;

//printf( "maxMipMapLevels %u\n", maxMipMapLevels );

		if( _numMipMaps == 0 )
		   _numMipMaps = maxMipMapLevels;
		else
			_numMipMaps = Min( _numMipMaps, maxMipMapLevels );
		
		//Allocate enough space
		
		u32 texelSize = m_Format.GetSizeInBytes();
				
		u32 firstMipMapSize = m_Width * m_Height * texelSize;
		u32 mipMapSize = firstMipMapSize;
		u32 faceSize = 0;

		for( u32 i=0 ; i < _numMipMaps ; ++i )
		{
			faceSize += mipMapSize;
			
			mipMapSize /= 4;
		}
		
		u32 numFaces;

		if( m_bIsCubeMap )
		{
			m_DataSize = faceSize * 6;
			numFaces = 6;
		}
		else
		{
			m_DataSize = faceSize;
			numFaces = 1;
		}

		
		float normalizationFactor;

		switch( _mipFilter )
		{
			//case MMFT_Point:
			case MMFT_BoxAvg: normalizationFactor = 0.25f; break;
			case MMFT_BoxAdd: normalizationFactor = 1.0f; break;
			default: normalizationFactor = 0.25f; break;
		}
		
		byte* pOldData = m_pData;
		m_pData = (byte*)malloc( m_DataSize );

		for( u32 i=0 ; i < numFaces ; ++i )
		{
			//Copy highest mip map
			Xmemcpy( m_pData + faceSize * i, 
					pOldData + firstMipMapSize * i, 
					firstMipMapSize );
		
			
			byte* pPreviousMipMap = m_pData + faceSize * i;
			byte* pCurMipMap = m_pData + faceSize * i + firstMipMapSize;
				
			u32 prevW = m_Width;
			u32 prevH = m_Height;
			u32 w = m_Width / 2;
			u32 h = m_Height / 2;
   
		
			//Downsample mip maps
			
			for( u32 i=1 ; i < _numMipMaps ; ++i )
			{
				#ifdef DBG_MIPMAPS
					u8 qR = Random( 0, 255 );
					u8 qG = Random( 0, 255 );
					u8 qB = Random( 0, 255 );
				#endif
			
				for( u32 y=0 ; y < h ; ++y )
				{
					for( u32 x=0 ; x < w ; ++x )
					{
						byte* destTexel = SAMPLE_MIPMAP( pCurMipMap, w, h, x, y );
					
						byte* srcTexel1 = SAMPLE_MIPMAP( pPreviousMipMap, prevW, prevH, x*2    , y*2 );
						byte* srcTexel2 = SAMPLE_MIPMAP( pPreviousMipMap, prevW, prevH, x*2 + 1, y*2 );
						byte* srcTexel3 = SAMPLE_MIPMAP( pPreviousMipMap, prevW, prevH, x*2    , y*2 + 1 );
						byte* srcTexel4 = SAMPLE_MIPMAP( pPreviousMipMap, prevW, prevH, x*2 + 1, y*2 + 1 );
					
						switch( m_Format.m_Fields.m_Layout )
						{
							case DataFormat::Layout::Layout_8:
							{
								DBG_CHECK( destTexel < m_pData + m_DataSize );

								float r = normalizationFactor * (((float)srcTexel1[0]) + ((float)srcTexel2[0]) + ((float)srcTexel3[0]) + ((float)srcTexel4[0]) );
								r = Min( r, 255.0f );

								#ifdef DBG_MIPMAPS
									destTexel[0] = qR;
								#else
									destTexel[0] = (u8)r;
								#endif
								break;
							}
							case DataFormat::Layout::Layout_8_8_8:
							{
								DBG_CHECK( destTexel + 2 < m_pData + m_DataSize );

								float r = normalizationFactor * (((float)srcTexel1[0]) + ((float)srcTexel2[0]) + ((float)srcTexel3[0]) + ((float)srcTexel4[0]) );
								float g = normalizationFactor * (((float)srcTexel1[1]) + ((float)srcTexel2[1]) + ((float)srcTexel3[1]) + ((float)srcTexel4[1]) );
								float b = normalizationFactor * (((float)srcTexel1[2]) + ((float)srcTexel2[2]) + ((float)srcTexel3[2]) + ((float)srcTexel4[2]) );

								r = Min( r, 255.0f );
								g = Min( g, 255.0f );
								b = Min( b, 255.0f );
							
								#ifdef DBG_MIPMAPS
									destTexel[0] = qR;
									destTexel[1] = qG;
									destTexel[2] = qB;
								#else
									destTexel[0] = (u8)r;
									destTexel[1] = (u8)g;
									destTexel[2] = (u8)b;
								#endif
								break;
							}
							case DataFormat::Layout::Layout_8_8_8_8:
							{
								DBG_CHECK( destTexel + 3 < m_pData + m_DataSize );

								float r = normalizationFactor * (((float)srcTexel1[0]) + ((float)srcTexel2[0]) + ((float)srcTexel3[0]) + ((float)srcTexel4[0]) );
								float g = normalizationFactor * (((float)srcTexel1[1]) + ((float)srcTexel2[1]) + ((float)srcTexel3[1]) + ((float)srcTexel4[1]) );
								float b = normalizationFactor * (((float)srcTexel1[2]) + ((float)srcTexel2[2]) + ((float)srcTexel3[2]) + ((float)srcTexel4[2]) );
								float a = normalizationFactor * (((float)srcTexel1[3]) + ((float)srcTexel2[3]) + ((float)srcTexel3[3]) + ((float)srcTexel4[3]) );

								r = Min( r, 255.0f );
								g = Min( g, 255.0f );
								b = Min( b, 255.0f );
								a = Min( a, 255.0f );

								destTexel[0] = (u8)r;
								destTexel[1] = (u8)g;
								destTexel[2] = (u8)b;
								destTexel[3] = (u8)a;
								break;
							}
							
						}
					}
				}
			
				pPreviousMipMap = pCurMipMap;
				pCurMipMap += w * h * texelSize;
			
				w /= 2;
				h /= 2;
				prevW /=2;
				prevH /= 2;
			}
		
		}

		free( pOldData );

		m_MipMapLevels = _numMipMaps;
		
		return true;
	}

	bool Bitmap::ConvertToCubeMap()
	{
		//We're using the http://cgtextures.com layout
		//Cube faces are stored horizontaly in the bitmap
		
		if( !m_pData || (m_MipMapLevels != 1) )
			return false;
		
		if( ( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8 ) && 
			( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8_8 ) )
			return false;

		if( !IsPow2( m_Height ) )
			return false;
		
		if( m_Width != m_Height * 6 )
			return false;
		
		u32 texelSize = m_Format.GetSizeInBytes();
		
		u32 newDataSize = texelSize * m_Width * m_Height;
		
		byte* pNewData = (byte*)malloc( newDataSize );
		
		if( !m_pData )
			return false;
		
		u32 cubeSize = m_Height;
		u32 cubeFaceDataSize = cubeSize * cubeSize * texelSize;

		for( u32 i=0 ; i < 6 ; ++i )
		{
			//For each cube-face
			
			byte* pCubeFaceData = pNewData + cubeFaceDataSize * i;
			byte* pDstTexel = pCubeFaceData;
			
			for( u32 y=0 ; y < cubeSize ; ++y )
			{
				for( u32 x=0 ; x < cubeSize ; ++x )
				{
					byte* pSrcTexel = m_pData + (y * m_Width + x + i * cubeSize) * texelSize;
					byte* pDstTexel = pCubeFaceData + (y * cubeSize + x) * texelSize;
					
					for( u32 j=0 ; j < texelSize ; ++j )
						*pDstTexel++ = *pSrcTexel++;					
				}
			}			
		
		}
		
		
		free( m_pData );
		m_pData = pNewData;
		m_DataSize = newDataSize;
		m_Width = m_Height;
		m_bIsCubeMap = true;

		return true;
	}

	bool Bitmap::ConvertToFormat( DataFormat _destFormat )
	{
		//TODO for floating point maps, converts them first to DF_R32G32B32A32_FLOAT
		//(DF_R8G8B8A8 for non floating point ones)
		//And then convert to dest format from here
		//Then if we code the conversion to and from those 2 generic formats, we can convert everything
		//to everything and avoiding coding each combination
		
		if( !m_pData )
			return false;

		if( _destFormat == m_Format )
			return true;
		
		u32 numPixels = m_Width * m_Height;

#ifdef XTM_SUPPORT_DXT_CONVERSION
		if( ( _destFormat.m_Fields.m_Layout == DataFormat::Layout::BC1 ) ||
			( _destFormat.m_Fields.m_Layout == DataFormat::Layout::BC2 ) ||
			( _destFormat.m_Fields.m_Layout == DataFormat::Layout::BC2 ) )
		{
			if( !ConvertToFormat( DataFormat::R8G8B8A8_UNORM ) )
				return false;
			
			u32 squishFormat;
			
			switch( _destFormat.m_Fields.m_Layout )
			{
				case DataFormat::Layout::BC1: squishFormat = squish::kDxt1; break;
				case DataFormat::Layout::BC2: squishFormat = squish::kDxt3; break;
				case DataFormat::Layout::BC3: squishFormat = squish::kDxt5; break;
				default: DBG_CHECK(false); break;
			}
			//Compute the size of the compressed bitmap 
			
			u32 newDataSize = 0;
			u32 newNumMipMaps = 0;
			u32 w = m_Width;
			u32 h = m_Height;
			
			for( u32 i=0 ; i < m_MipMapLevels ; ++i )
			{
				newDataSize += GetMipMapSize( _destFormat, w, h );
				
				++newNumMipMaps;
				
				w /= 2;
				h /= 2;
				
				//if( (w < 4) || (h < 4) )
				//	break;
			}

			byte* pNewData = (byte*)malloc( newDataSize );

			u32 offset=0;
			
			w = m_Width;
			h = m_Height;
			
			for( u32 i=0 ; i < newNumMipMaps ; ++i )
			{
				const byte* srcMipMap;
				u32 srcMipMapSize;
				
				GetMipMapData( 0, 0, i, &srcMipMap, &srcMipMapSize );
				
				squish::CompressImage( srcMipMap, w, h, pNewData + offset, squishFormat );
				
				offset += GetMipMapSize( _destFormat, w, h );
				
				w /= 2;
				h /= 2;
			}
			
			free( m_pData );
			m_pData = pNewData;
			m_Format = _destFormat;
			m_DataSize = newDataSize;
			m_MipMapLevels = newNumMipMaps;

			return true;
		}
		else
#endif
#ifdef XTM_SUPPORT_PVRTC_CONVERSION
		if( (( m_Format == DF_R8G8B8 ) || ( m_Format == DF_R8G8B8A8 )) && ( ( _destFormat == DF_PVRTC2 ) || ( _destFormat == DF_PVRTC4 ) ) ) 
		{
			//TODO support cubemaps
			CHECK( !IsCubeMap() )

			if( m_Width != m_Height )
				return false; //iPhone supports only square PVRTC

			if( ( _destFormat == DF_PVRTC2 ) && ((m_Width < 16) || (m_Height < 8)) )
				return false; //PVRTC2 minimum size is 16x8

			if( ( _destFormat == DF_PVRTC4 ) && ((m_Width < 8) || (m_Height < 8)) )
				return false; //PVRTC2 minimum size is 16x8

			bool bHasAlpha = ( m_Format == DF_R8G8B8A8 );

			if( !ConvertToFormat( DF_R8G8B8A8 ) )
				return false;

			pvrtexlib::PVRTextureUtilities* pvrUtility = pvrtexlib::PVRTextureUtilities::getPointer();
			
			pvrtexlib::CPVRTexture uncompressedTex( m_Width, m_Height, m_MipMapLevels - 1, 1,
													false, false, false, false, false, 
													bHasAlpha, 
													false,
													pvrtexlib::eInt8StandardPixelType, //(m_Format == DF_R8G8B8A8) ? pvrtexlib::OGL_RGBA_8888 : pvrtexlib::OGL_RGB_888,
													0.0f,
													m_pData );

			DBG_CHECK( uncompressedTex.getData().getDataSize() == m_DataSize );

			pvrtexlib::CPVRTexture pvrTexture( uncompressedTex.getHeader() );
			pvrTexture.setPixelType( ( _destFormat == DF_PVRTC2 ) ? pvrtexlib::OGL_PVRTC2 : pvrtexlib::OGL_PVRTC4 );

			pvrUtility->CompressPVR( uncompressedTex, pvrTexture );//TODO error handling ( PVRTexLib uses exceptions :( )
			

			free( m_pData );
			m_Format = _destFormat;
			m_DataSize = pvrTexture.getData().getDataSize();
			m_pData = (byte*)malloc( m_DataSize );
			Xmemcpy( m_pData, pvrTexture.getData().getData(), m_DataSize );
			
			return true;
		}
		else
#endif
		if( ( m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8 ) && ( _destFormat.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8_8 ) )
		{
			numPixels = m_DataSize / 3;

			u32 newDataSize = numPixels * 4; //includes mipmaps

			byte* pNewData = (byte*)malloc( newDataSize * 4 );

			
			for( u32 i=0 ; i < numPixels ; ++i )
			{
				byte* pNewPixel = pNewData + i * 4;
				const byte* pOldPixel = m_pData + i * 3;

				pNewPixel[0] = pOldPixel[0];
				pNewPixel[1] = pOldPixel[1];
				pNewPixel[2] = pOldPixel[2];
				pNewPixel[3] = 255;
			}

			free( m_pData );
			m_pData = pNewData;
			m_Format = _destFormat;
			m_DataSize = newDataSize;

			return true;
		}
		else if( ( m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8_8 ) && ( _destFormat.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8 ) )
		{
			numPixels = m_DataSize / 4;
			
			byte* pNewData = (byte*)malloc( numPixels * 3 );
			
			for( u32 i=0 ; i < numPixels ; ++i )
			{
				byte* pNewPixel = pNewData + i * 3;
				const byte* pOldPixel = m_pData + i * 4;
				
				pNewPixel[0] = pOldPixel[0];
				pNewPixel[1] = pOldPixel[1];
				pNewPixel[2] = pOldPixel[2];
			}
			
			free( m_pData );
			m_pData = pNewData;
			m_Format = _destFormat;
			m_DataSize = numPixels * 3;
			
			return true;
		}
		else if( ( m_Format == DataFormat::R16_UNORM ) && ( _destFormat == DataFormat::R32_FLOAT ) )
		{
			byte* pNewData = (byte*)malloc( numPixels * sizeof(float) );

			float miN = FLT_MAX;
			float maX = - FLT_MAX;

			for( u32 i=0 ; i < numPixels ; ++i )
			{
				float* pNewPixel = (float*)pNewData + i;
				const u16* pOldPixel = (const u16*)m_pData + i;
				
				*pNewPixel = (float)(*pOldPixel) / 65535.0f;
			
				miN = Min( miN, *pNewPixel );
				maX = Max( maX, *pNewPixel );
			}
			LOG( "TUE MOI!" );
			
			free( m_pData );
			m_pData = pNewData;
			m_Format = _destFormat;
			m_DataSize = numPixels * sizeof(float);
			m_MipMapLevels = 1;
			
			return true;			
		}
		else if( ( m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8 ) && ( _destFormat == DataFormat::R32_FLOAT ) )
		{
			byte* pNewData = (byte*)malloc( numPixels * sizeof(float) );
			
			for( u32 i=0 ; i < numPixels ; ++i )
			{
				float* pNewPixel = (float*)pNewData + i;
				const u8* pOldPixel = (const u8*)m_pData + i * 3;
				
				*pNewPixel = (float)(*pOldPixel) / 255.0f;
			}
			
			free( m_pData );
			m_pData = pNewData;
			m_Format = _destFormat;
			m_DataSize = numPixels * sizeof(float);
			m_MipMapLevels = 1;
			
			return true;			
		}
		else if( ( m_Format == DataFormat::R32_FLOAT ) && ( _destFormat.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8 ) )
		{
			byte* pNewData = (byte*)malloc( numPixels * sizeof(u8) * 3 );
			
			for( u32 i=0 ; i < numPixels ; ++i )
			{
				u8* pNewPixel = (u8*)pNewData + i * 3;
				const float* pOldPixel = (const float*)m_pData + i;
				
				u8 luminance = (u8)( *pOldPixel * 255.0f );
				pNewPixel[0] = luminance;
				pNewPixel[1] = luminance;
				pNewPixel[2] = luminance;
			}
			
			free( m_pData );
			m_pData = pNewData;
			m_Format = _destFormat;
			m_DataSize = numPixels * sizeof(u8) * 3;
			m_MipMapLevels = 1;
			
			return true;			
		}
		else
		{
			return false;
		}
	}
	
	bool Bitmap::ConvertToNormalMap( float _bumpScale )
	{
		if( ( m_MipMapLevels != 1 ) || ( m_Depth != 1 ) || ( m_ArraySize != 1 ) )
			return false;

		if( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8 )
			return false;

		rgb8_t* pNormalData = (rgb8_t*)malloc( 3 * m_Width * m_Height );

		for( s32 y = 0; y < m_Height; ++y )
		{
			for( s32 x = 0; x < m_Width; ++x )
			{
				
				float h = PointSample( Vec2i( x, y ) ).r;
				float hx = PointSample( Vec2i( x + 1, y ) ).r;
				float hy = PointSample( Vec2i( x, y + 1 ) ).r;

				Vec3 normal( hx - h, hy - h, 1.0f );
				normal.x *= _bumpScale;
				normal.y *= _bumpScale;
				normal.Normalize();
				normal = normal * 0.5f + 0.5f;

				rgb8_t& dst = pNormalData[y * m_Width + x];
				dst.r = (u8)(normal.x * 255.0f);
				dst.g = (u8)(normal.y * 255.0f);
				dst.b = (u8)(normal.z * 255.0f);
			}

		}

		free( m_pData );
		m_pData = (byte*)pNormalData;

		return true;
	}


	////////////////

#ifdef XTM_SUPPORT_JPG_LOADING

	bool Bitmap::LoadJPG( const char* _filename )
	{
		struct my_error_mgr {
			jpeg_error_mgr	pub;	/* "public" fields */
		//	jmp_buf			setjmp_buffer;	/* for return to caller */
		};

		struct jpeg_decompress_struct cinfo;
		struct my_error_mgr jerr;
		JSAMPARRAY buffer;      /* Output row buffer */
		int row_stride;     /* physical row width in output buffer */

		FILE* infile = fopen( _filename, "rb" );

		if( !infile )
		{
			//fprintf(stderr, "can't open %s\n", filename);
			return false;
		}

		/* We set up the normal JPEG error routines, then override error_exit. */
		cinfo.err = jpeg_std_error(&jerr.pub);
		//jerr.pub.error_exit = my_error_exit;

		/* Establish the setjmp return context for my_error_exit to use. */
/*		if( setjmp(jerr.setjmp_buffer) )
		{

			jpeg_destroy_decompress(&cinfo);
			fclose(infile);
			return false;
		}
*/
		jpeg_create_decompress( &cinfo );
		jpeg_stdio_src( &cinfo, infile );
		jpeg_read_header( &cinfo, TRUE );
		jpeg_start_decompress( &cinfo );

		//////////////////

		CHECK( cinfo.output_components == 3 );

		m_Width = cinfo.output_width;
		m_Height = cinfo.output_height;
		m_Depth = 1; 
		m_MipMapLevels = 1;
		
		m_Format = DataFormat::R8G8B8_UNORM_SRGB;
		
		m_DataSize = m_Width * m_Height * 4;
		m_pData = (byte*)malloc( m_DataSize );
		
		m_bIsCubeMap = false;
		
		/////////////////

		row_stride = m_Width * cinfo.output_components;
		
		/* Make a one-row-high sample array that will go away when done with image */
		buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

		while (cinfo.output_scanline < cinfo.output_height) 
		{
			//IT ALWAYS crash ON THIS JPEG_READ_SCANLINES FUNCTION CALL BELOW
			jpeg_read_scanlines( &cinfo, buffer, 1 );
			//counter += row_stride;

		#if 0 //swap y
			u32 y = (m_Height - cinfo.output_scanline) * row_stride;
		#else	
			u32 y = (cinfo.output_scanline - 1) * row_stride;
		#endif

			for( u32 x=0 ; x < 3 * m_Width; x += 3)
			{
				m_pData[y+x]   = buffer[0][x];
				m_pData[y+x+1] = buffer[0][x+1];
				m_pData[y+x+2] = buffer[0][x+2];
			}
		}
		
		jpeg_finish_decompress( &cinfo );
		jpeg_destroy_decompress( &cinfo );

		fclose( infile );
		return true;
	}

#endif //GeoGen: otherwise LoadJPG is implemented with stb_image in Bitmap_STB.cpp

	bool Bitmap::LoadTGA( const char* _filename )
	{
		return false;
	}


	bool Bitmap::LoadRAW_L32F( const char* _filename )
	{
		//TODO this is leaking
		
		FileSystem::FileSize fileSize;
		
		if( !FileSystem::Get().GetFileSize( _filename, fileSize ) )
			return false;
		
		if( fileSize % sizeof(float) != 0 )
			return false;
		
		u32 numTexels = (u32)( fileSize / sizeof(float) );
		
		u32 dimension = Sqrti( numTexels ); //Assuming square texture
		
		if( dimension * dimension != numTexels )
			return false;
		
		FileStream file;
		
		if( !file.Open( _filename, "rb" ) )
			return false;
		
		m_pData = (byte*)malloc( (size_t)fileSize );
		
		if( !m_pData )
			return false; //Not enough memory !
		
		file.ReadBytes( m_pData, (IByteStream::OffsetType)fileSize );

		m_Width = dimension;
		m_Height = dimension;
		m_Depth = 1;
		m_MipMapLevels = 1;
		
		m_Format = DataFormat::R32_FLOAT;
		m_DataSize = (u32)fileSize;

		m_bIsCubeMap = false;
		
		
		return true;
	}

	bool Bitmap::SaveRAW( const char* _filename ) const
	{
		FileStream file;

		if( !file.Open( _filename, "wb" ) )
			return false;

		u32 size = m_Width * m_Height * m_Format.GetSizeInBytes();

		file.WriteBytes( m_pData, size );

		return true;
	}

	bool Bitmap::StretchBlit( const Bitmap& _srcBitmap, s32 _destOffsetX, s32 _destOffsetY, s32 _destWidth, s32 _destHeight, PixelProcessorFn _pixelProcessor )
	{
		if( ( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8_8 )
		 && ( m_Format.m_Fields.m_Type != DataFormat::Type::Float ) )
			return false;

		u32 numChannels = 4;

		switch( m_Format.m_Fields.m_Layout )
		{
			case DataFormat::Layout::Layout_32: numChannels = 1; break;
			case DataFormat::Layout::Layout_32_32: numChannels = 2; break;
			case DataFormat::Layout::Layout_32_32_32: numChannels = 3; break;
			case DataFormat::Layout::Layout_32_32_32_32: numChannels = 4; break;
		}

		byte* pDstPixels;
		u32 unused;

		u32 pixelSize = m_Format.GetSizeInBytes();

		GetMipMapData( 0, 0, 0, &pDstPixels, &unused );

		for( u32 y = 0 ; y < _destHeight ; ++y )
		{
			s32 Y = (s32)y + _destOffsetY;

			if( (Y < 0) || (Y >= (s32)m_Height) )
				continue;

			//GeoGen: sample at the pixel center of the whole destination rect. The previous code remapped
			//the clipped uv range onto the full rect, which stretched rects crossing the bitmap edges.
			float v = ( (float)y + 0.5f ) / (float)_destHeight;

			for( u32 x = 0 ; x < _destWidth ; ++x )
			{
				s32 X = (s32)x + _destOffsetX;

				if( (X < 0) || (X >= (s32)m_Width) )
					continue;

				float u = ( (float)x + 0.5f ) / (float)_destWidth;

				Vec2 uv( u, v );
				//uv.y = 1.0f - uv.y; //TODO not sure why ...

				Color c = _srcBitmap.PointSample( uv ); //TODO linear, bicubic ...

				if( _pixelProcessor )
				{
					_pixelProcessor( c );
				}
				
				s32 pixelIndex = (y + _destOffsetY) * m_Width + (x + _destOffsetX);

				if( m_Format.m_Fields.m_Type == DataFormat::Type::Float )
				{
					float* dstPixel = ((float*)pDstPixels) + pixelIndex * numChannels;

					for( int iChannel = 0 ; iChannel < numChannels ; ++iChannel )
					{
						dstPixel[ iChannel ] = c[ iChannel ];
					}
				} 
				else if( m_Format.m_Fields.m_Type == DataFormat::Type::Norm )
				{
					c.Clamp();

					rgba8_t& dstPixel = ((rgba8_t*)pDstPixels)[ pixelIndex ];

					dstPixel.r = (u8)(c.r * 255.0f);
					dstPixel.g = (u8)(c.g * 255.0f);
					dstPixel.b = (u8)(c.b * 255.0f);
					dstPixel.a = (u8)(c.a * 255.0f);

				}

			}
		}

		return true;
	}

	bool Bitmap::Blit( const Bitmap& _srcBitmap, s32 _destOffsetX, s32 _destOffsetY )
	{
		if( _srcBitmap.GetFormat() != GetFormat() )
			return false;

		s32 srcMinX = 0;
		s32 srcMinY = 0;
		s32 srcMaxX = _srcBitmap.GetWidth();
		s32 srcMaxY = _srcBitmap.GetHeight();
		s32 dstMinX = _destOffsetX;
		s32 dstMinY = _destOffsetY;
		s32 dstMaxX = _destOffsetX + _srcBitmap.GetWidth();
		s32 dstMaxY = _destOffsetY + _srcBitmap.GetHeight();

		if( dstMinX < 0 )
		{
			dstMinX = 0;
			srcMinX = - _destOffsetX;
		}

		if( dstMaxX > (s32)m_Width )
		{
			dstMaxX = m_Width;
		}

		if( dstMinY < 0 )
		{
			dstMinY = 0;
			srcMinY = - _destOffsetY;
		}

		if( dstMaxY > (s32)m_Height )
		{
			dstMaxY = m_Height;
		}

		u32 srcX = srcMinX;
		u32 srcY = srcMinY;
		
		byte* pDstPixels;
		const byte* pSrcPixels;
		u32 unused;

		u32 pixelSize = m_Format.GetSizeInBytes();

		GetMipMapData( 0, 0, 0, (byte**)&pDstPixels, &unused );
		_srcBitmap.GetMipMapData( 0, 0, 0, (const byte**)&pSrcPixels, &unused );

		u32 rowSize = (dstMaxX - dstMinX) * pixelSize;

		for( s32 y = dstMinY ; y < dstMaxY ; ++y )
		{

			const byte* pSrcPixelRow = pSrcPixels + (srcY * _srcBitmap.GetWidth() + srcMinX) * pixelSize;
			byte* pDestPixelRow = pDstPixels + (y * GetWidth() + dstMinX) * pixelSize;

			Xmemcpy( pDestPixelRow, pSrcPixelRow, rowSize );

			++srcY;
		}

		return true;
	}

	//See http://www.fho-emden.de/~hoffmann/bicubic03042002.pdf

	bool Bitmap::Resize( u32 _sx, u32 _sy, u32 _sz )
	{
		NOT_YET_IMPLEMENTED
		
		return false;
	}

	bool Bitmap::Fill( const Color& _c )
	{
		if( !m_pData )
			return false;

		if( m_Format.m_Fields.m_Type == DataFormat::Type::Float )
			return Fill_Float( _c );

		if( ( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8_8 ) &&
			( m_Format.m_Fields.m_Layout != DataFormat::Layout::Layout_8_8_8 ) )
			return false;
		
		u32 texelSize = m_Format.GetSizeInBytes();

		u8 r = (u8)(_c.r * 255.0f);
		u8 g = (u8)(_c.g * 255.0f);
		u8 b = (u8)(_c.b * 255.0f);
		u8 a = (u8)(_c.a * 255.0f);

		for( u32 iMipMap=0 ; iMipMap < m_MipMapLevels ; ++iMipMap )
		{
			byte* pPixels;
			u32 size;
			GetMipMapData( 0, 0, iMipMap, &pPixels, &size );

			for( u32 y=0 ; y < m_Height ; ++y )
			{
				for( u32 x=0 ; x < m_Width ; ++x )
				{
					byte* pCurPixel = pPixels + (y * m_Width + x) * texelSize;
					pCurPixel[0] = r;
					pCurPixel[1] = g;
					pCurPixel[2] = b;

					if( m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8_8 )
						pCurPixel[3] = a;
				}
			}
		}

		return true;
	}

	bool Bitmap::Fill_Float( const Color& _c )
	{
		u32 numChannels = 0;

		if( m_Format.m_Fields.m_Type != DataFormat::Type::Float )
			return false;

		switch( m_Format.m_Fields.m_Layout )
		{
			case DataFormat::Layout::Layout_32: numChannels = 1; break;
			case DataFormat::Layout::Layout_32_32: numChannels = 2; break;
			case DataFormat::Layout::Layout_32_32_32: numChannels = 3; break;
			case DataFormat::Layout::Layout_32_32_32_32: numChannels = 4; break;
			default:
				return false;
		}

		for( u32 iMipMap = 0 ; iMipMap < m_MipMapLevels ; ++iMipMap )
		{
			u32 size;
			float* pPixels;
			GetMipMapData( 0, 0, iMipMap, (byte**)&pPixels, &size );

			for( u32 y = 0 ; y < m_Height ; ++y )
			{
				for( u32 x = 0 ; x < m_Width ; ++x )
				{
					float* pCurPixel = pPixels + (y * m_Width + x) * numChannels;

					for( int iChannel = 0 ; iChannel < numChannels ; ++iChannel )
					{
						pCurPixel[ iChannel ] = _c[ iChannel ];
					}

				}
			}
		}
		return true;
	}

	bool Bitmap::GetBitmapFileInfo(	const char* _filename, 
									DataFormat& _format, 
									u32& _width, 
									u32& _height, 
									u32& _depth, 
									u32& _mipmaps )
	{
		if( ToLower( GetFileExtension( _filename ) ) == "bmp" )
			return GetBitmapFileInfoBMP( _filename, _format, _width, _height, _depth, _mipmaps );
		if( ToLower( GetFileExtension( _filename ) ) == "tif" )
			return GetBitmapFileInfoTIF( _filename, _format, _width, _height, _depth, _mipmaps );
		if( ToLower( GetFileExtension( _filename ) ) == "rawfp32" )
			return GetBitmapFileInfoRawFP32( _filename, _format, _width, _height, _depth, _mipmaps );

		return false;
	}


	bool Bitmap::GetBitmapGridInfo(	const Str&	_firstBitmapFilename,
									const char* _filenamePostfix,
									Str&		_filenameFormatingString,
									bool&		_bSwapXAndY,
									u32&		_tileResX,
									u32&		_tileResY,
									u32&		_minTileX, 
									u32&		_maxTileX,
									u32&		_minTileY, 
									u32&		_maxTileY )
	{
	
		_bSwapXAndY = false;

		const char* c = _filenamePostfix;
		
		Str tmp;

		while( *c != '\0' )
		{
			if( *c == '%' )
			{
				++c;
				_bSwapXAndY = (*c == 'x') || (*c == 'X');
				tmp += "%d";
			}
			else
			{
				tmp += *c;
			}

			++c;
		}
		


		Str::size_type p = _firstBitmapFilename.rfind( Format( tmp.c_str(), 0, 0 ) );
		if( p == Str::npos )
		{
			LOG_R( "file naming convention not recognized" );
			return false;
		}

		_filenameFormatingString = _firstBitmapFilename.substr( 0, p ) + tmp + '.' + GetFileExtension( _firstBitmapFilename );


		//Open the first file to see its format, dimensions etc
		//( we assume all tiles in the grid have the same format and dimensions )

		DataFormat format;
		u32 depth, mipmaps;

		if( !Bitmap::GetBitmapFileInfo( _firstBitmapFilename.c_str(), format, _tileResX, _tileResY, depth, mipmaps ) )
			return false;

		// Find the ranges for x

		_minTileX = (u32)-1;
		_maxTileX = 0;

		u32 x=0;

		while( true )
		{
			Str filename( Format( _filenameFormatingString.c_str(), _bSwapXAndY ? 0 : x, _bSwapXAndY ? x : 0 ) );
		
			if( _minTileX == (u32)-1 )
			{
				if( FileSystem::Get().FileExist( filename.c_str() ) )
				{
					_minTileX = x;
					_maxTileX = x;
				}
			}
			else
			{
				if( FileSystem::Get().FileExist( filename.c_str() ) )
				{
					_maxTileX = x;
				}
				else
				{
					break;
				}
			}

			++x;
		}

		// Find the ranges for y

		_minTileY = (u32)-1;
		_maxTileY = 0;

		u32 y=0;

		while( true )
		{
			Str filename( Format( _filenameFormatingString.c_str(), _bSwapXAndY ? y : 0, _bSwapXAndY ? 0 : y ) );
		
			if( _minTileY == (u32)-1 )
			{
				if( FileSystem::Get().FileExist( filename.c_str() ) )
				{
					_minTileY = y;
					_maxTileY = y;
				}
			}
			else
			{
				if( FileSystem::Get().FileExist( filename.c_str() ) )
				{
					_maxTileY = y;
				}
				else
				{
					break;
				}
			}

			++y;
		}
	
		return true;
	}


	
	bool Bitmap::GetBitmapFileInfoRawFP32( const char* _filename,
											DataFormat& _format,
											u32& _width,
											u32& _height,
											u32& _depth,
											u32& _mipmaps )
	{
		u64 fileSize;
		
		if( !FileSystem::Get().GetFileSize( _filename, fileSize ) )
			return false;

		if( fileSize % sizeof( float ) != 0 )
			return false;

		u32 numTexels = (u32)(fileSize / sizeof( float ));

		u32 dimension = Sqrti( numTexels ); //Assuming square texture

		if( dimension * dimension != numTexels )
			return false;

		_format = DataFormat::R32_FLOAT;
		_width = dimension;
		_height = dimension;
		_depth = 1;
		_mipmaps = 1;

		return true;
	}

	////////////////////
	//TODO move in Bitmap_TIF.cpp

	struct TIFFileHeader
	{
		char	ByteOrder[2];
		u16		MagicNumber;
		u32		FirstDirectoryOffset;
	};
	
	enum TIFTag
	{
		TIFTag_ImageWidth = 0x0100,
		TIFTag_ImageHeight = 0x0101,
		TIFTag_BitsPerSample = 0x0102,
		TIFTag_Compression = 0x0103,
		TIFTag_PhotometricInterpretation = 0x0106,
		TIFTag_StripOffsets = 0x0111,
		TIFTag_SamplesPerPixel = 0x0115,
		TIFTag_RowsPerStrip = 0x0116,
		TIFTag_StripByteCounts = 0x0117,
		TIFTag_PlanarConfiguration = 0x011c,
		//TIFTag_ = 0x0153,
		TIFTag_Force16Bits = 0XFFFF
	};
	
	struct TIFDirectoryEntry
	{
		u16 ID;
		u16 Type;
		u32 NumComponents; //(channels?)
		u32 Data;
	};
	
	
	bool Bitmap::GetBitmapFileInfoTIF(	const char* _filename, 
										DataFormat& _format, 
										u32& _width, 
										u32& _height, 
										u32& _depth, 
										u32& _mipmaps )
	{
		FileStream file;
        if( !file.Open( _filename, "rb" ) )
			return false;
		
		TIFFileHeader header;
		if( !Read( file, header ) )
			return false;
		
		if( header.MagicNumber != 42 )
			return false;

		u32 directoryOffset = header.FirstDirectoryOffset;

		//while( directoryOffset )
		{
			file.Seek( directoryOffset, IByteStream::SeekStart );
		
			u16	numDirectoryEntries;

			if( !Read( file, numDirectoryEntries ) )
				return false;
		
			u32 stripOffsetArray = 0;
			
			TIFDirectoryEntry entry;
			
			for( u16 i=0 ; i < numDirectoryEntries ; ++i )
			{
				if( !Read( file, entry ) )
					return false;
				
				//TODO
				//LOG( "entry %u: ID %u (0x%x) Type %u Channels %u DataOffset %u", 
				//	i, entry.ID, entry.ID, entry.Type, entry.NumComponents, entry.Data );
				
				switch( (TIFTag)entry.ID )
				{
					case TIFTag_ImageWidth: _width = entry.Data; break;
					case TIFTag_ImageHeight: _height = entry.Data; break;
					case TIFTag_BitsPerSample:
						if( entry.Data == 16 )
							_format = DataFormat::R16_UNORM;
						break;
						
					case TIFTag_Compression: 
						if( entry.Data != 1 )
							return false;
						break;
						
				}
			
			}
				
			_depth = 1;
			_mipmaps = 1;
			
		}
		
		return true;
	}
	
	bool Bitmap::LoadTIF( const char* _filename )
	{
		//WARNING this early code does currently ONLY support TIFF heightmaps from WorldMachine ( greyscale 16 bit )
		
		SAFE_DELETE( m_pData );
		
		m_bIsCubeMap = false;
		
		FileStream file;
        if( !file.Open( _filename, "rb" ) )
			return false;
		
		TIFFileHeader header;
		if( !Read( file, header ) )
			return false;
		
		if( header.MagicNumber != 42 )
			return false;

		u32 directoryOffset = header.FirstDirectoryOffset;

		while( directoryOffset )
		{
			file.Seek( directoryOffset, IByteStream::SeekStart );
		
			u16	numDirectoryEntries;

			if( !Read( file, numDirectoryEntries ) )
				return false;
		
			u32 stripOffsetArray = 0;
			
			TIFDirectoryEntry entry;
			
			for( u16 i=0 ; i < numDirectoryEntries ; ++i )
			{
				if( !Read( file, entry ) )
					return false;
				
				//TODO
				//LOG( "entry %u: ID %u (0x%x) Type %u Channels %u DataOffset %u", 
				//	i, entry.ID, entry.ID, entry.Type, entry.NumComponents, entry.Data );
				
				switch( (TIFTag)entry.ID )
				{
					case TIFTag_ImageWidth: m_Width = entry.Data; break;
					case TIFTag_ImageHeight: m_Height = entry.Data; break;
					case TIFTag_BitsPerSample:
						if( entry.Data == 16 )
							m_Format = DataFormat::R16_UNORM;
						break;
						
					case TIFTag_Compression: 
						if( entry.Data != 1 )
							return false;
						break;
						
					//case TIFTag_PhotometricInterpretation:
					case TIFTag_StripOffsets:
						stripOffsetArray = entry.Data;
						break;
					//case TIFTag_SamplesPerPixel:
					//case TIFTag_RowsPerStrip:
					//case TIFTag_StripByteCounts:
					//case TIFTag_PlanarConfiguration:
				}
			
			}
				
			if( !Read( file, directoryOffset ) )
				return false;
			
			m_Depth = 1;
			m_MipMapLevels = 1;
			
			u32 pixelSize = sizeof(u16); //TODO !!!
			
			m_DataSize = m_Width * m_Height * pixelSize;
			
			m_pData = (byte*)malloc( m_DataSize );
			
			if( !m_pData )
				return false;
			
			bool bFlip = false;

			for( u32 y=0 ; y < m_Height ; ++y )
			{
				file.Seek( stripOffsetArray + y * sizeof(u32), IByteStream::SeekStart );

				u32 stripOffset;
				Read( file, stripOffset );
				
				file.Seek( stripOffset, IByteStream::SeekStart );

				u32 y2 = bFlip ? m_Height - y - 1 : y;
				
				file.ReadBytes( m_pData + (y2 * m_Width) * pixelSize, m_Width * pixelSize );
				
			}
			
		}
		
		/*
		if( (header.FileType[0] != 'B') ||
		   (header.FileType[1] != 'M') )
		{
			return false;
		}
		
		BMP_BITMAP_HEADER header2;
		Read( file, header2 );
		
		*/
		
		return true;
	}

	void float2rgbe( float red, float green, float blue, u8 rgbe[4] )
	{
	  float v;
	  int e;

	  v = red;
	  if (green > v) v = green;
	  if (blue > v) v = blue;
	  if (v < 1e-32) 
	  {
		rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
	  }
	  else 
	  {
		v = frexp(v,&e) * 256.0f/v;
		rgbe[0] = (unsigned char) (red * v);
		rgbe[1] = (unsigned char) (green * v);
		rgbe[2] = (unsigned char) (blue * v);
		rgbe[3] = (unsigned char) (e + 128);
	  }
	}

	/* standard conversion from rgbe to float pixels */
	/* note: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels */
	/*       in the range [0,1] to map back into the range [0,1].            */
	void rgbe2float(float& red, float& green, float& blue, u8 rgbe[4])
	{
	  float f;

	  if (rgbe[3]) 
	  {
		/*nonzero pixel*/
		f = ldexp(1.0f,rgbe[3]-(int)(128+8));
		red = rgbe[0] * f;
		green = rgbe[1] * f;
		blue = rgbe[2] * f;
	  }
	  else
		red = green = blue = 0.0f;
	}



	bool Bitmap::SaveHDR( const char* _filename ) const
	{
		if( m_Format != DataFormat::R32G32B32_FLOAT )
			return false;

		FileStream file;
		
		if( !file.Open( _filename, "wb" ) )
			return false;

		Str header = Format( "#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y %d +X %d\n", m_Height, m_Width );
		file.WriteBytes( (const byte*)header.c_str(), header.length() );

		for( u32 y=0 ; y < m_Height ; ++y )
		{
			for( u32 x=0 ; x < m_Width ; ++x )
			{
				const Vec3& p = ((const Vec3*)m_pData)[ x + y * m_Width ];
				u8 rgbe[4];
				float2rgbe( p.x, p.y, p.z, rgbe );
				file.WriteBytes( rgbe, 4 );
			}
		}

		return true;
	}

	void Bitmap::FlipVertically()
	{
		DBG_CHECK( m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8_8 );

		rgba8_t* rgba = (rgba8_t*)m_pData;

		for( int y = 0 ; y < m_Height / 2 ; ++y )
		{
			for( int x = 0 ; x < m_Width ; ++x )
			{
				
				TSwap(	rgba[             y      * m_Width + x ],
						rgba[ (m_Height - y - 1) * m_Width + x ] );
			}

		}
	}


	Vec4 Bitmap::SumPixels() const
	{
		Vec4 sum( Vec4::Zero );

		DBG_CHECK( m_Depth == 1 );

		switch( m_Format.m_Fields.m_Layout )
		{
		case DataFormat::Layout::Layout_8_8_8:
			for( u32 y=0 ; y < m_Height ; ++y )
				for( u32 x=0 ; x < m_Width ; ++x )
				{
					u32 p = (x + y * m_Width) * 3;
					sum.x += m_pData[ p++ ];
					sum.y += m_pData[ p++ ];
					sum.z += m_pData[ p ];
				}
		
				sum /= 255.0f;
				break;

		default:
			DBG_CHECK(false);
		}
		
		return sum;
	}

	Color Bitmap::PointSample( const Vec2i & _coord, bool _bRepeat ) const
	{
		DBG_CHECK( m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8 );

		const u8* pTexels;
		u32 rowPitch;

		GetMipMapData( 0, 0, 0, &pTexels, nullptr, &rowPitch );


		s32 x = _coord.x;
		s32 y = _coord.y;

		if( _bRepeat )
		{
			x %= m_Width;
			y %= m_Height;
		}
		else
		{
			x = Clamp<s32>( x, 0, m_Width - 1 );
			y = Clamp<s32>( y, 0, m_Height - 1 );
		}

		const u32 pixelSize = 3;
		pTexels += y * rowPitch + x * pixelSize;

		return Color( pTexels[ 0 ], pTexels[ 1 ], pTexels[ 2 ], 255 );
	}


	Color Bitmap::PointSample( const Vec2 & _uv ) const
	{
		DBG_CHECK( m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8_8 );

		const u8* pTexels;
		u32 rowPitch;

		GetMipMapData( 0, 0, 0, &pTexels, nullptr, &rowPitch );

		float tu = fmodf( _uv.x * 0.99999f, 1.0f );
		float tv = fmodf( _uv.y * 0.99999f, 1.0f );

		if( tu < 0.0f )
			tu += 1.0f;

		if( tv < 0.0f )
			tv += 1.0f;

		u32 x = (u32)( tu * (float)m_Width );
		u32 y = (u32)( tv * (float)m_Height );
		DBG_CHECK( x < m_Width );
		DBG_CHECK( y < m_Height );

		const u32 pixelSize = 4;
		pTexels += y * rowPitch + x * pixelSize;

		return Color( pTexels[0], pTexels[ 1 ], pTexels[ 2 ], pTexels[ 3 ] );
	}

	Color Bitmap::BilinearSample( const Vec2 & _uv ) const
	{


		DBG_CHECK( (m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8_8 )
				|| (m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_32) );

		const u8* pTexels;
		u32 rowPitch;

		GetMipMapData( 0, 0, 0, &pTexels, nullptr, &rowPitch );
		
		float tu = _uv.x * (float)( m_Width - 1 );
		float tv = _uv.y * (float)( m_Height - 1 );


		u32 x1 = (u32)tu;
		u32 y1 = (u32)tv;
		u32 x2 = x1 + 1;
		u32 y2 = y1 + 1;

		float fu = tu - (float)x1;
		float fv = tv - (float)y1;

		//wrap
		x1 %= m_Width;
		y1 %= m_Height;
		x2 %= m_Width;
		y2 %= m_Height;

		Color t1, t2, t3, t4;

		if( m_Format.m_Fields.m_Layout == DataFormat::Layout::Layout_32 )
		{
			const u32 pixelSize = 4;

			const float* pTexel1 = (float*)pTexels + y1 * m_Width + x1;
			const float* pTexel2 = (float*)pTexels + y1 * m_Width + x2;
			const float* pTexel3 = (float*)pTexels + y2 * m_Width + x1;
			const float* pTexel4 = (float*)pTexels + y2 * m_Width + x2;

			t1 = Color( pTexel1[ 0 ], pTexel1[ 0 ], pTexel1[ 0 ], pTexel1[ 0 ] );
			t2 = Color( pTexel2[ 0 ], pTexel2[ 0 ], pTexel2[ 0 ], pTexel2[ 0 ] );
			t3 = Color( pTexel3[ 0 ], pTexel3[ 0 ], pTexel3[ 0 ], pTexel3[ 0 ] );
			t4 = Color( pTexel4[ 0 ], pTexel4[ 0 ], pTexel4[ 0 ], pTexel4[ 0 ] );
		}
		else
		{

			const u32 pixelSize = 4;

			const u8* pTexel1 = pTexels + y1 * rowPitch + x1 * pixelSize;
			const u8* pTexel2 = pTexels + y1 * rowPitch + x2 * pixelSize;
			const u8* pTexel3 = pTexels + y2 * rowPitch + x1 * pixelSize;
			const u8* pTexel4 = pTexels + y2 * rowPitch + x2 * pixelSize;

			t1 = Color( pTexel1[ 0 ], pTexel1[ 1 ], pTexel1[ 2 ], pTexel1[ 3 ] );
			t2 = Color( pTexel2[ 0 ], pTexel2[ 1 ], pTexel2[ 2 ], pTexel2[ 3 ] );
			t3 = Color( pTexel3[ 0 ], pTexel3[ 1 ], pTexel3[ 2 ], pTexel3[ 3 ] );
			t4 = Color( pTexel4[ 0 ], pTexel4[ 1 ], pTexel4[ 2 ], pTexel4[ 3 ] );
		}

		return Lerp( Lerp( t1, t2, fu ), Lerp( t3, t4, fu ), fv );
	}

