#include "stdafx.h"
#include "SnowNode.h"

#include "GridMesh.h"

#include "GeoGen.h"

extern const char* initSimShaderSource =
	"Texture2D<float> InputMap : register(t0);\n"
	"RWTexture2D<float> BedRockMap : register(u0);\n"

	"SamplerState _bilinearClampSampler;\n"

	"#include \"Shaders/MonteCarloErosionConstants.h\"\n"

	"[numthreads( 32, 32, 1 )]\n"
	"void ConvertInputAltitude( uint2 _pos : SV_DispatchThreadID )\n"
	"{\n"
	"	float altitude = InputMap.SampleLevel( _bilinearClampSampler, _pos / (float)(Resolution), 0.0f );\n"
	"	BedRockMap[ _pos ] = lerp( MinAltitude, MaxAltitude, altitude );\n"
	"}\n";

const char* finalizeShaderSource =
	"Texture2D<float> TerrainMap : register(t0);\n"
	"Texture2D<float> SnowMap : register(t1);\n"
	"RWTexture2D<float> HeightPlusSnowMap : register(u0);\n"

	"#include \"Shaders/MonteCarloErosionConstants.h\"\n"

	"[numthreads( 32, 32, 1 )]\n"
	"void TerrainPlusSnow( uint2 _pos : SV_DispatchThreadID )\n"
	"{\n"
	"	HeightPlusSnowMap[ _pos ] = ( TerrainMap[ _pos ] + SnowMap[ _pos ] - MinAltitude )  / (MaxAltitude - MinAltitude);\n"
	"}\n";

SnowNode::SnowNode()
{
	m_bIsIterative = true;

	SetUIName( "Snow" );

	AddInput( "HeightMap", IOType::Float );
	AddInput( "TemperatureMap", IOType::Float, true );
	AddInput( "SmoothingMap", IOType::Float, true );
	AddOutput( "HeightMap", IOType::Float );
	AddOutput( "SnowMap", IOType::Float );

	m_ParamIterations = AddParam( "Main", "Iterations", IOType::Integer, ParamEdition::Slider, 300, 1, 3000 );
	m_ParamTalusAngle = AddParam( "Main", "TalusAngle", IOType::Float, ParamEdition::Slider, 0.2f, 0.0f, 2.0f );
	m_ParamSpeed      = AddParam( "Main", "Speed", IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 0.6f );
	m_ParamInitialSnowFall = AddParam( "Main", "InitialSnowfall", IOType::Float, ParamEdition::Slider, 10.0f, 0.0f, 200.0f );
	m_ParamSnowFall = AddParam( "Main", "SnowFall", IOType::Float, ParamEdition::Slider, 0.4f, 0.0f, 1.0f );
	m_ParamEvaporation = AddParam( "Main", "Evaporation", IOType::Float, ParamEdition::Slider, 0.4f, 0.0f, 1.0f );
	m_ParamSmoothSnow = AddParam( "Main", "Smooth", IOType::Float, ParamEdition::Slider, 0.4f, 0.0f, 50.0f );

	if( !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "VSMainSnow", nullptr, &m_pPreviewVertexShader )  //assume same input layout as m_pTerrainVertexShader
	 || !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "PSMainSnow", nullptr, &m_pPreviewPixelShader ) )
	{
		assert( false );
		//return false; //TODO not in constructor, have an Init method
	}


}


SnowNode::~SnowNode()
{
}

bool SnowNode::OneTimeInit()
{
	if( !g_Renderer.CreateShaderFromMemory( initSimShaderSource, "ConvertInputAltitude", nullptr, &m_pInitSimCS ) )
		return false;

	if( !g_Renderer.CreateShaderFromMemory( finalizeShaderSource, "TerrainPlusSnow", nullptr, &m_pTerrainPlusSnowCS ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/MonteCarloErosion.hlsl", "SnowGravityEvent", nullptr, &m_CS ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/MonteCarloErosion.hlsl", "SnowSmoothEvent", nullptr, &m_pSmoothSnowCS ) )
		return false;

	if( !m_CB.Init( g_Renderer.GetDevice() ) )
		return false;

	return true;
}

bool SnowNode::OnResolutionChanged()
{
	if(	   !m_HeightMap.Init( GetResolution(), IOType::Float )
		|| !m_BedRockMap.Init( GetResolution(), IOType::Float )
		|| !m_SnowMap.Init( GetResolution(), IOType::Float ) )
		return false;

	m_HeightMap.SetDebugName( "HeightMap" );

	m_CB.Resolution = GetResolution();
	m_CB.MinAltitude = theApp.GetMinAltitude();
	m_CB.MaxAltitude = theApp.GetMaxAltitude();
	m_CB.TerrainExtent = (float)theApp.GetTerrainExtent();
	m_CB.CellSize = m_CB.TerrainExtent / (float)m_CB.Resolution;

	m_CB.UploadToGPU( g_Renderer.GetImmediateDeviceContext() );

	return true;
}

