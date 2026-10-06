#include "stdafx.h"
#include "GeoGen.h"
#include "DEMGrabView.h"

#include "DownloadDEMDlg.h"

#include "NodeEditor.h"
#include "InputBitmapNode.h"
#include "TerrainTileProviderSetupDlg.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TILE_RES 256

DEMGrabView::DEMGrabView() :
	m_Zoom( 1.0f ),
	m_bLeftMouseButtonDown( false ),
	m_ViewCenter( Vec2( 500.0f, 500.0f ) ),
	m_pRootNode( nullptr ),
	m_pStreamingThread( nullptr ),
	m_PreviewMode( PreviewMode_ColorSat )
{
	TCHAR apiKeyBuffer[ 64 ];

	CRegKey cKey;

	if( cKey.Open( HKEY_CURRENT_USER, _T( "Software\\GeoGen" ) ) == ERROR_SUCCESS )
	{
		ULONG len = 64;
		if( cKey.QueryStringValue( _T( "MapZenAPIKey" ), apiKeyBuffer, &len ) == ERROR_SUCCESS )
			m_MapZenAPIKey = apiKeyBuffer;

		len = 64;
		if( cKey.QueryStringValue( _T( "HereAppId" ), apiKeyBuffer, &len ) == ERROR_SUCCESS )
			m_HereAppId = apiKeyBuffer;

		len = 64;
		if( cKey.QueryStringValue( _T( "HereAppCode" ), apiKeyBuffer, &len ) == ERROR_SUCCESS )
			m_HereAppCode = apiKeyBuffer;

	}
}

DEMGrabView::~DEMGrabView()
{
	if( m_pStreamingThread )
	{
		m_pStreamingThread->SuspendThread();
		m_pStreamingThread->ExitInstance(); //Doesn't seem to work, so we also suspend
	}

	if( m_pRootNode ) 
		delete m_pRootNode;
}


