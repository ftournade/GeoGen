Texture2D<float> BedRockHardnessMap	: register(t0);
Texture2D<float> TemperatureMap		: register(t1);
Texture2D<float> RainMap			: register(t2);
Texture2D<float> SmoothingMap		: register(t3);
/*
Texture2D<float> BedRockMap			: register(t4);
Texture2D<float> BrockenRockMap		: register(t5);
Texture2D<float> SandMap			: register(t6);
Texture2D<float> HumusMap			: register(t7);
Texture2D<float> VegetationMap		: register(t8);
Texture2D<float> DeadVegetationMap	: register(t9);
*/
//----------------------

RWTexture2D<float> RWBedRockMap			: register(u0);
RWTexture2D<float> RWBrockenRockMap		: register(u1);	
RWTexture2D<float> RWSandMap			: register(u2);
RWTexture2D<float> RWHumusMap			: register(u3);
RWTexture2D<float> RWVegetationMap		: register(u4);
RWTexture2D<float> RWDeadVegetationMap	: register(u5);

RWTexture2D<float> RWSnowMap			: register(u6);
RWTexture2D<float> RWFlowMap			: register(u7);

//----------------------

SamplerState _pointClampSampler : register(s0);


#include "Shaders/MonteCarloErosionConstants.h"
#include "Shaders/Noise.h"

float2 Random( float2 _pos ) { return NoiseHash2_InigoQuilez( _pos + Seed ); }

struct RandomPick
{
	float m_Probability[ 8 ];
	float m_Probabilitysum;
};


int RandomPick_PickRandomly( float _Probability[ 8 ], float _Probabilitysum, float2 _rnd )
{
	float rnd = Random( _rnd ).x * _Probabilitysum * 0.999999f;

	float sum = 0.0f;

	[unroll]
	for( int i = 0; i < 8 /*rp.m_ItemCount*/; ++i )
	{
		sum += _Probability[ i ];

		if( rnd < sum )
			return i;
	}

	return 0;
}


bool OutOfMap( int2 _p )
{
	return ((_p.x < 0) || (_p.x >= Resolution) || (_p.y < 0) || (_p.y >= Resolution) );
}

float GetAltitude( int2 _p, RWTexture2D<float> _GranularMaterialMap )
{
	return RWBedRockMap[ _p ] + _GranularMaterialMap[ _p ];
}

static const int2 neighboor[ 8 ] =
{
	{ -1, -1 },
	{ -1,  0 },
	{ -1,  1 },
	{ 0, -1 }, 
	{ 0,  1 }, 
	{ 1, -1 }, 
	{ 1,  0 }, 
	{ 1,  1 }  
};

float GetSlope( int2 p, out int2 _lowestNGB, RWTexture2D<float> _GranularMaterialMap )
{
	float minAlt = 9999999.0f;

	//find lowest neighbor
	[loop]
	for( int i = 0; i < 8; ++i )
	{
		int2 ngb = p + neighboor[ i ];

		if( OutOfMap( ngb ) )
			continue;

		float alt = GetAltitude( ngb, _GranularMaterialMap );

		if( alt < minAlt )
		{
			minAlt = alt;
			_lowestNGB = neighboor[ i ];
		}
	}

	float dx = _lowestNGB.x * CellSize;
	float dy = _lowestNGB.y * CellSize;

	float dist = sqrt( dx * dx + dy * dy );

	return (GetAltitude( p, _GranularMaterialMap ) - minAlt) / dist;
}

float GetSlope( int2 a, float aAlt, int2 b, RWTexture2D<float> _GranularMaterialMap )
{
	float alt1 = aAlt;
	float alt2 = GetAltitude( b, _GranularMaterialMap );

	float dx = (b.x - a.x) * CellSize;
	float dy = (b.y - a.y) * CellSize;

	float dist = sqrt( dx * dx + dy * dy );

	return (alt2 - alt1) / dist;
}


