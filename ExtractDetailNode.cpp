#include "stdafx.h"
#include "ExtractDetailNode.h"


ExtractDetailNode::ExtractDetailNode()
{
	SetUIName( "Extract Detail" );

	AddParam( "Main", "Radius", IOType::Float, ParamEdition::Slider, 0.01f, 0.0f, 0.2f, false );

	AddInput( "FloatMap", IOType::Float );
	AddOutput( "FloatMap", IOType::Float );
}


ExtractDetailNode::~ExtractDetailNode()
{
}

const Map* ExtractDetailNode::GetOutput( uint32_t _idx ) const
{
	return &m_Output;
}


bool ExtractDetailNode::OneTimeInit()
{
	if( !g_Renderer.CreateShader( "Shaders/Blur.hlsl", "Blur", nullptr, &m_CSBlur ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/Blur.hlsl", "BlurAndSubtractFromOriginal", nullptr, &m_CSBlurAndSubtractFromOriginal ) )
		return false;
	
	return m_CB.Init( g_Renderer.GetDevice() );
}

bool ExtractDetailNode::OnResolutionChanged()
{
	if( !m_Output.Init( GetResolution(), IOType::Float )
	 || !m_IntermediateOutput.Init( GetResolution(), IOType::Float ) )
		return false;

	return true;
}

void ExtractDetailNode::InternalCompute()
{
	const Map* pInputMap = this->GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();
	
	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	ID3D11SamplerState* samplers[] = { g_Renderer.GetBilinearClampSampler() };
	pDevCtx->CSSetSamplers( 0, _countof( samplers ), samplers );
	
	const uint32_t threadGroupSizeX = 32;
	const uint32_t threadGroupSizeY = 32;

	uint32_t numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	//Horizontal blur pass
	ID3D11UnorderedAccessView* uavsHoriz[] = { m_IntermediateOutput.GetUAV() };
	ID3D11ShaderResourceView* srvsHoriz[] = { pInputMap->GetSRV() };

	pDevCtx->CSSetShader( m_CSBlur, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof( uavsHoriz ), uavsHoriz, nullptr );
	pDevCtx->CSSetShaderResources( 0, _countof( srvsHoriz ), srvsHoriz );

	m_CB.BlurRadius = m_ParameterSlots[ 0 ].m_Value.f;
	m_CB.KernelSize = (int)( (float)GetResolution() * m_ParameterSlots[0].m_Value.f );
	m_CB.BlurDir.x = 1.0f / (float)(GetResolution() - 1);
	m_CB.BlurDir.y = 0.0f;

	m_CB.UploadToGPU( pDevCtx );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//Vertical blur pass (and subtract from original)
	ID3D11UnorderedAccessView* uavsVert[] = { m_Output.GetUAV() };
	ID3D11ShaderResourceView* srvsVert[] = { m_IntermediateOutput.GetSRV(), pInputMap->GetSRV() };

	pDevCtx->CSSetShader( m_CSBlurAndSubtractFromOriginal, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof( uavsVert ), uavsVert, nullptr );
	pDevCtx->CSSetShaderResources( 0, _countof( srvsVert ), srvsVert );

	TSwap( m_CB.BlurDir.x, m_CB.BlurDir.y );
	m_CB.UploadToGPU( pDevCtx );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//Unbind everything
	static ID3D11ShaderResourceView* nullSRVs[ 2 ] = { NULL };
	pDevCtx->CSSetShaderResources( 0, _countof( nullSRVs ), nullSRVs );

	static ID3D11UnorderedAccessView* nullUAVs[ 1 ] = { NULL };
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof( nullUAVs ), nullUAVs, nullptr );
}