const Map* SnowNode::GetOutput( uint32_t _idx ) const
{
	AddSnowAndTerrain();

	switch( _idx )
	{
	case 0: return &m_HeightMap;
	case 1: return &m_SnowMap;
	}

	return nullptr;
}


void SnowNode::UpdateConstantBuffer()
{
	//General
	m_CB.Resolution = GetResolution();
	m_CB.MinAltitude = theApp.GetMinAltitude();
	m_CB.MaxAltitude = theApp.GetMaxAltitude();
	m_CB.TerrainExtent = (float)theApp.GetTerrainExtent();
	m_CB.CellSize = m_CB.TerrainExtent / (float)m_CB.Resolution;
	m_CB.Seed = (float)m_Step * 31.278631f;
	m_CB.Smooth = m_ParameterSlots[ m_ParamSmoothSnow ].m_Value.f * 1.0f;

	//Gravity (snow, sand, rocks)
	m_CB.TalusAngle = m_ParameterSlots[ m_ParamTalusAngle ].m_Value.f;
	m_CB.Speed = m_ParameterSlots[ m_ParamSpeed ].m_Value.f;

	m_CB.SnowFall = m_ParameterSlots[ m_ParamSnowFall ].m_Value.f;
	m_CB.SnowEvaporation = m_ParameterSlots[ m_ParamEvaporation ].m_Value.f;

	m_CB.UploadToGPU( g_Renderer.GetImmediateDeviceContext() );
}

void SnowNode::AddSnowAndTerrain() const
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	const uint32_t threadGroupSizeX = 32;
	const uint32_t threadGroupSizeY = 32;

	uint32_t numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] = { m_HeightMap.GetUAV() };
	ID3D11ShaderResourceView* srvs[] = { m_BedRockMap.GetSRV(), m_SnowMap.GetSRV() };
	ID3D11Buffer* pCB = m_CB.GetBuffer();

	pDevCtx->CSSetShader( m_pTerrainPlusSnowCS , nullptr, 0 );
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//unbind everything
	ID3D11UnorderedAccessView* nullUAVs[ 2 ] = { nullptr };
	ID3D11ShaderResourceView* nullSRVs[ 2 ] = { nullptr };

	pDevCtx->CSSetShader( m_CS, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( nullUAVs ), nullUAVs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( nullSRVs ), nullSRVs );

}


void SnowNode::InitSim()
{
	const Map* pInputMap = GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;

	int res = GetResolution();

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	float initialSnowFall = m_ParameterSlots[ m_ParamInitialSnowFall ].m_Value.f; //unit: meters

	FLOAT zero[ 4 ] = { initialSnowFall, initialSnowFall, initialSnowFall, initialSnowFall };
	pDevCtx->ClearUnorderedAccessViewFloat( m_SnowMap.GetUAV(), zero );

	//---------------------

	//Init bedrock map by remapping input heightmap from [0,1] to meters

	const uint32_t threadGroupSizeX = 32;
	const uint32_t threadGroupSizeY = 32;

	uint32_t numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] =	{ m_BedRockMap.GetUAV()	};
	ID3D11ShaderResourceView* srvs[] = { pInputMap->GetSRV() };
	ID3D11Buffer* pCB = m_CB.GetBuffer();

	UpdateConstantBuffer();

	pDevCtx->CSSetShader( m_pInitSimCS, nullptr, 0 );
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//unbind everything
	ID3D11UnorderedAccessView* nullUAVs[ countof( uavs ) ] = { nullptr };
	ID3D11ShaderResourceView* nullSRVs[ countof( srvs ) ] = { nullptr };

	pDevCtx->CSSetUnorderedAccessViews( 0, countof( nullUAVs ), nullUAVs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( nullSRVs ), nullSRVs );

	//------------


	m_Step = 0;
	//TODO...
}

