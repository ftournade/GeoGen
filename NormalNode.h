#pragma once

#include "CustomComputeNode.h"

class NormalNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( NormalNode )

	NormalNode() : CustomComputeNode()
	{
		SetUIName( "Noise" );

		AddInput( "HeightMap", IOType::Float );
		AddOutput( "NormalMap", IOType::Color );

		//	SetHLSLPrefix( "" );

		SetHLSLBody( //TODO clamp to borders
			"float x1 = Terrain( _input0[ uint2(_pos.x - 1, _pos.y) ] );\n"
			"float x2 = Terrain( _input0[ uint2(_pos.x + 1, _pos.y) ] );\n"
			"float y1 = Terrain( _input0[ uint2(_pos.x, _pos.y - 1) ] );\n"
			"float y2 = Terrain( _input0[ uint2(_pos.x, _pos.y + 1) ] );\n\n"
			"float quadLength = Extent / (float)Resolution;\n"
			"float3 normal = float3( x2 - x1, 2.0f * quadLength, y2 - y1 );\n"
			"_output0[ _pos ] = float4( normalize( normal ), 1.0f );\n"
		);
	}

	virtual ~NormalNode() {}
};