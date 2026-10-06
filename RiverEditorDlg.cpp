// RiverEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoGen.h"
#include "RiverEditorDlg.h"
#include "afxdialogex.h"

#include "MountainNode.h"

static const LPCTSTR RiverEditorWndClass = _T( "RiverEditorCtrl" );

#define WM_RIVER_DEL_KEY			(WM_USER + 530)

BEGIN_MESSAGE_MAP( RiverEditorControl, CWnd )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND( WM_RIVER_DEL_KEY, OnDeleteKey )
END_MESSAGE_MAP()

#define HANDLE_SIZE 7

RiverEditorControl::RiverEditorControl()
{

	RegisterWindowClass();
}

RiverEditorControl::~RiverEditorControl() 
{
}


BOOL RiverEditorControl::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if( !(::GetClassInfo( hInst, RiverEditorWndClass, &wndcls )) )
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
		wndcls.lpszClassName = RiverEditorWndClass;

		if( !AfxRegisterClass( &wndcls ) )
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}

inline CPoint TransformRiverToScreen( const CRect& _r, const Vec2& _p )
{
	return CPoint( _r.left + (LONG)(_p.x * (float)_r.Width()), _r.bottom - (LONG)(_p.y * (float)_r.Height()) );
}

inline Vec2 TransformScreenToRiver( const CRect& _r, const CPoint& _p )
{
	return Vec2( (float)(_p.x - _r.left) / (float)_r.Width(), 1.0f - (float)(_p.y - _r.top) / (float)_r.Height() );
}

void RiverEditorControl::RecDraw( CDC* _pDC, const CRect& _rect, const shared_ptr< RiverNode >& _node )
{
	shared_ptr< RiverNode > selectedNode( m_pSelectedRiverNode.lock() );


	for( int i = 0 ; i < _node->Childs.size() ; ++i )
	{
		CPoint A = TransformRiverToScreen( _rect, _node->Pos );
		CPoint B = TransformRiverToScreen( _rect, _node->Childs[i]->Pos );

		_pDC->MoveTo( A );
		_pDC->LineTo( B );

		CRect r( B, B );
		r.InflateRect( HANDLE_SIZE, HANDLE_SIZE );

		if( selectedNode == _node->Childs[i] )
			_pDC->FillSolidRect( &r, RGB( 255, 0, 0 ) );
		else
			_pDC->FillSolidRect( &r, RGB( 127, 127, 127 ) );
		


		RecDraw( _pDC, _rect, _node->Childs[i] );
	}
}

void RiverEditorControl::OnPaint()
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

	if( *m_ppRiverRoot )
		RecDraw( pMemDC, r, *m_ppRiverRoot );

/*
	Vec2 curvePt = m_River.GetValue( 0.0f );

	pMemDC->MoveTo( TransformRiverToScreen( r, curvePt.x, curvePt.y ) );

	int curveResolution = 1000;

	for( int i = 0 ; i < curveResolution ; ++i )
	{
		float t = (float)(i + 1) / (float)curveResolution;
		curvePt = m_River.GetValue( t );

		pMemDC->LineTo( TransformRiverToScreen( r, curvePt.x, curvePt.y ) );
	}

	uint32_t n = m_River.GetKeys().size();

	for( int i = 0; i < n ; ++i )
	{
		const Keyframe<Vec2>& key = m_River.GetKeys()[ i ];
	
		CPoint p = TransformRiverToScreen( r, key.Key.x, key.Key.y );
		CPoint leftTgt = TransformRiverToScreen( r, key.LeftTangent.x, key.LeftTangent.y );
		CPoint rightTgt = TransformRiverToScreen( r, key.RightTangent.x, key.RightTangent.y );
		
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
*/
	m_backBuffer.Blit( dc.GetSafeHdc() );
}

bool RiverEditorControl::RecHitTestRiverNodes( const CRect& _rect, const CPoint& _p, const shared_ptr< RiverNode >& _node, shared_ptr< RiverNode >& _hitNode )
{
	CPoint p = TransformRiverToScreen( _rect, _node->Pos );

	CRect r( p, p );
	r.InflateRect( HANDLE_SIZE, HANDLE_SIZE );

	if( r.PtInRect( _p ) )
	{
		_hitNode = _node;
		return true;
	}

	for( int i = 0 ; i < _node->Childs.size() ; ++i )
	{
		if( RecHitTestRiverNodes( _rect, _p, _node->Childs[i], _hitNode ) )
			return true;
	}

	return false;
}


