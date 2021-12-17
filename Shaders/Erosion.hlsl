//Implementation of "Fast Hydraulic and Thermal Erosion on the GPU"

RWTexture2D<float> HeightMap : register(u0);
RWTexture2D<float> WaterMap : register(u1);
RWTexture2D<float4> WaterOutFlowMap : register(u2); //L/R/T/B
RWTexture2D<float2> WaterVelocityMap : register(u3);
RWTexture2D<float> RWSuspendedSoilMap : register(u4);
RWTexture2D<float4> ThermalErosionOutFlowMap1 : register(u5); //L/R/T/B
RWTexture2D<float4> ThermalErosionOutFlowMap2 : register(u6); //UL / LL / UR / LR
Texture2D<float> SuspendedSoilMap : register(t0);
//Texture2D<float> HeightMapCopy : register(t1);

SamplerState g_BilinearSampler : register(s0);

#include "Shaders/ErosionConstants.h"

//Out flow defines

#define Left x
#define Right y
#define Top z
#define Bottom w

#define UpperLeft x
#define LowerLeft y
#define UpperRight z
#define LowerRight w


float TerrainHeight( int2 p )
{
	return lerp( MinAltitude, MaxAltitude, HeightMap[ p ] );
}


float TerrainPlusWaterHeight( int2 p )
{
	return TerrainHeight( p ) + WaterMap[ p ];
}


[numthreads( 32, 32, 1 )]
void AddWater( uint2 _pos : SV_DispatchThreadID )
{
	//Add water (rain)
	//////////////////
#if 0
	const int2 sourcePos = int2(220, 200);
	float2 r = float2((int2)_pos - sourcePos);
	if( length( r ) > 40.0f )
		return;
#endif

	float w = WaterMap[ _pos ];
	w += RainRate * dt;
	WaterMap[ _pos ] = w;
}

[numthreads( 32, 32, 1 )]
void UpdateWaterOutFlow( uint2 _pos : SV_DispatchThreadID )
{

	//Update water outflow
	//////////////////////
	const float A = 1.0f;//virtual pipe cross section
	const float l = 1.0f;//virtual pipe length
	const float g = 9.81f;//gravity
	
	float fluxFactor = dt * A * g / l;

	//TODO handle border condition
	
	float w = WaterMap[ _pos ];
	float t = TerrainHeight( _pos );
	float4 outFlow = WaterOutFlowMap[ _pos ];

	float tw = t + w;

	//TODO iron out signed/unsigned stuff
	//Left
	if( _pos.x > 0 )
	{
		float dh = tw - TerrainPlusWaterHeight( int2(_pos.x - 1, _pos.y) );
		outFlow.x += fluxFactor * dh;
		outFlow.x = max( outFlow.x, 0.0f );
	}
	else
	{
		outFlow.x = 0.0f;
	}

	//Right
	if( _pos.x < TerrainResolution - 1 )
	{
		float dh = tw - TerrainPlusWaterHeight( int2(_pos.x + 1, _pos.y) );
		outFlow.y += fluxFactor * dh;
		outFlow.y = max( outFlow.y, 0.0f );
	}
	else
	{
		outFlow.y = 0.0f;
	}

	//Top
	if( _pos.y > 0 )
	{
		float dh = tw - TerrainPlusWaterHeight( int2(_pos.x, _pos.y - 1) );
		outFlow.z += fluxFactor * dh;
		outFlow.z = max( outFlow.z, 0.0f );
	}
	else
	{
		outFlow.z = 0.0f;
	}

	//Bottom
	if( _pos.y < TerrainResolution - 1 )
	{
		float dh = tw - TerrainPlusWaterHeight( int2(_pos.x, _pos.y + 1) );
		outFlow.w += fluxFactor * dh;
		outFlow.w = max( outFlow.w, 0.0f );
	}
	else
	{
		outFlow.w = 0.0f;
	}

	float totalFlux = outFlow.x + outFlow.y + outFlow.z + outFlow.w;

	float K = (w * CellSizeX * CellSizeZ) / (totalFlux * dt);
	K = min( K, 1.0f ); //paper says max ? (I think the error is in the paper)

	outFlow *= K;

	WaterOutFlowMap[ _pos ] = outFlow;
}