BEGIN_MESSAGE_MAP(DEMGrabView, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND( ID_DOWNLOAD_DEM, &DEMGrabView::OnDownloadDem )
END_MESSAGE_MAP()

// DEMGrabView message handlers
UINT TileStreamingThread( void* _param )
{
	DEMGrabView* view = (DEMGrabView*)_param;

	while( true )
	{
		view->m_StreamingQueueCriticalSection.Lock();
		DEMGrabView::TileNode* pNode = view->m_StreamingQueue[ view->m_StreamingQueue.size() - 1 ];
		view->m_StreamingQueue.pop_back();
		view->m_StreamingQueueCriticalSection.Unlock();

		pNode->pBitmap = view->GetPreviewTile( pNode->x, pNode->y, pNode->depth, TILE_RES );

		view->Invalidate( FALSE );

		view->m_StreamingQueueCriticalSection.Lock();
		bool bQueueEmpty = view->m_StreamingQueue.empty();
		view->m_StreamingQueueCriticalSection.Unlock();

		if( bQueueEmpty )
		{
			view->m_bStreamingThreadPaused = FALSE;
			view->m_pStreamingThread->SuspendThread();
		}
	}
	return 1;
}


BOOL DEMGrabView::Init()
{
	if( m_MapZenAPIKey.IsEmpty() 
	 || m_HereAppId.IsEmpty()
	 || m_HereAppCode.IsEmpty() )
	{
		TerrainTileProviderSetupDlg dlg;
		dlg.m_MapZenAPIKey = m_MapZenAPIKey;
		dlg.m_HereAppId = m_HereAppId;
		dlg.m_HereAppCode = m_HereAppCode;

		if( dlg.DoModal() == IDOK )
		{
			m_MapZenAPIKey = dlg.m_MapZenAPIKey;
			m_HereAppId = dlg.m_HereAppId;
			m_HereAppCode = dlg.m_HereAppCode;

			if( m_MapZenAPIKey.IsEmpty() )
				return FALSE;

			CRegKey cKey;
			cKey.Create( HKEY_CURRENT_USER, _T( "Software\\GeoGen" ) );
			cKey.SetStringValue( _T( "MapZenAPIKey" ), m_MapZenAPIKey );
			cKey.SetStringValue( _T( "HereAppId" ), m_HereAppId );
			cKey.SetStringValue( _T( "HereAppCode" ), m_HereAppCode );

		}
		else
		{
			return FALSE;
		}
	}

	TCHAR my_documents[ MAX_PATH ];
	HRESULT result = SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents );

	if( result != S_OK )
	{
		AfxMessageBox( _T( "Failed to get \"My Documents\" directory" ), MB_OK );
		return FALSE;
	}

	CString geoGenDocDir( my_documents );
	geoGenDocDir += _T( "\\GeoGen" );
	m_DEMCacheDir = geoGenDocDir + _T( "\\DEMCache\\" );
	m_ColorSatCacheDir = geoGenDocDir + _T( "\\ColorSatCache\\" );

	BOOL error;
	error = CreateDirectory( geoGenDocDir, NULL );
	error &= CreateDirectory( m_ColorSatCacheDir, NULL );
	error &= CreateDirectory( m_DEMCacheDir, NULL );
	 
	if( error && ( GetLastError() != ERROR_ALREADY_EXISTS ) )
	{
		AfxMessageBox( _T( "Failed to create <Documents>\\GeoGen\\<CacheDirectories>" ), MB_OK );
		return FALSE;
	}

	m_pRootNode = new TileNode;
	m_pRootNode->TopLeft = Vec2( 0, 0 );
	m_pRootNode->BottomRight = Vec2( 1000, 1000 );
	m_pRootNode->x = 0;
	m_pRootNode->y = 0;
	m_pRootNode->depth = 0;
	m_pRootNode->pBitmap = GetPreviewTile( 0, 0, 0, TILE_RES );

	if( !m_pRootNode->pBitmap )
	{
		AfxMessageBox( _T( "Failed to download tiles from mapzen.com" ), MB_OK );
		return FALSE;
	}

	m_pStreamingThread = AfxBeginThread( TileStreamingThread, this );
	m_bStreamingThreadPaused = TRUE;
	m_pStreamingThread->SuspendThread();

	return TRUE;
}


BOOL DEMGrabView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if( !Init() )
		return FALSE;

	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

class ScopedCriticalSection
{
public:
	ScopedCriticalSection( CCriticalSection& cs ) : m_cs( cs ) { cs.Lock(); }
	~ScopedCriticalSection() { m_cs.Unlock(); }
private:
	CCriticalSection& m_cs;
};

void DEMGrabView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
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

	pMemDC->FillSolidRect( &r, RGB( 255,255,255 ) );
	
	m_StreamingQueueCriticalSection.Lock();
	m_StreamingQueue.clear();
	m_StreamingQueueCriticalSection.Unlock();

	RenderTiles( pMemDC, r, 1.0f, m_pRootNode, false );

	auto sortNodesByDepth = []( const TileNode* a, const TileNode* b ) { return a->depth > b->depth; };

	m_StreamingQueueCriticalSection.Lock();
	std::sort( m_StreamingQueue.begin(), m_StreamingQueue.end(), sortNodesByDepth );
	m_StreamingQueueCriticalSection.Unlock();


	//Draw capture rect

	m_CaptureRect = r;
	CPoint center = r.CenterPoint();

	if( r.Width() > r.Height() )
	{
		int w = r.Height() / 2;
		m_CaptureRect.left = center.x - w;
		m_CaptureRect.right = center.x + w;
	}
	else
	{
		int h = r.Width() / 2;
		m_CaptureRect.top = center.y - h;
		m_CaptureRect.bottom = center.y + h;
	}

	m_CaptureRect.DeflateRect( 30, 30 );

	CPen redPen( PS_SOLID, 2, RGB( 255, 0, 0 ) );
	pMemDC->SelectObject( &redPen );

	pMemDC->MoveTo( m_CaptureRect.TopLeft() );
	pMemDC->LineTo( CPoint( m_CaptureRect.right, m_CaptureRect.top ) );
	pMemDC->LineTo( m_CaptureRect.BottomRight() );
	pMemDC->LineTo( CPoint( m_CaptureRect.left, m_CaptureRect.bottom ) );
	pMemDC->LineTo( m_CaptureRect.TopLeft() );


	//Blit BackBuffer

	m_backBuffer.Blit( dc.GetSafeHdc() );
}

