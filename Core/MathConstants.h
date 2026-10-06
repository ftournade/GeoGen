#pragma once

	//Unless specified otherwise, all constants are in S.I. units ( meter, second, kilogram, liter ) etc

	//Base mathematic constants

	const double c_Pi = 3.1415926535897932384626433832795028841968;

	//Optics

	const double c_AirRefractionIndex = 1.000293;
	const double c_WaterRefractionIndex = 1.333;

	const double c_DepolarisationFactor = 0.0035; //For standard air

	//Earth atmosphere
	
	const double c_AirMolecularDensityAtSeaLevel = 2.545e+25;
		
	const double c_RedWaveLength = 700.0e-9; 		//680.0e-9f;
	const double c_GreenWaveLength = 530.0e-9;	//550.0e-9f;
	const double c_BlueWaveLength = 400.0e-9;		//440.0e-9f;


	//Celestial

	const double c_EarthRadius = 6360000.0;
	const double c_SunRadius = 6462650000.0; //m
	const double c_SunEarthDistance = 149597900000.0; //m


