#pragma once
#include "ComputeNode.h"

struct RiverNode
{
	RiverNode() {}
	RiverNode( const Vec2& _pos, float _slope = 0.1f ) : Pos(_pos), Slope(_slope) {}
	Vec2 Pos;
	float Slope;
	vector< std::shared_ptr< RiverNode > > Childs;
};



struct RiverSegment
{
	Vec3 A, B;
};

class MountainNode : public ComputeNode
{
public:
	COMPUTE_NODE_FACTORY( MountainNode )

	MountainNode();
	virtual ~MountainNode();

	virtual bool OneTimeInit();

	virtual const Map* GetOutput( uint32_t _idx ) const;

	virtual CDialogEx* GetCustomUI( CWnd* _pParent );

	void OnRiverChanged();

protected:
	virtual void InternalCompute();

	virtual bool OnResolutionChanged();

	void RecGatherRiverSegmentsForGPU( vector<RiverSegment>& _segments, shared_ptr< RiverNode > _node, float _altitude );
private:
	Map m_Output;

	struct Constants
	{
		uint32_t NumSegments;
		uint32_t Resolution;
		float MoutainSlope;
		float ValleyWidth;
		float ValleyShape;
		float Distortion;
		uint32_t pad[2];
	};
	
	ConstantBuffer< Constants > m_CB;

	D3DObject< ID3D11Buffer > m_SegmentBuffer;
	D3DObject< ID3D11ShaderResourceView > m_SegmentBufferSRV;
	D3DObject< ID3D11ComputeShader > m_CS;


	shared_ptr< RiverNode > m_pRiverRoot;

	float m_CurRiverSlopeMultiplier;
};

