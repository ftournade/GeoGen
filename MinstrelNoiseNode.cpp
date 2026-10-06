#include "stdafx.h"
#include "MinstrelNoiseNode.h"

/*
MinstrelNoiseNode::MinstrelNoiseNode()
{
	SetUIName( "Minstrel noise" );
	
	AddOutput( "FloatMap", IOType::Float );
	
	if( !g_Renderer.CreateShader( "Shaders/MinstrelNoise.hlsl", "SetupGrid", nullptr, &m_SetupGridCS ) )
	{
		
		assert( false );
		//return false; //TODO not in constructor, have an Init method
	}

	if( !g_Renderer.CreateShader( "Shaders/MinstrelNoise.hlsl", "Main", nullptr, &m_MainCS ) )
	{

		assert( false );
		//return false; //TODO not in constructor, have an Init method
	}

	if( !m_GridPointsMap.Init( 32, 32, DXGI_FORMAT_R32G32B32A32_FLOAT ) ) //RG=(x1,y1) GB=(x2,y2)
	{
		assert( false );
	}

}


MinstrelNoiseNode::~MinstrelNoiseNode()
{
}

bool MinstrelNoiseNode::OnResolutionChanged()
{
	m_resolution = GetResolution();

	if( !m_OutputMap.Init( GetResolution(), IOType::Float ) )
		return false;

	return true;
}

const Map* MinstrelNoiseNode::GetOutput( uint32_t _idx ) const
{
	return &m_OutputMap;
}

void MinstrelNoiseNode::InternalCompute()
{
	ID3D11DeviceContext* pDevCtx = g_Renderer.GetImmediateDeviceContext();

	//Compute points
	pDevCtx->CSSetShader( m_SetupGridCS, nullptr, 0 );
	
	ID3D11UnorderedAccessView* uavs1[] = { m_GridPointsMap.GetUAV() };
	pDevCtx->CSSetUnorderedAccessViews( 0, 1, uavs1, nullptr );
	
	pDevCtx->Dispatch( 1, 1, 1 ); //TODO

	//Compute fractal
	pDevCtx->CSSetShader( m_MainCS, nullptr, 0 );

	ID3D11UnorderedAccessView* uavs2[] = { m_OutputMap.GetUAV() };
	pDevCtx->CSSetUnorderedAccessViews( 0, 1, uavs2, nullptr );

	ID3D11ShaderResourceView* srvs[] = { m_GridPointsMap.GetSRV() };
	pDevCtx->CSSetShaderResources( 0, 1, srvs );

	const uint32_t threadGroupSizeX = 32;
	const uint32_t threadGroupSizeY = 32;

	uint32_t numGroupsX = (m_resolution + threadGroupSizeX - 1) / threadGroupSizeX;
	uint32_t numGroupsY = (m_resolution + threadGroupSizeY - 1) / threadGroupSizeY;

	pDevCtx->Dispatch( numGroupsX, numGroupsY, 1 );

	static ID3D11UnorderedAccessView* nullUAVs[] = { NULL };
	pDevCtx->CSSetUnorderedAccessViews( 0, 1, nullUAVs, nullptr );
}
*/