[numthreads( 32, 32, 1 )]
void UpdateWaterHeightAndVelocity( uint2 _pos : SV_DispatchThreadID )
{
	//Update water height
	float4 outFlow = WaterOutFlowMap[ _pos ];
	float outFlowL = WaterOutFlowMap[ int2(_pos.x - 1, _pos.y) ].Right;
	float outFlowR = WaterOutFlowMap[ int2(_pos.x + 1, _pos.y) ].Left;
	float outFlowT = WaterOutFlowMap[ int2(_pos.x, _pos.y - 1) ].Bottom;
	float outFlowB = WaterOutFlowMap[ int2(_pos.x, _pos.y + 1) ].Top;

	float totalInFlow = outFlowL + outFlowR + outFlowT + outFlowB;
	float totalOutFlow = outFlow.Left + outFlow.Right + outFlow.Top + outFlow.Bottom;

	float dV = dt * (totalInFlow - totalOutFlow);

	float w = WaterMap[ _pos ];
	float oldWater = w;

	w += dV / (CellSizeX * CellSizeZ);
	w = max( w, 0.0f );
	WaterMap[ _pos ] = w;

	//Update water velocity
	float meanWater = (oldWater + w) * 0.5f;

	float2 waterVelocity;

//	if( meanWater == 0.0f )
//	{
//		waterVelocity = float2( 0.0f, 0.0f );
//	}
//	else
	{
		waterVelocity.x = 0.5f * ( outFlowL - outFlow.Left - outFlowR + outFlow.Right );// / (CellSizeZ * meanWater);
		waterVelocity.y = 0.5f * ( outFlowT - outFlow.Top - outFlowB + outFlow.Bottom );// / (CellSizeX * meanWater);
	}

	WaterVelocityMap[ _pos ] = waterVelocity;
}

[numthreads( 32, 32, 1 )]
void ThermalErosionOutFlow( uint2 _pos : SV_DispatchThreadID )
{
	float f = (MaxAltitude - MinAltitude) / CellSizeX; //TODO for corners dist is longer
	float talusFactor = Tan_TalusAngle / f; //TODO precompute in CB
	float erosion = ThermalErosionSpeed * dt / 1000.0f;

	float h = HeightMap[ _pos ];

	float4 outFlow1 = float4(0, 0, 0, 0);
	float4 outFlow2 = float4(0, 0, 0, 0);

	//f = (MaxAlt - MinAlt) / CellSizeX;
	//atan( dH * f ) > a 
	//dH * f > tan( a )
	//dH > tan(a) / f

	#define OUTFLOW( X, Y, outflow ) 	if( h - HeightMap[int2(_pos.x + (X),_pos.y + (Y))] > talusFactor ) { outflow += erosion; }

	OUTFLOW( -1,  0, outFlow1.Left )
	OUTFLOW(  1,  0, outFlow1.Right )
	OUTFLOW(  0, -1, outFlow1.Top )
	OUTFLOW(  0,  1, outFlow1.Bottom )
	OUTFLOW( -1, -1, outFlow2.UpperLeft )
	OUTFLOW(  1, -1, outFlow2.UpperRight )
	OUTFLOW( -1,  1, outFlow2.LowerLeft )
	OUTFLOW(  1,  1, outFlow2.LowerRight )
		
	ThermalErosionOutFlowMap1[_pos] = outFlow1;
	ThermalErosionOutFlowMap2[_pos] = outFlow2;

	HeightMap[_pos] = h - (outFlow1.x + outFlow1.y + outFlow1.z + outFlow1.w + outFlow2.x + outFlow2.y + outFlow2.z + outFlow2.w);
}

[numthreads( 32, 32, 1 )]
void HydraulicErosion( uint2 _pos : SV_DispatchThreadID )
{
	// local velocity
	float water = WaterMap[ _pos ];
	float2 waterVelocity = WaterVelocityMap[ _pos ];
	float suspendedSoil = RWSuspendedSoilMap[ _pos ];

	// local terrain normal 
	float altitudeScale = MaxAltitude - MinAltitude;
	float dHdx = HeightMap[ int2(_pos.x + 1, _pos.y) ] - HeightMap[ int2(_pos.x - 1, _pos.y) ];
	float dHdy = HeightMap[ int2(_pos.x, _pos.y + 1) ] - HeightMap[ int2(_pos.x, _pos.y - 1) ];

	float3 normal = float3(	dHdx * altitudeScale,
							2 * CellSizeX,
							dHdy * altitudeScale ); //TODO handle aspect ratio != 1
	normal = normalize( normal );

	float cosAlpha = normal.y;
	float sinAlpha = sqrt( 1.0f - cosAlpha * cosAlpha );
	sinAlpha = max( sinAlpha, 0.1f );

	// local sediment capacity of the flow

	float v = length( waterVelocity );
	float fctr = 1.0f;//TODO ?  min( water, 0.01f ) / 0.01f;

	float capacity = SedimentCapacity * v * sinAlpha * fctr;
	float delta = (capacity - suspendedSoil) * dt;

	float d;

	if( delta >= 0.0f )
	{
		d = DissolutionRate * delta;
	}
	else if( delta < 0.0f )
	{
		d = DepositionRate * delta;
	}

	HeightMap[ _pos ] -= d / (MaxAltitude - MinAltitude);
	WaterMap[ _pos ] = water + d;
	RWSuspendedSoilMap[ _pos ] = suspendedSoil + d;

}

