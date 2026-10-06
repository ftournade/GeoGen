#include "stdafx.h"
#include "MonteCarloErosionNodeCPU.h"

#include "RandomPick.h"
#include "GridMesh.h"
#include "GeoGen.h"

MonteCarloErosionNodeCPU::MonteCarloErosionNodeCPU()
{
	m_bIsIterative = true;

	SetUIName( "Erosion (CPU)" );
	SetSize( Vec2( 120, 120 ) );
	

	AddInput( "HeightMap", IOType::Float );
	AddOutput( "HeightMap", IOType::Float );
	AddOutput( "BrockenRock", IOType::Float );
	AddOutput( "Sand", IOType::Float );
	AddOutput( "Humus", IOType::Float );
	AddOutput( "Vegetation", IOType::Float );
	
	AddOutput( "FlowMap", IOType::Float );

	m_ParamMultithreaded			= AddParam( "Main", "Multithreaded", IOType::Bool, ParamEdition::CheckBox, true, false );
	m_ParamIterations				= AddParam( "Main", "Iterations", IOType::Integer, ParamEdition::Slider, 10, 1, 5000 );
	m_ParamTimePerIteration			= AddParam( "Main", "TimePerIteration", IOType::Float, ParamEdition::Slider, 1.0f, 0.0f, 10.0f );
	
	m_ParamHydraulicErosionProb		= AddParam( "Main", "HydraulicErosionProbability", IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 1.0f );
	m_ParamThermalErosionProb		= AddParam( "Main", "ThermalErosionProbability", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 1.0f );
	m_ParamGravityProb				= AddParam( "Main", "GravityProbability", IOType::Float, ParamEdition::Slider, 0.8f, 0.0f, 1.0f );
	m_ParamLightningProb			= AddParam( "Main", "LightningProbability", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 1.0f );
	m_ParamEcosystemProb			= AddParam( "Main", "EcosystemProbability", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 1.0f );

	m_ParamSmoothTerrain			= AddParam( "Main", "SmoothTerrain", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 100.0f );

	m_ParamRainRate					= AddParam( "Hydraulic", "RainRate", IOType::Float, ParamEdition::Slider, 0.3f, 0.0f, 1.0f );
	m_ParamEvaporationRate			= AddParam( "Hydraulic", "EvaporationRate", IOType::Float, ParamEdition::Slider, 0.002f, 0.0f, 0.2f );
	m_ParamErosionRate				= AddParam( "Hydraulic", "ErosionRate", IOType::Float, ParamEdition::Slider, 0.15f, 0.0f, 0.5f );
	m_ParamDepositionRate			= AddParam( "Hydraulic", "DepositionRate", IOType::Float, ParamEdition::Slider, 0.1f, 0.0f, 0.5f );
	m_ParamSedimentCapacity			= AddParam( "Hydraulic", "SedimentCapacity", IOType::Float, ParamEdition::Slider, 30.0f, 0.0f, 200.0f );
	m_ParamErosionSlopeThreshold	= AddParam( "Hydraulic", "ErosionSlopeThreshold", IOType::Float, ParamEdition::Slider, 0.03f, 0.0f, 0.5f );

	m_paramTalusAngle				= AddParam( "Gravity", "TalusAngle", IOType::Float, ParamEdition::Slider, 0.9f, 0.0f, 2.5f );
	m_paramGravitySpeed				= AddParam( "Gravity", "Speed", IOType::Float, ParamEdition::Slider, 0.1f, 0.0f, 0.8f );
		
	if( !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "VSMainErosion2", nullptr, &m_pPreviewVertexShader )  //assume same input layout as m_pTerrainVertexShader
	 || !g_Renderer.CreateShader( "Shaders/Terrain.hlsl", "PSMainErosion2", nullptr, &m_pPreviewPixelShader ) )
	{
		assert( false );
		//return false; //TODO not in constructor, have an Init method
	}

}


MonteCarloErosionNodeCPU::~MonteCarloErosionNodeCPU()
{
}

