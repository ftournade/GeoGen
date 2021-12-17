#pragma once
#include "ComputeNode.h"

namespace MCSnow
{
	#define cbuffer struct
	#include "Shaders/MonteCarloErosionConstants.h"
	#undef cbuffer
}
class SnowNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( SnowNode )

	SnowNode();
	virtual ~SnowNode();

	virtual bool OneTimeInit();

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( u32 _idx ) const;

	virtual void InitSim();
	virtual void StepSim( bool _rebindResources, bool _unbindResourcesOnExit );
	virtual bool RenderSimPreview( const GridMesh& _gridMesh );

protected:
	virtual void InternalCompute();

	void SmoothSnow();

	void UpdateConstantBuffer();

	void AddSnowAndTerrain() const;

private:
	mutable Map m_HeightMap; //terrain + snow altitude (main output)
	Map m_BedRockMap, m_SnowMap; //snow altitude (secondary output)

	D3DObject< ID3D11ComputeShader > m_CS, m_pInitSimCS, m_pSmoothSnowCS, m_pTerrainPlusSnowCS;

	D3DObject< ID3D11VertexShader > m_pPreviewVertexShader;
	D3DObject< ID3D11PixelShader >  m_pPreviewPixelShader;

	ConstantBuffer< MCSnow::Constants > m_CB;

	int m_ParamIterations, m_ParamTalusAngle, m_ParamSpeed, m_ParamInitialSnowFall, 
		m_ParamSnowFall, m_ParamEvaporation, m_ParamSmoothSnow;

	int m_Step;

};

