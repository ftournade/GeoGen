#include "stdafx.h"
#include "Renderer.h"

#include <dxgi1_4.h>

#include <string>
#include <vector>

namespace
{
	bool CreateSampler( ID3D11Device* _pDevice, D3D11_FILTER _filter, D3D11_TEXTURE_ADDRESS_MODE _addressMode, ID3D11SamplerState** _ppSampler )
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter			= _filter;
		samplerDesc.AddressU		= _addressMode;
		samplerDesc.AddressV		= _addressMode;
		samplerDesc.AddressW		= _addressMode;
		samplerDesc.MaxAnisotropy	= 1;
		samplerDesc.ComparisonFunc	= D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD			= 0.0f;
		samplerDesc.MaxLOD			= D3D11_FLOAT32_MAX;

		return SUCCEEDED( _pDevice->CreateSamplerState( &samplerDesc, _ppSampler ) );
	}

	bool CreateConstantColorTexture( ID3D11Device* _pDevice, uint32_t _rgba, ID3D11ShaderResourceView** _ppSRV )
	{
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width				= 1;
		texDesc.Height				= 1;
		texDesc.MipLevels			= 1;
		texDesc.ArraySize			= 1;
		texDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count	= 1;
		texDesc.Usage				= D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem		= &_rgba;
		data.SysMemPitch	= sizeof( _rgba );

		D3DObject< ID3D11Texture2D > pTex;

		if( FAILED( _pDevice->CreateTexture2D( &texDesc, &data, &pTex ) ) )
			return false;

		return SUCCEEDED( _pDevice->CreateShaderResourceView( pTex, nullptr, _ppSRV ) );
	}

	UINT GetShaderCompileFlags()
	{
	#ifdef _DEBUG
		return D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	#else
		return D3DCOMPILE_OPTIMIZATION_LEVEL3;
	#endif
	}

	void LogShaderErrors( const char* _name, const char* _entryPoint, HRESULT _hr, ID3DBlob* _pErrors )
	{
		if( _pErrors )
		{
			LOG_R( "Failed to compile shader %s (%s):\n%s", _name, _entryPoint, (const char*)_pErrors->GetBufferPointer() );
		}
		else
		{
			LOG_R( "Failed to compile shader %s (%s), hr = 0x%08x", _name, _entryPoint, (unsigned int)_hr );
		}
	}
}

Renderer::Renderer() : m_BackBufferWidth( 0 ), m_BackBufferHeight( 0 )
{
}

Renderer::~Renderer()
{
	Shutdown();
}

bool Renderer::Init( HWND _hWnd, bool _fullscreen, uint32_t _width, uint32_t _height )
{
	Shutdown();

	m_BackBufferWidth = _width;
	m_BackBufferHeight = _height;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount							= 1;
	swapChainDesc.BufferDesc.Width						= _width;
	swapChainDesc.BufferDesc.Height						= _height;
	swapChainDesc.BufferDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator		= 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator	= 1;
	swapChainDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow							= _hWnd;
	swapChainDesc.SampleDesc.Count						= 1;
	swapChainDesc.Windowed								= _fullscreen ? FALSE : TRUE;
	swapChainDesc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;

	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

	UINT createFlags = 0;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = E_FAIL;

	for( int attempt = 0; attempt < 2 && FAILED( hr ); ++attempt )
	{
		hr = D3D11CreateDeviceAndSwapChain(	nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags,
											featureLevels, _countof( featureLevels ), D3D11_SDK_VERSION,
											&swapChainDesc, &m_pSwapChain, &m_pDevice, nullptr, &m_pImmediateContext );

		createFlags &= ~D3D11_CREATE_DEVICE_DEBUG; //retry without the debug layer (graphics tools may not be installed)
	}

	if( FAILED( hr ) )
	{
		LOG_R( "D3D11CreateDeviceAndSwapChain failed, hr = 0x%08x", (unsigned int)hr );
		return false;
	}

	if( !CreateBackBufferViews() || !CreateCommonResources() )
	{
		Shutdown();
		return false;
	}

	return true;
}

