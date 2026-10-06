//shared between C++ (FakeErosionV2Node.h) & HLSL (FakeErosionV2.hlsl) sides
//Scalars only, so the C++ struct and the HLSL constant buffer have the same layout (32 x 4 bytes)

cbuffer FakeErosionV2Constants
{
	float TerrainExtent;		//meters
	float MinAltitude;			//meters
	float MaxAltitude;			//meters
	float Scale;				//km

	float Strength;
	float GullyWeight;
	float Detail;
	int   Octaves;

	float Lacunarity;
	float Gain;
	float CellScale;
	float Normalization;

	float RidgeRounding;		//rounding.x
	float CreaseRounding;		//rounding.y
	float InputRounding;		//rounding.z
	float OctaveRoundingMult;	//rounding.w

	float Onset;				//onset.x
	float OctaveOnset;			//onset.y
	float RidgeMapOnset;		//onset.z
	float RidgeMapOctaveOnset;	//onset.w

	float AssumedSlope;
	float AssumedSlopeAmount;
	float HeightOffset;
	float PreserveExtremes;

	float FadeCenter;
	float FadeRange;
	float DrainageWidth;
	int   ClampHeight;

	float SlopeRadius;			//km, distance used to measure the input slope (at least one texel)
	float pad0, pad1, pad2;
};
