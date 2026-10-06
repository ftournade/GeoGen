// CurveEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoGen.h"
#include "CurveEditorDlg.h"
#include "afxdialogex.h"

#include "CurveNode.h"

static const LPCTSTR CurveEditorWndClass = _T( "CurveEditorCtrl" );

#define WM_CURVE_SET_CORNER			(WM_USER + 423)
#define WM_CURVE_SET_BEZIER_CORNER	(WM_USER + 424)
#define WM_CURVE_SET_BEZIER_SMOOTH	(WM_USER + 425)
#define WM_CURVE_DEL_KEY			(WM_USER + 430)

BEGIN_MESSAGE_MAP( CurveEditorControl, CWnd )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND( WM_CURVE_SET_CORNER       , OnSetKeyTypeCorner )
	ON_COMMAND( WM_CURVE_SET_BEZIER_CORNER, OnSetKeyTypeBezierCorner )
	ON_COMMAND( WM_CURVE_SET_BEZIER_SMOOTH, OnSetKeyTypeBezierSmooth )
	ON_COMMAND( WM_CURVE_DEL_KEY, OnDeleteKey )
END_MESSAGE_MAP()

#define HANDLE_SIZE 7

CurveEditorControl::CurveEditorControl() : m_DraggedKeyframe(-1), m_RightClickedKeyframe(-1)
{
	//TEST
	m_Curve.AddKey( 0.0f, Vec2( 0.0f, 0.5f ) );
	m_Curve.AddKey( 0.2f, Vec2( 0.2f, 0.1f ) );
	m_Curve.AddKey( 0.4f, Vec2( 0.4f, 0.8f ) );
	m_Curve.AddKey( 0.7f, Vec2( 0.7f, 0.3f ) );
	m_Curve.AddKey( 1.0f, Vec2( 1.0f, 1.0f ) );

	Keyframe<Vec2>* kf = &m_Curve.GetKeys()[ 1 ];
	kf->LeftTangent = kf->Key + Vec2( -0.25f, 0.0f );
	kf->RightTangent = kf->Key + Vec2(  0.25f, 0.0f );
	kf->KeyType = BezierSmooth;

	RegisterWindowClass();
}

CurveEditorControl::~CurveEditorControl() 
{
}


BOOL CurveEditorControl::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if( !(::GetClassInfo( hInst, CurveEditorWndClass, &wndcls )) )
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
		wndcls.lpszClassName = CurveEditorWndClass;

		if( !AfxRegisterClass( &wndcls ) )
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}

inline CPoint TransformCurveToScreen( const CRect& _r, float _x, float _y )
{
	return CPoint( _r.left + (LONG)(_x * (float)_r.Width()), _r.bottom - (LONG)(_y * (float)_r.Height()) );
}

inline Vec2 TransformScreenToCurve( const CRect& _r, const CPoint& _p )
{
	return Vec2( (float)(_p.x - _r.left) / (float)_r.Width(), 1.0f - (float)(_p.y - _r.top) / (float)_r.Height() );
}


void CurveEditorControl::OnPaint()
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

	pMemDC->FillSolidRect( &r, RGB( 70, 70, 70 ) );

	CPen whitePen;
	whitePen.CreatePen( PS_SOLID, 2, RGB( 255, 255, 255 ) );

	pMemDC->SelectObject( &whitePen );

	Vec2 curvePt = m_Curve.GetValue( 0.0f );

	pMemDC->MoveTo( TransformCurveToScreen( r, curvePt.x, curvePt.y ) );

	int curveResolution = 1000;

	for( int i = 0 ; i < curveResolution ; ++i )
	{
		float t = (float)(i + 1) / (float)curveResolution;
		curvePt = m_Curve.GetValue( t );

		pMemDC->LineTo( TransformCurveToScreen( r, curvePt.x, curvePt.y ) );
	}

	uint32_t n = m_Curve.GetKeys().size();

	for( int i = 0; i < n ; ++i )
	{
		const Keyframe<Vec2>& key = m_Curve.GetKeys()[ i ];
	
		CPoint p = TransformCurveToScreen( r, key.Key.x, key.Key.y );
		CPoint leftTgt = TransformCurveToScreen( r, key.LeftTangent.x, key.LeftTangent.y );
		CPoint rightTgt = TransformCurveToScreen( r, key.RightTangent.x, key.RightTangent.y );
		
		CRect keyRect( p, p );
		keyRect.InflateRect( HANDLE_SIZE, HANDLE_SIZE );
		pMemDC->FillSolidRect( &keyRect, RGB( 255, 255, 255 ) );

		if( key.KeyType != Corner )
		{
			pMemDC->MoveTo( leftTgt );
			pMemDC->LineTo( p );
			pMemDC->LineTo( rightTgt );

			keyRect = CRect( leftTgt, leftTgt );
			keyRect.InflateRect( HANDLE_SIZE, HANDLE_SIZE );
			pMemDC->FillSolidRect( &keyRect, RGB( 127, 127, 127 ) );

			keyRect = CRect( rightTgt, rightTgt );
			keyRect.InflateRect( HANDLE_SIZE, HANDLE_SIZE );
			pMemDC->FillSolidRect( &keyRect, RGB( 127, 127, 127 ) );
		}
	}

	m_backBuffer.Blit( dc.GetSafeHdc() );
}


