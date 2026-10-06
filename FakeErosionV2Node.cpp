#include "stdafx.h"
#include "FakeErosionV2Node.h"

#include "GeoGen.h"


FakeErosionV2Node::FakeErosionV2Node()
{
	SetUIName( "Fake Erosion V2" );
	SetSize( Vec2( 120, 80 ) );

	AddInput( "HeightMap", IOType::Float );
	AddInput( "StrengthMask", IOType::Float, true );

	AddOutput( "HeightMap", IOType::Float );
	AddOutput( "Erosion", IOType::Float );
	AddOutput( "Ridges", IOType::Float );
	AddOutput( "Drainage", IOType::Float );

	//Defaults are the (non animated) values of the original Shadertoy

	m_ParamScale				= AddParam( "Main", "Scale", IOType::Float, ParamEdition::Slider, 3.0f, 0.1f, 20.0f ); //km
	m_ParamStrength				= AddParam( "Main", "Strength", IOType::Float, ParamEdition::Slider, 0.22f, 0.0f, 1.0f );
	m_ParamGullyWeight			= AddParam( "Main", "GullyWeight", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f );
	m_ParamDetail				= AddParam( "Main", "Detail", IOType::Float, ParamEdition::Slider, 1.5f, 0.1f, 5.0f );
	m_ParamOctaves				= AddParam( "Main", "Octaves", IOType::Integer, ParamEdition::Slider, 5, 1, 8 );
	m_ParamLacunarity			= AddParam( "Main", "Lacunarity", IOType::Float, ParamEdition::Slider, 2.0f, 1.1f, 4.0f );
	m_ParamGain					= AddParam( "Main", "Gain", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f );
	m_ParamCellScale			= AddParam( "Main", "CellScale", IOType::Float, ParamEdition::Slider, 0.7f, 0.1f, 2.0f );
	m_ParamNormalization		= AddParam( "Main", "Normalization", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f );

	m_ParamRidgeRounding		= AddParam( "Rounding", "RidgeRounding", IOType::Float, ParamEdition::Slider, 0.1f, 0.0f, 1.0f );
	m_ParamCreaseRounding		= AddParam( "Rounding", "CreaseRounding", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 1.0f );
	m_ParamInputRounding		= AddParam( "Rounding", "InputRounding", IOType::Float, ParamEdition::Slider, 0.1f, 0.0f, 1.0f );
	m_ParamOctaveRoundingMult	= AddParam( "Rounding", "OctaveRoundingMult", IOType::Float, ParamEdition::Slider, 2.0f, 0.0f, 4.0f );

	m_ParamOnset				= AddParam( "Onset", "Onset", IOType::Float, ParamEdition::Slider, 1.25f, 0.0f, 5.0f );
	m_ParamOctaveOnset			= AddParam( "Onset", "OctaveOnset", IOType::Float, ParamEdition::Slider, 1.25f, 0.0f, 5.0f );
	m_ParamRidgeMapOnset		= AddParam( "Onset", "RidgeMapOnset", IOType::Float, ParamEdition::Slider, 2.8f, 0.0f, 5.0f );
	m_ParamRidgeMapOctaveOnset	= AddParam( "Onset", "RidgeMapOctaveOnset", IOType::Float, ParamEdition::Slider, 1.5f, 0.0f, 5.0f );

	m_ParamAssumedSlope			= AddParam( "Slope", "AssumedSlope", IOType::Float, ParamEdition::Slider, 0.7f, 0.0f, 2.0f );
	m_ParamAssumedSlopeAmount	= AddParam( "Slope", "AssumedSlopeAmount", IOType::Float, ParamEdition::Slider, 1.0f, 0.0f, 1.0f );
	m_ParamSlopeRadius			= AddParam( "Slope", "SlopeRadius", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 2.0f ); //km, 0: one texel (smoother gully directions on noisy inputs such as DEMs)

	m_ParamHeightOffset			= AddParam( "Height", "HeightOffset", IOType::Float, ParamEdition::Slider, -0.65f, -1.0f, 1.0f );
	m_ParamPreserveExtremes		= AddParam( "Height", "PreserveExtremes", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 1.0f );
	m_ParamFadeCenter			= AddParam( "Height", "FadeCenter", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f );
	m_ParamFadeRange			= AddParam( "Height", "FadeRange", IOType::Float, ParamEdition::Slider, 0.3f, 0.01f, 1.0f );
	m_ParamClamp				= AddParam( "Height", "Clamp", IOType::Bool, ParamEdition::CheckBox, false, false );

	m_ParamDrainageWidth		= AddParam( "Masks", "DrainageWidth", IOType::Float, ParamEdition::Slider, 0.3f, 0.01f, 1.0f );
}


FakeErosionV2Node::~FakeErosionV2Node()
{
}

bool FakeErosionV2Node::OneTimeInit()
{
	if( !g_Renderer.CreateShader( "Shaders/FakeErosionV2.hlsl", "Main", nullptr, &m_CS ) )
		return false;

	return m_CB.Init( g_Renderer.GetDevice() );
}

