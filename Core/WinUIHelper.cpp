//#include "stdafx.h"
#include "UIHelper.h"

#include "StandardLib.h"

#define SIMPLE_MESSAGE_BOX_BASE_BUTTON_INDEX 100

namespace xtm
{

	static HINSTANCE g_hAppInstance = NULL;

	HINSTANCE GetAppInstance()
	{
		return g_hAppInstance;
	}

	void SetAppInstance( HINSTANCE hInst )
	{
		g_hAppInstance = hInst;
	}

	//*********************

	static HWND g_hMainWindow = NULL;

	HWND GetMainWindow()
	{
		return g_hMainWindow;
	}

	void SetMainWindow( HWND hWnd )
	{
		g_hMainWindow = hWnd;
	}

	//*********************

	BOOL CreateSimpleMessageBoxButtons( HWND hWnd, SIZE size, const CREATESIMPLEMESSAGEBOX* pWndParam )
	{
		const s32 spacing = 10;
		const s32 buttonHeight = 40;

		s32 numButtons = (s32)pWndParam->Buttons.size();

		s32 buttonWidth = ( ( size.cx - spacing ) / numButtons ) - spacing;

		s32 x = spacing;

		s32 y = size.cy - ( buttonHeight + spacing );


		s32 idButton = SIMPLE_MESSAGE_BOX_BASE_BUTTON_INDEX;

		TList< Str >::const_iterator it;

		for( it=pWndParam->Buttons.begin() ; it != pWndParam->Buttons.end() ; ++it )
		{
			LPCSTR buttonText = (*it).c_str();

			HWND hButton = CreateWindowA("button",
										buttonText,
										WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
										x, y, buttonWidth, buttonHeight,
										hWnd,
										(HMENU)(s64)idButton,
										GetAppInstance(),
										NULL );
        
			//SendMessage( hButton, WM_SETFONT, (WPARAM)param->font,TRUE);
			
			if( !hButton )
				return FALSE;

			++idButton;

			x += buttonWidth + spacing;

		}

		return TRUE;
	}

	LRESULT CALLBACK SimpleMessageBoxWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{ 
		const s32 frameBorder = 15;

		static CREATESIMPLEMESSAGEBOX* pWndParam;

		switch( message )
		{
			case WM_CREATE:
			{
				pWndParam = (CREATESIMPLEMESSAGEBOX*)((LPCREATESTRUCT)lParam)->lpCreateParams;

				NONCLIENTMETRICSA ncm;
				ncm.cbSize = sizeof(NONCLIENTMETRICSA);

				SystemParametersInfoA( SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSA), &ncm, 0 );

				pWndParam->Font = CreateFontIndirectA( &ncm.lfMenuFont );


				RECT rect;
				GetClientRect( hWnd, &rect );

				SIZE size;
				size.cx = rect.right - rect.left;
				size.cy = rect.bottom - rect.top;

				CreateSimpleMessageBoxButtons( hWnd, size, pWndParam );
				

				GetWindowRect( hWnd, &rect );

				size.cx = rect.right - rect.left;
				size.cy = rect.bottom - rect.top;

				MoveWindow( hWnd,
							GetSystemMetrics(SM_CXSCREEN)/2 - size.cx/2,
							GetSystemMetrics(SM_CYSCREEN)/2 - size.cy/2, 
							size.cx,size.cy,
							TRUE );
				break;
			}

			case WM_PAINT:
			{
				RECT rect;
				GetClientRect( hWnd, &rect );

				PAINTSTRUCT	ps;

				HDC hdc = BeginPaint( hWnd, &ps );

				SelectObject( hdc, pWndParam->Font);

				SetBkColor( hdc, GetSysColor(COLOR_BTNFACE) );

				rect.left += frameBorder;
				rect.top  += frameBorder;
				rect.bottom  -= frameBorder + 60;
				rect.right -= frameBorder;

				DrawTextA( hdc, pWndParam->Text, (s16)Xstrlen( pWndParam->Text ), &rect, DT_LEFT );

				EndPaint( hWnd, &ps );

				return 0;
			}

			case WM_KEYDOWN:
			{			
				if (wParam == VK_RETURN)
				{
					/*
					if (param->style & 0xf)
					{
					param->ret = param->style & 0xf;
					param->end = true;
					DestroyWindow(hWnd);
					}
					*/
				}
				break;
			}

			case WM_COMMAND:
			{
				pWndParam->ButtonClicked = LOWORD(wParam) - SIMPLE_MESSAGE_BOX_BASE_BUTTON_INDEX;
				DestroyWindow( hWnd );
				break;
			}

			case WM_DESTROY:
			{
				pWndParam->EndModalLoop = true;
				DeleteObject( pWndParam->Font );
				break;
			}

		}

		return DefWindowProcA( hWnd, message, wParam, lParam );
	}


	bool CreateSimpleMessageBox( CREATESIMPLEMESSAGEBOX& csmb, s32 width, s32 height )
	{
		//Register window class

		WNDCLASSA wc;

		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)SimpleMessageBoxWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= ::GetModuleHandle(NULL);
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= "SimpleMessageBox";

		RegisterClassA(&wc);

		HWND hWnd = CreateWindowA(	"SimpleMessageBox", 
									csmb.Title, 
									WS_OVERLAPPED | WS_POPUPWINDOW | WS_CLIPCHILDREN |
									WS_DLGFRAME | DS_3DLOOK | DS_SETFONT | DS_MODALFRAME,
									0, 0, width, height, 
									GetMainWindow(),
									NULL,
									GetAppInstance(),
									&csmb );

		if( !hWnd )
			return FALSE;


		ShowWindow( hWnd, SW_SHOW );
		UpdateWindow( hWnd );
		ReleaseCapture();
		SetFocus( hWnd );


		//Enter modal loop

		MSG msg;

		csmb.EndModalLoop = false;

		while( !csmb.EndModalLoop && GetMessage(&msg, hWnd, 0, 0) ) 
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		return true;
	}

}