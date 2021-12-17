#pragma once

#include "ComputeNode.h"

class ScatterMapNode :	public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ScatterMapNode )

	ScatterMapNode();
	virtual ~ScatterMapNode();

	virtual bool OneTimeInit();

	virtual const Map* GetOutput( u32 _idx ) const;

	virtual void OnInputConnectionChanged( int _slot );
	
protected:
	bool CompileShaders();
	void ChangeBlendState();

	bool UpdateVertexBuffer();
	
	virtual void InternalCompute();

	virtual bool OnResolutionChanged();
private:
	Map m_Output;

	u32 m_SplatCount;

	D3DObject< ID3D11VertexShader > m_VS;
	D3DObject< ID3D11PixelShader > m_PS;

	D3DObject< ID3D11DepthStencilState >	m_DepthStencilState;
	D3DObject< ID3D11RasterizerState >		m_RasterizerState;
	D3DObject< ID3D11BlendState >			m_BlendState;

	D3DObject< ID3D11InputLayout > m_InputLayout;
	D3DObject< ID3D11Buffer> m_VB;
	
	int m_ParamBlendMode;
	int m_ParamDistribution;
	int m_ParamCount;
	int m_ParamRandomizePosition;
	int m_ParamAspectRatio;
	int m_ParamScale;
	int m_ParamRandomizeScale;
	int m_ParamRotation;
	int m_ParamRandomizeRotation;
	int m_ParamIntensity;
	int m_ParamRandomizeIntensity;
	int m_ParamScaleIntensityBySize;
	int m_ParamSeed;
};

