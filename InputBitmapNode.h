#pragma once

#include "ComputeNode.h"

class InputBitmapNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( InputBitmapNode )

	InputBitmapNode();
	virtual ~InputBitmapNode();

	const std::string& GetFilename();
	void SetFilename( const char* _filename );

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( uint32_t _idx ) const;
protected:
	virtual void InternalCompute();

private:
	Map m_Bitmap;
};

