#include "stdafx.h"
#include "ScatterMapNode.h"

#include <Core/Mat33.h>

ScatterMapNode::ScatterMapNode() :
	m_SplatCount(0)
{
	SetUIName( "Scatter Map" );

	AddInput( "Source", IOType::FloatOrColor );
	AddInput( "Mask", IOType::Float ); //TODO optional ? option to use Source.a ?
	//TODO AddInput( "Density", IOType::Float );

	AddOutput( "Output", IOType::FloatOrColor );
	
	m_ParamBlendMode = AddParam( "Main", "BlendMode", IOType::Integer, ParamEdition::ComboBox, 3, 0, 3, true );
	ParamSlot* pParam = &m_ParameterSlots[ m_ParamBlendMode ];
	pParam->AddEnum( "Alpha blend (Not yet working)" );
	pParam->AddEnum( "Additive" );
	pParam->AddEnum( "Min (Not yet working)" );
	pParam->AddEnum( "Max" );

	m_ParamCount = AddParam( "Main", "Count", IOType::Integer, ParamEdition::Slider, 50, 1, 1000 );

	m_ParamDistribution = AddParam( "Main", "Distribution", IOType::Integer, ParamEdition::ComboBox, 1, 0, 2, true );
	pParam = &m_ParameterSlots[ m_ParamDistribution ];
	pParam->AddEnum( "Random" );
	pParam->AddEnum( "Jittered grid" );
	pParam->AddEnum( "Poisson (Not yet working)" );

	m_ParamRandomizePosition	= AddParam( "Main", "RandomizePosition", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f );

	m_ParamAspectRatio			= AddParam( "Transform", "Aspect Ratio", IOType::Float, ParamEdition::Slider, 1.0f, 0.0001f, 100.0f );

	m_ParamScale				= AddParam( "Transform", "Scale", IOType::Float, ParamEdition::Slider, 0.1f, 0.0f, 0.4f );
	m_ParamRandomizeScale		= AddParam( "Transform", "Randomize Scale", IOType::Float, ParamEdition::Slider, 0.05f, 0.0f, 0.4f );

	m_ParamRotation				= AddParam( "Transform", "Rotation", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 360.0f );
	m_ParamRandomizeRotation	= AddParam( "Transform", "Randomize Rotation", IOType::Float, ParamEdition::Slider, 360.0f, 0.0f, 360.0f );

	m_ParamIntensity			= AddParam( "Transform", "Intensity", IOType::Float, ParamEdition::Slider, 1.0f, 0.0f, 2.0f );
	m_ParamRandomizeIntensity	= AddParam( "Transform", "Randomize Intensity", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 2.0f );

	m_ParamScaleIntensityBySize = AddParam( "Transform", "Scale Intensity By Size", IOType::Bool, ParamEdition::CheckBox, true, false );

	m_ParamSeed					= AddParam( "Main", "Seed", IOType::Integer, ParamEdition::Slider, 69280, 0, 100000 );
}


ScatterMapNode::~ScatterMapNode()
{
}

bool ScatterMapNode::OneTimeInit()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;

	g_Renderer.GetDevice()->CreateRasterizerState( &rasterizerDesc, &m_RasterizerState );

	//////////////////////////////

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	depthStencilStateDesc.DepthEnable = FALSE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilStateDesc.StencilEnable = FALSE;
	depthStencilStateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilStateDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	g_Renderer.GetDevice()->CreateDepthStencilState( &depthStencilStateDesc, &m_DepthStencilState );

	return true;
}


bool ScatterMapNode::OnResolutionChanged()
{
	if( !m_Output.Init( GetResolution(), IOType::Float, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET ) )
		return false;

	return true;
}

void ScatterMapNode::OnInputConnectionChanged( int _slot )
{
/*	const Map* input0 = GetRemoteInputMap( 0 );

	if( !input0 )
		return;

	if( input0->GetFormat() == DXGI_FORMAT_R32_FLOAT )
		SetOutputFormat( 0, DXGI_FORMAT_R32_FLOAT );
	else
		SetOutputFormat( 0, DXGI_FORMAT_R16G16B16A16_FLOAT );

	InvalidateShader();
*/
}