void DEMGrabView::CreateTileNode( TileNode* _pParent, uint32_t _childIndex )
{
	TileNode* pChild = new TileNode;
	_pParent->Childs[ _childIndex ] = pChild;

	pChild->depth = _pParent->depth + 1;

	Vec2 center( (_pParent->TopLeft + _pParent->BottomRight) * 0.5f );

	switch( _childIndex )
	{
	case 0: //TL
		pChild->TopLeft = _pParent->TopLeft;
		pChild->BottomRight = center;
		pChild->x = _pParent->x * 2;
		pChild->y = _pParent->y * 2;
		break;
	case 1: //TR
		pChild->TopLeft = Vec2( center.x, _pParent->TopLeft.y );
		pChild->BottomRight = Vec2( _pParent->BottomRight.x, center.y );
		pChild->x = _pParent->x * 2 + 1;
		pChild->y = _pParent->y * 2;
		break;
	case 2: //BL
		pChild->TopLeft = Vec2( _pParent->TopLeft.x, center.y );
		pChild->BottomRight = Vec2( center.x, _pParent->BottomRight.y );
		pChild->x = _pParent->x * 2;
		pChild->y = _pParent->y * 2 + 1;
		break;
	case 3: //BR
		pChild->TopLeft = center;
		pChild->BottomRight = _pParent->BottomRight;
		pChild->x = _pParent->x * 2 + 1;
		pChild->y = _pParent->y * 2 + 1;
		break;
	}

}


void DEMGrabView::AsyncLoadTileBitmap( TileNode* _pTileNode )
{
	m_StreamingQueueCriticalSection.Lock();
	m_StreamingQueue.push_back( _pTileNode );
	m_StreamingQueueCriticalSection.Unlock();

	m_bStreamingThreadPaused = FALSE;
	m_pStreamingThread->ResumeThread();
}


inline BOOL RectVisible( const CRect& _screenRect, const CRect& _tileRect )
{
	CRect i;
	return i.IntersectRect( &_screenRect, &_tileRect );
}