void Renderer::Shutdown()
{
	if( m_pImmediateContext )
	{
		m_pImmediateContext->ClearState();
		m_pImmediateContext->Flush();
	}

	if( m_pSwapChain )
		m_pSwapChain->SetFullscreenState( FALSE, nullptr );

	m_pDefaultDepthStencilState.Release();
	m_pDefaultBlendState.Release();

	for( auto& pRasterizerState : m_pRasterizerStates )
		pRasterizerState.Release();

	m_pBlackTextureSRV.Release();
	m_pWhiteTextureSRV.Release();

	m_pBilinearMirrorSampler.Release();
	m_pBilinearClampSampler.Release();
	m_pBilinearSampler.Release();
	m_pPointClampSampler.Release();
	m_pPointSampler.Release();

	ReleaseBackBufferViews();

	m_pSwapChain.Release();
	m_pImmediateContext.Release();
	m_pDevice.Release();

	m_BackBufferWidth = m_BackBufferHeight = 0;
}

bool Renderer::ResizeSwapChain( uint32_t _width, uint32_t _height )
{
	if( !m_pSwapChain || _width == 0 || _height == 0 )
		return false;

	m_pImmediateContext->OMSetRenderTargets( 0, nullptr, nullptr );

	ReleaseBackBufferViews();

	HRESULT hr = m_pSwapChain->ResizeBuffers( 0, _width, _height, DXGI_FORMAT_UNKNOWN, 0 );

	if( FAILED( hr ) )
	{
		LOG_R( "IDXGISwapChain::ResizeBuffers failed, hr = 0x%08x", (unsigned int)hr );
		return false;
	}

	m_BackBufferWidth = _width;
	m_BackBufferHeight = _height;

	return CreateBackBufferViews();
}

void Renderer::SwapBackBuffer()
{
	if( m_pSwapChain )
		m_pSwapChain->Present( 0, 0 );
}

bool Renderer::CreateBackBufferViews()
{
	D3DObject< ID3D11Texture2D > pBackBuffer;

	if( FAILED( m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) )
		return false;

	if( FAILED( m_pDevice->CreateRenderTargetView( pBackBuffer, nullptr, &m_pBackBufferRTV ) ) )
		return false;

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width				= m_BackBufferWidth;
	depthDesc.Height			= m_BackBufferHeight;
	depthDesc.MipLevels			= 1;
	depthDesc.ArraySize			= 1;
	depthDesc.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count	= 1;
	depthDesc.Usage				= D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;

	if( FAILED( m_pDevice->CreateTexture2D( &depthDesc, nullptr, &m_pZBuffer ) ) )
		return false;

	if( FAILED( m_pDevice->CreateDepthStencilView( m_pZBuffer, nullptr, &m_pZBufferDSV ) ) )
		return false;

	D3D11_VIEWPORT viewport = {};
	viewport.Width		= (float)m_BackBufferWidth;
	viewport.Height		= (float)m_BackBufferHeight;
	viewport.MaxDepth	= 1.0f;

	m_pImmediateContext->RSSetViewports( 1, &viewport );

	return true;
}

void Renderer::ReleaseBackBufferViews()
{
	m_pZBufferDSV.Release();
	m_pZBuffer.Release();
	m_pBackBufferRTV.Release();
}

