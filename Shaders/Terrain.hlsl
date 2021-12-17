Texture2D<float> g_HeightMap : register(t0);
Texture2D<float3> g_NormalMap : register(t1);
Texture2D<float3> g_AlbedoMap : register(t2);
Texture2D<float> g_WaterMap : register(t3);
Texture2D<float> g_FlowMap : register(t4);

SamplerState g_BilinearSampler : register(s0);

cbuffer Constants
{
	float4x4 c_WorldViewMatrix;
	float4x4 c_WorldViewProjMatrix;
	float c_TerrainExtent;
	uint c_TerrainResolution;
	float c_MinAltitude;
	float c_MaxAltitude;
	
	//float pad;
};

//////////////////////////////////////////////

struct Vertex
{
	float2 Pos : POSITION;
};

struct VS2PS
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 wsPos : WSPOS;
	float3 wsNormal : NORMAL;
	float LinearDepth : LINEARDEPTH;
};


VS2PS VSMain( Vertex IN )
{
	VS2PS OUT;

	float altitude = g_HeightMap.SampleLevel( g_BilinearSampler, IN.Pos, 0.0f );
	altitude = lerp( c_MinAltitude, c_MaxAltitude, altitude );
	float3 normal = float3(0, 1, 0);

	float4 osPos = float4( ( IN.Pos.x  - 0.5f ) * c_TerrainExtent, altitude, ( IN.Pos.y - 0.5f ) * c_TerrainExtent, 1.0f );

	OUT.Position = mul( osPos, c_WorldViewProjMatrix );
	OUT.TexCoord = float2( IN.Pos.x, IN.Pos.y );
	OUT.wsPos = osPos.xyz;
	OUT.wsNormal = normal;
	OUT.LinearDepth = mul( osPos, c_WorldViewMatrix ).z;
	return OUT;
}

float4 PSMain( VS2PS IN ) : SV_Target0
{
	float4 output;

	float3 wsNormal = g_NormalMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );
	float3 albedo = g_AlbedoMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );
	
	const float3 c_wsLightDir = float3(0.7f, -0.7f, 0.7f);
	float NdotL = max( dot( wsNormal, -c_wsLightDir ), 0.0f );

	float3 sunLight = float3(0.8f, 0.8f, 0.5f) * NdotL;
	float3 skyLight = float3(0.2f, 0.2f, 0.5f) * wsNormal.y; //dead simple skylight

	output.rgb = albedo * (sunLight + skyLight);

	output.a = 1.0f;
	return output;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VS2PS VSMainRGB( Vertex IN )
{
	VS2PS OUT;

	float altitude = 0.0f;

	float4 osPos = float4((IN.Pos.x - 0.5f) * c_TerrainExtent, altitude, (IN.Pos.y - 0.5f) * c_TerrainExtent, 1.0f);

	OUT.Position = mul( osPos, c_WorldViewProjMatrix );
	OUT.TexCoord = float2(IN.Pos.x, IN.Pos.y);
	OUT.wsPos = osPos.xyz;
	OUT.wsNormal = float3(0,1,0);
	OUT.LinearDepth = mul( osPos, c_WorldViewMatrix ).z;
	return OUT;
}

float4 PSMainRGB( VS2PS IN ) : SV_Target0
{
	float3 albedo = g_AlbedoMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );

	return float4(albedo, 1.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////


float TerrainPlusWaterHeight( float2 p )
{
	float terrain = g_HeightMap.SampleLevel( g_BilinearSampler, p, 0.0f );
	terrain = lerp( c_MinAltitude, c_MaxAltitude, terrain );

	return terrain + g_WaterMap.SampleLevel( g_BilinearSampler, p, 0.0f );
}


VS2PS VSMainErosion( Vertex IN )
{
	VS2PS OUT;

	float altitude = TerrainPlusWaterHeight( IN.Pos );

	float3 normal = float3(0, 1, 0);

	float4 osPos = float4((IN.Pos.x - 0.5f) * c_TerrainExtent, altitude, (IN.Pos.y - 0.5f) * c_TerrainExtent, 1.0f);

	OUT.Position = mul( osPos, c_WorldViewProjMatrix );
	OUT.TexCoord = float2(IN.Pos.x, IN.Pos.y);
	OUT.wsPos = osPos.xyz;
	OUT.wsNormal = normal;
	OUT.LinearDepth = mul( osPos, c_WorldViewMatrix ).z;
	return OUT;
}

float4 PSMainErosion( VS2PS IN ) : SV_Target0
{
	float4 output;

//	float altitude = g_HeightMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );
	
	float water = g_WaterMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );
	water = saturate( water * 10.0f );

	//Compute normal

	float2 uv = IN.TexCoord;
	const float epsilon = 1.0f / c_TerrainResolution;
	float3 wsNormal = float3(	TerrainPlusWaterHeight( float2(uv.x + epsilon, uv.y) ) - TerrainPlusWaterHeight( float2(uv.x - epsilon, uv.y) ),
								2.0f * epsilon * c_TerrainExtent,
								TerrainPlusWaterHeight( float2(uv.x, uv.y + epsilon) ) - TerrainPlusWaterHeight( float2(uv.x, uv.y - epsilon) ) );

	wsNormal = normalize( wsNormal );

	//Compute lighting

	const float3 c_wsLightDir = float3(0.7f, -0.7f, 0.7f);
	float NdotL = dot( wsNormal, -c_wsLightDir );
	NdotL = max( NdotL, 0.0f );

	float3 albedo = lerp( float3(1.0f, 0.7f, 0.2f), float3(0.3f, 0.3f, 0.95f), water );

	albedo.r += g_FlowMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f ) * 1.0f;
	
	output.rgb = albedo * NdotL;
	output.a = 1.0f;

	return output;
}

