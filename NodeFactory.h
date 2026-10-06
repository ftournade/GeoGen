#pragma once
#include <map>
using namespace xtm;//TODO ....

class ComputeNode;


enum class NodeCategory //Used for UI
{
	Generator,
	Modifier,
	Combiner,
	Mask,
	Natural,
	InputOutput
};

#define REGISTER_COMPUTE_NODE( _ClassName, _Category, _UIName, _ID ) g_NodeFactory.RegisterNode( #_ClassName, NodeCategory::_Category, _UIName, _ID, _ClassName::Instantiate )

class NodeFactory
{
public:
	typedef ComputeNode* (*CreateNodeFn)();

	struct NodeInfo
	{
		std::string m_ClassName, m_UIName;
		NodeCategory m_Category;
		CreateNodeFn m_Factory;
	};

	typedef std::map< int, NodeInfo > FactoryMap;
public:
	NodeFactory();
	~NodeFactory();

	void RegisterNode( const char* _className, NodeCategory _cat, const char* _UIName, int _guid, CreateNodeFn _createFn );
	
	ComputeNode* CreateNode( const char* _className );
	ComputeNode* CreateNode( int _guid );

	inline const FactoryMap& GetFactories() { return m_NodeFactories; }

private:
	std::map< int, NodeInfo > m_NodeFactories;
};

extern NodeFactory g_NodeFactory;