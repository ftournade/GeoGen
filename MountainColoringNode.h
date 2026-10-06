#pragma once
#include "ComputeNode.h"

namespace MountainColoring
{
	struct float4 { float x, y, z, w; };

	#define cbuffer struct
	#include "Shaders/MountainColoringConstants.h"
	#undef cbuffer
}

//Color map of runevision's "Advanced Terrain Erosion Filter" Shadertoy (MPL-2.0, see Shaders/MountainColoring.hlsl)
//Inputs: eroded heightmap, and the Erosion/Ridges masks of the Fake Erosion V2 node
//Outputs: albedo and lighting color maps (both gamma encoded, Albedo x Lighting = lit color), tree coverage mask
class MountainColoringNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( MountainColoringNode )

	MountainColoringNode();
	virtual ~MountainColoringNode();

	virtual bool OneTimeInit();

	virtual const Map* GetOutput( uint32_t _idx ) const;

protected:
	virtual void InternalCompute();

	virtual bool OnResolutionChanged();

private:
	void UpdateConstantBuffer();

	int AddColorParam( const char* _name, float _r, float _g, float _b ); //linear color, stored gamma encoded
	MountainColoring::float4 GetLinearColor( int _param ) const;

private:
	Map m_AlbedoMap, m_LightingMap, m_TreesMap;

	ConstantBuffer< MountainColoring::MountainColoringConstants > m_CB;

	D3DObject< ID3D11ComputeShader > m_CS;

	//Parameter slot indices
	int m_ParamWaterLevel, m_ParamGrassLevel, m_ParamCliffStart, m_ParamCliffEnd, m_ParamSnowStart, m_ParamSnowEnd;
	int m_ParamWater, m_ParamDrainage, m_ParamDrainageWidth, m_ParamTrees, m_ParamBreakupAmount, m_ParamBreakupScale;
	int m_ParamShadows, m_ParamSunAzimuth, m_ParamSunElevation, m_ParamExposure;
	int m_ParamCliffColor, m_ParamDirtColor, m_ParamTreeColor, m_ParamGrassColor1, m_ParamGrassColor2,
		m_ParamSandColor, m_ParamWaterColor, m_ParamWaterShoreColor;
};