bool Renderer::CreateCommonResources()
{
	if(    !CreateSampler( m_pDevice, D3D11_FILTER_MIN_MAG_MIP_POINT,	D3D11_TEXTURE_ADDRESS_WRAP,		&m_pPointSampler )
		|| !CreateSampler( m_pDevice, D3D11_FILTER_MIN_MAG_MIP_POINT,	D3D11_TEXTURE_ADDRESS_CLAMP,	&m_pPointClampSampler )
		|| !CreateSampler( m_pDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR,	D3D11_TEXTURE_ADDRESS_WRAP,		&m_pBilinearSampler )
		|| !CreateSampler( m_pDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR,	D3D11_TEXTURE_ADDRESS_CLAMP,	&m_pBilinearClampSampler )
		|| !CreateSampler( m_pDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR,	D3D11_TEXTURE_ADDRESS_MIRROR,	&m_pBilinearMirrorSampler ) )
	{
		LOG_R( "Renderer: failed to create samplers" );
		return false;
	}

	if(    !CreateConstantColorTexture( m_pDevice, 0xffffffff, &m_pWhiteTextureSRV )
		|| !CreateConstantColorTexture( m_pDevice, 0xff000000, &m_pBlackTextureSRV ) )
	{
		LOG_R( "Renderer: failed to create constant textures" );
		return false;
	}

	const D3D11_CULL_MODE cullModes[] = { D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK };

	for( int i = 0; i < _countof( cullModes ); ++i )
	{
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode			= D3D11_FILL_SOLID;
		rasterizerDesc.CullMode			= cullModes[ i ];
		rasterizerDesc.DepthClipEnable	= TRUE;

		if( FAILED( m_pDevice->CreateRasterizerState( &rasterizerDesc, &m_pRasterizerStates[ cullModes[ i ] - 1 ] ) ) )
		{
			LOG_R( "Renderer: failed to create rasterizer states" );
			return false;
		}
	}

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable	= TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc		= D3D11_COMPARISON_LESS;

	if(    FAILED( m_pDevice->CreateBlendState( &blendDesc, &m_pDefaultBlendState ) )
		|| FAILED( m_pDevice->CreateDepthStencilState( &depthStencilDesc, &m_pDefaultDepthStencilState ) ) )
	{
		LOG_R( "Renderer: failed to create default render states" );
		return false;
	}

	return true;
}

bool Renderer::GetMemoryStatistics( uint32_t& _freeMemoryKB, uint32_t& _totalMemoryKB ) const
{
	_freeMemoryKB = _totalMemoryKB = 0;

	if( !m_pDevice )
		return false;

	D3DObject< IDXGIDevice > pDXGIDevice;
	D3DObject< IDXGIAdapter > pAdapter;
	D3DObject< IDXGIAdapter3 > pAdapter3;

	if(    FAILED( m_pDevice->QueryInterface( __uuidof( IDXGIDevice ), (void**)&pDXGIDevice ) )
		|| FAILED( pDXGIDevice->GetAdapter( &pAdapter ) )
		|| FAILED( pAdapter->QueryInterface( __uuidof( IDXGIAdapter3 ), (void**)&pAdapter3 ) ) )
		return false;

	DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo;

	if( FAILED( pAdapter3->QueryVideoMemoryInfo( 0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo ) ) )
		return false;

	DXGI_ADAPTER_DESC adapterDesc;

	if( FAILED( pAdapter->GetDesc( &adapterDesc ) ) )
		return false;

	const UINT64 freeMemory = memoryInfo.Budget > memoryInfo.CurrentUsage ? memoryInfo.Budget - memoryInfo.CurrentUsage : 0;

	_freeMemoryKB	= (uint32_t)( freeMemory / 1024 );
	_totalMemoryKB	= (uint32_t)( adapterDesc.DedicatedVideoMemory / 1024 );

	return true;
}

void Renderer::SetDefaultRasterizerState( D3D11_CULL_MODE _cullMode )
{
	m_pImmediateContext->RSSetState( m_pRasterizerStates[ _cullMode - 1 ] );
}

void Renderer::SetDefaultBlendState()
{
	m_pImmediateContext->OMSetBlendState( m_pDefaultBlendState, nullptr, 0xffffffff );
}

void Renderer::SetDefaultDepthStencilState()
{
	m_pImmediateContext->OMSetDepthStencilState( m_pDefaultDepthStencilState, 0 );
}

//Shaders
/////////

ID3DBlob* Renderer::CompileShaderFromFile( const char* _filename, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, const char* _target )
{
	const int len = MultiByteToWideChar( CP_UTF8, 0, _filename, -1, nullptr, 0 );
	std::vector< wchar_t > wideFilename( len > 0 ? len : 1, L'\0' );
	MultiByteToWideChar( CP_UTF8, 0, _filename, -1, wideFilename.data(), len );

	ID3DBlob* pBlob = nullptr;
	D3DObject< ID3DBlob > pErrors;

	HRESULT hr = D3DCompileFromFile(	wideFilename.data(), _defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
										_entryPoint, _target, GetShaderCompileFlags(), 0, &pBlob, &pErrors );

	if( FAILED( hr ) )
	{
		LogShaderErrors( _filename, _entryPoint, hr, pErrors );
		return nullptr;
	}

	return pBlob;
}