/////////////////////////////////////////////////////////////

float TerrainHeight( float2 p )
{
	float terrain = g_HeightMap.SampleLevel( g_BilinearSampler, p, 0.0f );
	terrain = lerp( c_MinAltitude, c_MaxAltitude, terrain );

	return terrain;
}

VS2PS VSMainErosion2( Vertex IN )
{
	VS2PS OUT;

	float altitude = TerrainHeight( IN.Pos );

	float3 normal = float3(0, 1, 0);

	float4 osPos = float4((IN.Pos.x - 0.5f) * c_TerrainExtent, altitude, (IN.Pos.y - 0.5f) * c_TerrainExtent, 1.0f);

	OUT.Position = mul( osPos, c_WorldViewProjMatrix );
	OUT.TexCoord = float2(IN.Pos.x, IN.Pos.y);
	OUT.wsPos = osPos.xyz;
	OUT.wsNormal = normal;
	OUT.LinearDepth = mul( osPos, c_WorldViewMatrix ).z;
	return OUT;
}

float4 PSMainErosion2( VS2PS IN ) : SV_Target0
{
	float4 output;

	//	float altitude = g_HeightMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );

	float flow = g_FlowMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );
	flow = saturate( flow * 0.001f );

	//Compute normal

	float2 uv = IN.TexCoord;
	const float epsilon = 1.0f / c_TerrainResolution;
	float3 wsNormal = float3(TerrainHeight( float2(uv.x + epsilon, uv.y) ) - TerrainHeight( float2(uv.x - epsilon, uv.y) ),
		2.0f * epsilon * c_TerrainExtent,
		TerrainHeight( float2(uv.x, uv.y + epsilon) ) - TerrainHeight( float2(uv.x, uv.y - epsilon) ));

	wsNormal = normalize( wsNormal );

	//Compute lighting

	const float3 c_wsLightDir = float3(0.7f, -0.7f, 0.7f);
	float NdotL = dot( wsNormal, -c_wsLightDir );
	NdotL = max( NdotL, 0.0f );

	float3 albedo = lerp( float3(1.0f, 0.7f, 0.2f), float3(0.3f, 0.3f, 0.95f), flow );

	albedo += g_AlbedoMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f ).x * 0.0001f;
	albedo = saturate( albedo);

	output.rgb = albedo * NdotL;
	output.a = 1.0f;

	return saturate( output );
}

float TerrainPlusSnowHeight( float2 p )
{
	float terrain = g_HeightMap.SampleLevel( g_BilinearSampler, p, 0.0f );
	terrain = lerp( c_MinAltitude, c_MaxAltitude, terrain );

	float snow = g_WaterMap.SampleLevel( g_BilinearSampler, p, 0.0f );
	
	return terrain + snow;
}

VS2PS VSMainSnow( Vertex IN )
{
	VS2PS OUT;

	float altitude = TerrainPlusSnowHeight( IN.Pos );

	float3 normal = float3(0, 1, 0);

	float4 osPos = float4((IN.Pos.x - 0.5f) * c_TerrainExtent, altitude, (IN.Pos.y - 0.5f) * c_TerrainExtent, 1.0f);

	OUT.Position = mul( osPos, c_WorldViewProjMatrix );
	OUT.TexCoord = float2(IN.Pos.x, IN.Pos.y);
	OUT.wsPos = osPos.xyz;
	OUT.wsNormal = normal;
	OUT.LinearDepth = mul( osPos, c_WorldViewMatrix ).z;
	return OUT;
}

float4 PSMainSnow( VS2PS IN ) : SV_Target0
{
	float4 output;

	//	float altitude = g_HeightMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );

	float snow = g_WaterMap.SampleLevel( g_BilinearSampler, IN.TexCoord, 0.0f );
	snow = saturate( (snow - 0.1f) * 5.0f );

	//Compute normal

	float2 uv = IN.TexCoord;
	const float epsilon = 1.0f / c_TerrainResolution;
	float3 wsNormal = float3(TerrainPlusSnowHeight( float2(uv.x + epsilon, uv.y) ) - TerrainPlusSnowHeight( float2(uv.x - epsilon, uv.y) ),
	2.0f * epsilon * c_TerrainExtent,
		TerrainPlusSnowHeight( float2(uv.x, uv.y + epsilon) ) - TerrainPlusSnowHeight( float2(uv.x, uv.y - epsilon) ));

	wsNormal = normalize( wsNormal );

	//Compute lighting

	const float3 c_wsLightDir = float3(0.7f, -0.7f, 0.7f);
	float NdotL = dot( wsNormal, -c_wsLightDir );
	NdotL = max( NdotL, 0.0f );

	float3 albedo = lerp( float3(0.25f, 0.25f, 0.28f), float3(0.95f, 0.95f, 0.95f), snow );

	output.rgb = albedo * NdotL;
	output.a = 1.0f;

	return output;
}
