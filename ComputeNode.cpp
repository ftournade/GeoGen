#include "stdafx.h"
#include "ComputeNode.h"


#include "GeoGen.h"

bool Map::Init( uint32_t _resX, uint32_t _resY, DXGI_FORMAT _fmt, UINT _bind )
{
	m_Width = _resX;
	m_Height = _resY;
	m_Format = _fmt;

	m_pUAV.Release();
	m_pSRV.Release();
	m_pRTV.Release();
	m_pDSV.Release();
	m_pTex.Release();

	ID3D11Device* pDevice = g_Renderer.GetDevice();

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Format = _fmt;	
	texDesc.Width = _resX;
	texDesc.Height = _resY;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = _bind;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	if( FAILED( pDevice->CreateTexture2D( &texDesc, nullptr, &m_pTex ) ) )
		return false;

	/////////////////////////

	if( _bind & D3D11_BIND_SHADER_RESOURCE )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		if( FAILED( pDevice->CreateShaderResourceView( m_pTex, &srvDesc, &m_pSRV ) ) )
			return false;
	}

	/////////////////////////

	if( _bind & D3D11_BIND_UNORDERED_ACCESS )
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = texDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		if( FAILED( pDevice->CreateUnorderedAccessView( m_pTex, &uavDesc, &m_pUAV ) ) )
			return false;
	}


	if( _bind & D3D11_BIND_RENDER_TARGET )
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		if( FAILED( pDevice->CreateRenderTargetView( m_pTex, &rtvDesc, &m_pRTV ) ) )
			return false;
	}

	if( _bind & D3D11_BIND_DEPTH_STENCIL )
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = texDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		if( FAILED( pDevice->CreateDepthStencilView( m_pTex, &dsvDesc, &m_pDSV ) ) )
			return false;
	}

	return true;
}

bool Map::Init( uint32_t _resolution, IOType _type, UINT _bind )
{
	switch( _type )
	{
		case IOType::Integer: return Init( _resolution, _resolution, DXGI_FORMAT_R32_SINT, _bind );
		case IOType::Float:	  return Init( _resolution, _resolution, DXGI_FORMAT_R32_FLOAT, _bind );
		case IOType::Color:   return Init( _resolution, _resolution, DXGI_FORMAT_R16G16B16A16_FLOAT, _bind );
	}

	return false;
}

void Map::SetDebugName( const char* _name )
{
	D3D_SET_OBJECT_NAME_A( (ID3D11Texture2D*)m_pTex, _name );
}

void Map::CopyFromGPU( vector<float>& _data ) const
{
	D3D11_TEXTURE2D_DESC texDesc;
	m_pTex->GetDesc( &texDesc );

	uint32_t texelSize = (m_Format == DXGI_FORMAT_R32_FLOAT) ? 1 : 4;

	_data.resize( texDesc.Width * texDesc.Height * texelSize );

	CopyFromGPU( &_data[ 0 ] );
}

void Map::CopyFromGPU( float* _data ) const
{
	assert( m_Format == DXGI_FORMAT_R32_FLOAT );

	//Create staging texture
	////////////////////////

	D3D11_TEXTURE2D_DESC texDesc;
	m_pTex->GetDesc( &texDesc );

	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	ID3D11Texture2D* pStagingTex;
	if( FAILED( g_Renderer.GetDevice()->CreateTexture2D( &texDesc, nullptr, &pStagingTex ) ) )
	{
		assert( false );//TODO handle error
	}

	//Copy texture to staging texture
	/////////////////////////////////

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	pDevCtx->CopyResource( pStagingTex, m_pTex );

	//Copy staging texture to CPU
	/////////////////////////////

	D3D11_MAPPED_SUBRESOURCE rsrcMap;
	if( FAILED( pDevCtx->Map( pStagingTex, 0, D3D11_MAP_READ, 0, &rsrcMap ) ) )
	{
		assert( false );//TODO handle error
	}
	
	switch( m_Format )
	{
		case DXGI_FORMAT_R32_FLOAT:
			assert( texDesc.Width * sizeof( float ) == rsrcMap.RowPitch );

			memcpy( _data, rsrcMap.pData, rsrcMap.DepthPitch );
			break;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		{
			float16* pSrcTexel = (float16*)rsrcMap.pData;

			for( int y = 0 ; y < m_Height ; ++y )
			{
				for( int x = 0 ; x < m_Height ; ++x )
				{
					float16 v;

					v = *pSrcTexel++;
					*_data++ = v;

					v = *pSrcTexel++;
					*_data++ = v;

					v = *pSrcTexel++;
					*_data++ = v;

					v = *pSrcTexel++;
					*_data++ = v;
				}

			}

			break;
		}
		default:
			assert( !"unsupported format" );

	}
	//assert( texDesc.Format == DXGI_FORMAT_R32_FLOAT ); //TODO handle other formats

	pDevCtx->Unmap( pStagingTex, 0 );

	pStagingTex->Release();
}

