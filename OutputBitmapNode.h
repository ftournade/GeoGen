#pragma once

#include "ComputeNode.h"

class OutputBitmapNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( OutputBitmapNode )

	OutputBitmapNode();
	virtual ~OutputBitmapNode();

	const std::string& GetFilename();
	void SetFilename( const char* _filename );

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( uint32_t _idx ) const;
protected:
	virtual void InternalCompute();

};