bool PickRandomDownhillNeighboor( float2 _rnd, int2 _p, int2 _prev_p, out int2 _ngb, out float _slope, RWTexture2D<float> _GranularMaterialMap )
{

	float p_alt = GetAltitude( _p, _GranularMaterialMap );

	RandomPick randomPickDirection;
	float directionProbability[ 8 ];
	float directionProbabilitySum = 0.0f;

	bool bReachedLocalMinimum = true;

	float ngb_slope[ 8 ];

	[loop]
	for( int i = 0; i < 8; ++i )
	{
		int2 ngb = _p + neighboor[ i ];

		if(/* (ngb == _prev_p) || */OutOfMap( ngb ) ) //Don't backtrack and don't leave map
		{
			//TODO kill agent instead ?
			directionProbability[ i ] = 0.0f;
			continue;
		}

		ngb_slope[ i ] = GetSlope( _p, p_alt, ngb, _GranularMaterialMap );

		if( ngb_slope[ i ] > 0.0f )
		{
			//uphill
			directionProbability[ i ] = 0.0f;
			continue;
		}

		bReachedLocalMinimum = false;

		float dirProbability = -ngb_slope[ i ] + 0.001f;

		directionProbability[ i ] = dirProbability;
		directionProbabilitySum += dirProbability;
	}

	if( bReachedLocalMinimum )
		return false;

	int ngbIndex = RandomPick_PickRandomly( directionProbability, directionProbabilitySum, _rnd );

	_ngb = _p + neighboor[ ngbIndex ];
	_slope = ngb_slope[ ngbIndex ];

	return true;
}


float GetNeighborAverageAltitude( int2 _p, RWTexture2D<float> _GranularMaterialMap )
{
	float avg = 0;
	int cnt = 0;

	for( int i = 0; i < 8; ++i )
	{
		int2 ngb = _p + neighboor[ i ];

		if( OutOfMap( ngb ) )
			continue;

		avg += GetAltitude( ngb, _GranularMaterialMap );
		++cnt;
	}

	return avg / (float)cnt;
}

[numthreads( 8, 8, 1 )]
void HydraulicEvent( uint2 _pos : SV_DispatchThreadID )
{
	float2 p0 = Random( _pos );

	int2 p = p0 * (float)(Resolution - 1);
	int2 prev_p = int2( -1, -1 );

	float water = RainRate;
	float slope = 0.0f;
	float carriedSediment = 0.0f;

	while( water > 0.0f ) //TODO hard limit on event length
	{
		RWFlowMap[ p ] += water;

		int2 new_p;

		if( !PickRandomDownhillNeighboor( _pos * -0.748345f, p, prev_p, new_p, slope, RWSandMap ) )
		{
			//reached local minimum
			//"fill the hole" and then a bit more to exit minima

			float deposition = GetNeighborAverageAltitude( p, RWSandMap ) - RWBedRockMap[ p ] + 0.0001f;
			deposition = min( deposition, carriedSediment );

			carriedSediment -= deposition;
			RWSandMap[ p ] += deposition;

			if( !PickRandomDownhillNeighboor( _pos * -0.896731f, p, prev_p, new_p, slope, RWSandMap ) ) //Try again
				break;
		}


		if( slope < -ErosionSlopeThreshold )
		{
			if( carriedSediment < SedimentCapacity * water )
			{
				//erosion
				float erosion = abs( slope ) * ErosionRate;
				erosion = min( erosion, SedimentCapacity * water - carriedSediment ); //Don't exceed sediment capacity
				carriedSediment += erosion;
				RWBedRockMap[ p ] -= erosion;
			}
		}
		else
		{
			//deposition
			float deposition = DepositionRate;
			deposition = min( deposition, carriedSediment );
			carriedSediment -= deposition;
			RWSandMap[ p ] += deposition; //TODO add rocks or sand not bedrock
		}

		water -= EvaporationRate;
		water = max( water, 0.0f );

		if( carriedSediment > SedimentCapacity * water )
		{
			//deposit sediment if water capacity exceeded

			float deposition = carriedSediment - SedimentCapacity * water;
			carriedSediment -= deposition;
			RWSandMap[ p ] += deposition; //TODO add rocks or sand not bedrock
		}

		prev_p = p;
		p = new_p;
	}

}


