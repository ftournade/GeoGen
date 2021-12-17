#pragma once

#include "ComputeNode.h"


//Basic building block to most simple nodes, uses user-defined HLSL code snippet
//Node IO can only be FloatMaps

//Shader input coordinates:
//  _pos : pixel coordinate [0...Res - 1]
// _uv   : normalized coordinate [0...1]
// _wsPos: world-space coordinate [-extent/2 ....extent/2]

//Constants:
// MinAltitude
// MaxAltitude
// Extent
// Resolution

//Functions:
// float Terrain( float h )  : Converts from normalized altitude to altitude in meters
// float NormalizeTerrain( float h )  : Converts from altitude in meters to normalized altitude (doesn't clamp to [0...1] !)

class CustomComputeNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( CustomComputeNode )

	CustomComputeNode();
	CustomComputeNode( u32 _numInputSlots, u32 _numOutputSlots );
	virtual ~CustomComputeNode() {}

	void AddOutput( const char* _name, IOType _type );

	void SetThreadGroupSize( u32 _threadGroupSizeX, u32 _threadGroupSizeY );

	void SetHLSLPrefix( const char* _code ); //includes, resource declarations
	void SetHLSLBody( const char* _code ); //compute shader code

	virtual bool OnResolutionChanged();

	virtual const Map* GetOutput( u32 _idx ) const { return &m_Outputs[ _idx ]; }
	
	bool SetOutputFormat( int _slot, DXGI_FORMAT _fmt );
	
	virtual void OnCompileTimeShaderConstantChanged();

	void InvalidateShader() { m_pComputeShader.Release(); }
protected:
	virtual void InternalCompute();
private:
	bool CompileShader();
	void UpdateConstantBuffer();

private:
	D3DObject< ID3D11ComputeShader > m_pComputeShader;
	D3DObject< ID3D11Buffer > m_pConstantBuffer;

	const char* m_pHLSLPrefix;
	const char* m_pHLSLBody;

	vector< Map > m_Outputs;
	u32 m_threadGroupSizeX, m_threadGroupSizeY;
	
	bool m_bConstantBufferIsDirty;
};

