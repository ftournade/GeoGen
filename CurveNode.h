#pragma once
#include "ComputeNode.h"

#include <Core/Keyframer.h>

class CurveNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( CurveNode )

	CurveNode();
	virtual ~CurveNode();
	
	virtual bool OneTimeInit();

	virtual const Map* GetOutput( u32 _idx ) const;

	virtual CDialogEx* GetCustomUI( CWnd* _pParent );

	void OnCurveChanged();

	bool IsInColorMode() const;

	//Load/Save

	virtual bool Load( const tinyxml2::XMLElement* _xmlNode );
	virtual tinyxml2::XMLElement* Save( tinyxml2::XMLDocument& _xmlDoc ) const;

protected:
	virtual void InternalCompute();

	virtual bool OnResolutionChanged();
private:
	Map m_Output;
	Map m_GPUCurveLookUp;
	
	struct Constants
	{
		float m_Resolution, pad[ 3 ];
	};

	ConstantBuffer< Constants > m_CB;
	D3DObject<ID3D11ComputeShader> m_pComputeShader;
public:
	Keyframer< Vec2 > m_Curve;
};

