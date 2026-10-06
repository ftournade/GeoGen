//GeoGen: replaces xtm's Bitmap_PNG.cpp (LightZPNG) and libjpeg-based LoadJPG with stb_image
#include "Bitmap.h"

#include <Core/Log.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG

#pragma warning( push, 0 )
#include <stb_image.h>
#pragma warning( pop )


	bool Bitmap::LoadWithSTBImage( const char* _filename )
	{
		int width, height, numChannelsInFile;

		//Always decode to RGBA8 (no gamma/color management), so encoded data such as terrarium heights stays exact
		stbi_uc* pPixels = stbi_load( _filename, &width, &height, &numChannelsInFile, 4 );

		if( !pPixels )
		{
			LOG_R( "Bitmap: failed to load %s (%s)", _filename, stbi_failure_reason() );
			return false;
		}

		SAFE_DELETE_ARRAY( m_pData )

		m_Width = width;
		m_Height = height;
		m_Depth = 1;
		m_ArraySize = 1;
		m_MipMapLevels = 1;
		m_Format = DataFormat::R8G8B8A8_UNORM;
		m_bIsCubeMap = false;

		m_DataSize = width * height * 4;
		m_pData = new byte[ m_DataSize ];
		memcpy( m_pData, pPixels, m_DataSize );

		stbi_image_free( pPixels );

		return true;
	}

	bool Bitmap::LoadPNG( const char* _filename )
	{
		return LoadWithSTBImage( _filename );
	}

	bool Bitmap::LoadJPG( const char* _filename )
	{
		return LoadWithSTBImage( _filename );
	}
