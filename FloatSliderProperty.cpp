#include "stdafx.h"

#include "FloatSliderProperty.h"
#include "Resource.h"

#include "CustomSlider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CPropSliderCtrl : public CCustomSlider
{
public:
	CPropSliderCtrl( CFloatSliderProperty* pProp, COLORREF clr, float _min = 0.0f, float _max = 1.0f, int _numDecimal = 3 )
		: CCustomSlider( clr, _min, _max, _numDecimal ), m_pProp( pProp ) {}
	virtual ~CPropSliderCtrl() {}

protected:
	virtual void OnValueChanged();

protected:
	CFloatSliderProperty* m_pProp;
};

void CPropSliderCtrl::OnValueChanged()
{
	ASSERT_VALID( m_pProp );

	m_pProp->OnUpdateValue();
	m_pProp->Redraw();
}

/////////////////////////////////////////////////


CFloatSliderProperty::CFloatSliderProperty( const CString& _strName,
											LPCTSTR _lpszDescr,
											float _value,
											float _min,
											float _max,
											int _numDecimals,
											DWORD_PTR _dwData,
											COLORREF _clr ) :
	CMFCPropertyGridProperty( _strName, _value, _lpszDescr, _dwData ),
	m_Min( _min ),
	m_Max( _max ),
	m_NumDecimals( _numDecimals ),
	m_clr( _clr )
{

}

CWnd* CFloatSliderProperty::CreateInPlaceEdit( CRect rectEdit, BOOL& bDefaultFormat )
{
	CPropSliderCtrl* pWndSlider = new CPropSliderCtrl( this, m_clr, m_Min, m_Max, m_NumDecimals );

	rectEdit.DeflateRect( 2, 2 );
	pWndSlider->Create( _T( "STATIC" ), _T( "" ), WS_VISIBLE | WS_CHILD, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE );
	pWndSlider->SetValue( m_varValue.fltVal );

	bDefaultFormat = TRUE;
	return pWndSlider;
}

BOOL CFloatSliderProperty::OnUpdateValue()
{
	ASSERT_VALID( this );
	ASSERT_VALID( m_pWndInPlace );
	ASSERT_VALID( m_pWndList );
	ASSERT( ::IsWindow( m_pWndInPlace->GetSafeHwnd() ) );

	float lCurrValue = m_varValue.fltVal;

	CCustomSlider* pSlider = (CCustomSlider*)m_pWndInPlace;

	m_varValue = pSlider->GetValue();

	if( lCurrValue != m_varValue.fltVal ) {
		m_pWndList->OnPropertyChanged( this );
	}

	return TRUE;
}

void CFloatSliderProperty::OnDrawValue( CDC* pDC, CRect rect )
{
	//Mimics CCustomSlider::OnPaint

	if( m_pWndInPlace )
		return;
	
	rect.DeflateRect( 2, 2 );
	CRect r = rect;

	float n = (m_varValue.fltVal - m_Min) / (m_Max - m_Min);
	LONG split = r.left + (LONG)((float)r.Width() * n);

	r.right = split;
	pDC->FillSolidRect( &r, m_clr );

	r = rect;
	r.left = split;
	pDC->FillSolidRect( &r, RGB( 255, 255, 255 ) );

	//Draw text

	CString formatStr, str;
	formatStr.Format( _T( "%%.%df" ), m_NumDecimals );
	str.Format( formatStr.GetString(), m_varValue.fltVal );

	r = rect;

	CFont font;
	if( !font.CreatePointFont( 80, _T( "Arial" ), pDC ) )
		return;

	CFont* pOldFont = pDC->SelectObject( &font );
	//	pDC->SetTextColor( RGB( 0, 0, 0 ) );
	pDC->SetBkMode( TRANSPARENT );
	pDC->DrawText( str, r, DT_CENTER );

	pDC->SelectObject( pOldFont );

}