void GravityEventFn( uint2 _pos, RWTexture2D<float> _GranularMaterialMap )
{
	float2 p0 = Random( _pos );

	int2 p = p0 * (float)(Resolution - 1);
	int2 prev_p = int2( -1, -1 );

	int2 lowestNGB;

	float slope = GetSlope( p, lowestNGB, _GranularMaterialMap );

	int i = 0;

	[loop]
	while( ( i < 512 ) && ( slope > TalusAngle ) )
	{
		float movingSand = Speed * CellSize * (slope - TalusAngle);
		int2 new_p;

		if( !PickRandomDownhillNeighboor( _pos * -0.93481f, p, prev_p, new_p, slope, _GranularMaterialMap ) )
			return;//reached local minimum

		float sand = _GranularMaterialMap[ p ]; //(sand, snow, gravels ...)
		
		movingSand = min( movingSand, sand );

		_GranularMaterialMap[ p ] -= movingSand;
		_GranularMaterialMap[ new_p ] += movingSand;

		prev_p = p;
		p = new_p;

		slope = GetSlope( p, lowestNGB, _GranularMaterialMap );

		++i;
	}
}
/*
int2 UVToPixelCoord( float2 _uv, int2 _duv )
{
	int2 p = (int2)(_uv * (float)Resolution) + _duv;
	p.x = max( p.x, 0 );
	p.x = min( p.x, Resolution );
	p.y = max( p.y, 0 );
	p.y = min( p.y, Resolution );
	return p;
}
*/
float TotalTerrainAltitude( int2 _p, RWTexture2D<float> _GranularMaterialMap, inout bool _bAnyGranularMaterialAtAll )
{
	float bedRock = RWBedRockMap[ _p ];
	float granularMaterial = _GranularMaterialMap[ _p ];
	
	if( granularMaterial > 0.000001f )
		_bAnyGranularMaterialAtAll = true;

	return bedRock + granularMaterial;
}

void SmoothEventFn( uint2 _pos, RWTexture2D<float> _GranularMaterialMap )
{
	float2 p0 = Random( _pos );
	int2 p = p0 * (float)( Resolution - 1 );
	float2 uv = p0;

	float smoothing = SmoothingMap.SampleLevel( _pointClampSampler, uv, 0.0f );
	float blur = smoothing * Smooth;
	float scale = 1.0f / (1.0f + blur * 6.0f);

	bool bAnyGranularMaterialAtAll = false;
	
	float sum;
	sum  = TotalTerrainAltitude( p + int2( - 1, - 1 ), _GranularMaterialMap, bAnyGranularMaterialAtAll ) * 0.5f;
	sum += TotalTerrainAltitude( p + int2(   0, - 1 ), _GranularMaterialMap, bAnyGranularMaterialAtAll );
	sum += TotalTerrainAltitude( p + int2(   1, - 1 ), _GranularMaterialMap, bAnyGranularMaterialAtAll ) * 0.5f;
	sum += TotalTerrainAltitude( p + int2( - 1,   0 ), _GranularMaterialMap, bAnyGranularMaterialAtAll );
	sum += TotalTerrainAltitude( p + int2(   1,   0 ), _GranularMaterialMap, bAnyGranularMaterialAtAll );
	sum += TotalTerrainAltitude( p + int2( - 1,   1 ), _GranularMaterialMap, bAnyGranularMaterialAtAll ) * 0.5f;
	sum += TotalTerrainAltitude( p + int2(   0,   1 ), _GranularMaterialMap, bAnyGranularMaterialAtAll );
	sum += TotalTerrainAltitude( p + int2(   1,   1 ), _GranularMaterialMap, bAnyGranularMaterialAtAll ) * 0.5f;

	sum *= blur;
	sum += TotalTerrainAltitude( p + int2(0,0), _GranularMaterialMap, bAnyGranularMaterialAtAll );
	sum *= scale;

	if( !bAnyGranularMaterialAtAll )
		return;

	float terrain = RWBedRockMap[ p ];

	float granularMaterial = max( sum - terrain, 0.0f ); 

	float temperature = saturate( TemperatureMap.SampleLevel( _pointClampSampler, uv, 0.0f ) );

	granularMaterial += SnowFall * (1.0f - temperature);
	granularMaterial -= SnowEvaporation * temperature;

	granularMaterial = max( granularMaterial, 0.0f );

	_GranularMaterialMap[ p ] = granularMaterial;
}

[numthreads( 16, 16, 1 )]
void SnowGravityEvent( uint2 _pos : SV_DispatchThreadID )
{
	GravityEventFn( _pos, RWSnowMap );
}	

[numthreads	( 16, 16, 1 )]
void SnowSmoothEvent( uint2 _pos : SV_DispatchThreadID )
{
	SmoothEventFn( _pos, RWSnowMap );
}


[numthreads( 16, 16, 1 )]
void GravityEvent( uint2 _pos : SV_DispatchThreadID )
{
	GravityEventFn( _pos, RWSandMap );
}

[numthreads( 16, 16, 1 )]
void SmoothEvent( uint2 _pos : SV_DispatchThreadID )
{
	SmoothEventFn( _pos, RWSandMap );
}
