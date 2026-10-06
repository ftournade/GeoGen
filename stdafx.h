
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


#include <d3d11.h>

// xtm/Core subset (Core/)
#include <Core/xtm_prelude.h>
#include <Core/Bitmap.h>
#include <Core/Camera.h>
#include <Core/Color.h>
#include <Core/Curve.h>
#include <Core/Debug.h>
#include <Core/float16.h>
#include <Core/Keyframer.h>
#include <Core/Log.h>
#include <Core/Mat33.h>
#include <Core/Str.h>
#include <Core/TVector.h>
#include <Core/Vec2.h>
#include <Core/Vec2i.h>
#include <Core/Win32BackBuffer.h>

#include <SimpleD3DFramework/Renderer.h>

#include "UIRect.h"

#include <cassert>
#include <memory>
#include <vector>
#include <string>

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

#include <TinyXml2/tinyxml2.h>
