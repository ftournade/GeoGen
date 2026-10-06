//#include "stdafx.h"
#include "Bitmap.h"

#include <Core/FileStream.h>

//TODO strictly implement spec


	#pragma pack( push, 1 )

	struct BMP_FILE_HEADER
	{
		char	FileType[2];     /* File type, always 4D42h ("BM") */
		u32		FileSize;     /* Size of the file in bytes */
		u16		Reserved1;    /* Always 0 */
		u16		Reserved2;    /* Always 0 */
		u32		BitmapOffset; /* Starting position of image data in bytes */
	};
	
	struct BMP_BITMAP_HEADER_V2
	{
		u32 Size;            /* Size of this header in bytes */
		s16 Width;           /* Image width in pixels */
		s16 Height;          /* Image height in pixels */
		u16 Planes;          /* Number of color planes */
		u16 BitsPerPixel;    /* Number of bits per pixel */
	};
	
	struct BMP_BITMAP_HEADER_V3
	{
		u32 Size;            /* Size of this header in bytes */
		s32 Width;           /* Image width in pixels */
		s32 Height;          /* Image height in pixels */
		u16 Planes;          /* Number of color planes */
		u16 BitsPerPixel;    /* Number of bits per pixel */

		u32 Compression;     /* Compression methods used */
		u32 SizeOfBitmap;    /* Size of bitmap in bytes */
		s32 HorzResolution;  /* Horizontal resolution in pixels per meter */
		s32 VertResolution;  /* Vertical resolution in pixels per meter */
		u32 ColorsUsed;      /* Number of colors in the image */
		u32 ColorsImportant; /* Minimum number of important colors */
	};

	struct BMP_CIEXYZ
	{
		s32 x, y, z;
	};

	struct BMP_BITMAP_HEADER_V4 : public BMP_BITMAP_HEADER_V3
	{
		u32 RedMask;
		u32 GreenMask;
		u32 BlueMask;
		u32 AlphaMask;
		u32 CSType;
		s32 RedX;          /* X coordinate of red endpoint */
		s32 RedY;          /* Y coordinate of red endpoint */
		s32 RedZ;          /* Z coordinate of red endpoint */
		s32 GreenX;        /* X coordinate of green endpoint */
		s32 GreenY;        /* Y coordinate of green endpoint */
		s32 GreenZ;        /* Z coordinate of green endpoint */
		s32 BlueX;         /* X coordinate of blue endpoint */
		s32 BlueY;         /* Y coordinate of blue endpoint */
		s32 BlueZ;         /* Z coordinate of blue endpoint */
		u32 GammaRed;      /* Gamma red coordinate scale value */
		u32 GammaGreen;    /* Gamma green coordinate scale value */
		u32 GammaBlue;     /* Gamma blue coordinate scale value */
	};

	struct BMP_BITMAP_HEADER_V5 : public BMP_BITMAP_HEADER_V4
	{
		u32 Intent;
		u32 ProfileData;
		u32 ProfileSize;
		u32 Reserved;
	};

	#pragma pack( pop )


	bool Bitmap::GetBitmapFileInfoBMP( const char* _filename, DataFormat& _format, u32& _width, u32& _height, u32& _depth, u32& _mipmaps )
	{
		FileStream file;
        if( !file.Open( _filename, "rb" ) )
			return false;

		BMP_FILE_HEADER header;
		Read( file, header );

		if( (header.FileType[0] != 'B') ||
			(header.FileType[1] != 'M') )
		{
			return false;
		}

		BMP_BITMAP_HEADER_V3 header2;
		Read( file, header2 );
		
		_width = header2.Width;
		_height = header2.Height;
		_depth = 1;
		_mipmaps = 1;

		switch( header2.BitsPerPixel )
		{
			case 8: _format = DataFormat::R8_UNORM; break; //TODO warning can't it be paletized ?
			case 24: _format = DataFormat::R8G8B8_UNORM; break;
			case 32: _format = DataFormat::R8G8B8A8_UNORM; break;
			default: return false;
		}

		return true;
	}

	bool Bitmap::LoadBMP( const char* _filename )
	{
		SAFE_DELETE( m_pData );
		
		m_bIsCubeMap = false;

		FileStream file;
        if( !file.Open( _filename, "rb" ) )
			return false;

		BMP_FILE_HEADER header;
		Read( file, header );

		if( (header.FileType[0] != 'B') ||
			(header.FileType[1] != 'M') )
		{
			return false;
		}

		u32 bitmapHeaderSize;
		Read( file, bitmapHeaderSize );
		file.Seek( -4, FileStream::SeekCur );

		bool topDown = true;
		bool bSwapRedAndBlue = false;
		u32 bpp;

		switch( bitmapHeaderSize )
		{
			case sizeof(BMP_BITMAP_HEADER_V2) :
			{
				BMP_BITMAP_HEADER_V2 header2;
				file.ReadBytes((byte*)&header2, bitmapHeaderSize);

				m_Width = header2.Width;
				m_Height = (u32)Abs( (s32)header2.Height );
				topDown = header2.Height < 0;
				bpp = header2.BitsPerPixel;
				break;
			}
			case sizeof(BMP_BITMAP_HEADER_V3):
			{
				BMP_BITMAP_HEADER_V3 header2;
				file.ReadBytes((byte*)&header2, bitmapHeaderSize);

				m_Width = header2.Width;
				m_Height = (u32)Abs((s32)header2.Height);
				topDown = header2.Height < 0;
				bpp = header2.BitsPerPixel;
				if( bpp == 24 || bpp == 32 )
					bSwapRedAndBlue = true;
				//TODO what about 32 bits
				break;
			}
			case sizeof(BMP_BITMAP_HEADER_V4):
			{
				BMP_BITMAP_HEADER_V4 header2;
				file.ReadBytes((byte*)&header2, bitmapHeaderSize);

				m_Width = header2.Width;
				m_Height = (u32)Abs((s32)header2.Height);
				topDown = header2.Height < 0;
				bpp = header2.BitsPerPixel;

				bSwapRedAndBlue = header2.RedMask != 0x00FF0000; //TODO more sophisticated channel mask handling
				break;
			}
			case sizeof(BMP_BITMAP_HEADER_V5) :
			{
				BMP_BITMAP_HEADER_V5 header2;
				file.ReadBytes( (byte*)&header2, bitmapHeaderSize );

				m_Width = header2.Width;
				m_Height = (u32)Abs( (s32)header2.Height );
				topDown = header2.Height < 0;
				bpp = header2.BitsPerPixel;

				bSwapRedAndBlue = header2.RedMask != 0x00FF0000; //TODO more sophisticated channel mask handling
				break;
			}
			default:
				return false;
		}


		file.Seek( header.BitmapOffset, IByteStream::SeekStart );
		
		u32 numTexels = m_Width * m_Height;

		m_Depth = 1;

		m_MipMapLevels = 1;
		
		byte* pCurPixel;
		u32 texelSize = bpp / 8;

		m_DataSize = numTexels * texelSize;

		m_pData = (byte*)malloc( m_DataSize );

		u32 padCount = NeededPaddingToAlign( m_Width * texelSize, 4 );

		if( topDown )
			pCurPixel = m_pData;
		else
			pCurPixel = m_pData + ((m_Height - 1) * m_Width) * texelSize;


		if( bpp == 32 )
		{
			m_Format = DataFormat::R8G8B8A8_UNORM;
			

			for( u32 i = 0 ; i < m_Height ; ++i )
			{
				file.ReadBytes( pCurPixel, m_Width * texelSize );
				
				if( topDown )
					pCurPixel += m_Width * texelSize;
				else
					pCurPixel -= m_Width * texelSize;

				file.Seek( padCount, IByteStream::SeekCur );
			}

			if( bSwapRedAndBlue )
			{
				for( u32 i = 0; i < numTexels; ++i )
				{
					//BGRA
					u32 p = i * texelSize;

					u8 tmp = m_pData[p];
					m_pData[p] = m_pData[p + 2];
					m_pData[p + 2] = tmp;
				}
			}
		}
		else if( bpp == 24 )
		{
			m_Format = DataFormat::R8G8B8_UNORM;
			
			for( u32 i = 0 ; i < m_Height ; ++i )
			{
				file.ReadBytes( (byte*)pCurPixel, m_Width * texelSize );

				if( topDown )
					pCurPixel += m_Width * texelSize;
				else
					pCurPixel -= m_Width * texelSize;

				file.Seek( padCount, IByteStream::SeekCur );
			}

			if( bSwapRedAndBlue )
			{
				for( u32 i = 0; i < numTexels; ++i )
				{
					u32 p = i * texelSize;

					u8 tmp = m_pData[p];
					m_pData[p] = m_pData[p + 2];
					m_pData[p + 2] = tmp;
				}
			}
		}
		else if( bpp == 8 )
		{
			m_Format = DataFormat::R8_UNORM;
			
			for( u32 i = 0 ; i < m_Height ; ++i )
			{
				file.ReadBytes( (byte*)pCurPixel, m_Width * texelSize );

				if( topDown )
					pCurPixel += m_Width * texelSize;
				else
					pCurPixel -= m_Width * texelSize;

				file.Seek( padCount, IByteStream::SeekCur );
			}
			
			
		}
		
		return true;
	}

	bool Bitmap::SaveBMP( const char* _filename ) const
	{
		switch( m_Format.m_Fields.m_Layout )
		{
			case DataFormat::Layout::Layout_8_8_8:
			case DataFormat::Layout::Layout_8_8_8_8:
				break;
				
			default: return false;
		}
		
		u32 texelSize = m_Format.GetSizeInBytes();
		
		FileStream file;
		if( !file.Open( _filename, "wb" ) )
			return false;
		
		u32 padCount = NeededPaddingToAlign( m_Width * texelSize, 4 );

		BMP_FILE_HEADER header;
		BMP_BITMAP_HEADER_V3 header2;
		ZeroMemory( &header, sizeof(header) );
		ZeroMemory( &header2, sizeof(header2) );
		header.FileType[0] = 'B';
		header.FileType[1] = 'M';
		header.BitmapOffset = sizeof(BMP_FILE_HEADER) + sizeof(BMP_BITMAP_HEADER_V3);

		header2.Size = sizeof(BMP_BITMAP_HEADER_V3);
		header2.Width = m_Width;
		header2.Height = m_Height;
		header2.Planes = 1;
		header2.BitsPerPixel = (u16)(texelSize * 8);
		header2.Compression = 0;
		header2.SizeOfBitmap = m_Height * (m_Width * texelSize + padCount);

		header.FileSize = header.BitmapOffset + header2.SizeOfBitmap;

		Write( file, header );
		Write( file, header2 );

		u32 pad=0;

		byte* pTmpPixelRow = xtmNew byte[ m_Width * texelSize ];

		switch( m_Format.m_Fields.m_Layout )
		{
			//TODO case DF_R8:
			case DataFormat::Layout::Layout_8_8_8:
			{
				for( s32 y=m_Height - 1 ; y >= 0  ; --y )
				{
					const byte* pixelRow = m_pData + y * m_Width * texelSize;
					
					for( u32 x=0 ; x < m_Width ; ++x )
					{
						pTmpPixelRow[ x * texelSize     ] = pixelRow[ x * texelSize + 2 ];
						pTmpPixelRow[ x * texelSize + 1 ] = pixelRow[ x * texelSize + 1 ];
						pTmpPixelRow[ x * texelSize + 2 ] = pixelRow[ x * texelSize     ];
					}
					
					file.WriteBytes( pTmpPixelRow, m_Width * texelSize );
					file.WriteBytes( (const byte*)&pad, padCount );
				}
				break;
			}
			case DataFormat::Layout::Layout_8_8_8_8:
			{
				for( s32 y=m_Height - 1 ; y >= 0  ; --y )
				{
					const byte* pixelRow = m_pData + y * m_Width * texelSize;
					
					for( u32 x=0 ; x < m_Width ; ++x )
					{
						//TODO untested

						pTmpPixelRow[ x * texelSize     ] = pixelRow[ x * texelSize + 2 ];
						pTmpPixelRow[ x * texelSize + 1 ] = pixelRow[ x * texelSize + 1 ];
						pTmpPixelRow[ x * texelSize + 2 ] = pixelRow[ x * texelSize     ];
						pTmpPixelRow[ x * texelSize + 3 ] = pixelRow[ x * texelSize + 3 ];
					}
					
					file.WriteBytes( pTmpPixelRow, m_Width * texelSize );
					file.WriteBytes( (const byte*)&pad, padCount );
				}
				break;
			}

		}

		delete[] pTmpPixelRow;

		return true;
	}