[numthreads( 32, 32, 1 )]
void SoilTransportation( uint2 _pos : SV_DispatchThreadID )
{
	float2 waterVelocity = WaterVelocityMap[ _pos ];

	//take a step backward in time to advect soil
	float2 advectPos = float2(_pos) / (float)TerrainResolution - (waterVelocity / TerrainExtent) * dt;
//	advectPos /= TerrainResolution; //TODO is it right ?

	RWSuspendedSoilMap[ _pos ] = SuspendedSoilMap.SampleLevel( g_BilinearSampler, advectPos, 0.0f );

/*
	// integer coordinates
	int x0 = floor( advectPos.x );
	int y0 = floor( advectPos.y );
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// interpolation factors
	float fX = advectPos.x - x0;
	float fY = advectPos.y - y0;

	// clamp to grid borders //TODO us clamp sampler ?
	x0 = clamp( x0, 0, int( sediment.width() - 1 ) );
	x1 = clamp( x1, 0, int( sediment.width() - 1 ) );
	y0 = clamp( y0, 0, int( sediment.height() - 1 ) );
	y1 = clamp( y1, 0, int( sediment.height() - 1 ) );

	float newVal = mix( mix( sediment( y0, x0 ), sediment( y0, x1 ), fX ), mix( sediment( y1, x0 ), sediment( y1, x1 ), fX ), fY );
	tmpSediment( y, x ) = newVal;
*/
}

[numthreads( 32, 32, 1 )]
void ThermalErosion( uint2 _pos : SV_DispatchThreadID )
{
	float totalInFlow =
		ThermalErosionOutFlowMap1[int2(_pos.x - 1, _pos.y    )].Right +
		ThermalErosionOutFlowMap1[int2(_pos.x + 1, _pos.y    )].Left +
		ThermalErosionOutFlowMap1[int2(_pos.x    , _pos.y - 1)].Bottom +
		ThermalErosionOutFlowMap1[int2(_pos.x    , _pos.y + 1)].Top +
		ThermalErosionOutFlowMap2[int2(_pos.x - 1, _pos.y - 1)].LowerRight +
		ThermalErosionOutFlowMap2[int2(_pos.x + 1, _pos.y - 1)].LowerLeft +
		ThermalErosionOutFlowMap2[int2(_pos.x - 1, _pos.y + 1)].UpperRight +
		ThermalErosionOutFlowMap2[int2(_pos.x + 1, _pos.y + 1)].UpperLeft;

	HeightMap[_pos] += totalInFlow;
}

[numthreads( 32, 32, 1 )]
void WaterEvaporation( uint2 _pos : SV_DispatchThreadID )
{
	float water = WaterMap[ _pos ];

//	water = water * ( 1.0f - EvaporationRate * dt );
	water -= EvaporationRate * dt;
	water = max( water, 0.0f );

	WaterMap[ _pos ] = water;
}

[numthreads( 32, 32, 1 )]
void DepositAllSuspendedSoil( uint2 _pos : SV_DispatchThreadID )
{
	HeightMap[_pos] += RWSuspendedSoilMap[_pos] / (MaxAltitude - MinAltitude);
}

/*
[numthreads( 32, 32, 1 )]
void SmoothTerrain( uint2 _pos : SV_DispatchThreadID )
{
//	float smooth = Smoothing * dt;
	float smooth = Smoothing;

	float h = HeightMapCopy[ _pos ]
		+ ( HeightMapCopy[ int2(_pos.x - 1, _pos.y) ]
		+   HeightMapCopy[ int2(_pos.x + 1, _pos.y) ]
		+   HeightMapCopy[ int2(_pos.x, _pos.y - 1) ]
		+   HeightMapCopy[ int2(_pos.x, _pos.y + 1) ] ) * smooth;

	h /= 1.0f + 4.0f * smooth;
	
	HeightMap[ _pos ] = h;
}
*/