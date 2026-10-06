#include "stdafx.h"
#include "MountainColoringNode.h"

#include "GeoGen.h"

#include <cmath>


static u8 GammaEncodeToByte( float _linear )
{
	return (u8)lroundf( powf( Saturate( _linear ), 1.0f / 2.2f ) * 255.0f );
}

MountainColoringNode::MountainColoringNode()
{
	SetUIName( "Mountain Coloring" );
	SetSize( Vec2( 120, 80 ) );

	m_bPreviewAsHeightField = false;

	AddInput( "HeightMap", IOType::Float );
	AddInput( "Erosion", IOType::Float, true );
	AddInput( "Ridges", IOType::Float, true );

	AddOutput( "Albedo", IOType::Color );
	AddOutput( "Lighting", IOType::Color );
	AddOutput( "Trees", IOType::Float );

	//Levels are normalized altitudes. The defaults reproduce the original Shadertoy, whose eroded
	//terrain spans about [0.36, 0.56] (see ShaderHeight() in Shaders/MountainColoring.hlsl).
	//Note: the first parameter must not be a color (CPropertiesWnd hides the property grid in that case).

	m_ParamWaterLevel		= AddParam( "Levels", "WaterLevel", IOType::Float, ParamEdition::Slider, 0.0f, -0.5f, 1.0f );
	m_ParamGrassLevel		= AddParam( "Levels", "GrassLevel", IOType::Float, ParamEdition::Slider, 0.525f, -0.5f, 1.5f );
	m_ParamCliffStart		= AddParam( "Levels", "CliffStart", IOType::Float, ParamEdition::Slider, 0.2f, -0.5f, 1.5f );
	m_ParamCliffEnd			= AddParam( "Levels", "CliffEnd", IOType::Float, ParamEdition::Slider, 0.8f, -0.5f, 1.5f );
	m_ParamSnowStart		= AddParam( "Levels", "SnowStart", IOType::Float, ParamEdition::Slider, 0.85f, -0.5f, 2.0f );
	m_ParamSnowEnd			= AddParam( "Levels", "SnowEnd", IOType::Float, ParamEdition::Slider, 1.2f, -0.5f, 2.0f );

	m_ParamWater			= AddParam( "Features", "Water", IOType::Bool, ParamEdition::CheckBox, true, false );
	m_ParamDrainage			= AddParam( "Features", "Drainage", IOType::Bool, ParamEdition::CheckBox, true, false );
	m_ParamDrainageWidth	= AddParam( "Features", "DrainageWidth", IOType::Float, ParamEdition::Slider, 0.3f, 0.01f, 1.0f );
	m_ParamTrees			= AddParam( "Features", "Trees", IOType::Bool, ParamEdition::CheckBox, true, false );
	m_ParamBreakupAmount	= AddParam( "Features", "BreakupAmount", IOType::Float, ParamEdition::Slider, 1.0f, 0.0f, 2.0f );
	m_ParamBreakupScale		= AddParam( "Features", "BreakupScale", IOType::Float, ParamEdition::Slider, 1.0f, 0.1f, 10.0f );

	//Default sun: the Shadertoy's fixed sun direction normalize( -1, 0.4, 0.05 )
	m_ParamShadows			= AddParam( "Lighting", "Shadows", IOType::Bool, ParamEdition::CheckBox, true, false );
	m_ParamSunAzimuth		= AddParam( "Lighting", "SunAzimuth", IOType::Float, ParamEdition::Slider, 177.14f, 0.0f, 360.0f );
	m_ParamSunElevation		= AddParam( "Lighting", "SunElevation", IOType::Float, ParamEdition::Slider, 21.78f, 0.0f, 90.0f );
	m_ParamExposure			= AddParam( "Lighting", "Exposure", IOType::Float, ParamEdition::Slider, 1.0f, 0.0f, 4.0f );

	//The Shadertoy's (linear) colors
	m_ParamCliffColor		= AddColorParam( "CliffColor", 0.22f, 0.2f, 0.2f );
	m_ParamDirtColor		= AddColorParam( "DirtColor", 0.6f, 0.5f, 0.4f );
	m_ParamTreeColor		= AddColorParam( "TreeColor", 0.12f, 0.26f, 0.1f );
	m_ParamGrassColor1		= AddColorParam( "GrassColor1", 0.15f, 0.3f, 0.1f );
	m_ParamGrassColor2		= AddColorParam( "GrassColor2", 0.4f, 0.5f, 0.2f );
	m_ParamSandColor		= AddColorParam( "SandColor", 0.8f, 0.7f, 0.6f );
	m_ParamWaterColor		= AddColorParam( "WaterColor", 0.0f, 0.05f, 0.1f );
	m_ParamWaterShoreColor	= AddColorParam( "WaterShoreColor", 0.0f, 0.25f, 0.25f );
}


MountainColoringNode::~MountainColoringNode()
{
}

int MountainColoringNode::AddColorParam( const char* _name, float _r, float _g, float _b )
{
	//Stored gamma encoded, so the color picker shows the actual color
	Color c( GammaEncodeToByte( _r ), GammaEncodeToByte( _g ), GammaEncodeToByte( _b ) );

	return AddParam( "Colors", _name, IOType::Color, ParamEdition::ColorPickerControl, c, false );
}

