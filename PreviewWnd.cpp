
#include "stdafx.h"

#include "PreviewWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "GeoGen.h"

#include "NodeEditor.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CPreviewWnd::CPreviewWnd() :
	m_bRotatingView( false ),
	m_CameraU( 2.34f ),
	m_CameraV( -0.8f )
{
	Mat44d viewMatrix;
#if 1
	viewMatrix.MakeLookAt( Vec3d( 10000.0f, 10000.0f, 10000.0f ), Vec3d::Origin, Vec3d::YAxis );
#else
	//TODO fix this code
	viewMatrix.MakeRotationYawPitchRoll( m_CameraU, m_CameraV, 0.0f );
	viewMatrix.SetTranslation( -Vec3d( 1000.0f, 1000.0f, 1000.0f ) );
	viewMatrix.Inverse();
#endif
	m_Camera.SetViewMatrix( viewMatrix );
}

CPreviewWnd::~CPreviewWnd()
{
}

BEGIN_MESSAGE_MAP( CPreviewWnd, CDockablePane )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


void CPreviewWnd::InitRendererOrResizeSwapChainIfNeeded()
{
	CRect r;
	GetClientRect( &r );

	if( r.IsRectEmpty() )
		return;

	if( !g_Renderer.GetDevice() )
	{
		LOG( "Initializing D3D11" );

		if( !g_Renderer.Init( GetSafeHwnd(), false, r.Width(), r.Height() ) )
		{
			//TODO display error & exit
			LOG_R( "Failed" );

			return;
		}

		LOG_G( "OK" );
		
		InitResolutionDependentD3DStuff();
		InitResolutionIndependentD3DStuff();

		g_NodeEditor.Init();
	}
	else if( (r.Width() != g_Renderer.GetBackBufferWidth()) ||
		(r.Height() != g_Renderer.GetBackBufferHeight()) )
	{
		LOG( "Resizing D3D11 swap chain" );

		g_Renderer.ResizeSwapChain( r.Width(), r.Height() );
		InitResolutionDependentD3DStuff();

		LOG_G( "OK" );

	}

	float aspectRatio = (float)g_Renderer.GetBackBufferWidth() / (float)g_Renderer.GetBackBufferHeight();
	m_Camera.SetProjectionMatrix( theApp.m_CamFOV, aspectRatio, theApp.m_CamNearClip, theApp.m_CamFarClip );
}

bool CPreviewWnd::InitResolutionIndependentD3DStuff()
{
	D3D_SHADER_MACRO defines[] =
	{
		{ NULL, NULL }
	};

	ID3DBlob* pCompiledShaderTerrain;

	LOG( "Loading preview shaders" );

	if( !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "VSMain", defines, &m_pTerrainVertexShader, &pCompiledShaderTerrain ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "PSMain", defines, &m_pTerrainPixelShader ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "VSMainRGB", defines, &m_pRGBVertexShader ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "PSMainRGB", defines, &m_pRGBPixelShader ) )
		return false;
	
	LOG_G( "OK" );

	LOG( "Creating preview grid-mesh" );

	if( !m_GridMesh.Init( g_Renderer, pCompiledShaderTerrain ) )
		return false;

	pCompiledShaderTerrain->Release();
	

	LOG_G( "OK" );


	if( !m_TerrainCB.Init( g_Renderer.GetDevice() ) )
		return false;


	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width = 1;
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	uint32_t white = 0xFFFFFFFF;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &white;
	data.SysMemPitch = 4;
	data.SysMemSlicePitch = 4;

	if( FAILED( g_Renderer.GetDevice()->CreateTexture2D( &texDesc, &data, &m_pWhiteTex ) ) )
		return false;

	/////////////////////////

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	if( FAILED( g_Renderer.GetDevice()->CreateShaderResourceView( m_pWhiteTex, &srvDesc, &m_pWhiteTexSRV ) ) )
		return false;

	return OnNodesResolutionChanged();
}

bool CPreviewWnd::InitResolutionDependentD3DStuff()
{


	//TODO full screen render targets for PostFX ?!
	return true;
}

bool CPreviewWnd::OnNodesResolutionChanged()
{
	if( !m_GridMesh.CreateGrid( g_Renderer.GetDevice(), theApp.GetResolution() ) )
		return false;

	return true;
}
void CPreviewWnd::OnLButtonDown( UINT nFlags, CPoint point )
{
	SetFocus();

	CRect r;
	GetClientRect( &r );

	if( r.PtInRect( point ) )
	{
		m_bRotatingView = true;
	}
	else
	{
		CDockablePane::OnLButtonDown( nFlags, point );
	}
}