void DEMGrabView::RenderTiles( CDC* _pDC, const CRect& _screenRect, float _scale, TileNode* _pTileNode, bool _bCaptureMode, bool _bAllowDraw, vector< pair< CRect, TileNode* > >* _pNodesToDraw )
{
	assert( _pTileNode );
	//assert( _pTileNode->pBitmap );
	
	Vec2 c( _screenRect.CenterPoint().x, _screenRect.CenterPoint().y );

	Vec2 tl = ( _pTileNode->TopLeft     - m_ViewCenter ) * m_Zoom * _scale;
	Vec2 br = ( _pTileNode->BottomRight - m_ViewCenter ) * m_Zoom * _scale;

	tl += c;
	br += c;

	CRect tileRect( tl.x, tl.y, br.x, br.y );
		
	if( !RectVisible( _screenRect, tileRect ) )
		return;

	
	if( _bCaptureMode )
	{
		assert( _pNodesToDraw );

		bool bDetailedEnough = tileRect.Width() <= TILE_RES;

		if( bDetailedEnough )
		{		
			_pNodesToDraw->push_back( std::pair<CRect, TileNode*>( tileRect, _pTileNode ) );	
		}
		else
		{
			//Recurse

			for( int i = 0; i < 4; ++i )
			{
				if( !_pTileNode->Childs[ i ] )
					CreateTileNode( _pTileNode, i );

				RenderTiles( _pDC, _screenRect, _scale, _pTileNode->Childs[ i ], _bCaptureMode, _bAllowDraw, _pNodesToDraw );
			}

		}

	}
	else
	{
		//preview mode

		bool bDetailedEnough = tileRect.Width() < TILE_RES * 2;

		if( bDetailedEnough || (_pTileNode->depth > 14) ) 
		{
			if( _pTileNode->IsLoaded() )
			{
				if( _bAllowDraw )
					DrawTile( _pDC, tileRect, _pTileNode );
			}
			else
			{
				AsyncLoadTileBitmap( _pTileNode );
			}
		}
		else
		{
			bool bAllChildsLoaded = true;

			for( int i = 0; i < 4; ++i )
			{
				bAllChildsLoaded &= _pTileNode->Childs[ i ] && _pTileNode->Childs[ i ]->IsLoaded();
			}


			if( bAllChildsLoaded )
			{
				for( int i = 0; i < 4; ++i )
				{
					RenderTiles( _pDC, _screenRect, _scale, _pTileNode->Childs[ i ], _bCaptureMode, true );
				}

			}
			else
			{

				if( _bAllowDraw )
				{
					assert( _pTileNode->IsLoaded() );
					DrawTile( _pDC, tileRect, _pTileNode );
				}

				for( int i = 0; i < 4; ++i )
				{
					if( !_pTileNode->Childs[ i ] )
					{
						CreateTileNode( _pTileNode, i );
						AsyncLoadTileBitmap( _pTileNode->Childs[ i ] );
					}

					RenderTiles( _pDC, _screenRect, _scale, _pTileNode->Childs[ i ], _bCaptureMode, false );

				}


			}
		}
	}
}

void DEMGrabView::DrawTile( CDC* _pDC, const CRect& _tileRect, TileNode* _pTileNode ) const
{
	CDC bitmapDC;
	if( !bitmapDC.CreateCompatibleDC( _pDC ) )
	{
		assert( false );
		return;
	}

	CBitmap* pOldBitmap = bitmapDC.SelectObject( _pTileNode->pBitmap );

	_pDC->StretchBlt( _tileRect.left, _tileRect.top, _tileRect.Width(), _tileRect.Height(), &bitmapDC, 0, 0, TILE_RES, TILE_RES, SRCCOPY );

	bitmapDC.SelectObject( pOldBitmap ); //really necessary ???
	bitmapDC.DeleteDC();
}

void DEMGrabView::SwitchCaptureMode( bool _captureMode, float _halfExtent )
{
	if( _captureMode )
	{
		m_StreamingQueueCriticalSection.Lock();
		m_StreamingQueue.clear();
		m_StreamingQueueCriticalSection.Unlock();

		m_pStreamingThread->SuspendThread();
	}
	
	delete m_pRootNode;

	m_pRootNode = new TileNode;
	m_pRootNode->TopLeft = Vec2::Origin;
	m_pRootNode->BottomRight = Vec2( 2.0f * _halfExtent, 2.0f * _halfExtent );
	m_pRootNode->x = 0;
	m_pRootNode->y = 0;
	m_pRootNode->depth = 0;

	if( _captureMode )
		m_pRootNode->pBitmap = nullptr;
	else
		m_pRootNode->pBitmap = GetPreviewTile( 0, 0, 0, TILE_RES );

	//if( !_captureMode )
	//	m_pStreamingThread->ResumeThread();
}


