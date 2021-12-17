#include "stdafx.h"
#include "BoolProperty.h"

#include "resource.h"

CBoolProperty::CBoolProperty( const CString& _strName, BOOL _value, LPCTSTR lpszDescr, DWORD_PTR dwData )
	: CMFCPropertyGridProperty( _strName, (variant_t)_value, lpszDescr, dwData )
{
	CBitmap bmp;
	if( bmp.LoadBitmap( IDB_ONOFF ) )
	{
		m_images.Create( 16, 16, ILC_COLOR24, 0, 0 );
		m_images.Add( &bmp, RGB( 255, 0, 255 ) );
	}
}


CBoolProperty::~CBoolProperty()
{
}

CWnd* CBoolProperty::CreateInPlaceEdit( CRect rectEdit, BOOL& bDefaultFormat )
{
	//This MFC property grid is total crap, I had to hack my way around ...

	m_varValue.boolVal = !m_varValue.boolVal;
	Redraw();
	m_pWndList->OnPropertyChanged( this );
	return NULL;
}

/*
BOOL CBoolProperty::OnClickValue( UINT uiMsg, CPoint point )
{
	m_varValue.boolVal = !m_varValue.boolVal;
	Redraw();
	OnValueChanged();
	return TRUE;
}
*/

void CBoolProperty::OnDrawValue( CDC* pDC, CRect rect )
{
	m_images.Draw( pDC, m_varValue.boolVal ? 0 : 1, CPoint( rect.left + 3, rect.top + 3 ), ILD_NORMAL );
}