void CPreviewWnd::OnLButtonUp( UINT nFlags, CPoint point )
{
	CRect r;
	GetClientRect( &r );

	if( r.PtInRect( point ) )
	{
		m_bRotatingView = false;
	}
	else
	{
		CDockablePane::OnLButtonDown( nFlags, point );
	}

	CDockablePane::OnLButtonUp( nFlags, point );
}


void CPreviewWnd::OnMouseMove( UINT nFlags, CPoint point )
{
	const float mouseSpeed = 0.005f;
	Vec2 mousePos( (float)point.x, (float)point.y );
	Vec2 mouseMovement = (mousePos - m_LastMousePos) * mouseSpeed;

	if( m_bRotatingView )
	{
		m_CameraU += mouseMovement.x;
		m_CameraV -= mouseMovement.y;

		m_CameraV = Clamp( m_CameraV, -XTM_PI * 0.49f, XTM_PI * 0.49f );

		Mat44d v;
		v.MakeRotationYawPitchRoll( m_CameraU, m_CameraV, 0.0f );
		v.SetTranslation( m_Camera.GetPosition() );
		v.Inverse();
		m_Camera.SetViewMatrix( v );
		//	g_Cam.Rotate( mouseMovement.x, mouseMovement.y );

		Invalidate( FALSE );
	}

	m_LastMousePos = mousePos;

	CDockablePane::OnMouseMove( nFlags, point );
}

void CPreviewWnd::MoveCamera( const Vec3& _dir )
{
	float camSpeed = 500.0f;

	if( GetAsyncKeyState( VK_LSHIFT ) != 0 )
		camSpeed *= 10.0f;

	Mat44d v = m_Camera.GetViewMatrix();
	Vec3d camPos = m_Camera.GetPosition() + ToVec3d( _dir * camSpeed );

	v.Inverse();
	v.SetTranslation( camPos );
	v.Inverse();

	m_Camera.SetViewMatrix( v );
	Invalidate( FALSE );
}

void CPreviewWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) //TODO never called ... WTF ?!
{
	switch( nChar )
	{
		case 'Z': MoveCamera( m_Camera.GetForwardDirection() ); break;
		case 'S': MoveCamera( -m_Camera.GetForwardDirection() ); break;
		case 'Q': MoveCamera( m_Camera.GetRightDirection() ); break;
		case 'D': MoveCamera( -m_Camera.GetRightDirection() ); break;
		case 'W': MoveCamera( Vec3::YAxis ); break;
		case 'X': MoveCamera( -Vec3::YAxis ); break;
	}

	CDockablePane::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CPreviewWnd::OnPaint()
{
	CPaintDC dc( this );

	InitRendererOrResizeSwapChainIfNeeded();

	shared_ptr<ComputeNode> pIterativeSimNode = m_pIterativeSimNode.lock();
	
	if( pIterativeSimNode )
	{
		pIterativeSimNode->StepSim( true, true );

		Render();
		Invalidate( FALSE );
	}
	else
	{
		Render();
	}

#if 0//def DBG_RENDERDOC
	Invalidate( FALSE ); //for renderdoc
#endif
}

void CPreviewWnd::Render()
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	//Get inputs (potentially triggers nodes computation)

	shared_ptr<ComputeNode> pIterativeSimNode = m_pIterativeSimNode.lock();

	ID3D11ShaderResourceView* heightSRV = nullptr;
	ID3D11ShaderResourceView* normalSRV = nullptr;
	ID3D11ShaderResourceView* albedoSRV = nullptr;
	ID3D11ShaderResourceView* waterSRV =  nullptr;

	if( !pIterativeSimNode )
	{
		//WArning these might trigger computes and disrupt previously set D3D11 states, that's why we do this early in the function
		heightSRV = g_NodeEditor.GetPreviewHeight();
		normalSRV = g_NodeEditor.GetPreviewNormal();
		albedoSRV = g_NodeEditor.GetPreviewAlbedo();
		waterSRV =  g_NodeEditor.GetPreviewWater();
	}

	ID3D11RenderTargetView* pBackBufferRTV = g_Renderer.GetBackBufferRTV();

	//	FLOAT clearColor[] = { Random(), Random(), Random(), 1.0f };
	FLOAT clearColor[] = { 0.5f, 0.5f, 0.95f, 1.0f };
	pDevCtx->ClearRenderTargetView( pBackBufferRTV, clearColor );
	pDevCtx->ClearDepthStencilView( g_Renderer.GetZBufferDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
	pDevCtx->OMSetRenderTargets( 1, &pBackBufferRTV, g_Renderer.GetZBufferDSV() );

	g_Renderer.SetDefaultRasterizerState( D3D11_CULL_BACK );
	g_Renderer.SetDefaultBlendState();
	g_Renderer.SetDefaultDepthStencilState();
	

	CRect r;
	GetClientRect( &r );

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)r.Width();
	viewport.Height = (float)r.Height();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	pDevCtx->RSSetViewports( 1, &viewport );

	//Update constant buffer

	m_TerrainCB.c_WorldViewMatrix = ToMat44( m_Camera.GetViewMatrix() );
	m_TerrainCB.c_WorldViewMatrix.Transpose();
	m_TerrainCB.c_WorldViewProjMatrix = ToMat44( m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix() );
	m_TerrainCB.c_WorldViewProjMatrix.Transpose();
	m_TerrainCB.c_TerrainExtent = theApp.GetTerrainExtent();
	m_TerrainCB.c_TerrainResolution = theApp.GetResolution();
	m_TerrainCB.c_MinAltitude = theApp.GetMinAltitude();
	m_TerrainCB.c_MaxAltitude = theApp.GetMaxAltitude();

	m_TerrainCB.UploadToGPU( pDevCtx );

	ID3D11Buffer* pCB = m_TerrainCB.GetBuffer();

	pDevCtx->VSSetConstantBuffers( 0, 1, &pCB );
	pDevCtx->PSSetConstantBuffers( 0, 1, &pCB );

	
	if( pIterativeSimNode && pIterativeSimNode->RenderSimPreview( m_GridMesh ) )
	{
	}
	else
	{
		
		shared_ptr<ComputeNode> pPreviewedNode = g_NodeEditor.GetPreviewedNode().lock();

		bool bPreviewAsHeightField = pPreviewedNode && pPreviewedNode->GetPreviewMode();

		if( bPreviewAsHeightField && !g_NodeEditor.IsPreviewingColorMap() )
		{
			//Heightmap (with or without albedo)

			if( !albedoSRV )
				albedoSRV = m_pWhiteTexSRV;

			pDevCtx->VSSetShader( m_pTerrainVertexShader, nullptr, 0 );
			pDevCtx->PSSetShader( m_pTerrainPixelShader, nullptr, 0 );

			pDevCtx->VSSetShaderResources( 0, 1, &heightSRV );
			pDevCtx->PSSetShaderResources( 1, 1, &normalSRV );
			pDevCtx->PSSetShaderResources( 2, 1, &albedoSRV );
			pDevCtx->PSSetShaderResources( 3, 1, &waterSRV );
		}
		else
		{
			//Color map
			
			pDevCtx->VSSetShader( m_pRGBVertexShader, nullptr, 0 );
			pDevCtx->PSSetShader( m_pRGBPixelShader, nullptr, 0 );

			pDevCtx->PSSetShaderResources( 2, 1, &heightSRV ); //yeah i know ... color is in height ...
		}

		//Render

		ID3D11SamplerState* sampler = g_Renderer.GetBilinearClampSampler();
		pDevCtx->VSSetSamplers( 0, 1, &sampler );
		pDevCtx->PSSetSamplers( 0, 1, &sampler );

		m_GridMesh.Render( pDevCtx );
	}

	static ID3D11ShaderResourceView* nullSRVs[ 8 ] = { NULL };
	pDevCtx->VSSetShaderResources( 0, 8, nullSRVs );
	pDevCtx->PSSetShaderResources( 0, 8, nullSRVs );

	g_Renderer.SwapBackBuffer();
}

void CPreviewWnd::StartIterativeSim( std::shared_ptr<ComputeNode> _pIterativeSimNode )
{
#if 0//def USE_RENDERDOC
	g_Renderer.RenderDoc_CaptureMultipleFrames( 10 );
	g_Renderer.SwapBackBuffer();
#endif

	m_pIterativeSimNode = _pIterativeSimNode;
	m_pIterativeSimNode.lock()->InitSim();

	Invalidate( FALSE );
}

void CPreviewWnd::StopIterativeSim()
{
	m_pIterativeSimNode.reset();
}