bool MonteCarloErosionNodeCPU::OnResolutionChanged()
{
	if( !m_HeightMap.Init( GetResolution(), IOType::Float ) 
	 ||	!m_BrockenRockMap.Init( GetResolution(), IOType::Float )
	 ||	!m_SandMap.Init( GetResolution(), IOType::Float )
	 ||	!m_HumusMap.Init( GetResolution(), IOType::Float )
	 ||	!m_VegetationMap.Init( GetResolution(), IOType::Float )
	 || !m_FlowMap.Init( GetResolution(), IOType::Float ) )
		return false;

	m_HeightMap.SetDebugName( "HeightMap" );
	m_BrockenRockMap.SetDebugName( "BrockenRockMap" );
	m_SandMap.SetDebugName( "SandMap" );
	m_HumusMap.SetDebugName( "HumusMap" );
	m_VegetationMap.SetDebugName( "VegetationMap" );
	m_FlowMap.SetDebugName( "FlowMap" );

	return true;
}

const Map* MonteCarloErosionNodeCPU::GetOutput( uint32_t _idx ) const
{
	switch( _idx )
	{
		case 0: return &m_HeightMap;
		case 1: return &m_BrockenRockMap;
		case 2: return &m_SandMap;
		case 3: return &m_HumusMap;
		case 4: return &m_VegetationMap;
		case 5: return &m_FlowMap;
	}

	return nullptr;
}


void MonteCarloErosionNodeCPU::InitSim()
{
	const Map* pInputMap = GetRemoteInputMap( 0 );

	if( !pInputMap )
		return;


	int res = GetResolution();

	m_CPUBedRockMap.Resize( res, res );
	m_CPUBrockenRockMap.Resize( res, res );
	m_CPUSandMap.Resize( res, res );
	m_CPUHumusMap.Resize( res, res );
	m_CPUVegetationMap.Resize( res, res );
	m_CPUFlowMap.Resize( res, res );

	pInputMap->CopyFromGPU( m_CPUBedRockMap.GetData() );

	for( int y = 0; y < res ; ++y ) //TODO functor on CPUFloatMap, or better yet, rescale with GPU
	{
		for( int x = 0; x < res ; ++x )
		{
			m_CPUBedRockMap(Vec2i(x,y)) = Lerp( (float)theApp.GetMinAltitude(), (float)theApp.GetMaxAltitude(), m_CPUBedRockMap(Vec2i(x,y)) );
		}
	}

	m_CPUBrockenRockMap.Fill( 0.0f );
	m_CPUSandMap.Fill( 0.0f );
	m_CPUHumusMap.Fill( 0.0f );
	m_CPUFlowMap.Fill( 0.0f );

	m_CellSize = theApp.GetTerrainExtent() / (float)(res - 1);
}

static const Vec2i neighboor[ 8 ] =
{
	{ -1, -1 },
	{ -1,  0 },
	{ -1,  1 },
	{ 0, -1 },
	{ 0,  1 },
	{ 1, -1 },
	{ 1,  0 },
	{ 1,  1 }
};

bool MonteCarloErosionNodeCPU::PickRandomDownhillNeighboor( const Vec2i& _p, const Vec2i& _prev_p, Vec2i& _ngb, float& _slope ) const
{
	float p_alt = GetAltitude( _p );

	RandomPick<8> randomPickDirection;
	bool bReachedLocalMinimum = true;

	float ngb_slope[ 8 ];

	for( int i = 0; i < 8; ++i )
	{
		Vec2i ngb = _p + neighboor[ i ];

		if( (ngb == _prev_p) || OutOfMap( ngb ) ) //Don't backtrack and don't leave map
		{
			//TODO kill agent instead ?
			randomPickDirection.AddItem( 0.0f );
			continue;
		}

		ngb_slope[i] = GetSlope( _p, p_alt, ngb );

		if( ngb_slope[i] > 0.0f )
		{
			//uphill
			randomPickDirection.AddItem( 0.0f );
			continue;
		}

		bReachedLocalMinimum = false;

		float directionProbability = -ngb_slope[i] + 0.001f;

		randomPickDirection.AddItem( directionProbability );
	}

	if( bReachedLocalMinimum )
		return false;

	uint32_t ngbIndex = randomPickDirection.PickRandomly();

	_ngb = _p + neighboor[ ngbIndex ];
	_slope = ngb_slope[ ngbIndex ];

	return true;
}

