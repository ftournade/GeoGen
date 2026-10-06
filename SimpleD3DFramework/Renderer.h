#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdint.h>

#include "D3DObject.h"
#include "ConstantBuffer.h"

//Off-screen render target. GeoGen only stores RenderTarget pointers (never dereferenced), so
//only the declaration is recreated.
class RenderTarget;

//Minimal Direct3D 11 renderer: device, swap chain (with depth buffer), common samplers,
//default render states, 1x1 white/black textures and HLSL shader compilation helpers.
class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Init( HWND _hWnd, bool _fullscreen, uint32_t _width, uint32_t _height );
	void Shutdown();

	//Swap chain

	bool ResizeSwapChain( uint32_t _width, uint32_t _height );
	void SwapBackBuffer();

	inline uint32_t GetBackBufferWidth() const					{ return m_BackBufferWidth; }
	inline uint32_t GetBackBufferHeight() const					{ return m_BackBufferHeight; }

	inline ID3D11RenderTargetView* GetBackBufferRTV() const		{ return m_pBackBufferRTV; }
	inline ID3D11DepthStencilView* GetZBufferDSV() const		{ return m_pZBufferDSV; }

	//Device

	inline ID3D11Device*		GetDevice() const					{ return m_pDevice; }
	inline ID3D11DeviceContext*	GetImmediateDeviceContext() const	{ return m_pImmediateContext; }

	bool GetMemoryStatistics( uint32_t& _freeMemoryKB, uint32_t& _totalMemoryKB ) const;

	//Common resources

	inline ID3D11SamplerState* GetPointSampler() const			{ return m_pPointSampler; }
	inline ID3D11SamplerState* GetPointClampSampler() const		{ return m_pPointClampSampler; }
	inline ID3D11SamplerState* GetBilinearSampler() const		{ return m_pBilinearSampler; }
	inline ID3D11SamplerState* GetBilinearClampSampler() const	{ return m_pBilinearClampSampler; }
	inline ID3D11SamplerState* GetBilinearMirrorSampler() const	{ return m_pBilinearMirrorSampler; }

	inline ID3D11ShaderResourceView* GetWhiteTexture() const	{ return m_pWhiteTextureSRV; }
	inline ID3D11ShaderResourceView* GetBlackTexture() const	{ return m_pBlackTextureSRV; }

	//Default render states

	void SetDefaultRasterizerState( D3D11_CULL_MODE _cullMode = D3D11_CULL_BACK );
	void SetDefaultBlendState();
	void SetDefaultDepthStencilState();

	//Shaders (shader model 5.0). _ppCompiledShader optionally receives the bytecode (caller releases it)

	bool CreateShader( const char* _filename, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11ComputeShader** _ppShader, ID3DBlob** _ppCompiledShader = nullptr );
	bool CreateShader( const char* _filename, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11VertexShader** _ppShader, ID3DBlob** _ppCompiledShader = nullptr );
	bool CreateShader( const char* _filename, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11PixelShader** _ppShader, ID3DBlob** _ppCompiledShader = nullptr );

	bool CreateShaderFromMemory( const char* _source, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11ComputeShader** _ppShader, ID3DBlob** _ppCompiledShader = nullptr );
	bool CreateShaderFromMemory( const char* _source, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11VertexShader** _ppShader, ID3DBlob** _ppCompiledShader = nullptr );
	bool CreateShaderFromMemory( const char* _source, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, ID3D11PixelShader** _ppShader, ID3DBlob** _ppCompiledShader = nullptr );

	//RenderDoc integration (not available: these are no-ops)

	void RenderDoc_StartCapture() {}
	void RenderDoc_EndCapture() {}
	void RenderDoc_CaptureMultipleFrames( uint32_t _numFrames ) {}

private:
	bool CreateBackBufferViews();
	void ReleaseBackBufferViews();
	bool CreateCommonResources();

	ID3DBlob* CompileShaderFromFile( const char* _filename, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, const char* _target );
	ID3DBlob* CompileShaderFromMemory( const char* _source, const char* _entryPoint, const D3D_SHADER_MACRO* _defines, const char* _target );

	bool CreateShaderObject( ID3DBlob* _pBlob, ID3D11ComputeShader** _ppShader );
	bool CreateShaderObject( ID3DBlob* _pBlob, ID3D11VertexShader** _ppShader );
	bool CreateShaderObject( ID3DBlob* _pBlob, ID3D11PixelShader** _ppShader );

	template <class ShaderType>
	bool FinalizeShader( ID3DBlob* _pBlob, ShaderType** _ppShader, ID3DBlob** _ppCompiledShader );

private:
	D3DObject< ID3D11Device >				m_pDevice;
	D3DObject< ID3D11DeviceContext >		m_pImmediateContext;
	D3DObject< IDXGISwapChain >				m_pSwapChain;

	D3DObject< ID3D11RenderTargetView >		m_pBackBufferRTV;
	D3DObject< ID3D11Texture2D >			m_pZBuffer;
	D3DObject< ID3D11DepthStencilView >		m_pZBufferDSV;

	uint32_t m_BackBufferWidth, m_BackBufferHeight;

	D3DObject< ID3D11SamplerState >			m_pPointSampler;
	D3DObject< ID3D11SamplerState >			m_pPointClampSampler;
	D3DObject< ID3D11SamplerState >			m_pBilinearSampler;
	D3DObject< ID3D11SamplerState >			m_pBilinearClampSampler;
	D3DObject< ID3D11SamplerState >			m_pBilinearMirrorSampler;

	D3DObject< ID3D11ShaderResourceView >	m_pWhiteTextureSRV;
	D3DObject< ID3D11ShaderResourceView >	m_pBlackTextureSRV;

	D3DObject< ID3D11RasterizerState >		m_pRasterizerStates[ 3 ]; //indexed by D3D11_CULL_MODE - 1
	D3DObject< ID3D11BlendState >			m_pDefaultBlendState;
	D3DObject< ID3D11DepthStencilState >	m_pDefaultDepthStencilState;
};
