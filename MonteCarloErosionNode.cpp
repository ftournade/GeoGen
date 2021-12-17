#include "stdafx.h"
#include "MonteCarloErosionNode.h"

#include "GridMesh.h"

#include "GeoGen.h"

const char* initSimShaderSource2 =
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

const char* finalizeShaderSource2 =
	"Texture2D<float> BedRockMap : register(t0);\n"
	"Texture2D<float> BrockenRockMap : register(t1);\n"
	"Texture2D<float> SandMap : register(t2);\n"
	"Texture2D<float> HumusMap : register(t3);\n"

	"RWTexture2D<float> TotalHeightMap : register(u0);\n"

	"#include \"Shaders/MonteCarloErosionConstants.h\"\n"

	"[numthreads( 32, 32, 1 )]\n"
	"void TotalTerrainAltitude( uint2 _pos : SV_DispatchThreadID )\n"
	"{\n"
	"	float altitude = BedRockMap[ _pos ] + BrockenRockMap[ _pos ] + SandMap[ _pos ] + HumusMap[ _pos ];\n"
	"	altitude = (altitude - MinAltitude) / (MaxAltitude - MinAltitude);\n"
	"	TotalHeightMap[ _pos ] = saturate( altitude );\n"
	"}\n";

MonteCarloErosionNode::MonteCarloErosionNode()
{
	m_bIsIterative = true;

	SetUIName( "Erosion" );
	SetSize( Vec2( 120, 140 ) );

	AddInput( "HeightMap", IOType::Float );
	AddInput( "BedRockHardnessMap", IOType::Float, true );
	AddInput( "RainMap", IOType::Float, true );
	AddInput( "SmoothingMap", IOType::Float, true );
	AddOutput( "HeightMap", IOType::Float );
	AddOutput( "BedRockMap", IOType::Float );
	AddOutput( "BrockenRockMap", IOType::Float );
	AddOutput( "SandMap", IOType::Float );
	AddOutput( "HumusMap", IOType::Float );
	AddOutput( "VegetationMap", IOType::Float );
	AddOutput( "DeadVegetationMap", IOType::Float );
	AddOutput( "FlowMap", IOType::Float );

	m_ParamIterations = AddParam( "Main", "Iterations", IOType::Integer, ParamEdition::Slider, 30, 1, 1000 );
	m_ParamSmoothSoil = AddParam( "Main", "SmoothSoil", IOType::Float, ParamEdition::Slider, 0.4f, 0.0f, 1.0f );

	m_ParamTalusAngle = AddParam( "Gravity", "TalusAngle", IOType::Float, ParamEdition::Slider, 0.2f, 0.0f, 2.0f );
	m_ParamSpeed      = AddParam( "Gravity", "Speed", IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 0.6f );

	m_ParamHydraulicSpeedVsQuality  = AddParam( "Hydraulic", "HydraulicSpeedVsQuality", IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 1.0f );
	m_ParamRainRate					= AddParam( "Hydraulic", "RainRate",				IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 1.0f );
	m_ParamEvaporationRate			= AddParam( "Hydraulic", "EvaporationRate",			IOType::Float, ParamEdition::Slider, 0.01f, 0.0f, 0.1f );
	m_ParamErosionRate				= AddParam( "Hydraulic", "ErosionRate",				IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 1.0f );
	m_ParamDepositionRate			= AddParam( "Hydraulic", "DepositionRate",			IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 1.0f );
	m_ParamErosionSlopeThreshold	= AddParam( "Hydraulic", "ErosionSlopeThreshold",	IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 1.0f );
	m_ParamSedimentCapacity			= AddParam( "Hydraulic", "SedimentCapacity",		IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 50.0f );
	

	if( !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "VSMainSnow", nullptr, &m_pPreviewVertexShader )  //assume same input layout as m_pTerrainVertexShader
	 || !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "PSMainSnow", nullptr, &m_pPreviewPixelShader ) )
	{
		DBG_CHECK( false );
		//return false; //TODO not in constructor, have an Init method
	}


}


MonteCarloErosionNode::~MonteCarloErosionNode()
{
}

