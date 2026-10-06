#pragma once


using namespace xtm; //Bof ...
extern Renderer g_Renderer; //tmp hack

class GridMesh
{
public:
	GridMesh();
	~GridMesh();

	bool Init( Renderer& _renderer, ID3DBlob* _pCompiledShaderTerrain );

	bool CreateGrid( ID3D11Device* _pDevice, uint32_t _resolution );

	void ComputeNormals();

	void Render( ID3D11DeviceContext* _pDeviceContext ) const;

private:
	bool UploadToGPU( ID3D11Device* _pDevice );

private:
	D3DObject< ID3D11Buffer > m_pIB;
	D3DObject< ID3D11Buffer > m_pVB;

	D3DObject< ID3D11InputLayout > m_pInputLayout;

	//CPU Data

	TVector< Vec2 > m_Vertices;
	TVector< uint32_t > m_Indices;

	//	AABBox<float> m_bbox;


};


