//shared between C++ (MountainColoringNode.h) & HLSL (MountainColoring.hlsl) sides
//The float4 colors come first so that they stay 16 bytes aligned on both sides (52 x 4 bytes)

cbuffer MountainColoringConstants
{
	//Linear RGB colors (w unused)
	float4 CliffColor;
	float4 DirtColor;
	float4 TreeColor;
	float4 GrassColor1;
	float4 GrassColor2;
	float4 SandColor;
	float4 WaterColor;
	float4 WaterShoreColor;

	float TerrainExtent;	//meters
	float MinAltitude;		//meters
	float MaxAltitude;		//meters
	float Exposure;

	//Levels in normalized altitude [0, 1]
	float WaterLevel;
	float GrassLevel;
	float CliffStart;
	float CliffEnd;

	float SnowStart;
	float SnowEnd;
	float DrainageWidth;
	float BreakupAmount;

	float BreakupScale;
	float SunDirX;			//normalized direction towards the sun (y up)
	float SunDirY;
	float SunDirZ;

	int EnableWater;
	int EnableDrainage;
	int EnableTrees;
	int EnableShadows;
};