const Map* ScatterMapNode::GetOutput( u32 _idx ) const
{
	return &m_Output;
}


bool ScatterMapNode::CompileShaders()
{
	static const char* strShader =
		"Texture2D<%s>		_inputMap : register(t0);\n"
		"Texture2D<float>	_inputMask : register(t1);\n"

		"SamplerState		_bilinearSampler : register(s0);\n"

		"struct VSInput\n"
		"{\n"
		"	float2 Pos : POSITION;\n"
		"	float2 UV : TEXCOORD;\n"
		"	float  Intensity : INTENSITY;\n"
		"};\n"

		"struct VSOutput\n"
		"{\n"
		"	float4 Pos : SV_POSITION;\n"
		"	float2 UV : TEXCOORD;\n"
		"	float  Intensity : INTENSITY;\n"
		"};\n"

		"VSOutput VS( VSInput _input )\n"
		"{\n"
		"	VSOutput output;\n"
		"	output.Pos = float4( _input.Pos * 2.0f - 1.0f, 0.0f, 1.0f );\n"
		"	output.UV = _input.UV;\n"
		"	output.Intensity = _input.Intensity;\n"
		"	return output;\n"
		"}\n"

		"%s PS( VSOutput _input ) : SV_TARGET\n"
		"{\n"
		"	%s map = _inputMap.SampleLevel( _bilinearSampler, _input.UV, 0.0f );\n" //TODO mipmapping ?
		"	float mask = _inputMask.SampleLevel( _bilinearSampler, _input.UV, 0.0f );\n" //TODO mipmapping ?
/*
		"	const int blendType = %d;\n"

		"	switch( blendType )\n"
		"	{\n"
		"	case 0:\n"
		"		break;\n"
		"	}\n"
*/
		"	return map * mask * _input.Intensity;\n"
		"}\n";

	bool bIsColorMode = ( GetRemoteInputMap( 0 )->GetFormat() != DXGI_FORMAT_R32_FLOAT );

	const char* dataType = bIsColorMode ? "float4" : "float";
	int blendType = m_ParameterSlots[ m_ParamBlendMode ].m_Value.i;

	Str shaderSource( Format( strShader, dataType, dataType, dataType, blendType ) );

	ID3DBlob* pCompiledVS;

	if( !g_Renderer.CreateShaderFromMemory( shaderSource.c_str(), "PS", nullptr, &m_PS ) )
		return false;

	if( !g_Renderer.CreateShaderFromMemory( shaderSource.c_str(), "VS", nullptr, &m_VS, &pCompiledVS ) )
		return false;

	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INTENSITY", 0, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if( !g_Renderer.GetDevice()->CreateInputLayout( inputLayoutDesc, countof( inputLayoutDesc ), pCompiledVS->GetBufferPointer(), pCompiledVS->GetBufferSize(), &m_InputLayout ) )
		return false;

	pCompiledVS->Release();

	return true;
}