bool MonteCarloErosionNode::OneTimeInit()
{
	if( !g_Renderer.CreateShaderFromMemory( initSimShaderSource2, "ConvertInputAltitude", nullptr, &m_pInitSimCS ) )
		return false;

	if( !g_Renderer.CreateShaderFromMemory( finalizeShaderSource2, "TotalTerrainAltitude", nullptr, &m_pTotalTerrainAltituteCS ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/MonteCarloErosion.hlsl", "HydraulicEvent", nullptr, &m_HydraulicCS ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/MonteCarloErosion.hlsl", "GravityEvent", nullptr, &m_GravityCS ) )
		return false;

	if( !g_Renderer.CreateShader( "Shaders/MonteCarloErosion.hlsl", "SmoothEvent", nullptr, &m_pSmoothSoilCS ) )
		return false;

	if( !m_CB.Init( g_Renderer.GetDevice() ) )
		return false;

	return true;
}

bool MonteCarloErosionNode::OnResolutionChanged()
{
	if(	!m_HeightMap.Init( GetResolution(), IOType::Float )
	 || !m_FlowMap.Init( GetResolution(), IOType::Float )
	 || !m_BedRockMap.Init( GetResolution(), IOType::Float )
	 || !m_BrockenRockMap.Init( GetResolution(), IOType::Float )
	 || !m_SandMap.Init( GetResolution(), IOType::Float )
	 || !m_HumusMap.Init( GetResolution(), IOType::Float )
	 || !m_VegetationMap.Init( GetResolution(), IOType::Float )
	 || !m_DeadVegetationMap.Init( GetResolution(), IOType::Float ) )
		return false;

	m_HeightMap.SetDebugName( "Height" );
	m_FlowMap.SetDebugName( "Flow" );

	m_BedRockMap.SetDebugName( "BedRock" );
	m_BrockenRockMap.SetDebugName( "BrockenRock" );
	m_SandMap.SetDebugName( "Sand" );
	m_HumusMap.SetDebugName( "Humus" );
	m_VegetationMap.SetDebugName( "Vegetation" );
	m_DeadVegetationMap.SetDebugName( "DeadVegetation" );


	return true;
}

const Map* MonteCarloErosionNode::GetOutput( u32 _idx ) const
{
	ComputeTotalTerrainAltitude(); //TODO lazy eval (cache)

	switch( _idx )
	{
		case 0: return &m_HeightMap;
		case 1: return &m_BedRockMap;
		case 2: return &m_BrockenRockMap;
		case 3: return &m_SandMap;
		case 4: return &m_HumusMap;
		case 5: return &m_VegetationMap;
		case 6: return &m_DeadVegetationMap;
		case 7: return &m_FlowMap;
	}

	return nullptr;
}

void MonteCarloErosionNode::UpdateConstantBuffer()
{
	//General
	m_CB.Resolution = GetResolution();
	m_CB.MinAltitude = theApp.GetMinAltitude();
	m_CB.MaxAltitude = theApp.GetMaxAltitude();
	m_CB.TerrainExtent = (float)theApp.GetTerrainExtent();
	m_CB.CellSize = m_CB.TerrainExtent / (float)m_CB.Resolution;
	m_CB.Seed = (float)m_Step * 31.2783f;
	m_CB.Smooth = m_ParameterSlots[ m_ParamSmoothSoil ].m_Value.f / 100.0f;

	//Gravity (snow, sand, rocks)
	m_CB.TalusAngle = m_ParameterSlots[ m_ParamTalusAngle ].m_Value.f;
	m_CB.Speed = m_ParameterSlots[ m_ParamSpeed ].m_Value.f;

	//Hydraulic erosion
	float dt = m_ParameterSlots[ m_ParamHydraulicSpeedVsQuality ].m_Value.f * 1000.0f;

	m_CB.RainRate		 = dt * m_ParameterSlots[ m_ParamRainRate ].m_Value.f;
	m_CB.EvaporationRate = dt * m_ParameterSlots[ m_ParamEvaporationRate ].m_Value.f;
	m_CB.ErosionRate	 = dt * m_ParameterSlots[ m_ParamErosionRate ].m_Value.f;
	m_CB.DepositionRate  = dt * m_ParameterSlots[ m_ParamDepositionRate ].m_Value.f;
	m_CB.ErosionSlopeThreshold = m_ParameterSlots[ m_ParamErosionSlopeThreshold ].m_Value.f;
	m_CB.SedimentCapacity = m_ParameterSlots[ m_ParamSedimentCapacity ].m_Value.f;

	m_CB.UploadToGPU( g_Renderer.GetImmediateDeviceContext() );
}