ID3DBlob* Renderer::CompileShaderFromMemory( const char* _source, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, const char* _target )
{
	ID3DBlob* pBlob = nullptr;
	D3DObject< ID3DBlob > pErrors;

	//No source name: relative #includes are resolved from the current directory
	HRESULT hr = D3DCompile(	_source, strlen( _source ), nullptr, _defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
								_entryPoint, _target, GetShaderCompileFlags(), 0, &pBlob, &pErrors );

	if( FAILED( hr ) )
	{
		LogShaderErrors( "<memory>", _entryPoint, hr, pErrors );
		return nullptr;
	}

	return pBlob;
}

bool Renderer::CreateShaderObject( ID3DBlob* _pBlob, ID3D11ComputeShader** _ppShader )
{
	return SUCCEEDED( m_pDevice->CreateComputeShader( _pBlob->GetBufferPointer(), _pBlob->GetBufferSize(), nullptr, _ppShader ) );
}

bool Renderer::CreateShaderObject( ID3DBlob* _pBlob, ID3D11VertexShader** _ppShader )
{
	return SUCCEEDED( m_pDevice->CreateVertexShader( _pBlob->GetBufferPointer(), _pBlob->GetBufferSize(), nullptr, _ppShader ) );
}

bool Renderer::CreateShaderObject( ID3DBlob* _pBlob, ID3D11PixelShader** _ppShader )
{
	return SUCCEEDED( m_pDevice->CreatePixelShader( _pBlob->GetBufferPointer(), _pBlob->GetBufferSize(), nullptr, _ppShader ) );
}

template <class ShaderType>
bool Renderer::FinalizeShader( ID3DBlob* _pBlob, ShaderType** _ppShader, ID3DBlob** _ppCompiledShader )
{
	if( !_pBlob )
		return false;

	const bool bOk = CreateShaderObject( _pBlob, _ppShader );

	if( bOk && _ppCompiledShader )
	{
		*_ppCompiledShader = _pBlob; //ownership goes to the caller
	}
	else
	{
		_pBlob->Release();
	}

	return bOk;
}

bool Renderer::CreateShader( const char* _filename, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11ComputeShader** _ppShader, ID3DBlob** _ppCompiledShader )
{
	return FinalizeShader( CompileShaderFromFile( _filename, _entryPoint, _defines, "cs_5_0" ), _ppShader, _ppCompiledShader );
}

bool Renderer::CreateShader( const char* _filename, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11VertexShader** _ppShader, ID3DBlob** _ppCompiledShader )
{
	return FinalizeShader( CompileShaderFromFile( _filename, _entryPoint, _defines, "vs_5_0" ), _ppShader, _ppCompiledShader );
}

bool Renderer::CreateShader( const char* _filename, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11PixelShader** _ppShader, ID3DBlob** _ppCompiledShader )
{
	return FinalizeShader( CompileShaderFromFile( _filename, _entryPoint, _defines, "ps_5_0" ), _ppShader, _ppCompiledShader );
}

bool Renderer::CreateShaderFromMemory( const char* _source, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11ComputeShader** _ppShader, ID3DBlob** _ppCompiledShader )
{
	return FinalizeShader( CompileShaderFromMemory( _source, _entryPoint, _defines, "cs_5_0" ), _ppShader, _ppCompiledShader );
}

bool Renderer::CreateShaderFromMemory( const char* _source, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11VertexShader** _ppShader, ID3DBlob** _ppCompiledShader )
{
	return FinalizeShader( CompileShaderFromMemory( _source, _entryPoint, _defines, "vs_5_0" ), _ppShader, _ppCompiledShader );
}

bool Renderer::CreateShaderFromMemory( const char* _source, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11PixelShader** _ppShader, ID3DBlob** _ppCompiledShader )
{
	return FinalizeShader( CompileShaderFromMemory( _source, _entryPoint, _defines, "ps_5_0" ), _ppShader, _ppCompiledShader );
}