bool DEMGrabView::CaptureTerrain()
{

	CDownloadDEMDlg dlg( this );

	dlg.m_Resolution = theApp.GetResolution();
	dlg.m_ColorSatResolution = theApp.GetResolution();
	dlg.m_bAlsoCaptureColorSat = TRUE;

	if( dlg.DoModal() != IDOK )
		return false;

	dlg.m_ColorSatResolution = dlg.m_Resolution; //TODO make varying resolution work
	
	int resolution = dlg.m_Resolution;
	
	SwitchCaptureMode( true, 500.0f );// (float)m_CaptureRect.Width() * 0.5f );

	CRect rect( 0, 0, resolution, resolution );

	float scale = (float)resolution / (float)m_CaptureRect.Height();

	vector< pair< CRect, TileNode* > > nodesToDraw;
	RenderTiles( nullptr, rect, scale, m_pRootNode, true, true, &nodesToDraw );

	CRect screenRect;
	GetClientRect( &screenRect );

	CRect progressRect( CPoint( screenRect.CenterPoint() + CPoint( -200, -40 )),
						CPoint( screenRect.CenterPoint() + CPoint( 200, 40 ) ));
	

	CProgressCtrl progressCtrl;
	progressCtrl.Create( 0, progressRect, this, 8748516 );
	progressCtrl.SetRange( 0, nodesToDraw.size() );
	progressCtrl.SetPos( 0 );
	progressCtrl.ShowWindow( SW_SHOW );

	Bitmap outputAltitudeBitmap, outputColorSatBitmap;

	if( !outputAltitudeBitmap.Create( resolution, resolution, 1, 1, DataFormat::R32_FLOAT ) )
		return false;

	outputAltitudeBitmap.Fill( Color::Black );

	if( dlg.m_bAlsoCaptureColorSat )
	{
		if( !outputColorSatBitmap.Create( dlg.m_ColorSatResolution, dlg.m_ColorSatResolution, 1, 1, DataFormat::R8G8B8A8_UNORM_SRGB ) )
			return false;

		outputColorSatBitmap.Fill( Color::Black );
	}

	uint32_t i = 0;

	for( auto & node : nodesToDraw )
	{
		Bitmap* pTileBmp = GetAltitudeTile( node.second->x, node.second->y, node.second->depth, TILE_RES );

		if( !pTileBmp )
		{
			LOG_R( "Failed to download altitude tile (x%d y%d zoom%d)", node.second->x, node.second->y, node.second->depth );
			assert( false );
			progressCtrl.SetPos( i++ );
			continue;
		}

		auto fn = []( Color& _c )
		{ //MapZen 'terraria' decoding
			_c *= 255.0f;
			float altitude = (_c.r * 256.0f + _c.g + _c.b / 256.0f) - 32768.0f;
			_c = Color( altitude, altitude, altitude, altitude );
		};

		outputAltitudeBitmap.StretchBlit( *pTileBmp, node.first.left, node.first.top, node.first.Width(), node.first.Height(), fn );
		
		delete pTileBmp;

		//----------------- COLOR-SAT ---

		if( dlg.m_bAlsoCaptureColorSat )
		{
			pTileBmp = GetColorSatTile( node.second->x, node.second->y, node.second->depth, TILE_RES );

			if( !pTileBmp )
			{
				LOG_R( "Failed to download color tile (x%d y%d zoom%d)", node.second->x, node.second->y, node.second->depth );
				assert( false );
				progressCtrl.SetPos( i++ );
				continue;
			}

			outputColorSatBitmap.StretchBlit( *pTileBmp, node.first.left, node.first.top, node.first.Width(), node.first.Height() );

			delete pTileBmp;
		}

		progressCtrl.SetPos( i++ );
	}

	SwitchCaptureMode( false, 500.0f );

	float minAltitude, maxAltitude;
	ProcessCapturedTerrain( outputAltitudeBitmap, dlg.m_bClipAtSeaLevel, dlg.m_bUnderwater, minAltitude, maxAltitude );

	CT2A filename( dlg.m_Filename );

	if( !outputAltitudeBitmap.SaveRAW( filename.m_psz ) )
		return false;// TODO error

	std::string colorSatFilename;

	if( dlg.m_bAlsoCaptureColorSat )
	{
		colorSatFilename = filename.m_psz;

		StripFileExtension( colorSatFilename );
		colorSatFilename += ".bmp";

		if( !outputColorSatBitmap.SaveBMP( colorSatFilename.c_str() ) )
			return false;
	}

	if( dlg.m_bResizeWorld )
	{
		theApp.m_TerrainExtent = 1000.0f * (2.0f * 3.14159f * 6400.0f) / m_Zoom;
		theApp.m_MinAltitude = minAltitude;
		theApp.m_MaxAltitude = maxAltitude;
		//TODO theApp.m_SeaLevel ;
		theApp.m_Resolution = resolution;
		
		g_NodeEditor.OnResolutionChange();
	}

	//Add an inpout node 
	shared_ptr<InputBitmapNode> pInputNode( new InputBitmapNode );

	pInputNode->SetFilename( filename.m_psz );
	pInputNode->SetFixedResolution( resolution );
	pInputNode->SetPos( Vec2( 100, 100 ) );
	g_NodeEditor.AddNode( pInputNode );
	g_NodeEditor.SetPreviewNode( pInputNode );

	if( dlg.m_bAlsoCaptureColorSat )
	{
		shared_ptr<InputBitmapNode> pInputNode2( new InputBitmapNode );

		pInputNode2->SetFilename( colorSatFilename.c_str() );
		pInputNode2->SetFixedResolution( dlg.m_ColorSatResolution );
		pInputNode2->SetPos( Vec2( 100, 200 ) );
		g_NodeEditor.AddNode( pInputNode2 );
	}

	theApp.RedrawNodeEditor();
	theApp.RedrawPreview();
//	((CFrameWndEx*)GetParent())->OnClose();

	return true;
}