void Map::CopyToGPU( const rgba8_t* _pData )
{
	D3D11_TEXTURE2D_DESC desc;
	m_pTex->GetDesc( &desc );

	g_Renderer.GetImmediateDeviceContext()->UpdateSubresource( m_pTex, 0, nullptr, _pData, desc.Width * sizeof( rgba8_t ), 0 ); //TODO hanle other formats
}


void Map::CopyToGPU( const float* _pData )
{
	assert( m_Format == DXGI_FORMAT_R32_FLOAT );

	D3D11_TEXTURE2D_DESC desc;
	m_pTex->GetDesc( &desc );

	g_Renderer.GetImmediateDeviceContext()->UpdateSubresource( m_pTex, 0, nullptr, _pData, desc.Width * sizeof(float), 0 ); //TODO hanle other formats
}


//////////////////////////

ComputeNode::ComputeNode() : 
	m_bPreviewAsHeightField( true ), 
	m_bIsIterative( false ), 
	m_bIsDirty( true ),
	m_Resolution( 0 ), 
	m_FixedResolution( 0 ),
	m_ResolutionReference( Res_MainInput ),
	m_ResolutionModifier( 1 )
{
	//Whatever ...
	m_UIRect.Pos = Vec2( 10, 10 ); 
	m_UIRect.Size = Vec2( 120, 80 ); 
}


const int slotStart = 20;
const int slotSize = 10;
const int slotSpacing = 4;

void ComputeNode::AddInput( const char* _name, IOType _type, bool _optional )
{
	InputSlot slot;
	slot.m_Name = _name;
	slot.m_DataType = _type;
	slot.m_bOptional = _optional;

	slot.m_UIRect.Pos.x = 0;
	slot.m_UIRect.Pos.y = (float)(slotStart + m_InputSlots.size() * (slotSize + slotSpacing));
	slot.m_UIRect.Size.x = slotSize;
	slot.m_UIRect.Size.y = slotSize;

	m_InputSlots.push_back( slot );
}

void ComputeNode::AddOutput( const char* _name, IOType _type )
{
	OutputSlot slot;
	slot.m_Name = _name;
	slot.m_DataType = _type;
	slot.m_bOptional = true;

	slot.m_UIRect.Pos.x = m_UIRect.Size.x - slotSize;
	slot.m_UIRect.Pos.y = (float)(slotStart + m_OutputSlots.size() * (slotSize + slotSpacing));
	slot.m_UIRect.Size.x = slotSize;
	slot.m_UIRect.Size.y = slotSize;

	m_OutputSlots.push_back( slot );
}

int ComputeNode::AddParam( const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
												float _defaultValue, float _min, float _max, bool _invalidatesShader )
{
	//TODO check no space(s) in _name

	assert( _type == IOType::Float );

	ParamSlot slot;
	slot.m_CategoryName = _categoryName;
	slot.m_Name = _name;
	slot.m_DataType = _type;
	slot.m_DefaultValue.f = _defaultValue;
	slot.m_Value.f = _defaultValue;
	slot.m_Min = _min;
	slot.m_Max = _max;
	slot.m_Edition = _edition;
	slot.m_bCompileTimeShaderConstant = _invalidatesShader;

	m_ParameterSlots.push_back( slot );

	return m_ParameterSlots.size() - 1;
}

int ComputeNode::AddParam( const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
							int _defaultValue, int _min, int _max, bool _invalidatesShader )
{
	//TODO check no space(s) in _name

	assert( _type == IOType::Integer );

	ParamSlot slot;
	slot.m_CategoryName = _categoryName;
	slot.m_Name = _name;
	slot.m_DataType = _type;
	slot.m_DefaultValue.i = _defaultValue;
	slot.m_Value.i = _defaultValue;
	slot.m_Min = (float)_min;
	slot.m_Max = (float)_max;
	slot.m_Edition = _edition;
	slot.m_bCompileTimeShaderConstant = _invalidatesShader;

	m_ParameterSlots.push_back( slot );

	return m_ParameterSlots.size() - 1;
}

