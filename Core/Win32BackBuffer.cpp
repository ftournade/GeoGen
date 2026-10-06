#include "Win32BackBuffer.h"

#include <Core/Bitmap.h>

namespace xtm
{

	Win32BackBuffer::Win32BackBuffer() :
		m_hBitmap(NULL),
		m_pPixels(NULL),
		m_Width(0),
		m_Height(0)
	{
	}

	Win32BackBuffer::~Win32BackBuffer()
	{
		Release();
	}

	bool Win32BackBuffer::Init( HWND _hWnd, int _width, int _height )
	{
		Release();

		HDC hdc = GetDC(_hWnd);
		BITMAPINFO bi;
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = _width;
		bi.bmiHeader.biHeight = -_height; //negative to tell windows that (0,0) is top left
		bi.bmiHeader.biPlanes = 1; 
		bi.bmiHeader.biBitCount = 24; 
		bi.bmiHeader.biCompression = BI_RGB;
		bi.bmiHeader.biSizeImage = 0; 
		bi.bmiHeader.biXPelsPerMeter = 0; 
		bi.bmiHeader.biYPelsPerMeter = 0; 
		bi.bmiHeader.biClrUsed = 0; 
		bi.bmiHeader.biClrImportant = 0;

		m_hBitmap = CreateDIBSection( hdc, &bi, DIB_RGB_COLORS, &m_pPixels, NULL, NULL );

		if( !m_hBitmap || !m_pPixels )
			return false;

		m_hDC = CreateCompatibleDC( hdc );

		if( !m_hDC )
			return false;

		SelectObject( m_hDC, m_hBitmap );

		m_Width = _width;
		m_Height = _height;

		m_Pitch = ((_width * 24 + 31) & ~31) >> 3;

		//u32 padCount = NeededPaddingToAlign( _width * 3, 4 );

		return true;
	}

	void Win32BackBuffer::Release()
	{
		DeleteObject( m_hBitmap );
		DeleteObject( m_hDC );
		m_hBitmap = NULL;
		m_hDC = NULL;
		m_pPixels = NULL;
		m_Width = 0;
		m_Height = 0;
	}



	bool Win32BackBuffer::Blit( HDC hdc )
	{
		if( !BitBlt( hdc, 0, 0, m_Width, m_Height, m_hDC, 0, 0, SRCCOPY ) )
			return false;

		GdiFlush();

		return true;
	}
	
	bool Win32BackBuffer::CopyToBitmap( Bitmap& _bitmap ) const
	{
		if( (_bitmap.GetWidth() != m_Width) || (_bitmap.GetHeight() != m_Height) || (_bitmap.GetFormat() != DataFormat::R8G8B8A8_UNORM_SRGB) )
		{
			if( !_bitmap.Create( m_Width, m_Height, 1, 1, DataFormat::R8G8B8A8_UNORM_SRGB ) )
			{	
				DBG_CHECK( false );
				return false;
			}
		}

		u8* pDstPixels;
		u32 dstRowPitch;
		_bitmap.GetMipMapData( 0, 0, 0, &pDstPixels, NULL, &dstRowPitch );

		for( u32 y=0 ; y < (u32)m_Height ; ++y )
		{
			for( u32 x=0 ; x < (u32)m_Width ; ++x )
			{
				const u8* pSrcPixel = (const u8*)m_pPixels + (y * m_Width + x) * 3;
				u8* pDstPixel = pDstPixels + (m_Height - y - 1) * dstRowPitch + x * 4;
				
				pDstPixel[0] = pSrcPixel[2];
				pDstPixel[1] = pSrcPixel[1];
				pDstPixel[2] = pSrcPixel[0];
				pDstPixel[3] = 255;
			}
		}

		return true;
	}
	
	bool Win32BackBuffer::CopyFromBitmap( const Bitmap& _bitmap )
	{
		if( (_bitmap.GetWidth() != m_Width) || (_bitmap.GetHeight() != m_Height) || (_bitmap.GetFormat() != DataFormat::R8G8B8A8_UNORM_SRGB) )
		{
			DBG_CHECK( false );
			return false;
		}

		const u8* pSrcPixels;
		u32 srcRowPitch;
		_bitmap.GetMipMapData( 0, 0, 0, &pSrcPixels, NULL, &srcRowPitch );

		for( u32 y=0 ; y < (u32)m_Height ; ++y )
		{
			for( u32 x=0 ; x < (u32)m_Width ; ++x )
			{
				u8* pDstPixel = (u8*)m_pPixels + (y * m_Width + x) * 3;
				const u8* pSrcPixel = pSrcPixels + (m_Height - y - 1) * srcRowPitch + x * 4;
				
				pDstPixel[0] = pSrcPixel[2];
				pDstPixel[1] = pSrcPixel[1];
				pDstPixel[2] = pSrcPixel[0];
			}
		}

		return true;
	}


}
