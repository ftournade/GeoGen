#pragma once

#include "ComputeNode.h"

class PreviewNode :	public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( PreviewNode )

	PreviewNode();
	virtual ~PreviewNode();

	virtual bool OnResolutionChanged();

	virtual const Map* GetOutput( uint32_t _idx ) const;

	const Map* GetHeightMap() const;
	const Map* GetAlbedoMap() const;
	const Map* GetWaterMap() const;

protected:
	virtual void InternalCompute();

protected:
	//DATA
};


