/*
=====================================================================================

"Mountain Coloring" node: builds the color map of runevision's "Advanced Terrain Erosion
Filter" Shadertoy (https://www.shadertoy.com/view/wXcfWn) from the outputs of the
"Fake Erosion V2" node, as two maps:
 - Albedo:   the diffuse colors of the Image tab (cliffs, dirt, snow, sand, grass, trees,
             drainage, water), gamma encoded.
 - Lighting: the lighting of the Image tab for a white surface (ambient with erosion
             occlusion, sun with terrain shadows, bounce), gamma encoded, seen from above.
             Albedo x Lighting gives the gamma encoded lit color (gamma distributes over
             products). View dependent terms (specular, reflections, fog, tonemapping) are
             not included.

Adapted from the Shadertoy's Image tab (coloring and lighting), Buffer A (tree coverage)
and Buffer B (detail noise). Copyright (c) 2025 Rune Skovbo Johansen. The Shadertoy's
terrain rendering is itself derived from https://www.shadertoy.com/view/7ljcRW by Fewes,
and its BRDF functions from https://www.shadertoy.com/view/XlKSDR.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

Heights: the Shadertoy colors its terrain with absolute height thresholds, its eroded
terrain spanning about [0.36, 0.56]. Normalized GeoGen altitudes are mapped to that range
(ShaderHeight()), so the Shadertoy formulas below are kept as is.

=====================================================================================
*/

#include "Shaders/MountainColoringConstants.h"
#include "Shaders/FakeErosionCommon.h"

Texture2D<float> _heightMap		: register(t0);	//eroded heightmap
Texture2D<float> _erosionMap	: register(t1);	//Fake Erosion V2 "Erosion" output (1x1 white texture when not connected)
Texture2D<float> _ridgeMap		: register(t2);	//Fake Erosion V2 "Ridges" output (1x1 white texture when not connected)

RWTexture2D<float4> _outAlbedo		: register(u0);
RWTexture2D<float4> _outLighting	: register(u1);
RWTexture2D<float>  _outTrees		: register(u2);

SamplerState _bilinearClampSampler : register(s0);

#define SUN_COLOR		(float3( 1.0, 0.98, 0.95 ) * 2.0)
#define AMBIENT_COLOR	(float3( 0.3, 0.5, 0.7 ) * 0.1)

static const float c_ShaderHeightBase = 0.36;
static const float c_ShaderHeightScale = 0.2;

float ShaderHeight( float _altitude01 ) { return c_ShaderHeightBase + c_ShaderHeightScale * _altitude01; }

float Altitude01( float2 _uv ) { return _heightMap.SampleLevel( _bilinearClampSampler, _uv, 0.0 ); }
float AltitudeMeters( float2 _uv ) { return lerp( MinAltitude, MaxAltitude, Altitude01( _uv ) ); }

float3 GammaEncode( float3 _c ) { return pow( max( _c, 0.0 ), 1.0 / 2.2 ); }


// -----------------------------------------------------------------------------
// Detail noise (Buffer B)
// -----------------------------------------------------------------------------

float Breakup( float2 _uv )
{
	float3 color = float3( 0.0, 0.0, 0.0 );

	float a = 0.5;
	float f = 2.0;

	[unroll]
	for( int i = 0; i < 8; i++ )
	{
		color += fe_noised( _uv * f ) * a;
		a *= 0.95;
		f *= 2.0;
	}

	return color.x;
}


// -----------------------------------------------------------------------------
// Tree coverage (Buffer A)
// -----------------------------------------------------------------------------

float GetTreesAmount( float height, float normalY, float occlusion, float ridgeMap, float waterHeight, float grassHeight )
{
	float waterMask = EnableWater ? smoothstep( waterHeight + 0.000, waterHeight + 0.007, height ) : 1.0;

	return ((
		smoothstep(
			grassHeight + 0.05,
			grassHeight + 0.01,
			height + 0.01 + (occlusion - 0.8) * 0.05
		)
		* smoothstep(
			0.0,
			0.4,
			occlusion
		)
		* smoothstep( 0.95, 1.0, normalY )
		* smoothstep( -1.4, 0.0, ridgeMap )
		* waterMask
	) - 0.5) / 0.6;
}


// -----------------------------------------------------------------------------
// Lighting (Image tab, BRDF functions from https://www.shadertoy.com/view/XlKSDR)
// -----------------------------------------------------------------------------

float3 SkyColor( float3 rd, float3 sun )
{
	float costh = dot( rd, sun );
	return AMBIENT_COLOR * FE_PI * (1.0 - abs( costh ) * 0.8);
}

float pow5( float x )
{
	float x2 = x * x;
	return x2 * x2 * x;
}

