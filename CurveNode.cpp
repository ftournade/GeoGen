#include "stdafx.h"
#include "CurveNode.h"

#include "CurveEditorDlg.h"
#include "resource.h"

#include <Core/Log.h>

#define CURVE_RES 1024

const char* g_CurveCS =
"Texture2D<float> _input0 : register(t0);\n"
"Texture2D<float> _curveLookup : register(t1);\n"
"RWTexture2D<float> _output0 : register(u0);\n"
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
"	_output0[ _pos ] = _curveLookup.SampleLevel( _bilinearClamp, float2( input, 0.0f ), 0.0f );\n"
"}";

CurveNode::CurveNode()
{
	SetUIName( "Curve" );

	AddInput( "Input", IOType::FloatOrColor );
	AddOutput( "Output", IOType::FloatOrColor );

	m_Curve.AddKey( 0.0f, Vec2( 0.0f, 0.0 ) );
	m_Curve.AddKey( 1.0f, Vec2( 1.0f, 1.0 ) );
}


CurveNode::~CurveNode()
{
}

CDialogEx* CurveNode::GetCustomUI( CWnd* _pParent )
{
	CurveEditorDlg* pUI = new CurveEditorDlg;
	pUI->m_pComputeNode = this;
	pUI->m_CurveControl.m_Curve = m_Curve;

	if( !pUI->Create( IDD_DIALOG_CURVE_EDITOR, _pParent ) )
	{
		LOG_R( "Failed to create custom UI for node %s", GetNodeClassName() );
		DBG_CHECK( false );
		delete pUI;
		return nullptr;
	}

	return pUI;
}

bool CurveNode::OnResolutionChanged()
{
	if( !m_Output.Init( GetResolution(), IOType::Float ) )
		return false;

	m_CB.m_Resolution = (float)GetResolution();

	m_CB.UploadToGPU( g_Renderer.GetImmediateDeviceContext() );

	return true;
}

bool CurveNode::OneTimeInit()
{
	if( !m_CB.Init( g_Renderer.GetDevice() ) )
		return false;

	if( !g_Renderer.CreateShaderFromMemory( g_CurveCS, "Main", nullptr, &m_pComputeShader ) )
	{
		DBG_CHECK( false );
		LOG_R( "CurveNode: shader failed to compile" );
		return false;
	}

	if( !m_GPUCurveLookUp.Init( CURVE_RES, 1, DXGI_FORMAT_R32_FLOAT, D3D11_BIND_SHADER_RESOURCE ) )
	{
		DBG_CHECK( false );
		LOG_R( "CurveNode: failed to create color ramp texture" );
		return false;
	}

	OnCurveChanged();

	return true;
}

void CurveNode::OnCurveChanged()
{
	float curve[ CURVE_RES ];
	ZeroMemory( curve, sizeof( curve ) );

	u32 samples = CURVE_RES * 8;

	for( int i = 0 ; i < samples ; ++i )
	{
		float t = (float)i / (float)(samples - 1);
		Vec2 val = m_Curve.GetValue( t );

		s32 idx = val.x * (float)(CURVE_RES - 1);
		DBG_CHECK( idx >= 0 && idx < CURVE_RES );
		idx = Clamp<s32>( idx, 0, CURVE_RES - 1 );
		curve[ idx ] = val.y;
	}

	g_Renderer.GetImmediateDeviceContext()->UpdateSubresource( m_GPUCurveLookUp.GetTex(), 0, nullptr, curve, sizeof( curve ), 0 );
}

const Map* CurveNode::GetOutput( u32 _idx ) const
{
	return &m_Output;
}

void CurveNode::InternalCompute()
{
	if( !m_GPUCurveLookUp.GetTex() )
		OnCurveChanged();

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	const Map* pInput = GetRemoteInputMap( 0 );

	if( !pInput )
		return;

	ID3D11ShaderResourceView* srvs[] =
	{
		pInput->GetSRV(),
		m_GPUCurveLookUp.GetSRV()
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
	pDevCtx->CSSetShaderResources( 0, countof( srvs ), srvs );
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetSamplers( 0, countof( samplers ), samplers );

	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	const u32 threadGroupSizeX = 32;
	const u32 threadGroupSizeY = 32;

	u32 numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	u32 numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//Unbind everything
	static ID3D11ShaderResourceView* nullSRVs[ 2 ] = { NULL };
	pDevCtx->CSSetShaderResources( 0, countof( nullSRVs ), nullSRVs );

	static ID3D11UnorderedAccessView* nullUAVs[ 1 ] = { NULL };
	pDevCtx->CSSetUnorderedAccessViews( 0, countof( nullUAVs ), nullUAVs, nullptr );

}

//Load/Save

bool CurveNode::Load( const tinyxml2::XMLElement* _xmlNode )
{
	if( !ComputeNode::Load( _xmlNode ) )
		return false;
	
	const tinyxml2::XMLElement* xmlKey = _xmlNode->FirstChildElement( "Key" );

	m_Curve.GetKeys().clear();

	while( xmlKey )
	{
		float t = xmlKey->FloatAttribute( "t" );
		xtm::Interpolation type = (xtm::Interpolation)xmlKey->IntAttribute( "Type" );
		
		Vec2 key, leftTgt, rightTgt;
		key.x = xmlKey->FloatAttribute( "KeyX" );
		key.y = xmlKey->FloatAttribute( "KeyY" );

		leftTgt.x = xmlKey->FloatAttribute( "LTgtX" );
		leftTgt.y = xmlKey->FloatAttribute( "LTgtY" );

		rightTgt.x = xmlKey->FloatAttribute( "RTgtX" );
		rightTgt.y = xmlKey->FloatAttribute( "RTgtY" );

		m_Curve.AddKey( t, key, leftTgt, rightTgt, type );

		xmlKey = xmlKey->NextSiblingElement( "Key" );
	}
	
	return true;
}

tinyxml2::XMLElement* CurveNode::Save( tinyxml2::XMLDocument& _xmlDoc ) const
{
	tinyxml2::XMLElement* xmlNode = ComputeNode::Save( _xmlDoc );
	
	if( !xmlNode )
		return false;

	for( const auto & key : m_Curve.GetKeys() )
	{
		tinyxml2::XMLElement* xmlKey = _xmlDoc.NewElement( "Key" );

		xmlKey->SetAttribute( "t", key.Time );
		xmlKey->SetAttribute( "Type", key.KeyType );

		xmlKey->SetAttribute( "KeyX", key.Key.x );
		xmlKey->SetAttribute( "KeyY", key.Key.y );

		xmlKey->SetAttribute( "LTgtX", key.LeftTangent.x );
		xmlKey->SetAttribute( "LTgtY", key.LeftTangent.y );

		xmlKey->SetAttribute( "RTgtX", key.RightTangent.x );
		xmlKey->SetAttribute( "RTgtY", key.RightTangent.y );
		
		xmlNode->InsertEndChild( xmlKey );
	}
	
	return xmlNode;
}