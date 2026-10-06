#pragma once

#include "BasicTypes.h"
#include "TList.h"
#include "Str.h"

#ifdef XTM_WIN32
	HINSTANCE GetAppInstance();
	void SetAppInstance( HINSTANCE hInst );

	HWND GetMainWindow();
	void SetMainWindow( HWND hWnd );

	struct CREATESIMPLEMESSAGEBOX
	{
		//In
		LPCSTR						Text;
		LPCSTR						Title;
		TList< Str >				Buttons;
		
		//Out
		s32							ButtonClicked;
		
		//Internal use
		bool						EndModalLoop;
		HFONT						Font;
	};

#else

	struct CREATESIMPLEMESSAGEBOX
	{
		//In
		Str						Text;
		Str						Title;
		TList< Str >			Buttons;
		
		//Out
		s32						ButtonClicked;
		
		//Internal use
		bool					EndModalLoop;
		//HFONT					Font;
	};
	
#endif
	

	bool CreateSimpleMessageBox( CREATESIMPLEMESSAGEBOX& csmb, s32 width, s32 height );