void DEMGrabView::ProcessCapturedTerrain( const Bitmap& _bmp, bool _clipAtSeaLevel, bool _underwater, float& _minAltitude, float& _maxAltitude )
{
	assert( _bmp.GetFormat() == DataFormat::R32_FLOAT );

	float* pPixels = (float*)_bmp.GetRawData();

	uint32_t n = _bmp.GetWidth() * _bmp.GetHeight();

	_minAltitude = 99999999999.0f;
	_maxAltitude = -99999999999.0f;

	for( uint32_t i = 0 ; i < n ; ++i )
	{
		float& altitude = pPixels[ i ];

		_minAltitude = min( _minAltitude, altitude );
		_maxAltitude = max( _maxAltitude, altitude );
	}
		
	if( _clipAtSeaLevel )
		if( _underwater )
			_maxAltitude = min( _maxAltitude, 0.0f );
		else
			_minAltitude = max( _minAltitude, 0.0f );

	for( uint32_t i = 0 ; i < n ; ++i )
	{
		float altitude = pPixels[ i ];

		altitude = (altitude - _minAltitude) / (_maxAltitude - _minAltitude);
		altitude = Saturate( altitude );

		pPixels[ i ] = altitude;
	}
}

void DEMGrabView::OnLButtonDown( UINT nFlags, CPoint point )
{
//	CWnd::OnLButtonDown( nFlags, point );
	SetFocus();
	SetCapture();

	m_bLeftMouseButtonDown = true;
	Vec2 mousePos( (float)point.x, (float)point.y );
	m_LastMousePos = mousePos;

}


void DEMGrabView::OnLButtonUp( UINT nFlags, CPoint point )
{
//	CWnd::OnLButtonUp( nFlags, point );
	m_bLeftMouseButtonDown = false;
	ReleaseCapture();
}


void DEMGrabView::OnRButtonDown( UINT nFlags, CPoint point )
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnRButtonDown( nFlags, point );
}


void DEMGrabView::OnRButtonUp( UINT nFlags, CPoint point )
{
	Vec2 mousePos( (float)point.x, (float)point.y );
	
	//CWnd::OnRButtonUp( nFlags, point );
}


void DEMGrabView::OnMouseMove( UINT nFlags, CPoint point )
{
//	CWnd::OnMouseMove( nFlags, point );

	const float mouseSpeed = 1.0f;

	Vec2 mousePos( (float)point.x, (float)point.y );
	Vec2 mouseMovement = (mousePos - m_LastMousePos) * mouseSpeed;

	if( m_bLeftMouseButtonDown )
	{
		m_ViewCenter -= mouseMovement / m_Zoom;
		Invalidate( FALSE );
	}

	m_LastMousePos = mousePos;

}