void MonteCarloErosionNode::ComputeTotalTerrainAltitude() const
{
	const Map* pInputMap = GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	const u32 threadGroupSizeX = 32;
	const u32 threadGroupSizeY = 32;

	u32 numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	u32 numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] = { m_HeightMap.GetUAV() };
	ID3D11ShaderResourceView* srvs[] = { 
		m_BedRockMap.GetSRV(),
		m_BrockenRockMap.GetSRV(),
		m_SandMap.GetSRV(),
		m_HumusMap.GetSRV(),
		m_VegetationMap.GetSRV(),
		m_DeadVegetationMap.GetSRV() };

	ID3D11Buffer* pCB = m_CB.GetBuffer();

	pDevCtx->CSSetShader( m_pTotalTerrainAltituteCS, nullptr, 0 );
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//unbind everything
	ID3D11UnorderedAccessView* nullUAVs[ countof( uavs ) ] = { nullptr };
	ID3D11ShaderResourceView* nullSRVs[ countof( srvs ) ] = { nullptr };

	pDevCtx->CSSetUnorderedAccessViews( 0, countof( nullUAVs ), nullUAVs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( nullSRVs ), nullSRVs );

}


void MonteCarloErosionNode::InitSim()
{
	const Map* pInputMap = GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;

	int res = GetResolution();

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();
	
	FLOAT zero[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pDevCtx->ClearUnorderedAccessViewFloat( m_BrockenRockMap.GetUAV(), zero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_SandMap.GetUAV(), zero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_HumusMap.GetUAV(), zero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_VegetationMap.GetUAV(), zero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_DeadVegetationMap.GetUAV(), zero );

	pDevCtx->ClearUnorderedAccessViewFloat( m_FlowMap.GetUAV(), zero );
	
	//Init bedrock map by remapping input heightmap from [0,1] to meters

	const u32 threadGroupSizeX = 32;
	const u32 threadGroupSizeY = 32;

	u32 numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	u32 numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] = 
	{ 
		m_BedRockMap.GetUAV(),
		m_BrockenRockMap.GetUAV(),
		m_SandMap.GetUAV(),
		m_HumusMap.GetUAV(),
		m_VegetationMap.GetUAV(),
		m_DeadVegetationMap.GetUAV() 
	};

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
}

void MonteCarloErosionNode::SmoothSoil()
{

	if( m_CB.Smooth == 0.0f )
		return;

	const Map* pInputMap = this->GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;

	//float scale = 1.0f / (1.0f + m_CB.SmoothSnow * 8.0f);

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();
	
	const Map* pSmoothMap = this->GetRemoteInputMap( 3 );

	const u32 threadGroupSizeX = 32;
	const u32 threadGroupSizeY = 32;

	u32 numGroupsX = 8;// (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	u32 numGroupsY = 8;// (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] = { nullptr, nullptr, m_SandMap.GetUAV() };
	ID3D11ShaderResourceView* srvs[] = 
	{
		nullptr, nullptr, nullptr,
		pSmoothMap ? pSmoothMap->GetSRV() : g_Renderer.GetWhiteTexture(),
		m_BedRockMap.GetSRV()
	};

	ID3D11SamplerState* samplers[] = { g_Renderer.GetPointClampSampler() };

	pDevCtx->CSSetShader( m_pSmoothSoilCS, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );
	pDevCtx->CSSetSamplers( 0, countof( samplers ), samplers );

	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

}

