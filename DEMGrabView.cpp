#include "stdafx.h"
#include "GeoGen.h"
#include "DEMGrabView.h"

#include "DownloadDEMDlg.h"

#include "NodeEditor.h"
#include "InputBitmapNode.h"
#include "TerrainTileProviderSetupDlg.h"
#include "HTTPConnection.h"
#include "MainFrm.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TILE_RES 256

//Highest zoom level available from the elevation provider (AWS Terrain Tiles)
#define MAX_TILE_ZOOM 15

//Tile providers. Elevation: AWS Terrain Tiles (terrarium encoding, no key). Imagery: MapTiler Satellite (API key)
static LPCTSTR s_ElevationTileURL = _T( "https://s3.amazonaws.com/elevation-tiles-prod/terrarium/%d/%d/%d.png" ); //z, x, y
static LPCTSTR s_SatelliteTileURL = _T( "https://api.maptiler.com/maps/satellite/256/%d/%d/%d.jpg?key=%s" ); //z, x, y, key

static LPCTSTR s_Attribution = _T( "Elevation: AWS Terrain Tiles (Mapzen)  |  Imagery: (c) MapTiler (c) OpenStreetMap contributors" );

class ScopedCriticalSection
{
public:
	ScopedCriticalSection( CCriticalSection& cs ) : m_cs( cs ) { cs.Lock(); }
	~ScopedCriticalSection() { m_cs.Unlock(); }
private:
	CCriticalSection& m_cs;
};

DEMGrabView::DEMGrabView() :
	m_Zoom( 1.0f ),
	m_bLeftMouseButtonDown( false ),
	m_ViewCenter( Vec2( 500.0f, 500.0f ) ),
	m_pRootNode( nullptr ),
	m_pStreamingThread( nullptr ),
	m_StreamingWakeEvent( FALSE, FALSE ),	//auto-reset, not signaled
	m_StreamingIdleEvent( TRUE, TRUE ),		//manual-reset, signaled (idle)
	m_bStopStreaming( false ),
	m_PreviewMode( PreviewMode_ColorSat )
{
	m_MapTilerAPIKey = TerrainTileProviderSetupDlg::LoadMapTilerAPIKey();
}

DEMGrabView::~DEMGrabView()
{
	StopStreamingThread();

	if( m_pRootNode )
		delete m_pRootNode;
}


BEGIN_MESSAGE_MAP(DEMGrabView, CWnd)
	ON_WM_PAINT()
	ON_WM_DESTROY()
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
	((DEMGrabView*)_param)->StreamTiles();
	return 0;
}

void DEMGrabView::StreamTiles()
{
	for( ;; )
	{
		::WaitForSingleObject( m_StreamingWakeEvent, INFINITE );

		//Download queued tiles until the queue is empty
		for( ;; )
		{
			TileNode* pNode = nullptr;

			{
				ScopedCriticalSection lock( m_StreamingQueueCriticalSection );

				if( m_bStopStreaming )
					return;

				while( !pNode && !m_StreamingQueue.empty() )
				{
					pNode = m_StreamingQueue.back();
					m_StreamingQueue.pop_back();

					if( pNode->IsLoaded() )
						pNode = nullptr; //queued more than once
				}

				if( !pNode )
					break;

				//The node can't be deleted until the idle event is set again (see WaitForStreamingIdle)
				m_StreamingIdleEvent.ResetEvent();
			}

			CBitmap* pBitmap = GetPreviewTile( pNode->x, pNode->y, pNode->depth, TILE_RES );

			{
				ScopedCriticalSection lock( m_StreamingQueueCriticalSection );

				if( pNode->IsLoaded() )
					delete pBitmap;
				else
					pNode->pBitmap = pBitmap;

				m_StreamingIdleEvent.SetEvent();
			}

			if( pBitmap )
				::InvalidateRect( m_hWnd, nullptr, FALSE ); //posts WM_PAINT, safe from this thread
		}
	}
}

void DEMGrabView::WaitForStreamingIdle()
{
	{
		ScopedCriticalSection lock( m_StreamingQueueCriticalSection );
		m_StreamingQueue.clear();
	}

	::WaitForSingleObject( m_StreamingIdleEvent, INFINITE );
}

void DEMGrabView::StopStreamingThread()
{
	if( !m_pStreamingThread )
		return;

	{
		ScopedCriticalSection lock( m_StreamingQueueCriticalSection );
		m_StreamingQueue.clear();
		m_bStopStreaming = true;
	}

	m_StreamingWakeEvent.SetEvent();

	::WaitForSingleObject( m_pStreamingThread->m_hThread, INFINITE ); //at most one tile download (bounded by HTTP timeouts)

	delete m_pStreamingThread;
	m_pStreamingThread = nullptr;
}

