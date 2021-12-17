#pragma once
#include "ComputeNode.h"

namespace MCErosion
{
	#define cbuffer struct
	#include "Shaders/MonteCarloErosionConstants.h"
	#undef cbuffer
}

class MonteCarloErosionNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( MonteCarloErosionNode )

	MonteCarloErosionNode();
	virtual ~MonteCarloErosionNode();

	virtual bool OneTimeInit();

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( u32 _idx ) const;

	virtual void InitSim();
	virtual void StepSim( bool _rebindResources, bool _unbindResourcesOnExit );
	virtual bool RenderSimPreview( const GridMesh& _gridMesh );

protected:
	virtual void InternalCompute();

	void UpdateConstantBuffer();

	void ComputeHydraulicEvents();
	void ComputeGravityEvents();

	void SmoothSoil();
	void ComputeTotalTerrainAltitude() const;

private:
	mutable Map m_HeightMap; //node main output (sum of all layers below)

	Map m_BedRockMap,
		m_BrockenRockMap,
		m_SandMap,
		m_HumusMap,
		m_VegetationMap,
		m_DeadVegetationMap;

	Map m_FlowMap;

	D3DObject< ID3D11ComputeShader > m_pInitSimCS, m_HydraulicCS, m_GravityCS, m_pSmoothSoilCS, m_pTotalTerrainAltituteCS;

	D3DObject< ID3D11VertexShader > m_pPreviewVertexShader;
	D3DObject< ID3D11PixelShader >  m_pPreviewPixelShader;

	ConstantBuffer< MCErosion::Constants > m_CB;

	int m_ParamIterations, m_ParamSmoothSoil;

	//Gravity
	int m_ParamTalusAngle, m_ParamSpeed;

	//Hydraulic erosion
	int m_ParamHydraulicSpeedVsQuality, m_ParamRainRate, m_ParamEvaporationRate, m_ParamErosionRate,
		m_ParamDepositionRate, m_ParamErosionSlopeThreshold, m_ParamSedimentCapacity;

	int m_Step;

};