float F_Schlick( float f0, float f90, float VoH )
{
	return f0 + (f90 - f0) * pow5( 1.0 - VoH );
}

float Fd_Burley( float linearRoughness, float NoV, float NoL, float LoH )
{
	// Burley 2012, "Physically-Based Shading at Disney"
	float f90 = 0.5 + 2.0 * linearRoughness * LoH * LoH;
	float lightScatter = F_Schlick( 1.0, f90, NoL );
	float viewScatter  = F_Schlick( 1.0, f90, NoV );
	return lightScatter * viewScatter * (1.0 / FE_PI);
}

float Fd_Lambert()
{
	return 1.0 / FE_PI;
}

//Soft shadow: march from the surface towards the sun over the heightmap, keeping the smallest
//ratio between the height above the terrain and the distance travelled (as the Shadertoy does)
float SunShadow( float2 _uv, float _altitude, float3 _sun )
{
	float2 horizontalDir = _sun.xz;
	float horizontalLength = length( horizontalDir );

	if( horizontalLength < 1e-4 )
		return 1.0; //sun at the zenith

	uint2 res;
	_heightMap.GetDimensions( res.x, res.y );
	float texelSize = TerrainExtent / (float)max( res.x, 1u );

	const int numSteps = 96;
	float tStart = 2.0 * texelSize;
	float tEnd = 1.5 * TerrainExtent / horizontalLength; //long enough to leave the terrain
	float growth = exp( log( tEnd / tStart ) / (float)(numSteps - 1) ); //geometric steps from tStart to tEnd

	float s_t = 9999.0;
	float t = tStart;

	[loop]
	for( int i = 0; i < numSteps; ++i )
	{
		float3 offset = _sun * t;
		float2 uv = _uv + offset.xz / TerrainExtent;

		if( any( uv < 0.0 ) || any( uv > 1.0 ) )
			break;

		float rayAltitude = _altitude + offset.y;

		if( rayAltitude > MaxAltitude )
			break;

		float altitudeAboveTerrain = rayAltitude - AltitudeMeters( uv );
		s_t = max( 0.0, min( s_t, altitudeAboveTerrain / t ) );

		if( s_t <= 0.0 )
			break;

		t *= growth;
	}

	return 1.0 - exp( -s_t * 20.0 );
}

float3 Lighting( float3 _normal, float3 _sun, float _occlusion, float _shadow )
{
	const float3 v = float3( 0.0, 1.0, 0.0 ); //seen from above
	float3 h = normalize( v + _sun );

	float NoV = abs( dot( _normal, v ) ) + 1e-5;
	float NoL = clamp01( dot( _normal, _sun ) );
	float LoH = clamp01( dot( _sun, h ) );

	const float linearRoughness = 1.0; //smoothness 0 (ground)

	// Ambient
	float3 color = SkyColor( _normal, _sun ) * Fd_Lambert() * _occlusion;
	// Direct (diffuse part of Shade())
	color += Fd_Burley( linearRoughness, NoV, NoL, LoH ) * SUN_COLOR * _shadow * NoL;
	// Bounce
	color += SUN_COLOR * (dot( _normal, _sun * float3( 1.0, -1.0, 1.0 ) ) * 0.5 + 0.5) * Fd_Lambert() / FE_PI;

	return color;
}


// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

