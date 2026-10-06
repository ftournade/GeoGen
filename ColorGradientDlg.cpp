// ColorGradientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoGen.h"
#include "ColorGradientDlg.h"
#include "afxdialogex.h"

static const LPCTSTR ColorGradientWndClass = _T( "ColorGradientCtrl" );
const int KeyframeUISize = 7;

#define COLOR_RAMP_NOTIFICATION 69280

BEGIN_MESSAGE_MAP( ColorGradientWnd, CWnd )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE( WM_COLORPICKER_CHANGED, OnColorPickerChanged )
END_MESSAGE_MAP()

ColorGradientWnd::ColorGradientWnd() : m_SelectedKeyFrame(-1), m_bDraggingKey(false)
{
	RegisterWindowClass();
}

ColorGradientWnd::~ColorGradientWnd()
{

}

BOOL ColorGradientWnd::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if( !(::GetClassInfo( hInst, ColorGradientWndClass, &wndcls )) )
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
		wndcls.lpszClassName = ColorGradientWndClass;

		if( !AfxRegisterClass( &wndcls ) )
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}


void ColorGradientWnd::PreSubclassWindow()
{
	if( !m_wndColorPicker.Create( nullptr, _T( "Color Picker" ), WS_CHILD | WS_VISIBLE, CRect( 0, 0, 100, 100 ), this, 636 ) )
	{
		assert( false );
		return;
	}	

	UpdateLayout();

	CWnd::PreSubclassWindow();
}


BOOL ColorGradientWnd::UpdateLayout()
{
	CRect r;
	GetClientRect( &r );

	const int separator = 15;
	int rampWidth = (r.Width() - separator) * 1 / 3;
	int colorPickerWidth = (r.Width() - separator) * 2 / 3;

	m_ClearRect = CRect( CPoint( 0, 0 ), CSize( rampWidth + separator, r.Height() ) );

	m_RampRect = CRect( CPoint( 0, 0 ), CSize( rampWidth, r.Height() ) );
	m_ColorPickerRect = m_RampRect;
	m_ColorPickerRect.left = m_RampRect.right + separator;
	m_ColorPickerRect.right = r.Width();

	m_wndColorPicker.SetWindowPos( NULL, m_ColorPickerRect.left, m_ColorPickerRect.top, m_ColorPickerRect.Width(), m_ColorPickerRect.Height(), SWP_NOACTIVATE | SWP_NOZORDER );

	return TRUE;
}

LRESULT ColorGradientWnd::OnColorPickerChanged( WPARAM wParam, LPARAM lParam )
{
	if( (m_SelectedKeyFrame < 0) || (m_SelectedKeyFrame >= m_ColorGradient.GetKeys().size()) )
		return TRUE;

	Keyframe<Color>& key = m_ColorGradient.GetKeys()[ m_SelectedKeyFrame ];
	key.Key = m_wndColorPicker.m_Color;
	key.LeftTangent = m_wndColorPicker.m_Color;
	key.RightTangent = m_wndColorPicker.m_Color;

	OnColorGradientModified();

	return TRUE;
}

void ColorGradientWnd::OnPaint()
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


	UpdateLayout(); //TODO OnSize is never called (???), hence we update layout each frame ...
	
	pMemDC->FillSolidRect( &m_ClearRect, RGB( 230, 230, 230 ) );//TODO GetSysColor( COLOR_BACKGROUND ) );

	//Draw gradient

	for( int y = 0 ; y < m_RampRect.Height() ; ++y )
	{
		float t = (float)y / (float)(m_RampRect.Height() - 1);

		Color c = m_ColorGradient.GetValue( t );
		COLORREF C = c.ToWin32COLORREF();

		//TODO Draw lines with ever changing CPen cheaper ?
		for( int x = 0; x < m_RampRect.Width() ; ++x )
		{
			pMemDC->SetPixel( x + m_RampRect.left, m_RampRect.bottom - y - 1, C );
		}
	}

	pMemDC->Draw3dRect( &m_RampRect, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ) );


	//Draw keyframe boxes

	int kfX1 = m_RampRect.CenterPoint().x - KeyframeUISize;
	int kfX2 = m_RampRect.CenterPoint().x + KeyframeUISize;

	uint32_t iKey = 0;

	for( const Keyframe<Color>& kf : m_ColorGradient.GetKeys() )
	{
		int yKF = (int)Lerp<float>( m_RampRect.bottom, m_RampRect.top, kf.Time );

		CRect kfRect( kfX1, yKF - KeyframeUISize, kfX2, yKF + KeyframeUISize );

		pMemDC->FillSolidRect( &kfRect, kf.Key.ToWin32COLORREF() );

		if( iKey == m_SelectedKeyFrame )
			pMemDC->Draw3dRect( &kfRect, RGB(255, 0, 0), RGB(127, 0, 0) );
		else
			pMemDC->Draw3dRect( &kfRect, RGB(255,255,255), RGB(0,0,0) );

		++iKey;

	}

	m_backBuffer.Blit( dc.GetSafeHdc() );
}


