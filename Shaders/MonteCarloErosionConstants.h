
cbuffer Constants
{
	//General
	int Resolution;
	float MinAltitude;
	float MaxAltitude;
	float TerrainExtent;
	float CellSize;
	float Seed;

	//Hydraulic erosion
	float RainRate;
	float EvaporationRate;
	float ErosionRate;
	float DepositionRate;
	float ErosionSlopeThreshold;
	float SedimentCapacity;

	//Gravity (snow, sand, rocks)
	float TalusAngle;
	float Speed;

	//Snow
	float SnowFall;
	float SnowEvaporation;

	float Smooth;

	float pad[3];
};