int ComputeNode::AddParam( const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
												const Color& _defaultColor, bool _invalidatesShader )
{
	//TODO check no space(s) in _name

	assert( _type == IOType::Color );

	ParamSlot slot;
	slot.m_CategoryName = _categoryName;
	slot.m_Name = _name;
	slot.m_DataType = _type;
	slot.m_DefaultValue.c = _defaultColor.ToWin32COLORREF();
	slot.m_Value.c = _defaultColor.ToWin32COLORREF();
	slot.m_Edition = _edition;
	slot.m_bCompileTimeShaderConstant = _invalidatesShader;

	m_ParameterSlots.push_back( slot );

	return m_ParameterSlots.size() - 1;
}

int ComputeNode::AddParam( const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
												bool _defaultValue, bool _invalidatesShader )
{
	//TODO check no space(s) in _name

	assert( _type == IOType::Bool );

	ParamSlot slot;
	slot.m_CategoryName = _categoryName;
	slot.m_Name = _name;
	slot.m_DataType = _type;
	slot.m_DefaultValue.b = _defaultValue;
	slot.m_Value.b = _defaultValue;
	slot.m_Edition = _edition;
	slot.m_bCompileTimeShaderConstant = _invalidatesShader;

	m_ParameterSlots.push_back( slot );

	return m_ParameterSlots.size() - 1;
}


int ComputeNode::AddParam(	const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
												const char* _defaultString, bool _invalidatesShader )
{
	//TODO check no space(s) in _name

	assert( _type == IOType::String );

	ParamSlot slot;
	slot.m_CategoryName = _categoryName;
	slot.m_Name = _name;
	slot.m_DataType = _type;
	slot.m_ValueString = _defaultString;
	slot.m_Edition = _edition;
	slot.m_bCompileTimeShaderConstant = _invalidatesShader;

	m_ParameterSlots.push_back( slot );

	return m_ParameterSlots.size() - 1;
}


void ComputeNode::UpdateInternalResolution()
{
//	if( m_ResolutionReference == Res_MainInput )
//		m_ResolutionReference = Res_GlobalSetting; //TODO support MainInput (beware of order of evaluation between nodes)

	uint32_t res;

	switch( m_ResolutionReference )
	{
		case Res_GlobalSetting:
		{
			res = theApp.GetResolution(); //TODO ... bad dependency
			break;
		}

		case Res_MainInput:
		{
			assert( !m_InputSlots.empty() );
			shared_ptr<ComputeNode> pInputNode = m_InputSlots[ 0 ].m_RemoteOutputSlot.m_pNode.lock();

			if( pInputNode )
			{
				pInputNode->UpdateInternalResolution();
				res = pInputNode->GetResolution();
			}
			else
				res = theApp.GetResolution(); //TODO ... bad dependency

			break;
		}

		case Res_Fixed:
			res = m_FixedResolution;
			break;
	}

	if( m_ResolutionReference != Res_Fixed )
	{
		if( m_ResolutionModifier > 0 )
			m_Resolution = res * m_ResolutionModifier;
		else if( m_ResolutionModifier < -1 )
			m_Resolution = res / -m_ResolutionModifier;
	}

//	OnResolutionChanged();
}

bool ComputeNode::SetFixedResolution( uint32_t _resolution )
{
	m_ResolutionReference = Res_Fixed;
	m_Resolution = _resolution;
	m_FixedResolution = _resolution;

	UpdateInternalResolution();

	return OnResolutionChanged();
}

bool ComputeNode::SetResolutionModifier( int32_t _resolutionModifier )
{
	m_ResolutionModifier = _resolutionModifier;
	
	UpdateInternalResolution();

	return OnResolutionChanged();
}

bool ComputeNode::SetResolutionReference( ResolutionReference _resolutionReference )
{
	m_ResolutionReference = _resolutionReference;

	UpdateInternalResolution();

	return OnResolutionChanged();
}


void ComputeNode::SetDirty()
{
	m_bIsDirty = true;

	//propagate recursively to dependent nodes
	for( const OutputSlot& slot : m_OutputSlots )
	{
		for( const RemoteSlot& remoteSlot : slot.m_RemoteInputSlots )
		{
			shared_ptr<ComputeNode> pNode = remoteSlot.m_pNode.lock();

			if( pNode )
				pNode->SetDirty();
		}
	}
}

const Map* ComputeNode::GetRemoteInputMap( int _slot ) const
{
	if( _slot >= m_InputSlots.size() )
		return nullptr;

	 shared_ptr<ComputeNode> pNode = m_InputSlots[ _slot ].m_RemoteOutputSlot.m_pNode.lock();

	 if( !pNode )
		 return nullptr;

	 return pNode->GetOutput( m_InputSlots[ _slot ].m_RemoteOutputSlot.m_SlotIndex );
}