void DEMGrabView::OnDestroy()
{
	StopStreamingThread(); //before the window handle goes away

	CWnd::OnDestroy();
}


BOOL DEMGrabView::Init()
{
	//No key needed here: the preview only uses elevation tiles. The MapTiler key is asked for when capturing imagery.

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

	auto createDirectory = []( const CString& _dir )
	{
		return CreateDirectory( _dir, NULL ) || ( GetLastError() == ERROR_ALREADY_EXISTS );
	};

	if(    !createDirectory( geoGenDocDir )
		|| !createDirectory( m_ColorSatCacheDir )
		|| !createDirectory( m_DEMCacheDir ) )
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
		AfxMessageBox( _T( "Failed to download elevation tiles (AWS Terrain Tiles).\nCheck your internet connection." ), MB_OK );
		return FALSE;
	}

	m_pStreamingThread = AfxBeginThread( TileStreamingThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );

	if( !m_pStreamingThread )
		return FALSE;

	m_pStreamingThread->m_bAutoDelete = FALSE; //deleted by StopStreamingThread(), which waits on its handle
	m_pStreamingThread->ResumeThread();

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
	
	{
		//The lock also keeps the streaming thread from changing TileNode::pBitmap while tiles are drawn
		ScopedCriticalSection lock( m_StreamingQueueCriticalSection );

		m_StreamingQueue.clear();

		RenderTiles( pMemDC, r, 1.0f, m_pRootNode, false );

		//The streaming thread pops from the back: load coarse tiles first
		auto sortNodesByDepth = []( const TileNode* a, const TileNode* b ) { return a->depth > b->depth; };
		std::sort( m_StreamingQueue.begin(), m_StreamingQueue.end(), sortNodesByDepth );
	}


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

	//Provider attribution (required by the tile providers' terms)

	CRect attributionRect( r );
	attributionRect.DeflateRect( 6, 4 );
	pMemDC->SetBkMode( TRANSPARENT );
	pMemDC->SetTextColor( RGB( 40, 40, 40 ) );
	pMemDC->DrawText( s_Attribution, &attributionRect, DT_LEFT | DT_BOTTOM | DT_SINGLELINE );

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
	{
		ScopedCriticalSection lock( m_StreamingQueueCriticalSection );
		m_StreamingQueue.push_back( _pTileNode );
	}

	m_StreamingWakeEvent.SetEvent();
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

		//Past the provider's highest zoom level, the tiles get upsampled by StretchBlit
		if( bDetailedEnough || ( _pTileNode->depth >= MAX_TILE_ZOOM ) )
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

		if( bDetailedEnough || ( _pTileNode->depth >= MAX_TILE_ZOOM ) )
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

				if( _bAllowDraw && _pTileNode->IsLoaded() ) //not loaded if its download failed
				{
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
	//The streaming thread may be downloading a tile into a node of the tree we are about to delete
	WaitForStreamingIdle();

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

	//Satellite imagery needs a MapTiler key (it may have been set from the settings dialog meanwhile)

	m_MapTilerAPIKey = TerrainTileProviderSetupDlg::LoadMapTilerAPIKey();

	if( dlg.m_bAlsoCaptureColorSat && m_MapTilerAPIKey.IsEmpty() )
	{
		TerrainTileProviderSetupDlg keyDlg( this );

		if( keyDlg.DoModal() == IDOK )
		{
			TerrainTileProviderSetupDlg::SaveMapTilerAPIKey( keyDlg.m_MapTilerAPIKey );
			m_MapTilerAPIKey = TerrainTileProviderSetupDlg::LoadMapTilerAPIKey();
		}

		if( m_MapTilerAPIKey.IsEmpty() )
		{
			AfxMessageBox( _T( "No MapTiler API key: only elevations will be captured." ), MB_OK | MB_ICONINFORMATION );
			dlg.m_bAlsoCaptureColorSat = FALSE;
		}
	}

	int resolution = dlg.m_Resolution;

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

	//Captured area, in tile-tree units (the whole world is 1000 units wide), needed to resize the world below
	const float capturedWorldUnits = (float)m_CaptureRect.Height() / m_Zoom;
	const float capturedCenterY = m_ViewCenter.y;

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
	progressCtrl.Create( WS_CHILD | WS_VISIBLE, progressRect, this, 8748516 );
	progressCtrl.SetRange32( 0, (int)nodesToDraw.size() );
	progressCtrl.SetPos( 0 );
	progressCtrl.UpdateWindow();

	uint32_t i = 0;
	uint32_t numFailedAltitudeTiles = 0, numFailedColorSatTiles = 0;

	auto updateProgress = [&]()
	{
		progressCtrl.SetPos( ++i );
		progressCtrl.UpdateWindow(); //no message pump during the capture
	};

	for( auto & node : nodesToDraw )
	{
		Bitmap* pTileBmp = GetAltitudeTile( node.second->x, node.second->y, node.second->depth, TILE_RES );

		if( !pTileBmp )
		{
			LOG_R( "Failed to get altitude tile (x%d y%d zoom%d)", node.second->x, node.second->y, node.second->depth );
			++numFailedAltitudeTiles;
		}
		else
		{
			auto fn = []( Color& _c )
			{ //'terrarium' decoding
				_c *= 255.0f;
				float altitude = (_c.r * 256.0f + _c.g + _c.b / 256.0f) - 32768.0f;
				_c = Color( altitude, altitude, altitude, altitude );
			};

			outputAltitudeBitmap.StretchBlit( *pTileBmp, node.first.left, node.first.top, node.first.Width(), node.first.Height(), fn );

			delete pTileBmp;
		}

		//----------------- COLOR-SAT ---

		if( dlg.m_bAlsoCaptureColorSat )
		{
			pTileBmp = GetColorSatTile( node.second->x, node.second->y, node.second->depth, TILE_RES );

			if( !pTileBmp )
			{
				LOG_R( "Failed to get satellite tile (x%d y%d zoom%d)", node.second->x, node.second->y, node.second->depth );
				++numFailedColorSatTiles;
			}
			else
			{
				outputColorSatBitmap.StretchBlit( *pTileBmp, node.first.left, node.first.top, node.first.Width(), node.first.Height() );

				delete pTileBmp;
			}
		}

		updateProgress();
	}

	progressCtrl.DestroyWindow();

	SwitchCaptureMode( false, 500.0f );

	if( numFailedAltitudeTiles == nodesToDraw.size() )
	{
		AfxMessageBox( _T( "Failed to download the elevation tiles.\nCheck your internet connection (details in log.html)." ), MB_OK | MB_ICONERROR );
		return false;
	}

	if( ( numFailedAltitudeTiles > 0 ) || ( numFailedColorSatTiles > 0 ) )
	{
		CString msg;
		msg.Format( _T( "Some tiles could not be downloaded (details in log.html):\n%u of %u elevation tiles\n%u of %u satellite tiles" ),
					numFailedAltitudeTiles, (uint32_t)nodesToDraw.size(),
					numFailedColorSatTiles, dlg.m_bAlsoCaptureColorSat ? (uint32_t)nodesToDraw.size() : 0 );

		if( numFailedColorSatTiles == nodesToDraw.size() )
			msg += _T( "\n\nAll satellite tiles failed: check your MapTiler API key (Settings > GeoData provider)." );

		AfxMessageBox( msg, MB_OK | MB_ICONWARNING );
	}

	float minAltitude, maxAltitude;
	ProcessCapturedTerrain( outputAltitudeBitmap, dlg.m_bClipAtSeaLevel, dlg.m_bUnderwater, minAltitude, maxAltitude );

	CT2A filename( dlg.m_Filename );

	if( !outputAltitudeBitmap.SaveRAW( filename.m_psz ) )
	{
		AfxMessageBox( _T( "Failed to save the elevation file." ), MB_OK | MB_ICONERROR );
		return false;
	}

	std::string colorSatFilename;

	if( dlg.m_bAlsoCaptureColorSat )
	{
		colorSatFilename = filename.m_psz;

		StripFileExtension( colorSatFilename );
		colorSatFilename += ".bmp";

		if( !outputColorSatBitmap.SaveBMP( colorSatFilename.c_str() ) )
		{
			AfxMessageBox( _T( "Failed to save the satellite image." ), MB_OK | MB_ICONERROR );
			return false;
		}
	}

	if( dlg.m_bResizeWorld )
	{
		//Web Mercator: the tile tree spans the equator's circumference, and distances shrink by cos(latitude)
		const double earthCircumference = 40075016.686; //meters
		const double mercatorY = 1.0 - 2.0 * (double)capturedCenterY / 1000.0; //+1 at the top (north), -1 at the bottom
		const double latitude = atan( sinh( XTM_PI * mercatorY ) );

		theApp.m_TerrainExtent = (uint32_t)( earthCircumference * ( capturedWorldUnits / 1000.0 ) * cos( latitude ) );
		theApp.m_MinAltitude = (int)floorf( minAltitude );
		theApp.m_MaxAltitude = (int)ceilf( maxAltitude );
		//TODO theApp.m_SeaLevel ;
		theApp.m_Resolution = resolution;

		((CMainFrame*)theApp.GetMainWnd())->m_wndPreview.OnNodesResolutionChanged();
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
		return pBitmap;

	LOG_R( "Invalid preview tile in cache (x%d y%d zoom%d), deleting it", _x, _y, _zoom );
	DeleteFile( previewCacheFilename );
	delete pBitmap;
	return nullptr;
}


Bitmap* DEMGrabView::GetAltitudeTile( int _x, int _y, int _zoom, int _resolution )
{
	CString terrariumCacheFilename, previewCacheFilename;

	if( !GetAltitudeTile( _x, _y, _zoom, _resolution, terrariumCacheFilename, previewCacheFilename ) )
		return nullptr;

	CT2A aTerrariumCacheFilename( terrariumCacheFilename );

	Bitmap* pBitmap = new Bitmap;

	if( pBitmap->Load( aTerrariumCacheFilename.m_psz ) )
		return pBitmap;

	LOG_R( "Invalid elevation tile in cache (x%d y%d zoom%d), deleting it", _x, _y, _zoom );
	DeleteFile( terrariumCacheFilename );
	delete pBitmap;
	return nullptr;
}

Bitmap* DEMGrabView::GetColorSatTile( int _x, int _y, int _zoom, int _resolution )
{
	CString colorSatCacheFilename;

	if( !GetColorSatTile( _x, _y, _zoom, _resolution, colorSatCacheFilename ) )
		return nullptr;

	CT2A aColorSatCacheFilename( colorSatCacheFilename );

	Bitmap* pBitmap = new Bitmap;

	if( pBitmap->Load( aColorSatCacheFilename.m_psz ) )
		return pBitmap;

	LOG_R( "Invalid satellite tile in cache (x%d y%d zoom%d), deleting it", _x, _y, _zoom );
	DeleteFile( colorSatCacheFilename );
	delete pBitmap;
	return nullptr;
}

bool DEMGrabView::DownloadToCache( const CString& _url, const CString& _cacheFilename, const char* _providerName, int _x, int _y, int _zoom )
{
	std::vector< byte > data;
	DWORD httpStatus;

	if( !HTTPConnection::DownloadFile( _url, data, httpStatus ) )
	{
		//Don't log the URL, it may contain an API key (braces needed: LOG_R expands to two statements)
		if( httpStatus == 0 )
		{
			LOG_R( "%s: no response for tile x%d y%d zoom%d (network error or timeout)", _providerName, _x, _y, _zoom );
		}
		else
		{
			LOG_R( "%s: HTTP %u for tile x%d y%d zoom%d", _providerName, (unsigned int)httpStatus, _x, _y, _zoom );
		}

		return false;
	}

	//Write to a temporary file first, so an interrupted write never leaves a truncated tile in the cache
	CString tmpFilename = _cacheFilename + _T( ".tmp" );

	CFile cacheFile;
	if( !cacheFile.Open( tmpFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyWrite ) )
	{
		LOG_R( "%s: failed to write tile x%d y%d zoom%d to the cache", _providerName, _x, _y, _zoom );
		return false;
	}

	cacheFile.Write( data.data(), (UINT)data.size() );
	cacheFile.Close();

	if( !MoveFileEx( tmpFilename, _cacheFilename, MOVEFILE_REPLACE_EXISTING ) )
	{
		LOG_R( "%s: failed to write tile x%d y%d zoom%d to the cache", _providerName, _x, _y, _zoom );
		DeleteFile( tmpFilename );
		return false;
	}

	return true;
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
			//Download it from AWS Terrain Tiles (256x256 tiles, no key needed)
			assert( _resolution == 256 );

			CString url;
			url.Format( s_ElevationTileURL, _zoom, _x, _y );

			if( !DownloadToCache( url, _terrariumCacheFilename, "AWS Terrain Tiles", _x, _y, _zoom ) )
				return false;
		}

		Bitmap previewBMP;
		if( !previewBMP.LoadPNG( aTerrariumCacheFilename.m_psz ) )
		{
			DeleteFile( _terrariumCacheFilename ); //corrupted, download it again next time
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
	_colorSatCacheFilename.Format( _T( "%s%s_r%d_z%d_x%d_y%d.jpg" ), m_ColorSatCacheDir, _T( "maptiler_satellite" ), _resolution, _zoom, _x, _y );

	if( !PathFileExists( _colorSatCacheFilename ) )
	{
		//Download it from MapTiler (256x256 JPEG tiles, API key needed)
		assert( _resolution == 256 );

		if( m_MapTilerAPIKey.IsEmpty() )
			return false;

		CString url;
		url.Format( s_SatelliteTileURL, _zoom, _x, _y, (LPCTSTR)m_MapTilerAPIKey );

		if( !DownloadToCache( url, _colorSatCacheFilename, "MapTiler", _x, _y, _zoom ) )
			return false;
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
