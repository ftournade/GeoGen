#pragma once
#include "ComputeNode.h"

struct ExtractDetailNodeConstants
{
	int KernelSize;
	float BlurRadius;
	Vec2 BlurDir;
//	float pad[ 2 ];
};

//TODO handle Color bluring
class ExtractDetailNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ExtractDetailNode )

	ExtractDetailNode();
	virtual ~ExtractDetailNode();

	virtual bool OneTimeInit();

	virtual const Map* GetOutput( u32 _idx ) const;

protected:
	virtual void InternalCompute();

	virtual bool OnResolutionChanged();
private:
	Map m_Output, m_IntermediateOutput; //need a 2nd texture for separable gaussian (2-passes)

	ConstantBuffer< ExtractDetailNodeConstants > m_CB;

	D3DObject< ID3D11ComputeShader > m_CSBlur, m_CSBlurAndSubtractFromOriginal;
};

