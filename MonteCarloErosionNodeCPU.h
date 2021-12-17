#pragma once

#include "ComputeNode.h"
#include "CPUFloatMap.h"

struct HydraulicErosionEventParams
{
//	float dt,
	float	RainRate,
			EvaporationRate,
			ErosionRate,
			DepositionRate,
			SedimentCapacity,
			ErosionSlopeThreshold;
};

class MonteCarloErosionNodeCPU : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( MonteCarloErosionNodeCPU )
		
	MonteCarloErosionNodeCPU();
	virtual ~MonteCarloErosionNodeCPU();

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( u32 _idx ) const;

	virtual void InitSim();
	virtual void StepSim( bool _rebindResources, bool _unbindResourcesOnExit );
	virtual bool RenderSimPreview( const GridMesh& _gridMesh );

protected:
	virtual void InternalCompute();

private:
	void CopyResultsToGPU();

	inline float GetAltitude( const Vec2i& p ) const; //sum of BedRock + BrockenRock + Sand + Humus layers
	float GetNeighborAverageAltitude( const Vec2i& _p ) const;
		   float GetSlope( const Vec2i& p, Vec2i& _lowestNGB ) const; //same as above
	inline float GetSlope( const Vec2i& a, const Vec2i& b ) const; //same as above
	inline float GetSlope( const Vec2i& a, float aAlt, const Vec2i& b ) const; //same as above
	inline bool OutOfMap( const Vec2i & p ) const;

	bool PickRandomDownhillNeighboor( const Vec2i& _p, const Vec2i& _prev_p, Vec2i& _ngb, float& _slope ) const;

	void HydraulicErosionEvent( const Vec2i& _p0, const HydraulicErosionEventParams& _param );
	void ThermalErosionEvent( const Vec2i& _p0 );
	void GravityEvent( const Vec2i& _p0 );
	void LightningEvent( const Vec2i& _p0 );
	void EcosystemEvent( const Vec2i& _p0 );

	void SmoothTerrain(); //temp test, thermal erosion will smooth the terrain
private:
	Map m_HeightMap, m_BrockenRockMap, m_SandMap, m_HumusMap, m_VegetationMap, m_FlowMap;

	float m_CellSize;

	//TODO evaluate SOA vs AOS perfs ( CPUFloatMap<CellData> )
	CPUFloatMap m_CPUBedRockMap, 
				m_CPUBrockenRockMap,
				m_CPUSandMap, 
				m_CPUHumusMap, 
				m_CPUVegetationMap,
				m_CPUDeadVegetationMap,
				m_CPUFlowMap; //not part of sim, purely visual

	//TODO vegetation, humidity ...

	D3DObject< ID3D11VertexShader > m_pPreviewVertexShader;
	D3DObject< ID3D11PixelShader >  m_pPreviewPixelShader;

	int m_ParamMultithreaded;
	int m_ParamIterations;
	int m_ParamTimePerIteration;

	int m_ParamHydraulicErosionProb;
	int m_ParamThermalErosionProb;
	int m_ParamGravityProb;
	int m_ParamLightningProb;
	int m_ParamEcosystemProb;

	int m_ParamRainRate;
	int m_ParamEvaporationRate;
	int m_ParamErosionRate;
	int m_ParamDepositionRate;
	int m_ParamSedimentCapacity;
	int m_ParamErosionSlopeThreshold;

	int m_paramTalusAngle;
	int m_paramGravitySpeed;

	int m_ParamSmoothTerrain;
};


/////////////////////////////////////////

inline float MonteCarloErosionNodeCPU::GetAltitude( const Vec2i& p ) const
{
	return m_CPUBedRockMap( p ) + m_CPUBrockenRockMap( p ) + m_CPUSandMap( p ) + m_CPUHumusMap( p );
}

inline float MonteCarloErosionNodeCPU::GetSlope( const Vec2i& a, const Vec2i& b ) const
{
	float alt1 = GetAltitude( a );
	float alt2 = GetAltitude( b );

	float dx = (b.x - a.x) * m_CellSize;
	float dy = (b.y - a.y) * m_CellSize;

	float dist = Sqrt( dx * dx + dy * dy ); //this can be precomputed for all 8 possible neighboors

	return (alt2 - alt1) / dist;
}

inline float MonteCarloErosionNodeCPU::GetSlope( const Vec2i& a, float aAlt, const Vec2i& b ) const
{
	float alt1 = aAlt;
	float alt2 = GetAltitude( b );

	float dx = (b.x - a.x) * m_CellSize;
	float dy = (b.y - a.y) * m_CellSize;

	float dist = Sqrt( dx * dx + dy * dy );

	return (alt2 - alt1) / dist;
}

inline bool MonteCarloErosionNodeCPU::OutOfMap( const Vec2i & p ) const
{
	return (p.x < 0) || (p.x >= (int)GetResolution()) || (p.y < 0) || (p.y >= (int)GetResolution());
}