void CurveEditorControl::OnLButtonDown( UINT nFlags, CPoint point )
{
	CRect r;
	GetClientRect( &r );

	m_DraggedKeyframe = -1;

	uint32_t n = m_Curve.GetKeys().size();

	for( int i = 0; i < n; ++i )
	{
		const Keyframe<Vec2>& key = m_Curve.GetKeys()[ i ];

		CPoint p = TransformCurveToScreen( r, key.Key.x, key.Key.y );

		//Test control point
		CRect keyRect( p, p );
		keyRect.InflateRect( HANDLE_SIZE, HANDLE_SIZE );

		if( keyRect.PtInRect( point ) )
		{
			m_DraggedKeyframe = i;
			m_DraggedHandle = 0;
		}

		if( key.KeyType != Corner )
		{
			//Test left tangent handle
			p = TransformCurveToScreen( r, key.LeftTangent.x, key.LeftTangent.y );
			keyRect = CRect( p, p );
			keyRect.InflateRect( HANDLE_SIZE, HANDLE_SIZE );

			if( keyRect.PtInRect( point ) )
			{
				m_DraggedKeyframe = i;
				m_DraggedHandle = -1;
			}

			//Test right tangent handle
			p = TransformCurveToScreen( r, key.RightTangent.x, key.RightTangent.y );
			keyRect = CRect( p, p );
			keyRect.InflateRect( HANDLE_SIZE, HANDLE_SIZE );

			if( keyRect.PtInRect( point ) )
			{
				m_DraggedKeyframe = i;
				m_DraggedHandle = 1;
			}
		}

		if( m_DraggedKeyframe != -1 )
			break;
	}

	if( m_DraggedKeyframe == -1 )
	{
		Vec2 p = TransformScreenToCurve( r, point );
		m_DraggedKeyframe = m_Curve.AddKey( p.x, p );
		m_DraggedHandle = 0;
		Invalidate( FALSE );
	}

	SetCapture();
}


void CurveEditorControl::OnLButtonUp( UINT nFlags, CPoint point )
{
	m_DraggedKeyframe = -1;

	ReleaseCapture();
}

void CurveEditorControl::OnRButtonDown( UINT nFlags, CPoint point )
{

	
}

void CurveEditorControl::OnRButtonUp( UINT nFlags, CPoint point )
{
	m_RightClickedKeyframe = -1;

	CRect r;
	GetClientRect( &r );

	uint32_t n = m_Curve.GetKeys().size();
	int i;

	for( i = 0; i < n; ++i )
	{
		const Keyframe<Vec2>& key = m_Curve.GetKeys()[ i ];

		CPoint p = TransformCurveToScreen( r, key.Key.x, key.Key.y );

		//Test control point
		CRect keyRect( p, p );
		keyRect.InflateRect( HANDLE_SIZE, HANDLE_SIZE );

		if( keyRect.PtInRect( point ) )
		{
			m_RightClickedKeyframe = i;
			break;
		}
	}

	if( m_RightClickedKeyframe == -1 )
		return;

	CMenu popupMenu;
	popupMenu.CreatePopupMenu();
	popupMenu.AppendMenu( MF_STRING, WM_CURVE_SET_CORNER, _T( "Corner" ) );
	popupMenu.AppendMenu( MF_STRING, WM_CURVE_SET_BEZIER_CORNER, _T( "Bezier Corner" ) );
	popupMenu.AppendMenu( MF_STRING, WM_CURVE_SET_BEZIER_SMOOTH, _T( "Bezier Smooth" ) );
	popupMenu.AppendMenu( MF_SEPARATOR );
	popupMenu.AppendMenu( MF_STRING, WM_CURVE_DEL_KEY, _T( "Delete" ) );

	popupMenu.CheckMenuRadioItem( 0, 2, m_Curve.GetKeys()[ i ].KeyType, MF_BYPOSITION );

	ClientToScreen( &point );

	//SetForegroundWindow();

	popupMenu.TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, this );

	//PostMessage( WM_NULL ); //https://msdn.microsoft.com/en-us/library/windows/desktop/ms648002(v=vs.85).aspx

}