void SnowNode::SmoothSnow()
{

	if( m_CB.Smooth == 0.0f )
		return;

	//float scale = 1.0f / (1.0f + m_CB.Smooth * 8.0f);

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();


	const uint32_t threadGroupSizeX = 8;
	const uint32_t threadGroupSizeY = 8;

	uint32_t numGroupsX = 32;// (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = 32;// (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	const Map* pTemperatureMap = this->GetRemoteInputMap( 1 );
	const Map* pSmoothingMap = this->GetRemoteInputMap( 2 );

	ID3D11UnorderedAccessView* uavs[] = { m_BedRockMap.GetUAV(), nullptr, nullptr, nullptr, nullptr, nullptr, m_SnowMap.GetUAV() };
	ID3D11ShaderResourceView* srvs[] = { 
		nullptr,
		pTemperatureMap ? pTemperatureMap->GetSRV() : g_Renderer.GetBlackTexture(),
		nullptr,
		pSmoothingMap ? pSmoothingMap->GetSRV() : g_Renderer.GetWhiteTexture(),
		nullptr };

	ID3D11SamplerState* samplers[] = { g_Renderer.GetPointClampSampler() };

	pDevCtx->CSSetShader( m_pSmoothSnowCS, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );
	pDevCtx->CSSetSamplers( 0, countof( samplers ), samplers );

	m_CB.Seed = (float)m_Step * 31.2783f;
	m_CB.TalusAngle = m_ParameterSlots[ m_ParamTalusAngle ].m_Value.f;
	m_CB.Speed = m_ParameterSlots[ m_ParamSpeed ].m_Value.f;
	m_CB.UploadToGPU( pDevCtx );

	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

}

void SnowNode::StepSim( bool _rebindResources, bool _unbindResourcesOnExit )
{

//	float dt = m_ParameterSlots[ m_ParamTimePerIteration ].m_Value.f;

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	const uint32_t threadGroupSizeX = 8;
	const uint32_t threadGroupSizeY = 8;

	uint32_t numGroupsX = 4;// (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = 4;// (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] = { m_BedRockMap.GetUAV(), nullptr, nullptr, nullptr, nullptr, nullptr, m_SnowMap.GetUAV() };
//	ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr, nullptr, nullptr, m_BedRockMap.GetSRV() };
	//ID3D11SamplerState* samplers[] = { g_Renderer.GetBilinearClampSampler() };

	pDevCtx->CSSetShader( m_CS, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
//	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );
	//pDevCtx->CSSetSamplers( 0, countof( samplers ), samplers );

	UpdateConstantBuffer();

	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );


	//unbind everything
	ID3D11UnorderedAccessView* nullUAVs[8] = { nullptr };
	ID3D11ShaderResourceView* nullSRVs[8] = { nullptr };

	pDevCtx->CSSetUnorderedAccessViews( 0, countof( nullUAVs ), nullUAVs, nullptr );
//	pDevCtx->CSSetShaderResources( 0, countof( nullSRVs ), nullSRVs );

	SmoothSnow();

	pDevCtx->CSSetUnorderedAccessViews( 0, countof( nullUAVs ), nullUAVs, nullptr );
//	pDevCtx->CSSetShaderResources( 0, countof( nullSRVs ), nullSRVs );

	++m_Step;
}

void SnowNode::InternalCompute()
{
	InitSim();

	StepSim( true, false );

	int numIterations = m_ParameterSlots[ m_ParamIterations ].m_Value.i;

	for( int i = 0 ; i < numIterations - 1 ; ++i )
	{
		StepSim( false, false );
	}


	//	static ID3D11UnorderedAccessView* nullUAVs[8] = { NULL };
	//	pDevCtx->CSSetUnorderedAccessViews( 0, 8, nullUAVs, nullptr );
	//TODO unbind SRV
}


bool SnowNode::RenderSimPreview( const GridMesh& _gridMesh )
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	pDevCtx->VSSetShader( m_pPreviewVertexShader, nullptr, 0 );
	pDevCtx->PSSetShader( m_pPreviewPixelShader, nullptr, 0 );

	ID3D11ShaderResourceView* vsSRVs[] = { m_HeightMap.GetSRV(), nullptr, nullptr,  m_SnowMap.GetSRV(), nullptr };
	ID3D11ShaderResourceView* psSRVs[] = { m_HeightMap.GetSRV(), nullptr, nullptr,  m_SnowMap.GetSRV(), nullptr };

	pDevCtx->VSSetShaderResources( 0, countof( vsSRVs ), vsSRVs );
	pDevCtx->PSSetShaderResources( 0, countof( psSRVs ), psSRVs );


	ID3D11SamplerState* sampler = g_Renderer.GetBilinearClampSampler();
	pDevCtx->VSSetSamplers( 0, 1, &sampler );
	pDevCtx->PSSetSamplers( 0, 1, &sampler );

	_gridMesh.Render( pDevCtx );

	return true;
}