void MonteCarloErosionNode::ComputeHydraulicEvents()
{
	const Map* pInputMap = this->GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;

	const Map* pBedRockHardnessMap = this->GetRemoteInputMap( 1 );
	const Map* pRainMap = this->GetRemoteInputMap( 2 );

	//	float dt = m_ParameterSlots[ m_ParamTimePerIteration ].m_Value.f;

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	const u32 threadGroupSizeX = 8;
	const u32 threadGroupSizeY = 8;

	u32 numGroupsX = 8;// (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	u32 numGroupsY = 8;// (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] = 
	{ 
		m_BedRockMap.GetUAV(),
		m_BrockenRockMap.GetUAV(),
		m_SandMap.GetUAV(),
		m_HumusMap.GetUAV(),
		m_VegetationMap.GetUAV(),
		m_DeadVegetationMap.GetUAV(),
		nullptr,//RWSnowMap 
		m_FlowMap.GetUAV()
	};

	ID3D11ShaderResourceView* srvs[] = 
	{ 
		pBedRockHardnessMap ? pBedRockHardnessMap->GetSRV() : g_Renderer.GetWhiteTexture(),
		nullptr,
		pRainMap ? pRainMap->GetSRV() : g_Renderer.GetWhiteTexture()
	};
	
	//ID3D11SamplerState* samplers[] = { g_Renderer.GetBilinearClampSampler() };

	pDevCtx->CSSetShader( m_HydraulicCS, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );
	//pDevCtx->CSSetSamplers( 0, countof( samplers ), samplers );

	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );
}

void MonteCarloErosionNode::ComputeGravityEvents()
{
	const Map* pInputMap = this->GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;

	const Map* pBedRockHardnessMap = this->GetRemoteInputMap( 1 );

//	float dt = m_ParameterSlots[ m_ParamTimePerIteration ].m_Value.f;

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	const u32 threadGroupSizeX = 8;
	const u32 threadGroupSizeY = 8;

	u32 numGroupsX = 4;// (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	u32 numGroupsY = 4;// (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] = { nullptr, nullptr, m_SandMap.GetUAV() };
	ID3D11ShaderResourceView* srvs[] = { pBedRockHardnessMap ? pBedRockHardnessMap->GetSRV() : g_Renderer.GetWhiteTexture(),
										nullptr, nullptr, nullptr, pInputMap->GetSRV() };
	//ID3D11SamplerState* samplers[] = { g_Renderer.GetBilinearClampSampler() };

	pDevCtx->CSSetShader( m_GravityCS, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );
	//pDevCtx->CSSetSamplers( 0, countof( samplers ), samplers );

	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );
}

void MonteCarloErosionNode::StepSim( bool _rebindResources, bool _unbindResourcesOnExit )
{
	UpdateConstantBuffer();

	ComputeHydraulicEvents();
	ComputeGravityEvents();
	SmoothSoil();
	
	++m_Step;

	//unbind everything
	ID3D11UnorderedAccessView* nullUAVs[8] = { nullptr };
	ID3D11ShaderResourceView* nullSRVs[8] = { nullptr };

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	pDevCtx->CSSetUnorderedAccessViews( 0, countof( nullUAVs ), nullUAVs, nullptr );
	pDevCtx->CSSetShaderResources( 0, countof( nullSRVs ), nullSRVs );
}

void MonteCarloErosionNode::InternalCompute()
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


bool MonteCarloErosionNode::RenderSimPreview( const GridMesh& _gridMesh )
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	pDevCtx->VSSetShader( m_pPreviewVertexShader, nullptr, 0 );
	pDevCtx->PSSetShader( m_pPreviewPixelShader, nullptr, 0 );

	ID3D11ShaderResourceView* vsSRVs[] = { m_HeightMap.GetSRV(), nullptr, nullptr,  m_SandMap.GetSRV(), nullptr };
	ID3D11ShaderResourceView* psSRVs[] = { m_HeightMap.GetSRV(), nullptr, nullptr,  m_SandMap.GetSRV(), nullptr };

	pDevCtx->VSSetShaderResources( 0, countof( vsSRVs ), vsSRVs );
	pDevCtx->PSSetShaderResources( 0, countof( psSRVs ), psSRVs );


	ID3D11SamplerState* sampler = g_Renderer.GetBilinearClampSampler();
	pDevCtx->VSSetSamplers( 0, 1, &sampler );
	pDevCtx->PSSetSamplers( 0, 1, &sampler );

	_gridMesh.Render( pDevCtx );

	return true;
}
