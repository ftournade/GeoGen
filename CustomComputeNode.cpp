#include "stdafx.h"
#include "CustomComputeNode.h"
#include "GeoGen.h"

CustomComputeNode::CustomComputeNode() :
	m_pHLSLPrefix( nullptr ),
	m_pHLSLBody( nullptr ),
	m_threadGroupSizeX( 32 ),
	m_threadGroupSizeY( 32 )
{
	SetUIName( "HLSL" );
}

CustomComputeNode::CustomComputeNode( u32 _numInputSlots, u32 _numOutputSlots ) :
	m_pHLSLPrefix( nullptr ),
	m_pHLSLBody( nullptr ),
	m_threadGroupSizeX( 32 ),
	m_threadGroupSizeY( 32 ),
	m_bConstantBufferIsDirty( true )
{
	for( u32 i = 0 ; i < _numInputSlots ; ++i )
	{
		AddInput( "FloatMap", IOType::Float );
	}

	for( u32 i = 0 ; i < _numOutputSlots ; ++i )
	{
		AddOutput( "FloatMap", IOType::Float );
	}

}


void CustomComputeNode::AddOutput( const char* _name, IOType _type )
{
	ComputeNode::AddOutput( _name, _type );

	m_Outputs.resize( m_Outputs.size() + 1 );
}

void CustomComputeNode::SetThreadGroupSize( u32 _threadGroupSizeX, u32 _threadGroupSizeY )
{
	m_threadGroupSizeX = _threadGroupSizeX;
	m_threadGroupSizeY = _threadGroupSizeY;

	m_pComputeShader.Release();
}

void CustomComputeNode::SetHLSLPrefix( const char* _code )
{
	m_pHLSLPrefix = _code;
	m_pComputeShader.Release();
}

void CustomComputeNode::SetHLSLBody( const char* _code )
{
	m_pHLSLBody = _code;
	m_pComputeShader.Release();
}

bool CustomComputeNode::OnResolutionChanged()
{
	for( u32 i = 0 ; i < m_Outputs.size() ; ++i )
	{
		DXGI_FORMAT fmt = m_Outputs[ i ].GetFormat();

		if( fmt == DXGI_FORMAT_UNKNOWN )
		{
			IOType type = (m_OutputSlots[ i ].m_DataType == IOType::FloatOrColor) ? IOType::Float : m_OutputSlots[ i ].m_DataType;
			if( !m_Outputs[ i ].Init( GetResolution(), type ) )
				return false;
		}
		else
		{
			if( !m_Outputs[ i ].Init( GetResolution(), GetResolution(), m_Outputs[ i ].GetFormat() ) )
				return false;
		}
	}

	m_pComputeShader.Release(); //The shader code is dependant on resolution

	return true;
}


void CustomComputeNode::InternalCompute()
{
	DBG_CHECK( IsDirty() );

	if( !m_pComputeShader && !CompileShader() )
		return;

	UpdateConstantBuffer();

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	//Bind inputs
	ID3D11ShaderResourceView* srvs[ 8 ];
	u32 numSrvs = 0;

	for( u32 i = 0 ; i < m_InputSlots.size() ; ++i )
	{
		const InputSlot& slot = m_InputSlots[ i ];

		shared_ptr<ComputeNode> inputNode = slot.m_RemoteOutputSlot.m_pNode.lock();

		if( !inputNode )
		{
			if( slot.m_bOptional )
			{
				srvs[ numSrvs++ ] = nullptr;
				continue;
			}
			else
			{
				return; //TODO error code
			}
		}

		//DBG_CHECK( !inputNode->IsDirty() );

		const Map* inputMap = inputNode->GetOutput( slot.m_RemoteOutputSlot.m_SlotIndex );

		if( !inputMap )
			return; //TODO error code

		srvs[ numSrvs++ ] = inputMap->GetSRV();
	}

	if( numSrvs > 0 )
		pDevCtx->CSSetShaderResources( 0, numSrvs, srvs );

	pDevCtx->CSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	//Bind samplers

	ID3D11SamplerState* samplers[]=
	{
		g_Renderer.GetPointSampler(),
		g_Renderer.GetPointClampSampler(),
		g_Renderer.GetBilinearSampler(),
		g_Renderer.GetBilinearClampSampler(),
		g_Renderer.GetBilinearMirrorSampler()
	};

	pDevCtx->CSSetSamplers( 0, countof( samplers ), samplers );

	//Bind outputs
	ID3D11UnorderedAccessView* uavs[ 8 ];
	u32 numUavs = 0;

	for( u32 i = 0 ; i < m_OutputSlots.size() ; ++i )
	{
		const OutputSlot& slot = m_OutputSlots[ i ];

		uavs[ numUavs++ ] = m_Outputs[ i ].GetUAV();
	}

	if( numUavs > 0 )
	{
		//static const UINT uavInitialCounts[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		pDevCtx->CSSetUnorderedAccessViews( 0, numUavs, uavs, nullptr );
	}

	pDevCtx->CSSetShader( m_pComputeShader, nullptr, 0 );

	u32 numGroupsX = (GetResolution() + m_threadGroupSizeX - 1) / m_threadGroupSizeX;
	u32 numGroupsY = (GetResolution() + m_threadGroupSizeY - 1) / m_threadGroupSizeY;

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//Unbind everything
	static ID3D11ShaderResourceView* nullSRVs[ 8 ] = { NULL };
	pDevCtx->CSSetShaderResources( 0, numSrvs, nullSRVs );

	static ID3D11UnorderedAccessView* nullUAVs[ 8 ] = { NULL };
	pDevCtx->CSSetUnorderedAccessViews( 0, numUavs, nullUAVs, nullptr );
}