BOOL DEMGrabView::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	m_Zoom *= 1.0f + 0.0003f * zDelta; //Deactivate zoom for now as it's WIP

	Invalidate( FALSE );

//	return CWnd::OnMouseWheel( nFlags, zDelta, pt );
	return TRUE;
}

CBitmap* DEMGrabView::GetPreviewTile( int _x, int _y, int _zoom, int _resolution )
{
	CString terrariumCacheFilename, previewCacheFilename;

	if( !GetAltitudeTile( _x, _y, _zoom, _resolution, terrariumCacheFilename, previewCacheFilename ) )
		return nullptr;

	CPngImage* pBitmap = new CPngImage;

	if( pBitmap->LoadFromFile( previewCacheFilename ) )
	{
		return pBitmap;
	}
	else
	{
		assert( false );
		//TODO delete invalid cache file ?
		delete pBitmap;
		return nullptr;
	}
}


Bitmap* DEMGrabView::GetAltitudeTile( int _x, int _y, int _zoom, int _resolution )
{
	CString terrariumCacheFilename, previewCacheFilename;

	if( !GetAltitudeTile( _x, _y, _zoom, _resolution, terrariumCacheFilename, previewCacheFilename ) )
		return nullptr;

	CT2A aTerrariumCacheFilename( terrariumCacheFilename );

	Bitmap* pBitmap = new Bitmap;

	if( pBitmap->Load( aTerrariumCacheFilename.m_psz ) )
	{
		return pBitmap;
	}
	else
	{
		assert( false );
		//TODO delete invalid cache file ?
		delete pBitmap;
		return nullptr;
	}
}

Bitmap* DEMGrabView::GetColorSatTile( int _x, int _y, int _zoom, int _resolution )
{
	CString colorSatCacheFilename;

	if( !GetColorSatTile( _x, _y, _zoom, _resolution, colorSatCacheFilename ) )
		return nullptr;

	CT2A aColorSatCacheFilename( colorSatCacheFilename );

	Bitmap* pBitmap = new Bitmap;

	if( pBitmap->Load( aColorSatCacheFilename.m_psz ) )
	{
		return pBitmap;
	}
	else
	{
		assert( false );
		//TODO delete invalid cache file ?
		delete pBitmap;
		return nullptr;
	}
}

bool DEMGrabView::GetAltitudeTile( int _x, int _y, int _zoom, int _resolution, CString& _terrariumCacheFilename, CString& _previewCacheFilename )
{

	_previewCacheFilename.Format( _T( "%s%s_r%d_z%d_x%d_y%d.bmp" ), m_DEMCacheDir, _T( "preview" ), _resolution, _zoom, _x, _y ); //TODO jpg format
	_terrariumCacheFilename.Format( _T( "%s%s_r%d_z%d_x%d_y%d.png" ), m_DEMCacheDir, _T( "terrarium" ), _resolution, _zoom, _x, _y );

	CT2A aPreviewCacheFilename( _previewCacheFilename );
	CT2A aTerrariumCacheFilename( _terrariumCacheFilename );

	if( !PathFileExists( _previewCacheFilename ) )
	{
		//try getting the source 'terrarium' file from cache

		if( !PathFileExists( _terrariumCacheFilename ) )
		{
			//Download file from mapzen
		
		#if 0
			LPCTSTR baseURL = _T( "http://tile.mapzen.com/mapzen/terrain/v1" );
		#else
			LPCTSTR baseURL = _T( "http://tile.nextzen.org/tilezen/terrain/v1" );
		#endif


			CString url;
			url.Format( _T( "%s/%d/%s/%d/%d/%d.png?api_key=%s" ),
				baseURL, _resolution, _T( "terrarium" ), _zoom, _x, _y, m_MapZenAPIKey );

			CT2A urla( url );

			uint32_t fileSize;
			byte* pTileData = m_MapZenHttpConnection.DownloadFile( urla.m_psz, fileSize );

			if( !pTileData )
				return false;

			if( fileSize < 1024 )
			{
				//File not found on server or quotta exceeded ?
				delete pTileData;
				return false;
			}

			//save raw terrarium file to cache
			CFile cacheFile;
			if( !cacheFile.Open( _terrariumCacheFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone ) )
			{
				assert( false );
				//TODO disk full ?? handle gracefully
			}

			cacheFile.Write( pTileData, fileSize );

			delete pTileData;

		}

		Bitmap previewBMP;
		if( !previewBMP.LoadPNG( aTerrariumCacheFilename.m_psz ) )
		{
			assert( false );
			return false;
		}

		ProcessPreviewTile( previewBMP );

		if( !previewBMP.SaveBMP( aPreviewCacheFilename.m_psz ) )
			return false;
	}

	return true;
}

