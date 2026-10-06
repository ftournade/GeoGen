//GeoGen: replaces xtm's Bitmap_PNG.cpp, which depends on LightZPNG (not part of this repo)
#include "Bitmap.h"

#include <Core/Log.h>


	bool Bitmap::LoadPNG( const char* _filename )
	{
		LOG_R( "Bitmap::LoadPNG - PNG loading is not supported (%s)", _filename );
		return false;
	}