bool FakeErosionV2Node::OnResolutionChanged()
{
	if(    !m_HeightMap.Init( GetResolution(), IOType::Float )
		|| !m_ErosionMap.Init( GetResolution(), IOType::Float )
		|| !m_RidgeMap.Init( GetResolution(), IOType::Float )
		|| !m_DrainageMap.Init( GetResolution(), IOType::Float ) )
		return false;

	m_HeightMap.SetDebugName( "FakeErosionV2 Height" );
	m_ErosionMap.SetDebugName( "FakeErosionV2 Erosion" );
	m_RidgeMap.SetDebugName( "FakeErosionV2 Ridges" );
	m_DrainageMap.SetDebugName( "FakeErosionV2 Drainage" );

	return true;
}

const Map* FakeErosionV2Node::GetOutput( uint32_t _idx ) const
{
	switch( _idx )
	{
		case 0: return &m_HeightMap;
		case 1: return &m_ErosionMap;
		case 2: return &m_RidgeMap;
		case 3: return &m_DrainageMap;
	}

	return nullptr;
}

void FakeErosionV2Node::UpdateConstantBuffer()
{
	auto f = [this]( int _param ) { return m_ParameterSlots[ _param ].m_Value.f; };

	m_CB.TerrainExtent			= (float)theApp.GetTerrainExtent();
	m_CB.MinAltitude			= (float)theApp.GetMinAltitude();
	m_CB.MaxAltitude			= (float)theApp.GetMaxAltitude();

	m_CB.Scale					= f( m_ParamScale );
	m_CB.Strength				= f( m_ParamStrength );
	m_CB.GullyWeight			= f( m_ParamGullyWeight );
	m_CB.Detail					= f( m_ParamDetail );
	m_CB.Octaves				= m_ParameterSlots[ m_ParamOctaves ].m_Value.i;
	m_CB.Lacunarity				= f( m_ParamLacunarity );
	m_CB.Gain					= f( m_ParamGain );
	m_CB.CellScale				= f( m_ParamCellScale );
	m_CB.Normalization			= f( m_ParamNormalization );

	m_CB.RidgeRounding			= f( m_ParamRidgeRounding );
	m_CB.CreaseRounding			= f( m_ParamCreaseRounding );
	m_CB.InputRounding			= f( m_ParamInputRounding );
	m_CB.OctaveRoundingMult		= f( m_ParamOctaveRoundingMult );

	m_CB.Onset					= f( m_ParamOnset );
	m_CB.OctaveOnset			= f( m_ParamOctaveOnset );
	m_CB.RidgeMapOnset			= f( m_ParamRidgeMapOnset );
	m_CB.RidgeMapOctaveOnset	= f( m_ParamRidgeMapOctaveOnset );

	m_CB.AssumedSlope			= f( m_ParamAssumedSlope );
	m_CB.AssumedSlopeAmount		= f( m_ParamAssumedSlopeAmount );
	m_CB.SlopeRadius			= Max( f( m_ParamSlopeRadius ), 0.0f );

	m_CB.HeightOffset			= f( m_ParamHeightOffset );
	m_CB.PreserveExtremes		= f( m_ParamPreserveExtremes );
	m_CB.FadeCenter				= f( m_ParamFadeCenter );
	m_CB.FadeRange				= Max( f( m_ParamFadeRange ), 0.001f );
	m_CB.DrainageWidth			= Max( f( m_ParamDrainageWidth ), 0.001f );
	m_CB.ClampHeight			= m_ParameterSlots[ m_ParamClamp ].m_Value.b ? 1 : 0;

	m_CB.UploadToGPU( g_Renderer.GetImmediateDeviceContext() );
}

void FakeErosionV2Node::InternalCompute()
{
	const Map* pInputMap = GetRemoteInputMap( 0 );

	if( !pInputMap || !m_CS )
		return;

	const Map* pStrengthMask = GetRemoteInputMap( 1 );

	UpdateConstantBuffer();

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	ID3D11ShaderResourceView* srvs[] =
	{
		pInputMap->GetSRV(),
		pStrengthMask ? pStrengthMask->GetSRV() : g_Renderer.GetWhiteTexture()
	};

	ID3D11UnorderedAccessView* uavs[] =
	{
		m_HeightMap.GetUAV(),
		m_ErosionMap.GetUAV(),
		m_RidgeMap.GetUAV(),
		m_DrainageMap.GetUAV()
	};

	ID3D11SamplerState* samplers[] = { g_Renderer.GetBilinearClampSampler() };
	ID3D11Buffer* pCB = m_CB.GetBuffer();

	pDevCtx->CSSetShader( m_CS, nullptr, 0 );
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );
	pDevCtx->CSSetSamplers( 0, _countof( samplers ), samplers );
	pDevCtx->CSSetShaderResources( 0, _countof( srvs ), srvs );
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof( uavs ), uavs, nullptr );

	const uint32_t threadGroupSize = 32;
	uint32_t numGroups = (GetResolution() + threadGroupSize - 1) / threadGroupSize;

	pDevCtx->Dispatch( numGroups, numGroups, 1 );

	//Unbind everything
	ID3D11ShaderResourceView* nullSRVs[ _countof( srvs ) ] = { nullptr };
	ID3D11UnorderedAccessView* nullUAVs[ _countof( uavs ) ] = { nullptr };

	pDevCtx->CSSetShaderResources( 0, _countof( nullSRVs ), nullSRVs );
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof( nullUAVs ), nullUAVs, nullptr );
}
