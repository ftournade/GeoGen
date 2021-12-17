#pragma once
#include "ComputeNode.h"
#include <Core/Curve.h>
#include <Core/Keyframer.h>

class ColorGradientNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ColorGradientNode )

	ColorGradientNode();
	virtual ~ColorGradientNode();

	virtual bool OneTimeInit();

	virtual const Map* GetOutput( u32 _idx ) const;

	virtual CDialogEx* GetCustomUI( CWnd* _pParent );

	void OnColorGradientChanged();
	

	//Load/Save

	virtual bool Load( const tinyxml2::XMLElement* _xmlNode );
	virtual tinyxml2::XMLElement* Save( tinyxml2::XMLDocument& _xmlDoc ) const;

protected:
	virtual void InternalCompute();

	virtual bool OnResolutionChanged();

private:
	Map m_Output;
	Map m_GPUColorGradient; //just a COLOR_RAMP_RES x 1 texture

	struct Constants
	{
		float m_Resolution, pad[ 3 ];
	};

	ConstantBuffer< Constants > m_CB;
	D3DObject<ID3D11ComputeShader> m_pComputeShader;
public:
	Keyframer< Color > m_ColorGradient;
};

