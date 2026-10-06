#include "stdafx.h"
#include "NodeFactory.h"

NodeFactory g_NodeFactory;

NodeFactory::NodeFactory()
{
}

NodeFactory::~NodeFactory()
{
}

void NodeFactory::RegisterNode( const char* _className, NodeCategory _cat, const char* _UIName, int _guid, CreateNodeFn _createFn )
{
	assert( m_NodeFactories.find( _guid ) == m_NodeFactories.end() ); //duplicate guid

	NodeInfo nodeInfo;
	nodeInfo.m_ClassName = _className;
	nodeInfo.m_UIName = _UIName;
	nodeInfo.m_Category = _cat;
	nodeInfo.m_Factory = _createFn;

	m_NodeFactories[ _guid ] = nodeInfo;
}

ComputeNode* NodeFactory::CreateNode( const char* _className )
{
	for( const auto& nodeInfo : m_NodeFactories )
	{
		if( nodeInfo.second.m_ClassName == _className )
		{
			return nodeInfo.second.m_Factory();
		}
	}

	return nullptr;
}

ComputeNode* NodeFactory::CreateNode( int _guid )
{
	FactoryMap::const_iterator it = m_NodeFactories.find( _guid );

	if( it == m_NodeFactories.end() )
		return nullptr;

	return it->second.m_Factory();
}
