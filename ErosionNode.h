#pragma once

#include "ComputeNode.h"

#define cbuffer struct
#include "Shaders/ErosionConstants.h"
#undef cbuffer

class ErosionNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ErosionNode )
		
	ErosionNode();
	virtual ~ErosionNode();

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( uint32_t _idx ) const;

	//Used by LiveSim
	virtual void InitSim();
	virtual void StepSim( bool _rebindResources, bool _unbindResourcesOnExit );
	virtual bool RenderSimPreview( const GridMesh& _gridMesh );

protected:
	virtual void InternalCompute();

private:
	Map m_HeightMap, m_WaterMap, m_WaterOutFlowMap, m_WaterVelocityMap, m_ThermalErosionOutFlowMap1, m_ThermalErosionOutFlowMap2;
	Map m_SuspendedSoilMap[2]; //ping-pong buffers
	int m_PingPongIndex;

	D3DObject< ID3D11ComputeShader > m_AddWaterCS;
	D3DObject< ID3D11ComputeShader > m_UpdateWaterOutFlowCS;
	D3DObject< ID3D11ComputeShader > m_ThermalErosionOutFlowCS;
	D3DObject< ID3D11ComputeShader > m_ThermalErosionCS;
	D3DObject< ID3D11ComputeShader > m_UpdateWaterHeightAndVelocityCS;
	D3DObject< ID3D11ComputeShader > m_HydraulicErosionCS;
	D3DObject< ID3D11ComputeShader > m_SoilTransportationCS;
	D3DObject< ID3D11ComputeShader > m_WaterEvaporationCS;
	D3DObject< ID3D11ComputeShader > m_DepositAllSuspendedSoilCS;
	//D3DObject< ID3D11ComputeShader > m_SmoothTerrainCS;

	ConstantBuffer<ErosionConstants> m_CB;

	D3DObject< ID3D11VertexShader > m_pPreviewVertexShader;
	D3DObject< ID3D11PixelShader >  m_pPreviewPixelShader;

};

