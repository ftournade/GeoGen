#include "stdafx.h"
#include "ErosionNode.h"

#include "GeoGen.h"
#include "GridMesh.h"

ErosionNode::ErosionNode()
{
	m_bIsIterative = true;

	SetUIName( "Erosion" );

	AddInput( "HeightMap", IOType::Float );
	AddOutput( "HeightMap", IOType::Float );
	AddOutput( "WaterMap", IOType::Float );
//	AddOutput( "FlowMap", IOType::Float );

	AddParam( "Main", "Iterations", IOType::Integer, ParamEdition::Slider, 30, 1, 1000 );
	AddParam( "Main", "TimePerIteration", IOType::Float, ParamEdition::Slider, 0.7f, 0.0f, 5.0f );
	AddParam( "Main", "RainRate", IOType::Float, ParamEdition::Slider, 0.012f, 0.0f, 0.05f );
	AddParam( "Main", "EvaporationRate", IOType::Float, ParamEdition::Slider, 0.01f, 0.0f, 0.5f );
	AddParam( "Main", "DissolutionRate", IOType::Float, ParamEdition::Slider, 0.005f, 0.0f, 0.01f );
	AddParam( "Main", "DepositionRate", IOType::Float, ParamEdition::Slider, 0.005f, 0.0f, 0.5f );
	AddParam( "Main", "SedimentCapacity", IOType::Float, ParamEdition::Slider, 0.02f, 0.0f, 0.2f );
//	AddParam( "Main", "Smoothing", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 0.05f );
		
	AddParam( "Main", "TalusAngle", IOType::Float, ParamEdition::Slider, 45.0f, 0.0f, 90.0f );
	AddParam( "Main", "ThermalErosion", IOType::Float, ParamEdition::Slider, 0.009f, 0.0f, 0.1f );

	if(    !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "AddWater", nullptr, &m_AddWaterCS ) 
		|| !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "UpdateWaterOutFlow", nullptr, &m_UpdateWaterOutFlowCS )
		|| !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "UpdateWaterHeightAndVelocity", nullptr, &m_UpdateWaterHeightAndVelocityCS )
		|| !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "HydraulicErosion", nullptr, &m_HydraulicErosionCS )
		|| !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "ThermalErosionOutFlow", nullptr, &m_ThermalErosionOutFlowCS )
		|| !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "ThermalErosion", nullptr, &m_ThermalErosionCS )
		|| !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "SoilTransportation", nullptr, &m_SoilTransportationCS )
		|| !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "WaterEvaporation", nullptr, &m_WaterEvaporationCS )
		|| !g_Renderer.CreateShader( "Shaders/Erosion.hlsl", "DepositAllSuspendedSoil", nullptr, &m_DepositAllSuspendedSoilCS ) )
	{
		assert( false );
		//return false; //TODO not in constructor, have an Init method
	}


	if( !m_CB.Init( g_Renderer.GetDevice() ) )
	{
		assert( false );
		//return false; //TODO not in constructor, have an Init method
	}

	if( !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "VSMainErosion", nullptr, &m_pPreviewVertexShader )  //assume same input layout as m_pTerrainVertexShader
	 || !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "PSMainErosion", nullptr, &m_pPreviewPixelShader ) )
	{
		assert( false );
		//return false; //TODO not in constructor, have an Init method
	}
}


ErosionNode::~ErosionNode()
{
}

bool ErosionNode::OnResolutionChanged()
{
	if( !m_HeightMap.Init( GetResolution(), IOType::Float ) ||
		!m_WaterMap.Init( GetResolution(), IOType::Float ) ||
		!m_WaterOutFlowMap.Init( GetResolution(), GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT ) ||
		!m_WaterVelocityMap.Init( GetResolution(), GetResolution(), DXGI_FORMAT_R16G16_FLOAT ) ||
		!m_ThermalErosionOutFlowMap1.Init( GetResolution(), GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT ) ||
		!m_ThermalErosionOutFlowMap2.Init( GetResolution(), GetResolution(), DXGI_FORMAT_R16G16B16A16_FLOAT ) ||
		!m_SuspendedSoilMap[0].Init( GetResolution(), IOType::Float ) ||
		!m_SuspendedSoilMap[1].Init( GetResolution(), IOType::Float ) )
		return false;

	m_HeightMap.SetDebugName( "HeightMap" );
	m_WaterMap.SetDebugName( "WaterMap" );
	m_WaterOutFlowMap.SetDebugName( "WaterOutFlowMap" );
	m_WaterVelocityMap.SetDebugName( "WaterVelocityMap" );
	m_ThermalErosionOutFlowMap1.SetDebugName( "m_ThermalErosionOutFlowMap1" );
	m_ThermalErosionOutFlowMap2.SetDebugName( "m_ThermalErosionOutFlowMap2" );
	m_SuspendedSoilMap[ 0 ].SetDebugName( "SuspendedSoilMap0" );
	m_SuspendedSoilMap[ 1 ].SetDebugName( "SuspendedSoilMap1" );

	return true;
}

