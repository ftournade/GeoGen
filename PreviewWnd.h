#pragma once

#include "GridMesh.h"

class ComputeNode;
class ErosionNode;

struct TerrainConstants //TODO Share HLSL/C++ struct
{
	xtm::Mat44 c_WorldViewMatrix;
	xtm::Mat44 c_WorldViewProjMatrix;

	float c_TerrainExtent;
	uint32_t c_TerrainResolution;
	float c_MinAltitude;
	float c_MaxAltitude;
	//float pad;
};

class CPreviewWnd : public CDockablePane
{
public:
	CPreviewWnd();
	virtual ~CPreviewWnd();

	void StartIterativeSim( std::shared_ptr<ComputeNode> _pIterativeSimNode );
	void StopIterativeSim();

	bool OnNodesResolutionChanged();

protected:
	void InitRendererOrResizeSwapChainIfNeeded();
	bool InitResolutionIndependentD3DStuff();
	bool InitResolutionDependentD3DStuff();

	afx_msg void OnPaint();
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );

	DECLARE_MESSAGE_MAP()

	void Render();
	void MoveCamera( const Vec3& _dir );

private:
	xtm::Camera m_Camera;

	bool m_bRotatingView;
	Vec2 m_LastMousePos;
	float m_CameraU, m_CameraV;

	GridMesh m_GridMesh;

	D3DObject< ID3D11VertexShader > m_pTerrainVertexShader;
	D3DObject< ID3D11PixelShader > m_pTerrainPixelShader;

	D3DObject< ID3D11VertexShader > m_pRGBVertexShader;
	D3DObject< ID3D11PixelShader > m_pRGBPixelShader;

	ConstantBuffer<TerrainConstants> m_TerrainCB;

	std::weak_ptr<ComputeNode> m_pIterativeSimNode;

	D3DObject< ID3D11Texture2D >			m_pWhiteTex;
	D3DObject< ID3D11ShaderResourceView >	m_pWhiteTexSRV;
};

