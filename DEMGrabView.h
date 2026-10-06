#pragma once

#include <afxmt.h>

class DEMGrabView : public CWnd
{
// Construction
public:
	DEMGrabView();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~DEMGrabView();

	// Generated message map functions
protected:

	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg void OnDownloadDem();

private:
	struct TileNode
	{
		TileNode() : pBitmap( nullptr ) {
			Childs[ 0 ] = nullptr;  Childs[ 1 ] = nullptr; Childs[ 2 ] = nullptr; Childs[ 3 ] = nullptr;
		}

		~TileNode()
		{
			if( Childs[ 0 ] ) delete Childs[ 0 ];
			if( Childs[ 1 ] ) delete Childs[ 1 ];
			if( Childs[ 2 ] ) delete Childs[ 2 ];
			if( Childs[ 3 ] ) delete Childs[ 3 ];

			delete pBitmap;
		}

		inline bool IsLoaded() const { return pBitmap != nullptr; }

		Vec2 TopLeft, BottomRight;
		int x, y, depth;
		CBitmap* pBitmap;

		TileNode* Childs[ 4 ];
	};

	BOOL Init();
	void CreateTileNode( TileNode* _pParent, uint32_t _childIndex );
	void AsyncLoadTileBitmap( TileNode* _pTileNode );

	void StreamTiles(); //streaming thread main loop
	void WaitForStreamingIdle(); //clears the streaming queue and waits for the tile in flight (if any)
	void StopStreamingThread();

	bool DownloadToCache( const CString& _url, const CString& _cacheFilename, const char* _providerName, int _x, int _y, int _zoom );

	void DrawTile( CDC* _pDC, const CRect& _tileRect, TileNode* _pTileNode ) const;

	void RenderTiles( CDC* _pDC, const CRect& _screenRect, float _scale, TileNode* _pTileNode,
						bool _bCaptureMode, bool _AllowDraw = true, 
						vector< pair< CRect, TileNode* > >* _pNodesToDraw = nullptr ); //recursive quadtree rendering (updates streaming queue)
	
	bool			GetAltitudeTile( int _x, int _y, int _zoom, int _resolution, CString& _terrariumCacheFilename, CString& _previewCacheFilename );
	bool			GetColorSatTile( int _x, int _y, int _zoom, int _resolution, CString& _colorSatCacheFilename );
	CBitmap*		GetPreviewTile( int _x, int _y, int _zoom, int _resolution );
	Bitmap*	GetAltitudeTile( int _x, int _y, int _zoom, int _resolution );
	Bitmap*	GetColorSatTile( int _x, int _y, int _zoom, int _resolution );
	
	void ProcessPreviewTile( Bitmap& _bmp );

	bool CaptureTerrain();
	void SwitchCaptureMode( bool _onOff, float _halfExtent );
	void ProcessCapturedTerrain( const Bitmap& _bmp, bool _clipAtSeaLevel, bool _underwater, float& _minAltitude, float& _maxAltitude );

private:

	friend UINT TileStreamingThread( void* _param );
	
	Win32BackBuffer m_backBuffer;

	CString m_DEMCacheDir;
	CString m_ColorSatCacheDir;

	CString m_MapTilerAPIKey; //satellite imagery (elevation tiles need no key)

	bool m_bLeftMouseButtonDown;
	Vec2 m_LastMousePos;
	Vec2 m_ViewCenter;
	float m_Zoom;

	CWinThread* m_pStreamingThread;
	CEvent m_StreamingWakeEvent;	//auto-reset, set when tiles are queued or the thread must stop
	CEvent m_StreamingIdleEvent;	//manual-reset, set while no tile is being downloaded
	bool m_bStopStreaming;			//protected by m_StreamingQueueCriticalSection

	vector< TileNode* > m_StreamingQueue;
	CCriticalSection m_StreamingQueueCriticalSection; //also protects TileNode::pBitmap

	CRect m_CaptureRect;
	TileNode* m_pRootNode;
	
	enum PreviewMode
	{
		PreviewMode_Altitude,
		PreviewMode_ColorSat,
		PreviewMode_Map,
	};
		
	PreviewMode m_PreviewMode;
};