bool ScatterMapNode::UpdateVertexBuffer()
{
	m_SplatCount = m_ParameterSlots[ m_ParamCount ].m_Value.i;

	m_VB.Release();

	struct Vertex
	{
		Vec2 Pos, UV;
		float Intensity;
	};


	int distributionMethod = m_ParameterSlots[ m_ParamDistribution ].m_Value.i;
	float positionRndParam = m_ParameterSlots[ m_ParamRandomizePosition ].m_Value.f; //aka jitter (only used for jittered grid)
	
	const float aspectRatioParam = m_ParameterSlots[ m_ParamAspectRatio ].m_Value.f;

	const float scaleParam = m_ParameterSlots[ m_ParamScale ].m_Value.f;
	const float scaleRndParam = m_ParameterSlots[ m_ParamRandomizeScale ].m_Value.f;

	const float rotationParam = m_ParameterSlots[ m_ParamRotation ].m_Value.f;
	const float rotationRndParam = m_ParameterSlots[ m_ParamRandomizeRotation ].m_Value.f;

	const float intensityParam = m_ParameterSlots[ m_ParamIntensity ].m_Value.f;
	const float intensityRndParam = m_ParameterSlots[ m_ParamRandomizeIntensity ].m_Value.f;

	const bool bScaleIntensityBySize = m_ParameterSlots[ m_ParamScaleIntensityBySize ].m_Value.b;

	int seed = m_ParameterSlots[ m_ParamSeed ].m_Value.i;
	RandomSeed( seed );


	vector< Vertex > vertices( m_SplatCount * 6 );

	static const Vertex unitSquareVtx[] =
	{
		{ { -1, -1 }, { 0, 0 }, 1.0f },
		{ { -1,  1 }, { 0, 1 }, 1.0f },
		{ {  1,  1 }, { 1, 1 }, 1.0f },
		{ {  1, -1 }, { 1, 0 }, 1.0f }
	};

	u32 numSplats = m_SplatCount;
	u32 gridSize;

	if( distributionMethod == 1) //jittered grid
	{ 
		gridSize = (int)floorf( sqrtf( (float)m_SplatCount ) );
		m_SplatCount = gridSize * gridSize;
	}

	for( u32 iSplat = 0 ; iSplat < m_SplatCount ; ++iSplat )
	{
		Vec2 splatPos;
		
		switch( distributionMethod )
		{
			case 0://Random
				splatPos.x = Random();
				splatPos.y = Random();
				break;
			case 1://Jittered grid
			{
				int x = iSplat % gridSize;
				int y = iSplat / gridSize;
				splatPos.x = (0.5f + (float)x + Random( -0.5f, 0.5f ) * positionRndParam) / (float)gridSize;
				splatPos.y = (0.5f + (float)y + Random( -0.5f, 0.5f ) * positionRndParam) / (float)gridSize;
				break;
			}
		}

		float scale = scaleParam + Random( -scaleRndParam, scaleRndParam );
		float rotation = rotationParam + Random( -rotationRndParam, rotationRndParam );

		Mat33 t, r, s;
		t.MakeTranslation( Vec3( splatPos.x, splatPos.y, 0.0f ) );
		r.MakeRotationZ( DegToRad( rotation ) );
		s.MakeScaling( Vec3( scale, scale * aspectRatioParam, 1.0f ) );

		Mat33 m = s * r * t;

		float intensity = intensityParam + Random( -intensityRndParam, intensityRndParam );

		if( bScaleIntensityBySize )
			intensity *= scale;

		u32 baseIdx = iSplat * 6;

		vertices[ baseIdx     ].Pos = m.TransformPosition( unitSquareVtx[ 0 ].Pos );
		vertices[ baseIdx     ].UV = unitSquareVtx[ 0 ].UV;
		vertices[ baseIdx     ].Intensity = intensity;

		vertices[ baseIdx + 1 ].Pos = m.TransformPosition( unitSquareVtx[ 1 ].Pos );
		vertices[ baseIdx + 1 ].UV = unitSquareVtx[ 1 ].UV;
		vertices[ baseIdx + 1 ].Intensity = intensity;

		vertices[ baseIdx + 2 ].Pos = m.TransformPosition( unitSquareVtx[ 3 ].Pos );
		vertices[ baseIdx + 2 ].UV = unitSquareVtx[ 3 ].UV;
		vertices[ baseIdx + 2 ].Intensity = intensity;

		vertices[ baseIdx + 3 ].Pos = m.TransformPosition( unitSquareVtx[ 2 ].Pos );
		vertices[ baseIdx + 3 ].UV = unitSquareVtx[ 2 ].UV;
		vertices[ baseIdx + 3 ].Intensity = intensity;

		vertices[ baseIdx + 4 ] = vertices[ baseIdx + 2 ];
		vertices[ baseIdx + 5 ] = vertices[ baseIdx + 1 ];
	}

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( Vertex ) * m_SplatCount * 6;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &vertices[ 0 ];
	initialData.SysMemPitch = bufferDesc.ByteWidth;
	initialData.SysMemSlicePitch = 0;

	if( FAILED( g_Renderer.GetDevice()->CreateBuffer( &bufferDesc, &initialData, &m_VB ) ) )
		return false;

	return true;
}

