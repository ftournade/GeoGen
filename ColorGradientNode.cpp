#include "stdafx.h"
#include "ColorGradientNode.h"
#include "ColorGradientDlg.h"

#include "resource.h"


#define COLOR_RAMP_RES 2048

const char* g_ColorGradientCS =
"Texture2D<float> _input0 : register(t0);\n"
"Texture2D<float4> _colorRamp : register(t1);\n"
"RWTexture2D<float4> _output0 : register(u0);\n"
"SamplerState _bilinear : register(s0);\n"
"SamplerState _bilinearClamp : register(s1);\n"
"\n"
"cbuffer Constants\n"
"{ float Resolution; float3 pad; }\n"
"\n"
"[ numthreads( 32, 32, 1 ) ]\n"
"void Main( uint2 _pos : SV_DispatchThreadID )\n"
"{\n"
"	float input = _input0.SampleLevel( _bilinear, _pos / Resolution, 0.0f );\n"
"	_output0[ _pos ] = _colorRamp.SampleLevel( _bilinearClamp, float2( input, 0.0f ), 0.0f );\n"
"}";

ColorGradientNode::ColorGradientNode()
{
	SetUIName( "Color Ramp" );

	AddInput( "FloatMap", IOType::Float );
	AddOutput( "ColorMap", IOType::Color );

	m_ColorGradient.AddKey( 0.0f, Color::Green );
	m_ColorGradient.AddKey( 1.0f, Color::Red );
}


ColorGradientNode::~ColorGradientNode()
{
}

CDialogEx* ColorGradientNode::GetCustomUI( CWnd* _pParent )
{
	ColorGradientDlg* pUI = new ColorGradientDlg;
	pUI->m_pComputeNode = this;
	pUI->m_ColorGradientCtrl.m_ColorGradient = m_ColorGradient;

	if( !pUI->Create( IDD_DLG_COLOR_RAMP, _pParent ) )
	{
		LOG_R( "Failed to create custom UI for node %s", GetNodeClassName() );
		assert( false );
		delete pUI;
		return nullptr;
	}

	return pUI;
}

bool ColorGradientNode::OnResolutionChanged()
{
	if( !m_Output.Init( GetResolution(), IOType::Color ) )
		return false;

	m_CB.m_Resolution = (float)GetResolution();

	m_CB.UploadToGPU( g_Renderer.GetImmediateDeviceContext() );

	return true;
}

bool ColorGradientNode::OneTimeInit()
{
	if( !m_CB.Init( g_Renderer.GetDevice() ) )
		return false;

	if( !g_Renderer.CreateShaderFromMemory( g_ColorGradientCS, "Main", nullptr, &m_pComputeShader ) )
	{
		assert( false );
		LOG_R( "ColorGradientNode: shader failed to compile" );
		return false;
	}

	if( !m_GPUColorGradient.Init( COLOR_RAMP_RES, 1, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D11_BIND_SHADER_RESOURCE ) )
	{
		assert( false );
		LOG_R( "ColorGradientNode: failed to create color ramp texture" );
		return false;
	}

	OnColorGradientChanged();

	return true;
}

void ColorGradientNode::OnColorGradientChanged()
{
	if( !m_GPUColorGradient.GetTex() )
		return;

	uint32_t colorRamp[ COLOR_RAMP_RES ];

	for( int i = 0 ; i < COLOR_RAMP_RES ; ++i )
	{
		float t = (float)i / (float)(COLOR_RAMP_RES - 1);
		Color c = m_ColorGradient.GetValue( t );
		
		//colorRamp[ i ] = c.ToR8G8B8A8();
		colorRamp[i] = c.ToA8B8G8R8();
	}

	g_Renderer.GetImmediateDeviceContext()->UpdateSubresource( m_GPUColorGradient.GetTex(), 0, nullptr, colorRamp, sizeof(colorRamp), 0 );
}

const Map* ColorGradientNode::GetOutput( uint32_t _idx ) const
{
	return &m_Output;
}

void ColorGradientNode::InternalCompute()
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	const Map* pInput = GetRemoteInputMap( 0 );

	if( !pInput )
		return;

	ID3D11ShaderResourceView* srvs[] =
	{
		pInput->GetSRV(),
		m_GPUColorGradient.GetSRV()
	};

	ID3D11UnorderedAccessView* uavs[] =
	{
		m_Output.GetUAV()
	};

	ID3D11SamplerState* samplers[] =
	{
		g_Renderer.GetBilinearSampler(),
		g_Renderer.GetBilinearClampSampler()
	};

	pDevCtx->CSSetShader( m_pComputeShader, nullptr, 0 );
	pDevCtx->CSSetShaderResources( 0, _countof( srvs ), srvs );
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetSamplers( 0, _countof( samplers ), samplers );

	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	const uint32_t threadGroupSizeX = 32;
	const uint32_t threadGroupSizeY = 32;

	uint32_t numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//Unbind everything
	static ID3D11ShaderResourceView* nullSRVs[ 2 ] = { NULL };
	pDevCtx->CSSetShaderResources( 0, _countof(nullSRVs), nullSRVs );

	static ID3D11UnorderedAccessView* nullUAVs[ 1 ] = { NULL };
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof(nullUAVs), nullUAVs, nullptr );
}


//Load/Save

bool ColorGradientNode::Load( const tinyxml2::XMLElement* _xmlNode )
{
	if( !ComputeNode::Load( _xmlNode ) )
		return false;

	const tinyxml2::XMLElement* xmlKey = _xmlNode->FirstChildElement( "Key" );
	
	m_ColorGradient.GetKeys().clear();

	while( xmlKey )
	{
		float t = xmlKey->FloatAttribute( "t" );
		Color c;
		c.r = xmlKey->FloatAttribute( "R" );
		c.g = xmlKey->FloatAttribute( "G" );
		c.b = xmlKey->FloatAttribute( "B" );
		c.a = xmlKey->FloatAttribute( "A" );
		Interpolation type = (Interpolation)xmlKey->IntAttribute( "Type" );
		
		m_ColorGradient.AddKey( t, c, type );

		xmlKey = xmlKey->NextSiblingElement( "Key" );
	}

	return true;
}

tinyxml2::XMLElement* ColorGradientNode::Save( tinyxml2::XMLDocument& _xmlDoc ) const
{
	tinyxml2::XMLElement* xmlNode = ComputeNode::Save( _xmlDoc );

	if( !xmlNode )
		return false;

	for( const auto & key : m_ColorGradient.GetKeys() )
	{
		tinyxml2::XMLElement* xmlKey = _xmlDoc.NewElement( "Key" );

		xmlKey->SetAttribute( "t", key.Time );
		xmlKey->SetAttribute( "R", key.Key.r );
		xmlKey->SetAttribute( "G", key.Key.g );
		xmlKey->SetAttribute( "B", key.Key.b );
		xmlKey->SetAttribute( "A", key.Key.a );
		xmlKey->SetAttribute( "Type", key.KeyType );

		xmlNode->InsertEndChild( xmlKey );
	}

	return xmlNode;
}