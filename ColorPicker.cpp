#include "stdafx.h"
#include "ColorPicker.h"


static const LPCTSTR ColorPickerWndClass = _T( "ColorPickerCtrl" );

#define COLOR_PICKER_CHANGED 69310


BEGIN_MESSAGE_MAP( ColorPickerControl, CWnd )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

ColorPickerControl::ColorPickerControl() : m_Color( Color::White ), m_LeftButtonDownArea(-2)
{
	RegisterWindowClass();
}


ColorPickerControl::~ColorPickerControl()
{
}

BOOL ColorPickerControl::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if( !(::GetClassInfo( hInst, ColorPickerWndClass, &wndcls )) )
	{
		// otherwise we need to register a new class
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
		wndcls.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = ColorPickerWndClass;

		if( !AfxRegisterClass( &wndcls ) )
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}

BOOL ColorPickerControl::UpdateLayout( int w, int h )
{
	CRect r;
	GetClientRect( &r );

	const float HueSatPickerRatio = 2.0f / 3.0f;
	const float separatorToSliderRatio = 0.5f;

	m_HueSatPickerRect = CRect( 0, 0, r.Width(), (int)((float)r.Height() * HueSatPickerRatio) );

	float ratio = 6.0f + 5.0f * separatorToSliderRatio;

	int sliderWidth    = (float)r.Width() / ratio;
	int separatorWidth = (float)r.Width() * separatorToSliderRatio / ratio;
	
	int slidersTop = m_HueSatPickerRect.bottom + 30;
	int slidersBottom = r.Height();

	for( int i = 0 ; i < 6 ; ++i )
	{
		//RGB HSV

		m_SliderRect[ i ].left = (sliderWidth + separatorWidth) * i;
		m_SliderRect[ i ].right = m_SliderRect[ i ].left + sliderWidth;
		m_SliderRect[ i ].top = slidersTop;
		m_SliderRect[ i ].bottom = slidersBottom;
	}

	return TRUE;
}


void ColorPickerControl::OnPaint()
{
	CPaintDC dc( this );

	CRect r;
	GetClientRect( &r );

	if( (r.Width() != m_backBuffer.GetWidth()) ||
		(r.Height() != m_backBuffer.GetHeight()) )
	{
		if( !m_backBuffer.Init( GetSafeHwnd(), r.Width(), r.Height() ) )
		{
			//TODO error handling
			ASSERT( false );
		}
	}

	CDC* pMemDC = CDC::FromHandle( m_backBuffer.GetBackBufferDC() );

	UpdateLayout( r.Width(), r.Height() ); //TODO OnSize is never called (???), hence we update layout each frame ...

	pMemDC->FillSolidRect( &r, RGB( 230, 230, 230 ) );//TODO GetSysColor( COLOR_BACKGROUND ) );
	
	float HSV[ 3 ];
	m_Color.ToHSV( HSV[ 0 ], HSV[ 1 ], HSV[ 2 ] );

	//Draw HS box

	for( int y = m_HueSatPickerRect.top ; y < m_HueSatPickerRect.bottom ; ++y )
	{
		float hue = (float)(y - m_HueSatPickerRect.top) / (float)m_HueSatPickerRect.Height();

		for( int x = m_HueSatPickerRect.left ; x < m_HueSatPickerRect.bottom ; ++x )
		{
			float saturation = (float)(x - m_HueSatPickerRect.left) / (float)m_HueSatPickerRect.Width();

			Color c;
			c.FromHSV( hue, saturation, HSV[2] );

			pMemDC->SetPixel( x, y, c.ToWin32COLORREF() );
		}
	}

	pMemDC->Draw3dRect( &m_HueSatPickerRect, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ) );
	
	int hsvDotX = (int)Lerp<float>( m_HueSatPickerRect.left, m_HueSatPickerRect.right, HSV[ 1 ] );
	int hsvDotY = (int)Lerp<float>( m_HueSatPickerRect.top, m_HueSatPickerRect.bottom, HSV[ 0 ] );

	const int dotSize = 4;

	pMemDC->Draw3dRect( &CRect( hsvDotX - dotSize, hsvDotY - dotSize, hsvDotX + dotSize, hsvDotY + dotSize ), RGB( 255, 255, 255 ), RGB( 0, 0, 0 ) );

	//Draw sliders
		
	pMemDC->SetBkMode( TRANSPARENT );

	const float* pData[] = { &m_Color.r,  &m_Color.g,  &m_Color.b, &HSV[0], &HSV[1], &HSV[2] };

	LPCTSTR RGBHSV[6] =
	{
		_T( "R" ), _T( "G" ), _T( "B" ), _T( "H" ), _T( "S" ), _T( "V" ),
	};

	Color slidersColor[6] = 
	{
		Color::Red, Color::Green, Color::Blue, Color::Purple, Color::Cyan, Color::Grey
	};

	for( int i = 0 ; i < 6 ; ++i )
	{
		pMemDC->TextOut( m_SliderRect[i].left, m_SliderRect[i].top - 20, RGBHSV[i], 1 );

		if( i < 3 )
		{
			//RGB
			DrawSlider( pMemDC, m_SliderRect[ i ], slidersColor[ i ].ToWin32COLORREF(), m_Color[i] );
		}
		else
		{
			//HSV
			float HSV[ 3 ];
			m_Color.ToHSV( HSV[ 0 ], HSV[ 1 ], HSV[ 2 ] );

			DrawSlider( pMemDC, m_SliderRect[ i ], slidersColor[ i ].ToWin32COLORREF(), HSV[ i - 3 ] );
		}
	}

	m_backBuffer.Blit( dc.GetSafeHdc() );
}

