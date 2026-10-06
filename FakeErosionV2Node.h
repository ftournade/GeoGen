#pragma once
#include "ComputeNode.h"

namespace FakeErosionV2
{
	#define cbuffer struct
	#include "Shaders/FakeErosionV2Constants.h"
	#undef cbuffer
}

//Erosion filter from runevision's "Advanced Terrain Erosion Filter" Shadertoy (MPL-2.0, see Shaders/FakeErosionCommon.h)
//Inputs: base heightmap, optional erosion strength mask
//Outputs: eroded heightmap, and erosion/ridges/drainage masks (e.g. for the Mountain Coloring node)
class FakeErosionV2Node : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( FakeErosionV2Node )

	FakeErosionV2Node();
	virtual ~FakeErosionV2Node();

	virtual bool OneTimeInit();

	virtual const Map* GetOutput( uint32_t _idx ) const;

protected:
	virtual void InternalCompute();

	virtual bool OnResolutionChanged();

private:
	void UpdateConstantBuffer();

private:
	Map m_HeightMap, m_ErosionMap, m_RidgeMap, m_DrainageMap;

	ConstantBuffer< FakeErosionV2::FakeErosionV2Constants > m_CB;

	D3DObject< ID3D11ComputeShader > m_CS;

	//Parameter slot indices
	int m_ParamScale, m_ParamStrength, m_ParamGullyWeight, m_ParamDetail, m_ParamOctaves, m_ParamLacunarity,
		m_ParamGain, m_ParamCellScale, m_ParamNormalization;
	int m_ParamRidgeRounding, m_ParamCreaseRounding, m_ParamInputRounding, m_ParamOctaveRoundingMult;
	int m_ParamOnset, m_ParamOctaveOnset, m_ParamRidgeMapOnset, m_ParamRidgeMapOctaveOnset;
	int m_ParamAssumedSlope, m_ParamAssumedSlopeAmount, m_ParamSlopeRadius;
	int m_ParamHeightOffset, m_ParamPreserveExtremes, m_ParamFadeCenter, m_ParamFadeRange, m_ParamClamp;
	int m_ParamDrainageWidth;
};
