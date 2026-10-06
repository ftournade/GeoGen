#include "stdafx.h"
#include "MountainNode.h"
#include "RiverEDitorDlg.h"
#include "resource.h"


MountainNode::MountainNode() :
	m_CurRiverSlopeMultiplier(0.0f)
{
	m_ResolutionReference = Res_GlobalSetting;

	SetUIName( "Mountain" );

	AddParam( "Main", "MoutainSlope", IOType::Float, ParamEdition::Slider, 3.0f, 0.0f, 10.0f, false );
	AddParam( "Main", "RiverSlope", IOType::Float, ParamEdition::Slider, 1.0f, 0.0f, 10.0f, false );
	AddParam( "Main", "ValleyWidth", IOType::Float, ParamEdition::Slider, 0.12f, 0.01f, 0.4f, false );
	AddParam( "Main", "ValleyShape", IOType::Float, ParamEdition::Slider, 2.0f, 2.0f, 6.0f, false );
	AddParam( "Main", "Distortion", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 0.3f, false );

	AddInput( "DistortionX", IOType::Float, true );
	AddInput( "DistortionY", IOType::Float, true );

	AddOutput( "FloatMap", IOType::Float );
}


MountainNode::~MountainNode()
{
}

const Map* MountainNode::GetOutput( uint32_t _idx ) const
{
	return &m_Output;
}


bool MountainNode::OneTimeInit()
{
	if( !g_Renderer.CreateShader("Shaders/Mountain.hlsl", "Main", nullptr, &m_CS) )
		return false;

	if( !m_CB.Init( g_Renderer.GetDevice() ) )
		return false;
	
	m_CB.NumSegments = 0;

	return true;
}

bool MountainNode::OnResolutionChanged()
{
	if( !m_Output.Init( GetResolution(), IOType::Float ) )
		return false;

	return true;
}

void MountainNode::OnRiverChanged()
{
	if( !m_pRiverRoot )
		return;
	
	m_CurRiverSlopeMultiplier = m_ParameterSlots[1].m_Value.f;

	m_SegmentBufferSRV.Release();
	m_SegmentBuffer.Release();

	vector<RiverSegment> segments;

	RecGatherRiverSegmentsForGPU( segments, m_pRiverRoot, 0.0f );
	
	m_CB.NumSegments = segments.size();
	
	if( segments.empty() )
		return;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( RiverSegment ) * segments.size();
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof( RiverSegment );

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &segments[0];
	initialData.SysMemPitch = bufferDesc.ByteWidth;
	initialData.SysMemSlicePitch = bufferDesc.ByteWidth;

	if( FAILED( g_Renderer.GetDevice()->CreateBuffer( &bufferDesc, &initialData, &m_SegmentBuffer ) ) )
	{
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;

	if( FAILED( g_Renderer.GetDevice()->CreateShaderResourceView( m_SegmentBuffer, &srvDesc, &m_SegmentBufferSRV ) ) )
	{
		return;
	}

}

void MountainNode::RecGatherRiverSegmentsForGPU( vector<RiverSegment>& _segments, shared_ptr< RiverNode > _node, float _altitude )
{
	const float riverSlopeMultiplier = m_ParameterSlots[1].m_Value.f;

	for( int i = 0 ; i < _node->Childs.size() ; ++i )
	{
		float dist = Distance( _node->Pos, _node->Childs[i]->Pos );

		RiverSegment segment;
		segment.A = Vec3( _node->Pos, _altitude );
		segment.B = Vec3( _node->Childs[i]->Pos, _altitude + _node->Slope * riverSlopeMultiplier * dist );


		_segments.push_back( segment );

		RecGatherRiverSegmentsForGPU( _segments, _node->Childs[i], segment.B.z );
	}
}


CDialogEx* MountainNode::GetCustomUI( CWnd* _pParent )
{
	RiverEditorDlg* pUI = new RiverEditorDlg;
	pUI->m_pComputeNode = this;
	pUI->m_RiverControl.m_ppRiverRoot = &m_pRiverRoot;

	if( !pUI->Create( IDD_DIALOG_RIVER_EDITOR, _pParent ) )
	{
		//LOG_R( "Failed to create custom UI for node %s", GetNodeClassName() );
		assert( false );
		delete pUI;
		return nullptr;
	}

	return pUI;
}

void MountainNode::InternalCompute()
{
	if( m_CurRiverSlopeMultiplier != m_ParameterSlots[1].m_Value.f )
		OnRiverChanged(); //recompute river slope

	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	ID3D11Buffer* pCB = m_CB.GetBuffer();
	pDevCtx->CSSetConstantBuffers( 0, 1, &pCB );

	//	ID3D11SamplerState* samplers[] = { g_Renderer.GetBilinearClampSampler() };
	//	pDevCtx->CSSetSamplers( 0, _countof( samplers ), samplers );

	const uint32_t threadGroupSizeX = 32;
	const uint32_t threadGroupSizeY = 32;

	uint32_t numGroupsX = (GetResolution() + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = (GetResolution() + threadGroupSizeY - 1) / threadGroupSizeY;

	ID3D11UnorderedAccessView* uavs[] = { m_Output.GetUAV() };
	ID3D11ShaderResourceView* srvs[3] = { m_SegmentBufferSRV, nullptr, nullptr };
	uint32_t numSrvs = 1;
	
	//Add optional disto srvs

	for( uint32_t i = 0 ; i < m_InputSlots.size() ; ++i )
	{
		const InputSlot& slot = m_InputSlots[i];

		shared_ptr<ComputeNode> inputNode = slot.m_RemoteOutputSlot.m_pNode.lock();

		if( !inputNode )
		{
			if( slot.m_bOptional )
			{
				srvs[numSrvs++] = nullptr;
				continue;
			}
			else
			{
				return; //TODO error code
			}
		}

		//assert( !inputNode->IsDirty() );

		const Map* inputMap = inputNode->GetOutput( slot.m_RemoteOutputSlot.m_SlotIndex );

		if( !inputMap )
			return; //TODO error code

		srvs[numSrvs++] = inputMap->GetSRV();
	}

	pDevCtx->CSSetShader( m_CS, nullptr, 0 );
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof( uavs ), uavs, nullptr );
	pDevCtx->CSSetShaderResources( 0, _countof( srvs ), srvs );

	m_CB.Resolution = GetResolution();
	m_CB.MoutainSlope = m_ParameterSlots[0].m_Value.f;
	m_CB.ValleyWidth = m_ParameterSlots[2].m_Value.f;
	m_CB.ValleyShape = m_ParameterSlots[3].m_Value.f;
	m_CB.Distortion = m_ParameterSlots[4].m_Value.f;
	m_CB.UploadToGPU( pDevCtx );

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	//Unbind everything
	static ID3D11ShaderResourceView* nullSRVs[ 2 ] = { NULL };
	pDevCtx->CSSetShaderResources( 0, _countof( nullSRVs ), nullSRVs );

	static ID3D11UnorderedAccessView* nullUAVs[ 1 ] = { NULL };
	pDevCtx->CSSetUnorderedAccessViews( 0, _countof( nullUAVs ), nullUAVs, nullptr );
	
}