void ComputeNode::Compute()
{
	if( !m_bIsDirty )
		return;

	//Recursively compute inputs

	for( const InputSlot& slot : m_InputSlots )
	{
		shared_ptr<ComputeNode> pInputNode = slot.m_RemoteOutputSlot.m_pNode.lock();

		if( !pInputNode && !slot.m_bOptional )
			return; //A mandatory input is not available //TODO handle error

		if( pInputNode && pInputNode->IsDirty() )
			pInputNode->Compute();
	}


	//Compute this node

	InternalCompute();

#ifndef DBG_RENDERDOC
	m_bIsDirty = false;
#endif
}

bool ComputeNode::Load( const tinyxml2::XMLElement* _xmlNode )
{
	SetUIName( _xmlNode->Attribute( "Name" ) );
	m_UIRect.Pos.x = _xmlNode->FloatAttribute( "UIPosX" );
	m_UIRect.Pos.y = _xmlNode->FloatAttribute( "UIPosY" );

	m_bPreviewAsHeightField = _xmlNode->BoolAttribute( "PreviewAsHeightField" );
	m_ResolutionReference = (ResolutionReference)_xmlNode->IntAttribute( "ResRef" );

	switch( m_ResolutionReference )
	{
	case Res_GlobalSetting:
		break;
	case Res_MainInput:
		m_ResolutionModifier = _xmlNode->IntAttribute( "ResModifier" );
		break;
	case Res_Fixed:
		m_FixedResolution = _xmlNode->UnsignedAttribute( "FixedRes" );
		break;
	}


	for( ParamSlot& param : m_ParameterSlots )
	{
		switch( param.m_DataType )
		{
			case IOType::Float:
				param.m_Value.f = _xmlNode->FloatAttribute( param.m_Name.c_str() );
				break;
			
			case IOType::Integer:
				param.m_Value.i = _xmlNode->IntAttribute( param.m_Name.c_str() );
				break;

			case IOType::Color:
			{
				const char* strColor = _xmlNode->Attribute( param.m_Name.c_str() );
				Color c;
				sscanf( strColor, "%f %f %f", &c.r, &c.g, &c.b );

				param.m_Value.c = c.ToWin32COLORREF();
				break;
			}
			case IOType::Bool:
				param.m_Value.b = _xmlNode->BoolAttribute( param.m_Name.c_str() );
				break;

			case IOType::String:
				param.m_ValueString = _xmlNode->Attribute( param.m_Name.c_str() );
				break;

		}
	}

	return true;
}

tinyxml2::XMLElement* ComputeNode::Save( tinyxml2::XMLDocument& _xmlDoc ) const
{
	tinyxml2::XMLElement* xmlNode = _xmlDoc.NewElement( GetNodeClassName() );

	xmlNode->SetAttribute( "Name", m_UIName.c_str() );

	xmlNode->SetAttribute( "UIPosX", m_UIRect.Pos.x );
	xmlNode->SetAttribute( "UIPosY", m_UIRect.Pos.y );
	xmlNode->SetAttribute( "PreviewAsHeightField", m_bPreviewAsHeightField );

	xmlNode->SetAttribute( "ResRef", m_ResolutionReference );

	switch( m_ResolutionReference )
	{
		case Res_GlobalSetting:
			break;
		case Res_MainInput:
			xmlNode->SetAttribute( "ResModifier", m_ResolutionModifier );
			break;
		case Res_Fixed:
			xmlNode->SetAttribute( "FixedRes", (unsigned int)m_FixedResolution );
			break;
	}

	for( const ParamSlot& param : m_ParameterSlots )
	{
		switch( param.m_DataType )
		{
			case IOType::Float:
				xmlNode->SetAttribute( param.m_Name.c_str(), param.m_Value.f );
				break;

			case IOType::Integer:
				xmlNode->SetAttribute( param.m_Name.c_str(), param.m_Value.i );
				break;

			case IOType::Color:
			{
				Color c;
				c.FromWin32COLORREF( param.m_Value.c );
				std::string strColor = Format( "%f %f %f", c.r, c.g, c.b );
				
				xmlNode->SetAttribute( param.m_Name.c_str(), strColor.c_str() );
				break;
			}

			case IOType::Bool:
				xmlNode->SetAttribute( param.m_Name.c_str(), param.m_Value.b );
				break;

			case IOType::String:
				xmlNode->SetAttribute( param.m_Name.c_str(), param.m_ValueString.c_str() );
				break;
		}
	}
	
	return xmlNode;
}