const char* GetIOHLSLTypeString( IOType _type, const Map* _pMap )
{
	switch( _type )
	{
		case IOType::Float:   return "float";
		case IOType::Integer: return "int";
		case IOType::Color:   return "float4";
		case IOType::Bool:    return "bool";
		
		case IOType::FloatOrColor:
		{
			if( !_pMap )
				return "float";

			switch( _pMap->GetFormat() )
			{
			case DXGI_FORMAT_R32_FLOAT: return "float";
			case DXGI_FORMAT_R16G16B16A16_FLOAT: return "float4";
			case DXGI_FORMAT_R32G32B32A32_FLOAT: return "float4";
			default: return "<error>";
			}
			
			break;
		}
		default:  return "<error>";

	}
	
}

void CustomComputeNode::OnCompileTimeShaderConstantChanged()
{
	m_pComputeShader.Release();
}

bool CustomComputeNode::CompileShader()
{
	//Create constant buffer (TODO move that elsewhere)

	u32 numConstants = 0;

	for( const ParamSlot & slot : m_ParameterSlots )
	{
		if( !slot.m_bCompileTimeShaderConstant ) //TODO
			++numConstants;
	}

	m_pConstantBuffer.Release();

	if( numConstants > 0 )
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = numConstants * sizeof( float ) * 4; //we don't bother with tight packing, each param uses 128bits
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		if( FAILED( g_Renderer.GetDevice()->CreateBuffer( &bufferDesc, nullptr, &m_pConstantBuffer ) ) )
			return false;
	}

	//Generate shader code

	Str shaderCode;

	//IO textures

	for( u32 i = 0 ; i < m_InputSlots.size() ; ++i )
	{
		shaderCode += Format( "Texture2D<%s> _input%d : register(t%d);\n", GetIOHLSLTypeString( m_InputSlots[ i ].m_DataType, GetRemoteInputMap(i) ), i, i );
	}

	for( u32 i = 0 ; i < m_OutputSlots.size() ; ++i )
	{
		shaderCode += Format( "RWTexture2D<%s> _output%d : register(u%d);\n", GetIOHLSLTypeString( m_OutputSlots[ i ].m_DataType, &m_Outputs[i] ), i, i );
	}

	shaderCode += "\n\n";

	//Samplers

	shaderCode += "SamplerState _pointSampler : register(s0);\n";
	shaderCode += "SamplerState _pointClampSampler : register(s1);\n";
	shaderCode += "SamplerState _bilinearSampler : register(s2);\n";
	shaderCode += "SamplerState _bilinearClampSampler : register(s3);\n";
	shaderCode += "SamplerState _bilinearMirrorSampler : register(s4);\n";

	shaderCode += "\n\n";

	//run-time constants (constant buffer)

	if( numConstants > 0 )
	{

		shaderCode += "cbuffer Constants : register(b0)\n{\n";

		u32 padding = 0;

		for( const ParamSlot & slot : m_ParameterSlots )
		{
			if( slot.m_bCompileTimeShaderConstant )
				continue;

			switch( slot.m_DataType )
			{
				case IOType::Float:
					shaderCode += Format( "float %s; float3 pad%d;\n", slot.m_Name.c_str(), padding++ );
					break;
				case IOType::Integer:
					shaderCode += Format( "int %s; float3 pad%d;\n", slot.m_Name.c_str(), padding++ );
					break;
				case IOType::Bool:
					shaderCode += Format( "bool %s; float3 pad%d;\n", slot.m_Name.c_str(), padding++ );
					break;
				case IOType::Color:
					shaderCode += Format( "float4 %s; \n", slot.m_Name.c_str(), padding++ );
					break;
			}
		}

		shaderCode += "}\n\n";
	}

	//compile-time constants

	for( const ParamSlot & slot : m_ParameterSlots )
	{
		if( !slot.m_bCompileTimeShaderConstant )
			continue;

		switch( slot.m_DataType )
		{
			case IOType::Float:
				shaderCode += Format( "static const float %s = %f;\n", slot.m_Name.c_str(), slot.m_Value.f );
				break;
			case IOType::Integer:
				shaderCode += Format( "static const int %s = %d;\n", slot.m_Name.c_str(), slot.m_Value.i );
				break;
			case IOType::Bool:
				shaderCode += Format( "static const bool %s = %s;\n", slot.m_Name.c_str(), slot.m_Value.b ? "true" : "false" );
				break;
			case IOType::Color:
			{
				Color c;
				c.FromWin32COLORREF( slot.m_Value.c );
				shaderCode += Format( "static const float4 %s = float4( %f, %f, %f, %f );\n", slot.m_Name.c_str(), c.r, c.g, c.b, c.a );
				break;
			}
		}
	}

	shaderCode += m_pHLSLPrefix;
	shaderCode += "\n\n";

	//helper constants & functions

	shaderCode += Format( "static const float MinAltitude = %.1ff;\n", (float)theApp.GetMinAltitude() );
	shaderCode += Format( "static const float MaxAltitude = %.1ff;\n", (float)theApp.GetMaxAltitude() );
	shaderCode += Format( "static const float Extent = %.1ff;\n", (float)theApp.GetTerrainExtent() );
	shaderCode += Format( "static const int Resolution = %d;\n\n", theApp.GetResolution() );
	
	shaderCode += "float Terrain( float h ) { return lerp( MinAltitude, MaxAltitude, h ); }\n";
	shaderCode += "float NormalizeTerrain( float h ) { return (h - MinAltitude) / (MaxAltitude - MinAltitude); }\n";

	shaderCode += "\n\n";

	//shader body
	
	shaderCode += Format( "\n\n[numthreads( %d, %d, 1 )]\n", m_threadGroupSizeX, m_threadGroupSizeY );

	shaderCode += "void Main( uint2 _pos : SV_DispatchThreadID )\n{\n";

	shaderCode += "\tfloat2 _uv = float2(_pos) / (float)(Resolution - 1);\n";
	shaderCode += "\tfloat2 _wsPos = (_uv - 0.5f) * Extent;\n\n";

	shaderCode += m_pHLSLBody;

	shaderCode += "\n}\n";

	if( !g_Renderer.CreateShaderFromMemory( shaderCode.c_str(), "Main", nullptr, &m_pComputeShader ) )
	{
		ASSERT( FALSE );
		return false;
	}

	return true;
}

