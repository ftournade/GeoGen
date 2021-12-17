#pragma once

#include "ComputeNode.h"

class InputBitmapNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( InputBitmapNode )

	InputBitmapNode();
	virtual ~InputBitmapNode();

	const Str& GetFilename();
	void SetFilename( const char* _filename );

	virtual bool OnResolutionChanged();
	virtual const Map* GetOutput( u32 _idx ) const;
protected:
	virtual void InternalCompute();

private:
	Map m_Bitmap;
};

