#include "stdafx.h"
#include "PreviewNode.h"


PreviewNode::PreviewNode()
{
	SetUIName( "Preview" );

	AddInput( "HeightMap", IOType::Float );
	AddInput( "AlbedoMap", IOType::Color );
	AddInput( "WaterMap", IOType::Float );
}


PreviewNode::~PreviewNode()
{
}

bool PreviewNode::OnResolutionChanged()
{

	return true;
}

const Map* PreviewNode::GetOutput( uint32_t _idx ) const
{
	return nullptr;
}

void PreviewNode::InternalCompute()
{

}

const Map* PreviewNode::GetHeightMap() const
{
	shared_ptr<ComputeNode> pInputNode = m_InputSlots[ 0 ].m_RemoteOutputSlot.m_pNode.lock();

	return pInputNode ? pInputNode->GetOutput( m_InputSlots[ 0 ].m_RemoteOutputSlot.m_SlotIndex ) : nullptr;
}

const Map* PreviewNode::GetAlbedoMap() const
{
	shared_ptr<ComputeNode> pInputNode = m_InputSlots[ 1 ].m_RemoteOutputSlot.m_pNode.lock();

	return pInputNode ? pInputNode->GetOutput( m_InputSlots[ 1 ].m_RemoteOutputSlot.m_SlotIndex ) : nullptr;
}

const Map* PreviewNode::GetWaterMap() const
{
	shared_ptr<ComputeNode> pInputNode = m_InputSlots[ 2 ].m_RemoteOutputSlot.m_pNode.lock();

	return pInputNode ? pInputNode->GetOutput( m_InputSlots[ 2 ].m_RemoteOutputSlot.m_SlotIndex ) : nullptr;
}