void RiverEditorControl::OnLButtonDown( UINT nFlags, CPoint point )
{
	CRect r;
	GetClientRect( &r );

	if( *m_ppRiverRoot )
	{
		shared_ptr< RiverNode > hitNode;
		if( RecHitTestRiverNodes( r, point, *m_ppRiverRoot, hitNode ) )
		{
			m_pSelectedRiverNode = hitNode;
			Invalidate( FALSE );
			return;
		}
	}

	Vec2 p = TransformScreenToRiver( r, point );

	shared_ptr< RiverNode > selectedNode( m_pSelectedRiverNode.lock() );

	if( !selectedNode )
	{
		*m_ppRiverRoot = make_shared< RiverNode >( p );

		m_pSelectedRiverNode = *m_ppRiverRoot;
	} 
	else
	{
		selectedNode->Childs.push_back( make_shared< RiverNode >( p ) );
		m_pSelectedRiverNode = selectedNode->Childs.back();
	}

/*
	m_DraggedKeyframe = -1;

	uint32_t n = m_River.GetKeys().size();

	for( int i = 0; i < n; ++i )
	{
		const Keyframe<Vec2>& key = m_River.GetKeys()[ i ];

		CPoint p = TransformRiverToScreen( r, key.Key.x, key.Key.y );

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
			p = TransformRiverToScreen( r, key.LeftTangent.x, key.LeftTangent.y );
			keyRect = CRect( p, p );
			keyRect.InflateRect( HANDLE_SIZE, HANDLE_SIZE );

			if( keyRect.PtInRect( point ) )
			{
				m_DraggedKeyframe = i;
				m_DraggedHandle = -1;
			}

			//Test right tangent handle
			p = TransformRiverToScreen( r, key.RightTangent.x, key.RightTangent.y );
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
		Vec2 p = TransformScreenToRiver( r, point );
		m_DraggedKeyframe = m_River.AddKey( p.x, p );
		m_DraggedHandle = 0;
		Invalidate( FALSE );
	}
*/
	OnRiverModified();
	SetCapture();
}


void RiverEditorControl::OnLButtonUp( UINT nFlags, CPoint point )
{
	//m_DraggedKeyframe = -1;

	ReleaseCapture();
}

void RiverEditorControl::OnRButtonDown( UINT nFlags, CPoint point )
{

	
}

void RiverEditorControl::OnRButtonUp( UINT nFlags, CPoint point )
{
/*	m_RightClickedKeyframe = -1;

	CRect r;
	GetClientRect( &r );

	uint32_t n = m_River.GetKeys().size();
	int i;

	for( i = 0; i < n; ++i )
	{
		const Keyframe<Vec2>& key = m_River.GetKeys()[ i ];

		CPoint p = TransformRiverToScreen( r, key.Key.x, key.Key.y );

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
	popupMenu.AppendMenu( MF_STRING, WM_RIVER_SET_CORNER, _T( "Corner" ) );
	popupMenu.AppendMenu( MF_STRING, WM_RIVER_SET_BEZIER_CORNER, _T( "Bezier Corner" ) );
	popupMenu.AppendMenu( MF_STRING, WM_RIVER_SET_BEZIER_SMOOTH, _T( "Bezier Smooth" ) );
	popupMenu.AppendMenu( MF_SEPARATOR );
	popupMenu.AppendMenu( MF_STRING, WM_RIVER_DEL_KEY, _T( "Delete" ) );

	popupMenu.CheckMenuRadioItem( 0, 2, m_River.GetKeys()[ i ].KeyType, MF_BYPOSITION );

	ClientToScreen( &point );

	//SetForegroundWindow();

	popupMenu.TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, this );
*/
	//PostMessage( WM_NULL ); //https://msdn.microsoft.com/en-us/library/windows/desktop/ms648002(v=vs.85).aspx

}

void RiverEditorControl::OnMouseMove( UINT nFlags, CPoint point )
{
	/*
	if( m_DraggedKeyframe == -1 )
		return;

	CRect r;
	GetClientRect( &r );

	Keyframe<Vec2>& key = m_River.GetKeys()[ m_DraggedKeyframe ];
	Vec2 p = TransformScreenToRiver( r, point );

	//TODO confine to neighbooring keys

	float minX, maxX;

	if( m_DraggedKeyframe == 0 )
	{
		minX = 0.0f;
		maxX = 0.0f;
	}
	else if( m_DraggedKeyframe == m_River.GetKeys().size() - 1 )
	{
		minX = 1.0f;
		maxX = 1.0f;
	}
	else
	{
		minX = m_River.GetKeys()[ m_DraggedKeyframe - 1 ].Key.x + 0.0001f;
		maxX = m_River.GetKeys()[ m_DraggedKeyframe + 1 ].Key.x - 0.0001f;
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
	*/
	//OnRiverModified();
}


void RiverEditorControl::OnDeleteKey()
{

	OnRiverModified();
}

void RiverEditorControl::OnRiverModified()
{

	//GetParent()->
	//SendNotifyMessage( RIVER_NOTIFICATION, 0, 0 );
	((RiverEditorDlg*)GetParent())->OnRiverModified( nullptr, nullptr ); //hack around MFC ...
	Invalidate( FALSE );

}


// RiverEditorDlg dialog

IMPLEMENT_DYNAMIC(RiverEditorDlg, CDialogEx)

RiverEditorDlg::RiverEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_RIVER_EDITOR, pParent)
{

}

RiverEditorDlg::~RiverEditorDlg()
{
}

void RiverEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_RIVER_EDITOR_CTRL, m_RiverControl );
}


BEGIN_MESSAGE_MAP(RiverEditorDlg, CDialogEx)
END_MESSAGE_MAP()


// RiverEditorDlg message handlers
void RiverEditorDlg::OnRiverModified( NMHDR*, LRESULT* )
{
	//	shared_ptr< ColorGradientNode > pNode = m_pComputeNode.lock();
	MountainNode* pNode = m_pComputeNode;

	if( !pNode )
		return ;

///	pNode->m_River = m_RiverControl.m_River;
	pNode->OnRiverChanged();
	pNode->SetDirty();
	theApp.RedrawPreview();
}