float MonteCarloErosionNodeCPU::GetNeighborAverageAltitude( const Vec2i& _p ) const
{
	float avg = 0;
	int cnt = 0;

	for( int i = 0; i < 8; ++i )
	{
		Vec2i ngb = _p + neighboor[ i ];

		if( OutOfMap( ngb ) )
			continue;

		avg += GetAltitude( ngb );
		++cnt;
	}

	return avg / (float)cnt;
}


inline float MonteCarloErosionNodeCPU::GetSlope( const Vec2i& p, Vec2i& _lowestNGB ) const
{
	float minAlt = 9999999.0f;

	//find lowest neighbor
	for( int i = 0; i < 8; ++i )
	{
		Vec2i ngb = p + neighboor[ i ];

		if( OutOfMap( ngb ) )
			continue;

		float alt = GetAltitude( ngb );

		if( alt < minAlt )
		{
			minAlt = alt;
			_lowestNGB = neighboor[ i ];
		}
	}

	float dx = _lowestNGB.x * m_CellSize;
	float dy = _lowestNGB.y * m_CellSize;

	float dist = Sqrt( dx * dx + dy * dy );

	return (GetAltitude( p ) - minAlt) / dist;
}


void MonteCarloErosionNodeCPU::HydraulicErosionEvent( const Vec2i& _p0, const HydraulicErosionEventParams& _param )
{
	Vec2i p = _p0;
	Vec2i prev_p( -1, -1 );

	float water = _param.RainRate;
	float slope = 0.0f;
	float carriedSediment = 0.0f;

	while( water > 0.0f ) //TODO hard limit on event length
	{
		m_CPUFlowMap( p ) += water;

		Vec2i new_p;

		if( !PickRandomDownhillNeighboor( p, prev_p, new_p, slope ) )
		{
			//reached local minimum
			//"fill the hole" and then a bit more to exit minima

			float deposition = GetNeighborAverageAltitude( p ) - m_CPUBedRockMap( p ) + 0.0001f;
			deposition = Min( deposition, carriedSediment );

			carriedSediment -= deposition;
			m_CPUBedRockMap( p ) += deposition;

			if( !PickRandomDownhillNeighboor( p, prev_p, new_p, slope ) ) //Try again
				break;
		}


		if( slope < -_param.ErosionSlopeThreshold )
		{
			if( carriedSediment < _param.SedimentCapacity * water )
			{
				//erosion
				float erosion = Abs( slope ) * _param.ErosionRate;
				erosion = Min( erosion, _param.SedimentCapacity * water - carriedSediment ); //Don't exceed sediment capacity
				carriedSediment += erosion;
				m_CPUBedRockMap( p ) -= erosion;
			}
		}
//		else
		{
			//deposition
			float deposition = _param.DepositionRate;
			deposition = min( deposition, carriedSediment );
			carriedSediment -= deposition;
			m_CPUBedRockMap( p ) += deposition; //TODO add rocks or sand not bedrock
		}

		water -= _param.EvaporationRate;
		water = Max( water, 0.0f );

		if( carriedSediment > _param.SedimentCapacity * water )
		{
			//deposit sediment if water capacity exceeded

			float deposition = carriedSediment - _param.SedimentCapacity * water;
			carriedSediment -= deposition;
			m_CPUBedRockMap( p ) += deposition; //TODO add rocks or sand not bedrock
		}

		prev_p = p;
		p = new_p;
	}

}

void MonteCarloErosionNodeCPU::ThermalErosionEvent( const Vec2i& _p0 )
{

}

void MonteCarloErosionNodeCPU::GravityEvent( const Vec2i& _p0 )
{
	Vec2i p = _p0;
	Vec2i prev_p( -1, -1 );

	float talusAngle = m_ParameterSlots[ m_paramTalusAngle ].m_Value.f;
	float speed = m_ParameterSlots[ m_paramGravitySpeed ].m_Value.f;
	
	Vec2i lowestNGB;

	float slope = GetSlope( p, lowestNGB );
		
	while( slope > talusAngle )
	{
		float materialQty = /*speed * */m_CellSize * (slope - talusAngle);
		materialQty = Min( materialQty, speed * 10.0f );
		//materialQty *= Random();


		Vec2i new_p;

		if( !PickRandomDownhillNeighboor( p, prev_p, new_p, slope ) )
		{
			//reached local minimum
			return;
		}

		m_CPUBedRockMap( p ) -= materialQty;
		m_CPUBrockenRockMap( new_p ) += materialQty;

		prev_p = p;
		p = new_p;

		slope = GetSlope( p, lowestNGB );
	}
}

