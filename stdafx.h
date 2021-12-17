
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


#include <Core/stdafx.h>
#include <Core/Vec2.h>

//#include <Windows.h>
//#include <Windowsx.h>

#include <memory>

template <typename T>
typename std::vector< std::weak_ptr<T> >::iterator Find( std::vector< std::weak_ptr<T> >& _vector, const std::shared_ptr<T>& _value )
{
	std::vector< std::weak_ptr<T> >::iterator it;

	for( it = _vector.begin() ; it != _vector.end() ; ++it )
	{
		if( it->lock() == _value )
			break;
	}

	return it;
}

template <typename T>
bool Remove( std::vector< T >& _vector, const T& _value )
{
	std::vector< T >::iterator it;

	bool bRemoved = false;

	for( it = _vector.begin() ; it != _vector.end() ; ++it )
	{
		if( *it == _value )
		{
			bRemoved = true;
			it = _vector.erase( it );

			if( it == _vector.end() )
				break;
		}
	}

	return bRemoved;
}

struct UIRect
{
	xtm::Vec2 Pos, Size;

	bool PointInRect( const xtm::Vec2& p ) const
	{
		return (p.x >= Pos.x) && (p.y >= Pos.y) && (p.x <= Pos.x + Size.x) && (p.y <= Pos.y + Size.y);
	}

	xtm::Vec2 Center() const
	{
		return Pos + Size * 0.5f;
	}

	UIRect operator*( float f ) const
	{
		UIRect r;
		r.Pos = Pos * f;
		r.Size = Size * f;

		return r;
	}

	UIRect operator/( float f ) const
	{
		UIRect r;
		r.Pos = Pos / f;
		r.Size = Size / f;

		return r;
	}

	UIRect operator+( const xtm::Vec2& v ) const
	{
		UIRect r;
		r.Pos = Pos + v;
		r.Size = Size;

		return r;
	}

	UIRect operator-( const xtm::Vec2& v ) const
	{
		UIRect r;
		r.Pos = Pos - v;
		r.Size = Size;

		return r;
	}
};

#include <TinyXml2/tinyxml2.h>
