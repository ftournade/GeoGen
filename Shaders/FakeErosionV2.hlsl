/*
=====================================================================================

"Fake Erosion V2" node: applies runevision's Advanced Terrain Erosion Filter
(https://www.shadertoy.com/view/wXcfWn, see FakeErosionCommon.h) to an input heightmap.

Adapted from the Shadertoy's Buffer A (Heightmap function), which is copyright (c) 2025
Rune Skovbo Johansen.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

Units: horizontal positions and altitudes are both in kilometers, so slopes are real slopes
(the Shadertoy also uses the same unit horizontally and vertically).

=====================================================================================
*/

#include "Shaders/FakeErosionV2Constants.h"
#include "Shaders/FakeErosionCommon.h"

Texture2D<float> _heightMap		: register(t0);	//normalized altitude [0, 1] -> [MinAltitude, MaxAltitude]
Texture2D<float> _strengthMask	: register(t1);	//1x1 white texture when not connected

RWTexture2D<float> _outHeight	: register(u0);
RWTexture2D<float> _outErosion	: register(u1);	//0: carved (gullies), 1: raised (ridges)
RWTexture2D<float> _outRidges	: register(u2);	//0: creases, 1: ridges
RWTexture2D<float> _outDrainage	: register(u3);	//1: drainage lines at the bottom of creases

SamplerState _bilinearClampSampler : register(s0);

//Input altitude in meters (bilinear, _uv clamped to the map)
float InputAltitude( float2 _uv )
{
	return lerp( MinAltitude, MaxAltitude, _heightMap.SampleLevel( _bilinearClampSampler, saturate( _uv ), 0.0 ) );
}

[numthreads( 32, 32, 1 )]
void Main( uint2 _pos : SV_DispatchThreadID )
{
	uint2 res;
	_outHeight.GetDimensions( res.x, res.y );

	if( any( _pos >= res ) )
		return;

	//Same conventions as the other nodes (see CustomComputeNode): _uv in [0, 1], _wsPos in meters
	float2 uv = float2( _pos ) / float2( res - 1 );
	float2 wsPos = (uv - 0.5) * TerrainExtent;
	//Input height and slope: central differences over SlopeRadius (at least one texel), one-sided at the borders.
	//A larger radius gives smoother gully directions on noisy inputs such as real DEMs.

	float2 texelUV = 1.0 / float2( res );
	float2 centerUV = ( float2( _pos ) + 0.5 ) * texelUV;
	float2 radiusUV = max( texelUV, SlopeRadius * 1000.0 / TerrainExtent );

	float altitude = InputAltitude( centerUV );

	float2 uvx0 = float2( max( centerUV.x - radiusUV.x, 0.5 * texelUV.x ), centerUV.y );
	float2 uvx1 = float2( min( centerUV.x + radiusUV.x, 1.0 - 0.5 * texelUV.x ), centerUV.y );
	float2 uvy0 = float2( centerUV.x, max( centerUV.y - radiusUV.y, 0.5 * texelUV.y ) );
	float2 uvy1 = float2( centerUV.x, min( centerUV.y + radiusUV.y, 1.0 - 0.5 * texelUV.y ) );

	float2 slope;
	slope.x = (InputAltitude( uvx1 ) - InputAltitude( uvx0 )) / max( (uvx1.x - uvx0.x) * TerrainExtent, 1e-3 );
	slope.y = (InputAltitude( uvy1 ) - InputAltitude( uvy0 )) / max( (uvy1.y - uvy0.y) * TerrainExtent, 1e-3 );

	//Erosion (in km)

	float3 heightAndSlope = float3( altitude * 0.001, slope );

	float altitude01 = (altitude - MinAltitude) / (MaxAltitude - MinAltitude);

	// The fade target should strive to be -1 at valleys and 1 at peaks, but overshooting is ok.
	float fadeTarget = clamp( (altitude01 - FadeCenter) / FadeRange, -1.0, 1.0 );

	float mask = _strengthMask.SampleLevel( _bilinearClampSampler, ( float2( _pos ) + 0.5 ) / float2( res ), 0.0 );

	float ridgeMap, debug;
	float4 h = ErosionFilter(
		wsPos * 0.001, heightAndSlope, fadeTarget,
		Strength * mask, GullyWeight, Detail,
		float4( RidgeRounding, CreaseRounding, InputRounding, OctaveRoundingMult ),
		float4( Onset, OctaveOnset, RidgeMapOnset, RidgeMapOctaveOnset ),
		float2( AssumedSlope, AssumedSlopeAmount ),
		Scale, Octaves, Lacunarity,
		Gain, CellScale, Normalization,
		ridgeMap, debug );

	// Offset according to the height offset parameter by multiplying it with the magnitude.
	float offset = lerp( HeightOffset, -fadeTarget, PreserveExtremes ) * h.w;
	float erodedAltitude = (heightAndSlope.x + h.x + offset) * 1000.0;

	float eroded01 = (erodedAltitude - MinAltitude) / (MaxAltitude - MinAltitude);

	if( ClampHeight != 0 )
		eroded01 = saturate( eroded01 );

	//Masks

	//No erosion (zero strength) is reported as raised and as a ridge, so that it gets neither dirt nor drainage.
	//Both masks are relative to the local erosion magnitude, so they are faded by the strength mask to avoid a
	//hard edge where the strength reaches 0.
	float erosion = (h.w > 1e-8) ? saturate( h.x / h.w * 0.5 + 0.5 ) : 1.0;
	erosion = lerp( 1.0, erosion, saturate( mask ) );

	float ridges = saturate( ridgeMap * 0.5 + 0.5 );
	ridges = lerp( 1.0, ridges, saturate( mask ) );

	float drainage = saturate( (1.0 - saturate( ridges / DrainageWidth )) * 1.5 );

	_outHeight[ _pos ] = eroded01;
	_outErosion[ _pos ] = erosion;
	_outRidges[ _pos ] = ridges;
	_outDrainage[ _pos ] = drainage;
}
