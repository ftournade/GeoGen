#pragma once

#include <d3d11.h>
#include <string.h>

#include "D3DObject.h"

//GPU constant buffer whose CPU-side content is the T struct itself:
//fill the inherited fields, then call UploadToGPU() and bind GetBuffer().
template <class T>
class ConstantBuffer : public T
{
public:
	ConstantBuffer() : T() {}

	bool Init( ID3D11Device* _pDevice )
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth		= (UINT)( ( sizeof( T ) + 15 ) & ~15 ); //constant buffers size must be a multiple of 16 bytes
		bufferDesc.Usage			= D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags	= D3D11_CPU_ACCESS_WRITE;

		m_pBuffer.Release();

		return SUCCEEDED( _pDevice->CreateBuffer( &bufferDesc, nullptr, &m_pBuffer ) );
	}

	bool UploadToGPU( ID3D11DeviceContext* _pDevCtx )
	{
		if( !m_pBuffer )
			return false;

		D3D11_MAPPED_SUBRESOURCE mappedResource;

		if( FAILED( _pDevCtx->Map( m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource ) ) )
			return false;

		memcpy( mappedResource.pData, static_cast< const T* >( this ), sizeof( T ) );

		_pDevCtx->Unmap( m_pBuffer, 0 );

		return true;
	}

	inline ID3D11Buffer* GetBuffer() const { return m_pBuffer; }

private:
	D3DObject< ID3D11Buffer > m_pBuffer;
};
