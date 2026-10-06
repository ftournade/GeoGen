#include "stdafx.h"
#include "GridMesh.h"


Renderer g_Renderer;

GridMesh::GridMesh()
{
}


GridMesh::~GridMesh()
{
}



bool GridMesh::CreateGrid( ID3D11Device* _pDevice, uint32_t _resolution )
{
	uint32_t numQuads = (_resolution - 1) * (_resolution - 1);

	//	Reset();
	m_Vertices.resize( _resolution * _resolution );
	m_Indices.resize( numQuads * 2 * 3 );

	for( uint32_t y = 0 ; y < _resolution ; ++y )
	{
		for( uint32_t x = 0 ; x < _resolution ; ++x )
		{
			Vec2& vtx = m_Vertices[ y * _resolution + x ];

			vtx.x = (float)x / (float)(_resolution - 1);
			vtx.y = (float)y / (float)(_resolution - 1);
		}
	}

	uint32_t idx = 0;

	for( uint32_t y = 0 ; y < _resolution - 1 ; ++y )
	{
		for( uint32_t x = 0 ; x < _resolution - 1 ; ++x )
		{
			uint32_t i = y * _resolution + x;

			//Clockwise on screen when seen from above (front faces for D3D11_CULL_BACK)
			m_Indices[ idx++ ] = i;
			m_Indices[ idx++ ] = i + 1;
			m_Indices[ idx++ ] = i + _resolution;

			m_Indices[ idx++ ] = i + _resolution;
			m_Indices[ idx++ ] = i + 1;
			m_Indices[ idx++ ] = i + _resolution + 1;
		}
	}

	return UploadToGPU( _pDevice );
}

bool GridMesh::Init( Renderer& _renderer, ID3DBlob* _pCompiledShaderTerrain )
{

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if( FAILED( _renderer.GetDevice()->CreateInputLayout( layout, sizeof( layout ) / sizeof( layout[ 0 ] ),
		_pCompiledShaderTerrain->GetBufferPointer(),
		_pCompiledShaderTerrain->GetBufferSize(),
		&m_pInputLayout ) ) )
		return false;

	return true;
}


void GridMesh::ComputeNormals()
{

}

bool GridMesh::UploadToGPU( ID3D11Device* _pDevice )
{
	m_pVB.Release();
	m_pIB.Release();

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	desc.ByteWidth = sizeof( Vec2 ) * m_Vertices.size();
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_Vertices[ 0 ];
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	if( FAILED( _pDevice->CreateBuffer( &desc, &data, &m_pVB ) ) )
		return false;

	desc.ByteWidth = sizeof( uint32_t ) * m_Indices.size();
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	data.pSysMem = &m_Indices[ 0 ];

	if( FAILED( _pDevice->CreateBuffer( &desc, &data, &m_pIB ) ) )
		return false;

	return true;
}

void GridMesh::Render( ID3D11DeviceContext* _pDeviceContext ) const
{
	UINT vtxStride = sizeof( Vec2 );
	UINT vtxOffset = 0;

	_pDeviceContext->IASetIndexBuffer( m_pIB, DXGI_FORMAT_R32_UINT, 0 );
	_pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVB, &vtxStride, &vtxOffset );
	_pDeviceContext->IASetInputLayout( m_pInputLayout );
	_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	_pDeviceContext->DrawIndexed( m_Indices.size(), 0, 0 );
}