int ColorGradientWnd::MouseHitTestKeyframe( const CPoint& p ) //-1 if not found, keyframe index otherwise
{
	if( !m_RampRect.PtInRect( p ) )
		return -1;

	int kfX1 = m_RampRect.CenterPoint().x - KeyframeUISize;
	int kfX2 = m_RampRect.CenterPoint().x + KeyframeUISize;

	uint32_t i = 0;

	for( const Keyframe<Color>& kf : m_ColorGradient.GetKeys() )
	{
		int yKF = (int)Lerp<float>( m_RampRect.bottom, m_RampRect.top, kf.Time );

		CRect kfRect( kfX1, yKF - KeyframeUISize, kfX2, yKF + KeyframeUISize );
	
		if( kfRect.PtInRect( p ) )
			return i;

		++i;
	}

	return  -1;
}

void ColorGradientWnd::OnLButtonDown( UINT nFlags, CPoint point )
{
	
	if( m_RampRect.PtInRect( point ) )
	{
		int kfHitTest = MouseHitTestKeyframe( point );

		if( kfHitTest >= 0 )
		{
			m_SelectedKeyFrame = kfHitTest;

			m_wndColorPicker.m_Color = m_ColorGradient.GetKeys()[ m_SelectedKeyFrame ].Key;
		}
		else
		{
			Vec2 uv = PointUV( m_RampRect, point );
			float t = 1.0f - uv.y;
			m_wndColorPicker.m_Color = m_ColorGradient.GetValue( t );

			m_SelectedKeyFrame = m_ColorGradient.AddKey( t, m_wndColorPicker.m_Color );

			assert( m_SelectedKeyFrame != -1 );
		}

		m_bDraggingKey = true;
		SetCapture();
		m_wndColorPicker.Invalidate( FALSE );

		OnColorGradientModified();
	}
}

void ColorGradientWnd::OnLButtonUp( UINT nFlags, CPoint point )
{
	m_bDraggingKey = false;
	ReleaseCapture();
}

void ColorGradientWnd::OnRButtonDown( UINT nFlags, CPoint point )
{
	if( m_RampRect.PtInRect( point ) )
	{
		int kfHitTest = MouseHitTestKeyframe( point );

		if( kfHitTest >= 0 )
		{
			m_ColorGradient.DelKey( kfHitTest );
			m_SelectedKeyFrame = -1;
			OnColorGradientModified();
		}
	}
}

void ColorGradientWnd::OnRButtonUp( UINT nFlags, CPoint point )
{

}

void ColorGradientWnd::OnMouseMove( UINT nFlags, CPoint point )
{
	
	if( m_bDraggingKey )
	{

		Vec2 uv = PointUV( m_RampRect, point );

		float t = 1.0f - uv.y;

		Color c = m_ColorGradient.GetKeys()[ m_SelectedKeyFrame ].Key;
		m_ColorGradient.GetKeys()[ m_SelectedKeyFrame ].Time = t;
		m_ColorGradient.SortKeys();

		//if keys changed order, m_SelectedKeyFrame is invalid

		m_SelectedKeyFrame = -1;

		uint32_t i = 0;
		for( const Keyframe<Color>& key : m_ColorGradient.GetKeys() )
		{
			if( ( key.Time == t ) && ( key.Key == c ) )
			{
				m_SelectedKeyFrame = i;
			}
			++i;
		}

		if( m_SelectedKeyFrame < 0 )
			m_bDraggingKey = false; //Weird ....

		OnColorGradientModified();
	}
}

void ColorGradientWnd::OnColorGradientModified()
{
	//GetParent()->
	//SendNotifyMessage( COLOR_RAMP_NOTIFICATION, 0, 0 );
	((ColorGradientDlg*)GetParent())->OnColorGradientModified( nullptr, nullptr ); //hack around MFC ...

	Invalidate( FALSE );
}

// ColorGradientDlg dialog //////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(ColorGradientDlg, CDialogEx)



ColorGradientDlg::ColorGradientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_COLOR_RAMP, pParent),
	m_pComputeNode(nullptr)

{

}

ColorGradientDlg::~ColorGradientDlg()
{
}

void ColorGradientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COLOR_RAMP_CTRL, m_ColorGradientCtrl );
}


BEGIN_MESSAGE_MAP(ColorGradientDlg, CDialogEx)
	ON_NOTIFY( COLOR_RAMP_NOTIFICATION, IDC_COLOR_RAMP_CTRL, OnColorGradientModified )
END_MESSAGE_MAP()

// ColorGradientDlg message handlers


BOOL ColorGradientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void ColorGradientDlg::OnColorGradientModified( NMHDR*, LRESULT* )
{
	//	shared_ptr< ColorGradientNode > pNode = m_pComputeNode.lock();
	ColorGradientNode* pNode = m_pComputeNode;

	if( !pNode )
		return ;

	pNode->m_ColorGradient = m_ColorGradientCtrl.m_ColorGradient;
	pNode->OnColorGradientChanged();
	pNode->SetDirty();
	theApp.RedrawPreview();
}