//Inefficient but "stateless" implementation
MinstrelNoiseNode::MinstrelNoiseNode() : CustomComputeNode( 1, 1 )
{
	SetUIName( "Fake Erosion" );

	AddParam( "Main", "FeatureSize", IOType::Float, ParamEdition::Slider, 50.0f, 0.000001f, 200.0f, false );
	AddParam( "Main", "BaseScale", IOType::Float, ParamEdition::Slider, 1.0f, 0.000001f, 2.0f, false );
	AddParam( "Main", "ErosionScale", IOType::Float, ParamEdition::Slider, 0.08f, -0.3f, 0.3f, false );
	AddParam( "Main", "Chaos", IOType::Float, ParamEdition::Slider, 1.0f, 0.0f, 2.0f, false );

	SetHLSLPrefix(
		"float HashX( int x, int y ) { return Chaos * (frac( sin( float( x ) + float( y ) * 793.34f ) * 12345.0f ) - 0.5f); }\n"
		"float HashY( int x, int y ) { return Chaos * (frac( sin( float( x ) + float( y ) * 934.78f ) * 5432.0f ) - 0.5f); }\n"

		"float sqr( float x ) { return x * x; }\n"

		"float BaseHeightmap( float2 _uv ) { return _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f ); }\n"

		"float PseudoErosion( float2 _wsPos, float _gridSize )\n"
		"{\n"
		"	int iX = int( 0.5f + _wsPos.x / _gridSize );\n"
		"	int iY = int( 0.5f + _wsPos.y / _gridSize );\n"

		"	float x1, y1, x2, y2;\n"
		"	float minh = 1000000.0f;\n"

		"	for( int OY = -1 ; OY <= 1 ; ++OY )\n"
		"	{\n"
		"		for( int OX = -1 ; OX <= 1 ; ++OX )\n"
		"		{\n"
		"			x1 = (float( iX + OX ) + HashX( iX + OX, iY + OY )) * _gridSize;\n"
		"			y1 = (float( iY + OY ) + HashY( iX + OX, iY + OY )) * _gridSize;\n"

		"			// find lowest neighbor (x2, y2) on base heightmap\n"

		"			float lowestNeighbor = 100000.0f;\n"

		"			for( int oy = -1 ; oy <= 1 ; ++oy )\n"
		"			{\n"
		"				for( int ox = -1 ; ox <= 1 ; ++ox )\n"
		"				{\n"
		"					float2 candidate;\n"
		"					candidate.x = (float( iX + ox + OX ) + HashX( iX + ox + OX, iY + oy + OY )) * _gridSize;\n"
		"					candidate.y = (float( iY + oy + OY ) + HashY( iX + ox + OX, iY + oy + OY )) * _gridSize;\n"

		"					float height = BaseHeightmap( candidate );\n"

		"					if( height < lowestNeighbor )\n"
		"					{\n"
		"						x2 = candidate.x;\n"
		"						y2 = candidate.y;\n"
		"						lowestNeighbor = height;\n"
		"					}\n"
		"				}\n"
		"			}\n"

		"			//compute for each neighbor h and keep lowest\n"

		"			float dx = x1 - x2;\n"
		"			float dy = y1 - y2;\n"
		"			float dd = sqr( dx ) + sqr( dy );\n"

		"			float f1 = (dy * (_wsPos.y - y1) + dx * (_wsPos.x - x1)) / dd;\n"

		"			float h;\n"

		"			if( f1 > 0.0f )\n"
		"			{\n"
		"				h = sqrt( sqr( _wsPos.x - x1 ) + sqr( _wsPos.y - y1 ) );\n"
		"			}\n"
		"			else if( f1 < -1.0f )\n"
		"			{\n"
		"				h = sqrt( sqr( _wsPos.x - x2 ) + sqr( _wsPos.y - y2 ) );\n"
		"			}\n"
		"			else\n"
		"			{\n"
		"				float f2 = abs( (dy * (_wsPos.x - x1) - dx * (_wsPos.y - y1)) / sqrt( dd ) );\n"
		"				h = f2;\n"
		"			}\n"

		"			if( h < minh )\n"
		"				minh = h;\n"
		"		}\n"
		"	}\n"

		"	minh /= _gridSize;\n"

		"	return minh;\n"
		"}\n" );

	SetHLSLBody(
		"float2 wsPos = _uv;\n"
		//"_wsPos *= 0.001f;\n"
		"float gridSize = FeatureSize * 0.001f;\n"

		"float baseMap = BaseHeightmap( _uv ) * BaseScale;\n"

		"float r1 = PseudoErosion( wsPos, gridSize );\n"
		"float erosion = sqr( r1 );\n"

		"float r2 = PseudoErosion( wsPos, gridSize / 2.0f );\n"
		"erosion += r1 * r2 / 2.0f;\n"

		"float r3 = PseudoErosion( wsPos, gridSize / 4.0f );\n"
		"erosion += sqrt( r1*r2 )*r3 / 3.0f;\n"

		"_output0[ _pos ] = baseMap * BaseScale + erosion * ErosionScale;\n"
	);
}