void ScatterMapNode::ChangeBlendState()
{
	m_BlendState.Release();

	D3D11_BLEND_DESC blendDesc;
	memset( &blendDesc, 0, sizeof( blendDesc ) );
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[ 0 ].BlendEnable = TRUE;
	blendDesc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	int blendType = m_ParameterSlots[ m_ParamBlendMode ].m_Value.i;

	switch( blendType )
	{
	case 0: //Alpha blend
		blendDesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
		break;
	case 1: //Additive
		blendDesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
		break;
	case 2: //Min
		blendDesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_MAX;
		break;
	case 3: //Max
		blendDesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_MAX;
		break;
	}

	g_Renderer.GetDevice()->CreateBlendState( &blendDesc, &m_BlendState );
}

void ScatterMapNode::InternalCompute()
{
	const Map* pInputMap = GetRemoteInputMap( 0 );
	const Map* pInputMask = GetRemoteInputMap( 1 );

	if( !pInputMap || !pInputMask )
		return;

	if( !m_VS || !m_PS )
		CompileShaders();

//TODO	if( IsBlendStateDirty() )
		ChangeBlendState();

//TODO	if( IsVertexBufferDirty() )
		UpdateVertexBuffer();

	//Now rasterize them

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();


	float clearRTV = 0.0f;

	int blendType = m_ParameterSlots[ m_ParamBlendMode ].m_Value.i;

	switch( blendType )
	{
	case 0: //Alpha blend
		break;
	case 1: //Additive
		break;
	case 2: //Min
		clearRTV = 1.0f;
		break;
	case 3: //Max
		break;
	}

	const FLOAT black[] = { clearRTV, clearRTV, clearRTV, 0.0f };
	pDevCtx->ClearRenderTargetView( m_Output.GetRTV(), black );

	ID3D11RenderTargetView* rtv = m_Output.GetRTV();

	pDevCtx->OMSetRenderTargets( 1, &rtv, nullptr );

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)GetResolution();
	viewport.Height = (float)GetResolution();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	pDevCtx->RSSetViewports( 1, &viewport );

//	FLOAT blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	pDevCtx->OMSetBlendState( m_BlendState, nullptr, 0xFFFFFFFF );
	pDevCtx->OMSetDepthStencilState( m_DepthStencilState, 0 );
	pDevCtx->RSSetState( m_RasterizerState );

	pDevCtx->VSSetShader( m_VS, nullptr, 0 );
	pDevCtx->PSSetShader( m_PS, nullptr, 0 );

	ID3D11ShaderResourceView* srvs[] = { pInputMap->GetSRV(), pInputMask->GetSRV() };
	pDevCtx->PSSetShaderResources( 0, countof( srvs ), srvs );

	ID3D11SamplerState* samplers[] = { g_Renderer.GetBilinearSampler() };
	pDevCtx->PSSetSamplers( 0, countof( samplers ), samplers );

	pDevCtx->IASetInputLayout( m_InputLayout );

	pDevCtx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	ID3D11Buffer* pVB = m_VB;

	const UINT stride = 5 * sizeof(float);
	const UINT offset = 0;

	pDevCtx->IASetVertexBuffers( 0, 1, &pVB, &stride, &offset );

	int vtxCount = m_SplatCount * 6;
	pDevCtx->Draw( vtxCount, 0 );

	//Unbind everything
	static ID3D11RenderTargetView* nullRTV = nullptr;
	static ID3D11ShaderResourceView* nullSRVs[2] = { nullptr };

	pDevCtx->OMSetRenderTargets( 1, &nullRTV, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( nullSRVs ), nullSRVs );

}
