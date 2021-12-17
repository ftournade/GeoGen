#include "stdafx.h"

#include "CustomSlider.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP( CCustomSlider, CWnd )
	//{{AFX_MSG_MAP(CCustomSlider)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CCustomSlider::CCustomSlider( COLORREF clr, float _min, float _max, int _numDecimals ) :
	m_NumDecimals( _numDecimals ),
	m_clr( clr ),
	m_Min( _min ),
	m_Max( _max ),
	m_Value( _min ),
	m_bMouseButtonDown( FALSE )
{
}

CCustomSlider::~CCustomSlider() {
}

void CCustomSlider::OnPaint()
{
	CPaintDC dc( this );

	CRect r;
	GetClientRect( &r );

	float n = ( m_Value - m_Min ) / (m_Max - m_Min);
	LONG split = r.left + (LONG)((float)r.Width() * n);

	r.right = split;
	dc.FillSolidRect( &r, m_clr );

	GetClientRect( &r );
	r.left = split;
	dc.FillSolidRect( &r, RGB( 255, 255, 255 ) );

	//Draw text

	CString formatStr, str;
	formatStr.Format( _T("%%.%df"), m_NumDecimals );
	str.Format( formatStr.GetString(), GetValue() );

	GetClientRect( &r );

	CFont font;
	if( !font.CreatePointFont( 80, _T( "Arial" ), &dc ) )
		return;

	CFont* pOldFont = dc.SelectObject( &font );
//	dc.SetTextColor( RGB( 0, 0, 0 ) );
	dc.SetBkMode( TRANSPARENT );
	dc.DrawText( str, r, DT_CENTER );

	dc.SelectObject( pOldFont );
}

void CCustomSlider::OnMouseMove( UINT nFlags, CPoint point )
{
	if( !m_bMouseButtonDown )
		return;

	ChangeSliderValue( point );

	Invalidate(FALSE);
}

void CCustomSlider::OnLButtonDown( UINT nFlags, CPoint point )
{
	m_bMouseButtonDown = TRUE;
	SetCapture();

	ChangeSliderValue( point );

	Invalidate( FALSE );
}

void CCustomSlider::OnLButtonUp( UINT nFlags, CPoint point )
{
	m_bMouseButtonDown = FALSE;
	ReleaseCapture();
}

void CCustomSlider::SetValue( float v )
{
	m_Value = v;
	m_Value = min( m_Value, m_Max );
	m_Value = max( m_Value, m_Min );

	Invalidate(FALSE);
}

void CCustomSlider::ChangeSliderValue( const CPoint& point )
{
	CRect r;
	GetClientRect( &r );

	float v = (float)(point.x - r.left) / (float)r.Width();
	
	m_Value = m_Min + v * (m_Max - m_Min);
	m_Value = min( m_Value, m_Max );
	m_Value = max( m_Value, m_Min );

	OnValueChanged();
/*
//Fucking Win32 madness that doesn't work, instead we do a good old fashioned virtual call

#if 1
	//GetParent()->
	SendNotifyMessage( NOTIFY_CUSTOM_SLIDER_CHANGED, m_Value, 0 );
#else
	NMHDR nmh;
	nmh.code = NOTIFY_CUSTOM_SLIDER_CHANGED;
	nmh.idFrom = GetDlgCtrlID();
	nmh.hwndFrom = GetSafeHwnd();

	SendMessage( WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh );
#endif
*/
}

