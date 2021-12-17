//shared between C++ & HLSL sides

cbuffer ErosionConstants
{
	float TerrainExtent;
	int	  TerrainResolution;
	float CellSizeX, CellSizeZ; // extent/(resolution-1)
	float MinAltitude;
	float MaxAltitude;

	float RainRate, EvaporationRate, DissolutionRate, DepositionRate,
		SedimentCapacity, dt;

	float Tan_TalusAngle;
	float ThermalErosionSpeed;
	//float Smoothing;
	float pad1, pad2;
};