const Map* ErosionNode::GetOutput( uint32_t _idx ) const
{
	switch( _idx )
	{
		case 0: return &m_HeightMap;
		case 1: return &m_WaterMap;
	//	case 2: return &m_FlowMap;
	}

	return nullptr;
}

void ErosionNode::InitSim()
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	const Map* pInputMap = GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;

	//Copy input to Output

	pDevCtx->CopyResource( m_HeightMap.GetTex(), pInputMap->GetTex() );

	//Clear UAVs

	FLOAT fZero[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pDevCtx->ClearUnorderedAccessViewFloat( m_WaterMap.GetUAV(), fZero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_WaterOutFlowMap.GetUAV(), fZero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_WaterVelocityMap.GetUAV(), fZero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_SuspendedSoilMap[ 0 ].GetUAV(), fZero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_SuspendedSoilMap[ 1 ].GetUAV(), fZero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_ThermalErosionOutFlowMap1.GetUAV(), fZero );
	pDevCtx->ClearUnorderedAccessViewFloat( m_ThermalErosionOutFlowMap2.GetUAV(), fZero );

	m_PingPongIndex = 0;
}

void ErosionNode::StepSim( bool _rebindResources, bool _unbindResourcesOnExit )
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();


	//Update constant buffer //TODO if not LiveSim(tm) update CB once in InitSim

	m_CB.TerrainResolution = GetResolution();
	m_CB.TerrainExtent = theApp.GetTerrainExtent();
	m_CB.CellSizeX = theApp.GetTerrainExtent() / (float)(GetResolution() - 1);
	m_CB.CellSizeZ = theApp.GetTerrainExtent() / (float)(GetResolution() - 1);
	m_CB.MinAltitude = theApp.GetMinAltitude();
	m_CB.MaxAltitude = theApp.GetMaxAltitude();

	m_CB.dt = m_ParameterSlots[ 1 ].m_Value.f;
	m_CB.RainRate = m_ParameterSlots[ 2 ].m_Value.f;
	m_CB.EvaporationRate = m_ParameterSlots[ 3 ].m_Value.f;
	m_CB.DissolutionRate = m_ParameterSlots[ 4 ].m_Value.f;
	m_CB.DepositionRate = m_ParameterSlots[ 5 ].m_Value.f;
	m_CB.SedimentCapacity = m_ParameterSlots[ 6 ].m_Value.f;
	m_CB.Tan_TalusAngle = tanf( DegToRad( m_ParameterSlots[7].m_Value.f ) );
	m_CB.ThermalErosionSpeed = m_ParameterSlots[8].m_Value.f;

	m_CB.UploadToGPU( pDevCtx );


	if( _rebindResources )
	{
		ID3D11Buffer* pCB = m_CB.GetBuffer();
		pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

		ID3D11UnorderedAccessView* uavs[] =
		{
			m_HeightMap.GetUAV(),
			m_WaterMap.GetUAV(),
			m_WaterOutFlowMap.GetUAV(),
			m_WaterVelocityMap.GetUAV(),
			nullptr,
			m_ThermalErosionOutFlowMap1.GetUAV(),
			m_ThermalErosionOutFlowMap2.GetUAV()
		};

		uint32_t numUAVs = sizeof( uavs ) / sizeof( uavs[ 0 ] );

		pDevCtx->CSSetUnorderedAccessViews( 0, numUAVs, uavs, nullptr );

		ID3D11SamplerState* samplers[] = { g_Renderer.GetBilinearClampSampler() };
		pDevCtx->CSSetSamplers( 0, 1, samplers );
	}

	const uint32_t threadGroupSizeX = 32;
	const uint32_t threadGroupSizeY = 32;

	uint32_t numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11ShaderResourceView*  suspendedSoilSRVs[] = { m_SuspendedSoilMap[ 0 ].GetSRV(), m_SuspendedSoilMap[ 1 ].GetSRV() };
	ID3D11UnorderedAccessView* suspendedSoilUAVs[] = { m_SuspendedSoilMap[ 0 ].GetUAV(), m_SuspendedSoilMap[ 1 ].GetUAV() };

	pDevCtx->CSSetShader( m_AddWaterCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//----------

	pDevCtx->CSSetShader( m_UpdateWaterOutFlowCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//----------

	pDevCtx->CSSetShader( m_UpdateWaterHeightAndVelocityCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//----------

	pDevCtx->CSSetShader( m_ThermalErosionOutFlowCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//----------

	pDevCtx->CSSetUnorderedAccessViews( 4, 1, suspendedSoilUAVs + m_PingPongIndex, nullptr );

	pDevCtx->CSSetShader( m_HydraulicErosionCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//----------

	pDevCtx->CSSetUnorderedAccessViews( 4, 1, suspendedSoilUAVs + (1 - m_PingPongIndex), nullptr );
	pDevCtx->CSSetShaderResources( 0, 1, suspendedSoilSRVs + m_PingPongIndex );

	pDevCtx->CSSetShader( m_SoilTransportationCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	ID3D11ShaderResourceView*  nullSRV = nullptr;
	pDevCtx->CSSetShaderResources( 0, 1, &nullSRV );

	//----------

	pDevCtx->CSSetShader( m_ThermalErosionCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//----------

	pDevCtx->CSSetShader( m_WaterEvaporationCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );


	//---------

	m_PingPongIndex = 1 - m_PingPongIndex;

	if( _unbindResourcesOnExit )
	{
		static ID3D11UnorderedAccessView* nullUAVs[ 8 ] = { NULL };
		static ID3D11ShaderResourceView* nullSRVs[ 1 ] = { NULL };

		pDevCtx->CSSetUnorderedAccessViews( 0, 8, nullUAVs, nullptr );
		pDevCtx->CSSetShaderResources( 0, 8, nullSRVs );
	}
}

void ErosionNode::InternalCompute()
{
#ifdef USE_RENDERDOC
	g_Renderer.RenderDoc_StartCapture();
#endif

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	InitSim();

	StepSim( true, false );

	int numIterations = m_ParameterSlots[ 0 ].m_Value.i;

	for( int i = 0 ; i < numIterations - 1 ; ++i )
	{
		StepSim( false, false );
	}

	//Deposit all suspended soil

	const uint32_t threadGroupSizeX = 32;
	const uint32_t threadGroupSizeY = 32;

	uint32_t numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;
	
	pDevCtx->CSSetShader( m_DepositAllSuspendedSoilCS, nullptr, 0 );
	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );
	
	//Unbind resources
	static ID3D11UnorderedAccessView* nullUAVs[8] = { NULL };
	pDevCtx->CSSetUnorderedAccessViews( 0, 8, nullUAVs, nullptr );

	//TODO unbind SRV
#ifdef USE_RENDERDOC
	g_Renderer.RenderDoc_EndCapture();
#endif

}

bool ErosionNode::RenderSimPreview( const GridMesh& _gridMesh )
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	pDevCtx->VSSetShader( m_pPreviewVertexShader, nullptr, 0 );
	pDevCtx->PSSetShader( m_pPreviewPixelShader, nullptr, 0 );

	ID3D11ShaderResourceView* heightSRV = m_HeightMap.GetSRV();
	ID3D11ShaderResourceView* waterSRV = m_WaterMap.GetSRV();
	ID3D11ShaderResourceView* suspendedSoilSRV = m_SuspendedSoilMap[m_PingPongIndex].GetSRV();

	pDevCtx->VSSetShaderResources( 0, 1, &heightSRV );
	pDevCtx->PSSetShaderResources( 0, 1, &heightSRV );
	pDevCtx->VSSetShaderResources( 3, 1, &waterSRV );
	pDevCtx->PSSetShaderResources( 3, 1, &waterSRV );

	pDevCtx->PSSetShaderResources( 4, 1, &suspendedSoilSRV );

	ID3D11SamplerState* sampler = g_Renderer.GetBilinearClampSampler();
	pDevCtx->VSSetSamplers( 0, 1, &sampler );
	pDevCtx->PSSetSamplers( 0, 1, &sampler );

	_gridMesh.Render( pDevCtx );

	return true;
}
