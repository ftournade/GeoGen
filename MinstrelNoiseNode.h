#pragma once
#include "ComputeNode.h"

//GPU port of https://github.com/smcameron/pseudo-erosion & https://www.reddit.com/r/proceduralgeneration/comments/797fgw/iterative_pseudoerosion/
/*
class MinstrelNoiseNode : public ComputeNode
{
public:
	MinstrelNoiseNode();
	virtual ~MinstrelNoiseNode();

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( u32 _idx ) const;

protected:
	virtual void InternalCompute();

private:
	Map m_GridPointsMap;
	Map m_OutputMap;

	D3DObject< ID3D11ComputeShader > m_SetupGridCS, m_MainCS;
};

*/

#include "CustomComputeNode.h"

class MinstrelNoiseNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( MinstrelNoiseNode )

	MinstrelNoiseNode();
	virtual ~MinstrelNoiseNode() {}
};