[numthreads( 32, 32, 1 )]
void Main( uint2 _pos : SV_DispatchThreadID )
{
	uint2 res;
	_outAlbedo.GetDimensions( res.x, res.y );

	if( any( _pos >= res ) )
		return;

	float2 uv = ( float2( _pos ) + 0.5 ) / float2( res );
	float2 texel = 1.0 / float2( res );
	float cellSize = TerrainExtent / (float)res.x; //meters between texel centers

	//Height and normal (central differences in meters)

	float altitude01 = Altitude01( uv );
	float altitude = lerp( MinAltitude, MaxAltitude, altitude01 );

	float2 uvx0 = float2( max( uv.x - texel.x, 0.5 * texel.x ), uv.y ), uvx1 = float2( min( uv.x + texel.x, 1.0 - 0.5 * texel.x ), uv.y );
	float2 uvy0 = float2( uv.x, max( uv.y - texel.y, 0.5 * texel.y ) ), uvy1 = float2( uv.x, min( uv.y + texel.y, 1.0 - 0.5 * texel.y ) );

	float dhdx = (AltitudeMeters( uvx1 ) - AltitudeMeters( uvx0 )) / max( (uvx1.x - uvx0.x) * res.x * cellSize, 1e-3 );
	float dhdz = (AltitudeMeters( uvy1 ) - AltitudeMeters( uvy0 )) / max( (uvy1.y - uvy0.y) * res.y * cellSize, 1e-3 );

	float3 normal = normalize( float3( -dhdx, 1.0, -dhdz ) );

	//Fake Erosion V2 masks

	float erosion = _erosionMap.SampleLevel( _bilinearClampSampler, uv, 0.0 ) * 2.0 - 1.0;	//[-1, 1]
	float ridgemap01 = _ridgeMap.SampleLevel( _bilinearClampSampler, uv, 0.0 );			//[0, 1]
	float occlusion = clamp01( erosion + 0.5 );

	float breakup = Breakup( uv * BreakupScale ) * BreakupAmount;

	//Shadertoy heights

	float height = ShaderHeight( altitude01 );
	float waterHeight = ShaderHeight( WaterLevel );
	float grassHeight = ShaderHeight( GrassLevel );

	//Trees (Buffer A): coverage in [0, 1] (0 when disabled, like the Shadertoy's packed value)

	float trees = 0.0;

	if( EnableTrees )
	{
		float treesAmount = GetTreesAmount( height, normal.y, erosion + 0.5, ridgemap01 * 2.0 - 1.0, waterHeight, grassHeight );
		float treeNoise = fe_noised( (uv * BreakupScale + 0.5) * 200.0 ).x * 0.5 + 0.5;
		float treesRaw = (1.0 - treeNoise * treeNoise - 1.0 + 1.0 * treesAmount) * 1.5; //pow( treeNoise, 2.0 ) in the Shadertoy
		trees = clamp01( treesRaw * 0.5 + 0.5 );
	}

	//Albedo (Image tab, M_GROUND and M_WATER materials)

	float3 diffuseColor;
	bool bWater = EnableWater && (height < waterHeight);

	if( !bWater )
	{
		// Cliffs / Dirt
		diffuseColor = CliffColor.rgb * smoothstep( ShaderHeight( CliffStart ), ShaderHeight( CliffEnd ), height );
		diffuseColor = lerp( diffuseColor, DirtColor.rgb, smoothstep( 0.6, 0.0, occlusion + breakup * 1.5 ) );

		// Snow
		diffuseColor = lerp( diffuseColor, float3( 1.0, 1.0, 1.0 ), smoothstep( ShaderHeight( SnowStart ), ShaderHeight( SnowEnd ), height + breakup * 0.1 ) );

		// Sand (beach)
		if( EnableWater )
			diffuseColor = lerp( diffuseColor, SandColor.rgb, smoothstep( waterHeight + 0.005, waterHeight, height + breakup * 0.01 ) );

		// Grass
		float3 grassMix = lerp( GrassColor1.rgb, GrassColor2.rgb, smoothstep( 0.4, 0.6, height - erosion * 0.05 + breakup * 0.3 ) );
		diffuseColor = lerp( diffuseColor, grassMix,
			smoothstep( grassHeight + 0.05, grassHeight + 0.02, height + 0.01 + (occlusion - 0.8) * 0.05 - breakup * 0.02 )
			* smoothstep( 0.8, 1.0, 1.0 - (1.0 - normal.y) * (1.0 - trees) + breakup * 0.1 ) );

		// Trees
		diffuseColor = lerp( diffuseColor, TreeColor.rgb * pow( trees, 8.0 ), clamp01( trees * 2.2 - 0.8 ) * 0.6 );

		diffuseColor *= 1.0 + breakup * 0.5;

		// Drainage (rivers, creeks, debris flow)
		if( EnableDrainage )
		{
			float drainage = clamp01( (1.0 - clamp01( ridgemap01 / DrainageWidth )) * 1.5 );
			diffuseColor = lerp( diffuseColor, float3( 1.0, 1.0, 1.0 ), drainage );
		}
	}
	else
	{
		float depth = waterHeight - height;
		float shore = exp( -depth * 60.0 );
		float foam = smoothstep( 0.005, 0.0, depth + breakup * 0.005 );

		diffuseColor = lerp( WaterColor.rgb, WaterShoreColor.rgb, shore );
		diffuseColor = lerp( diffuseColor, float3( 1.0, 1.0, 1.0 ), foam );

		normal = float3( 0.0, 1.0, 0.0 );
		occlusion = 1.0;
		altitude = lerp( MinAltitude, MaxAltitude, WaterLevel );
	}

	//Lighting

	float3 sun = float3( SunDirX, SunDirY, SunDirZ );
	float shadow = EnableShadows ? SunShadow( uv, altitude + 0.1, sun ) : 1.0;
	float3 lighting = Lighting( normal, sun, occlusion, shadow ) * Exposure;

	_outAlbedo[ _pos ] = float4( GammaEncode( diffuseColor ), 1.0 );
	_outLighting[ _pos ] = float4( GammaEncode( lighting ), 1.0 );
	_outTrees[ _pos ] = trees;
}