void MonteCarloErosionNodeCPU::LightningEvent( const Vec2i& _p0 )
{

}

void MonteCarloErosionNodeCPU::EcosystemEvent( const Vec2i& _p0 ) 
{

}


void MonteCarloErosionNodeCPU::StepSim( bool _rebindResources, bool _unbindResourcesOnExit )
{
	HydraulicErosionEventParams hydraulicErosionParams;
	float dt										= m_ParameterSlots[ m_ParamTimePerIteration ].m_Value.f;
	hydraulicErosionParams.RainRate					= m_ParameterSlots[ m_ParamRainRate ].m_Value.f * dt;
	hydraulicErosionParams.EvaporationRate			= m_ParameterSlots[ m_ParamEvaporationRate ].m_Value.f * dt;
	hydraulicErosionParams.ErosionRate				= m_ParameterSlots[ m_ParamErosionRate ].m_Value.f * dt;
	hydraulicErosionParams.DepositionRate			= m_ParameterSlots[ m_ParamDepositionRate ].m_Value.f * dt;
	hydraulicErosionParams.SedimentCapacity			= m_ParameterSlots[ m_ParamSedimentCapacity ].m_Value.f;
	hydraulicErosionParams.ErosionSlopeThreshold	= m_ParameterSlots[ m_ParamErosionSlopeThreshold ].m_Value.f;

	RandomPick<5> randomEventPicker;
	randomEventPicker.AddItem( m_ParameterSlots[ m_ParamHydraulicErosionProb ].m_Value.f );
	randomEventPicker.AddItem( m_ParameterSlots[ m_ParamThermalErosionProb ].m_Value.f );
	randomEventPicker.AddItem( m_ParameterSlots[ m_ParamGravityProb ].m_Value.f );
	randomEventPicker.AddItem( m_ParameterSlots[ m_ParamLightningProb ].m_Value.f );
	randomEventPicker.AddItem( m_ParameterSlots[ m_ParamEcosystemProb ].m_Value.f );


	int numEventsPerStep = 50000;
	bool bMultiThreaded = m_ParameterSlots[ m_ParamMultithreaded ].m_Value.b;

	if( bMultiThreaded )
	{

		#pragma omp parallel for //there's no thread safety in this algorithm, but events should rarely overlap, and when they do it shouldn't be a big deal
		for( int evt = 0; evt < numEventsPerStep; ++evt )
		{
			Vec2i p0( xtm::Random( 0, (int)GetResolution() - 1 ), Random( 0, (int)GetResolution() - 1 ) );

			switch( randomEventPicker.PickRandomly() )
			{
			case 0: HydraulicErosionEvent( p0, hydraulicErosionParams ); break;
			case 1: ThermalErosionEvent( p0 ); break;
			case 2: GravityEvent( p0 ); break;
			case 3: LightningEvent( p0 ); break;
			case 4: EcosystemEvent( p0 ); break;
			}

		}
	}
	else
	{
		for( int evt = 0; evt < numEventsPerStep; ++evt )
		{
			Vec2i p0( xtm::Random( 0, (int)GetResolution() - 1 ), Random( 0, (int)GetResolution() - 1 ) );

			switch( randomEventPicker.PickRandomly() )
			{
			case 0: HydraulicErosionEvent( p0, hydraulicErosionParams ); break;
			case 1: ThermalErosionEvent( p0 ); break;
			case 2: GravityEvent( p0 ); break;
			case 3: LightningEvent( p0 ); break;
			case 4: EcosystemEvent( p0 ); break;
			}

		}

	}

	SmoothTerrain();

	if( _unbindResourcesOnExit ) //means we're live previewing
		CopyResultsToGPU();
}