MountainColoring::float4 MountainColoringNode::GetLinearColor( int _param ) const
{
	Color c;
	c.FromWin32COLORREF( m_ParameterSlots[ _param ].m_Value.c );

	MountainColoring::float4 linear = { powf( c.r, 2.2f ), powf( c.g, 2.2f ), powf( c.b, 2.2f ), 1.0f };
	return linear;
}

bool MountainColoringNode::OneTimeInit()
{
	if( !g_Renderer.CreateShader( "Shaders/MountainColoring.hlsl", "Main", nullptr, &m_CS ) )
		return false;

	return m_CB.Init( g_Renderer.GetDevice() );
}

bool MountainColoringNode::OnResolutionChanged()
{
	if(    !m_AlbedoMap.Init( GetResolution(), IOType::Color )
		|| !m_LightingMap.Init( GetResolution(), IOType::Color )
		|| !m_TreesMap.Init( GetResolution(), IOType::Float ) )
		return false;

	m_AlbedoMap.SetDebugName( "MountainColoring Albedo" );
	m_LightingMap.SetDebugName( "MountainColoring Lighting" );
	m_TreesMap.SetDebugName( "MountainColoring Trees" );

	return true;
}

const Map* MountainColoringNode::GetOutput( uint32_t _idx ) const
{
	switch( _idx )
	{
		case 0: return &m_AlbedoMap;
		case 1: return &m_LightingMap;
		case 2: return &m_TreesMap;
	}

	return nullptr;
}

void MountainColoringNode::UpdateConstantBuffer()
{
	auto f = [this]( int _param ) { return m_ParameterSlots[ _param ].m_Value.f; };
	auto b = [this]( int _param ) { return m_ParameterSlots[ _param ].m_Value.b ? 1 : 0; };

	m_CB.CliffColor			= GetLinearColor( m_ParamCliffColor );
	m_CB.DirtColor			= GetLinearColor( m_ParamDirtColor );
	m_CB.TreeColor			= GetLinearColor( m_ParamTreeColor );
	m_CB.GrassColor1		= GetLinearColor( m_ParamGrassColor1 );
	m_CB.GrassColor2		= GetLinearColor( m_ParamGrassColor2 );
	m_CB.SandColor			= GetLinearColor( m_ParamSandColor );
	m_CB.WaterColor			= GetLinearColor( m_ParamWaterColor );
	m_CB.WaterShoreColor	= GetLinearColor( m_ParamWaterShoreColor );

	m_CB.TerrainExtent		= (float)theApp.GetTerrainExtent();
	m_CB.MinAltitude		= (float)theApp.GetMinAltitude();
	m_CB.MaxAltitude		= (float)theApp.GetMaxAltitude();
	m_CB.Exposure			= f( m_ParamExposure );

	m_CB.WaterLevel			= f( m_ParamWaterLevel );
	m_CB.GrassLevel			= f( m_ParamGrassLevel );
	m_CB.CliffStart			= f( m_ParamCliffStart );
	m_CB.CliffEnd			= f( m_ParamCliffEnd );

	m_CB.SnowStart			= f( m_ParamSnowStart );
	m_CB.SnowEnd			= f( m_ParamSnowEnd );
	m_CB.DrainageWidth		= Max( f( m_ParamDrainageWidth ), 0.001f );
	m_CB.BreakupAmount		= f( m_ParamBreakupAmount );

	const float azimuth = DegToRad( f( m_ParamSunAzimuth ) );
	const float elevation = DegToRad( f( m_ParamSunElevation ) );

	m_CB.BreakupScale		= f( m_ParamBreakupScale );
	m_CB.SunDirX			= cosf( elevation ) * cosf( azimuth );
	m_CB.SunDirY			= sinf( elevation );
	m_CB.SunDirZ			= cosf( elevation ) * sinf( azimuth );

	m_CB.EnableWater		= b( m_ParamWater );
	m_CB.EnableDrainage		= b( m_ParamDrainage );
	m_CB.EnableTrees		= b( m_ParamTrees );
	m_CB.EnableShadows		= b( m_ParamShadows );

	m_CB.UploadToGPU( g_Renderer.GetImmediateDeviceContext() );
}

void MountainColoringNode::InternalCompute()
{
	const Map* pHeightMap = GetRemoteInputMap( 0 );

	if( !pHeightMap || !m_CS )
		return;

	const Map* pErosionMap = GetRemoteInputMap( 1 );
	const Map* pRidgeMap = GetRemoteInputMap( 2 );

	UpdateConstantBuffer();

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	ID3D11ShaderResourceView* srvs[] =
	{
		pHeightMap->GetSRV(),
		pErosionMap ? pErosionMap->GetSRV() : g_Renderer.GetWhiteTexture(),
		pRidgeMap ? pRidgeMap->GetSRV() : g_Renderer.GetWhiteTexture()
	};

	ID3D11UnorderedAccessView* uavs[] =
	{
		m_AlbedoMap.GetUAV(),
		m_LightingMap.GetUAV(),
		m_TreesMap.GetUAV()
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