void ColorPickerControl::DrawSlider( CDC* _pDC, const CRect& _r, COLORREF _c, float _v )
{
	CRect r( _r );
	r.top = (int)Lerp<float>( r.bottom, r.top, Saturate( _v ) );

	_pDC->FillSolidRect( &r, _c );

	_pDC->Draw3dRect( &_r, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ) );

}


Vec2 PointUV( const CRect& r, CPoint p )
{
	return Vec2(	Saturate( (float)(p.x - r.left) / (float)r.Width() ),
					Saturate( (float)(p.y - r.top) / (float)r.Height() ) );
}

int ColorPickerControl::MouseHitTest( const CPoint& p ) //-1 huesat picker - 2 if not found, slider index otherwise
{
	if( m_HueSatPickerRect.PtInRect( p ) )
		return -1;

	for( int i = 0 ; i < 6 ; ++i )
	{

		if( m_SliderRect[ i ].PtInRect( p ) )
			return i;
	}

	return -2;
}



void ColorPickerControl::OnLButtonDown( UINT nFlags, CPoint point )
{
	int hit = MouseHitTest( point );
	m_LeftButtonDownArea = hit;

	float HSV[ 3 ];
	m_Color.ToHSV( HSV[ 0 ], HSV[ 1 ], HSV[ 2 ] );

	if( m_LeftButtonDownArea > -2 )
	{
		SetCapture();
	}

	switch( hit )
	{
	case -2: return;
		case -1: //HueSat
		{
			Vec2 uv = PointUV( m_HueSatPickerRect, point );
			HSV[ 0 ] = uv.y;
			HSV[ 1 ] = uv.x;
			m_Color.FromHSV( HSV[ 0 ], HSV[ 1 ], HSV[ 2 ] );
			break;
		}
			//RGB sliders
		case 0:
		case 1:
		case 2:
			m_Color[hit] = 1.0f - PointUV( m_SliderRect[ hit ], point ).y;
			break;

			//HSV sliders			
		case 3:
		case 4:
		case 5: 
		{
			HSV[ hit - 3 ] = 1.0f - PointUV( m_SliderRect[ hit ], point ).y;
			m_Color.FromHSV( HSV[ 0 ], HSV[ 1 ], HSV[ 2 ] );
			break;
		}

	}


	OnColorModified();

}

void ColorPickerControl::OnLButtonUp( UINT nFlags, CPoint point )
{
	ReleaseCapture();
	m_LeftButtonDownArea = -2;
}

void ColorPickerControl::OnRButtonDown( UINT nFlags, CPoint point )
{

}

void ColorPickerControl::OnRButtonUp( UINT nFlags, CPoint point )
{

}

void ColorPickerControl::OnMouseMove( UINT nFlags, CPoint point )
{
	if( m_LeftButtonDownArea == -2 )
		return;

	float HSV[ 3 ];
	m_Color.ToHSV( HSV[ 0 ], HSV[ 1 ], HSV[ 2 ] );

	switch( m_LeftButtonDownArea )
	{
		case -1:
		{
			Vec2 uv = PointUV( m_HueSatPickerRect, point );

			HSV[ 0 ] = uv.y;
			HSV[ 1 ] = uv.x;
			m_Color.FromHSV( HSV[ 0 ], HSV[ 1 ], HSV[ 2 ] );
			break;
		}
			//RGB sliders
		case 0:
		case 1:
		case 2:
			m_Color[ m_LeftButtonDownArea ] = 1.0f - PointUV( m_SliderRect[ m_LeftButtonDownArea ], point ).y;
			break;

			//HSV sliders			
		case 3:
		case 4:
		case 5:
		{
			HSV[ m_LeftButtonDownArea - 3 ] = 1.0f - PointUV( m_SliderRect[ m_LeftButtonDownArea ], point ).y;
			m_Color.FromHSV( HSV[ 0 ], HSV[ 1 ], HSV[ 2 ] );
			break;
		}

	}

	OnColorModified();
}

void ColorPickerControl::OnColorModified()
{
	//GetParent()->
	//SendNotifyMessage( COLOR_RAMP_NOTIFICATION, 0, 0 );
	//((ColorGradientDlg*)GetParent())->OnColorGradientModified( nullptr, nullptr ); //hack around MFC ...
	GetParent()->SendMessage( WM_COLORPICKER_CHANGED );
	Invalidate( FALSE );
}