void MonteCarloErosionNodeCPU::SmoothTerrain()
{ //temp hack until thermal erosion is implemented

	const float blur = m_ParameterSlots[ m_ParamSmoothTerrain ].m_Value.f / 10000.0f;

	if( blur == 0.0f )
		return;

	CPUFloatMap src = m_CPUBedRockMap;

	float scale = 1.0f / (1.0f + blur * 8.0f);

	for( int y = 1 ; y < GetResolution() - 1 ; ++y )
	{
		for( int x = 1 ; x < GetResolution() - 1 ; ++x )
		{
			float h;
			h  = src( Vec2i( x - 1, y - 1 ) );
			h += src( Vec2i( x,     y - 1 ) );
			h += src( Vec2i( x + 1, y - 1 ) );

			h += src( Vec2i( x - 1, y ) );
			h += src( Vec2i( x + 1, y ) );

			h += src( Vec2i( x - 1, y + 1 ) );
			h += src( Vec2i( x,     y + 1 ) );
			h += src( Vec2i( x + 1, y + 1 ) );

			h *= blur;

			h += src( Vec2i( x, y ) );
			h *= scale;

			m_CPUBedRockMap( Vec2i( x, y ) ) = h;

		}
	}
}


void MonteCarloErosionNodeCPU::InternalCompute()
{

	InitSim();

	StepSim( true, false );

	int numIterations = m_ParameterSlots[ m_ParamIterations ].m_Value.i;

	for( int i = 0 ; i < numIterations - 1 ; ++i )
	{
		StepSim( false, false );
	}

	CopyResultsToGPU();


//	static ID3D11UnorderedAccessView* nullUAVs[8] = { NULL };
//	pDevCtx->CSSetUnorderedAccessViews( 0, 8, nullUAVs, nullptr );
	//TODO unbind SRV
}

void MonteCarloErosionNodeCPU::CopyResultsToGPU()
{
	float scale = 1.0f / (float)(theApp.GetMaxAltitude() - theApp.GetMinAltitude());

	vector< float > heightmap( GetResolution() * GetResolution() );

	Vec2i i;

	for( i.y = 0; i.y < GetResolution(); ++i.y ) //TODO functor on CPUFloatMap, or better yet, rescale with GPU
	{
		for( i.x = 0; i.x < GetResolution(); ++i.x )
		{
			float altitude = m_CPUBedRockMap( i ) + m_CPUBrockenRockMap( i ) + m_CPUSandMap( i );

			altitude = ( altitude - (float)theApp.GetMinAltitude() ) * scale;

			heightmap[ i.x + i.y * GetResolution() ] = altitude;
		}
	}

	m_HeightMap.CopyToGPU( &heightmap[0] );

	m_FlowMap.CopyToGPU( m_CPUFlowMap.GetData() );
	m_BrockenRockMap.CopyToGPU( m_CPUBrockenRockMap.GetData() );
	m_SandMap.CopyToGPU( m_CPUSandMap.GetData() );
	m_HumusMap.CopyToGPU( m_CPUHumusMap.GetData() );
	m_VegetationMap.CopyToGPU( m_CPUVegetationMap.GetData() );

}

bool MonteCarloErosionNodeCPU::RenderSimPreview( const GridMesh& _gridMesh )
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	pDevCtx->VSSetShader( m_pPreviewVertexShader, nullptr, 0 );
	pDevCtx->PSSetShader( m_pPreviewPixelShader, nullptr, 0 );

	ID3D11ShaderResourceView* heightSRV = m_HeightMap.GetSRV();
	pDevCtx->VSSetShaderResources( 0, 1, &heightSRV );

	ID3D11ShaderResourceView* srvs[] = { m_HeightMap.GetSRV(), nullptr, m_BrockenRockMap.GetSRV(), nullptr, m_FlowMap.GetSRV() };
	pDevCtx->PSSetShaderResources( 0, countof(srvs), srvs );


	ID3D11SamplerState* sampler = g_Renderer.GetBilinearClampSampler();
	pDevCtx->VSSetSamplers( 0, 1, &sampler );
	pDevCtx->PSSetSamplers( 0, 1, &sampler );

	_gridMesh.Render( pDevCtx );

	return true; 
}