bool DEMGrabView::GetColorSatTile( int _x, int _y, int _zoom, int _resolution, CString& _colorSatCacheFilename )
{
	_colorSatCacheFilename.Format( _T( "%s%s_r%d_z%d_x%d_y%d.png" ), m_ColorSatCacheDir, _T( "satellite" ), _resolution, _zoom, _x, _y );


	if( !PathFileExists( _colorSatCacheFilename ) )
	{
		//Download file from here.com
			
		CString url;
		url.Format( _T( "https://1.aerial.maps.cit.api.here.com/maptile/2.1/maptile/newest/%s.day/%d/%d/%d/%d/%s?app_id=%s&app_code=%s" ),
			_T( "satellite" ), _zoom, _x, _y, _resolution, _T( "png" ), m_HereAppId, m_HereAppCode );

		CT2A urla( url );

		uint32_t fileSize;
		byte* pTileData = m_HereDotComHttpConnection.DownloadFile( urla.m_psz, fileSize );

		if( !pTileData )
			return false;

		if( fileSize < 1024 )
		{
			//File not found on server or quotta exceeded ?
			delete pTileData;
			return false;
		}

		//save raw terrarium file to cache
		CFile cacheFile;
		if( !cacheFile.Open( _colorSatCacheFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone ) )
		{
			assert( false );
			//TODO disk full ?? handle gracefully
		}

		cacheFile.Write( pTileData, fileSize );

		delete pTileData;

	}


	return true;
}

void DEMGrabView::ProcessPreviewTile( Bitmap& _bmp )
{

	//const float minAlt = -10000.0f; //underwater
	const float minAlt = 0.0f;
	const float maxAlt = 6000.0f;

	rgba8_t* pPixels = (rgba8_t*)_bmp.GetRawData();
	
	uint32_t numPixels = _bmp.GetWidth() * _bmp.GetHeight();

	float min = 99999999999.0f;
	float max = -99999999999.0f;

	for( uint32_t i = 0; i < numPixels; ++i )
	{
		rgba8_t& pixel = pPixels[ i ];

		float altitude = ((float)pixel.r * 256.0f + (float)pixel.g + (float)pixel.b / 256.0f) - 32768.0f;

		Color c;

		if( altitude > 0.0f )
			c = Color::White;
		else
			c = Color( 0.3f, 0.5f, 0.9f );

		altitude = Abs( altitude );
		altitude = (altitude - minAlt) / (maxAlt - minAlt);
		altitude = Saturate( altitude );
		altitude = Pow( altitude, 0.5f ); //more contrast in low altitudes
		
		c *= altitude;

		pixel.r = (u8)(c.r * 255.0f);
		pixel.g = (u8)(c.g * 255.0f);
		pixel.b = (u8)(c.b * 255.0f);
		pixel.a = 255;
	}

}


void DEMGrabView::OnDownloadDem()
{
	CaptureTerrain();
}
