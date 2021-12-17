#pragma once

#include "ComputeNode.h"

class OutputBitmapNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( OutputBitmapNode )

	OutputBitmapNode();
	virtual ~OutputBitmapNode();

	const Str& GetFilename();
	void SetFilename( const char* _filename );

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( u32 _idx ) const;
protected:
	virtual void InternalCompute();

};