void CustomComputeNode::UpdateConstantBuffer()
{
	u32 numConstants = 0;

	for( const ParamSlot & slot : m_ParameterSlots )
	{
		if( !slot.m_bCompileTimeShaderConstant )
			++numConstants;
	}

	if( numConstants == 0 )
		return;

	union cst
	{
		float f;
		int i;
		bool b;
	};

	struct cst4
	{
		cst v[ 4 ];
	};

	static_assert( sizeof( cst4 ) == 16, "bad constant size" );

	vector< cst4 > constants( numConstants ); //TODO handle integers

	u32 i = 0;

	for( const ParamSlot & slot : m_ParameterSlots )
	{
		if( slot.m_bCompileTimeShaderConstant )
			continue;

		switch( slot.m_DataType )
		{
			case IOType::Float:
				constants[ i ].v[ 0 ].f = slot.m_Value.f;
				break;
			case IOType::Integer:
				constants[ i ].v[ 0 ].i = slot.m_Value.i;
				break;
			case IOType::Color:
			{
				Color c;
				c.FromWin32COLORREF( slot.m_Value.c );

				constants[ i ].v[ 0 ].f = c.r;
				constants[ i ].v[ 1 ].f = c.g;
				constants[ i ].v[ 2 ].f = c.b;
				constants[ i ].v[ 3 ].f = c.a;
				break;
			}
			case IOType::Bool:
				constants[ i ].v[ 0 ].b = slot.m_Value.b;
				break;
		}


		++i;
	}

	g_Renderer.GetImmediateDeviceContext()
		->UpdateSubresource( m_pConstantBuffer, 0, nullptr, &constants[ 0 ], 0, 0 );
}

bool CustomComputeNode::SetOutputFormat( int _slot, DXGI_FORMAT _fmt )
{
	if( _slot >= m_Outputs.size() )
		return false;

	return m_Outputs[ _slot ].Init( GetResolution(), GetResolution(), _fmt );
}