void CurveEditorControl::OnMouseMove( UINT nFlags, CPoint point )
{
	if( m_DraggedKeyframe == -1 )
		return;

	CRect r;
	GetClientRect( &r );

	Keyframe<Vec2>& key = m_Curve.GetKeys()[ m_DraggedKeyframe ];
	Vec2 p = TransformScreenToCurve( r, point );

	//TODO confine to neighbooring keys

	float minX, maxX;

	if( m_DraggedKeyframe == 0 )
	{
		minX = 0.0f;
		maxX = 0.0f;
	}
	else if( m_DraggedKeyframe == m_Curve.GetKeys().size() - 1 )
	{
		minX = 1.0f;
		maxX = 1.0f;
	}
	else
	{
		minX = m_Curve.GetKeys()[ m_DraggedKeyframe - 1 ].Key.x + 0.0001f;
		maxX = m_Curve.GetKeys()[ m_DraggedKeyframe + 1 ].Key.x - 0.0001f;
	}

	p.x = min( p.x, maxX );
	p.x = max( p.x, minX );

	switch( m_DraggedHandle )
	{
		case 0:
		{
			Vec2 movement = p - key.Key;
			key.Time = p.x;
			key.Key = p;
			key.LeftTangent += movement;
			key.RightTangent += movement;
			break;
		}
		case -1:
		{
			key.LeftTangent = p;
			key.LeftTangent.x = Min( key.LeftTangent.x, key.Key.x );

			if( key.KeyType == BezierSmooth )
			{
				key.RightTangent = key.Key * 2.0f - key.LeftTangent;
			}
			break;
		}
		case 1:
		{
			key.RightTangent = p;
			key.RightTangent.x = Max( key.RightTangent.x, key.Key.x );

			if( key.KeyType == BezierSmooth )
			{
				key.LeftTangent = key.Key * 2.0f - key.RightTangent;
			}
			break;
		}
	}

	OnCurveModified();
}

void CurveEditorControl::OnSetKeyTypeCorner()
{
	Keyframe<Vec2>& kf = m_Curve.GetKeys()[ m_RightClickedKeyframe ];
	
	kf.LeftTangent = kf.Key;
	kf.RightTangent = kf.Key;
	kf.KeyType = Corner;

	OnCurveModified();
}

void CurveEditorControl::OnSetKeyTypeBezierCorner()
{
	m_Curve.GetKeys()[ m_RightClickedKeyframe ].KeyType = BezierCorner;

	OnCurveModified();
}

void CurveEditorControl::OnSetKeyTypeBezierSmooth()
{
	Keyframe<Vec2>& kf = m_Curve.GetKeys()[ m_RightClickedKeyframe ];

	kf.KeyType = BezierSmooth;
	kf.RightTangent = kf.Key * 2.0f - kf.LeftTangent;

	OnCurveModified();
}

void CurveEditorControl::OnDeleteKey()
{
	if( (m_RightClickedKeyframe == 0) || (m_RightClickedKeyframe == m_Curve.GetKeys().size()) )
		return;

	m_Curve.DelKey( m_RightClickedKeyframe );

	OnCurveModified();
}

void CurveEditorControl::OnCurveModified()
{

	//GetParent()->
	//SendNotifyMessage( CURVE_NOTIFICATION, 0, 0 );
	((CurveEditorDlg*)GetParent())->OnCurveModified( nullptr, nullptr ); //hack around MFC ...
	Invalidate( FALSE );

}


// CurveEditorDlg dialog

IMPLEMENT_DYNAMIC(CurveEditorDlg, CDialogEx)

CurveEditorDlg::CurveEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_CURVE_EDITOR, pParent)
{

}

CurveEditorDlg::~CurveEditorDlg()
{
}

void CurveEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_CURVE_EDITOR_CTRL, m_CurveControl );
}


BEGIN_MESSAGE_MAP(CurveEditorDlg, CDialogEx)
END_MESSAGE_MAP()


// CurveEditorDlg message handlers
void CurveEditorDlg::OnCurveModified( NMHDR*, LRESULT* )
{
	//	shared_ptr< ColorGradientNode > pNode = m_pComputeNode.lock();
	CurveNode* pNode = m_pComputeNode;

	if( !pNode )
		return ;

	pNode->m_Curve = m_CurveControl.m_Curve;
	pNode->OnCurveChanged();
	pNode->SetDirty();
	theApp.RedrawPreview();
}