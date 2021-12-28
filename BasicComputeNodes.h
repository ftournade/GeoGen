#pragma once

#include "CustomComputeNode.h"

class PerlinNoiseNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( PerlinNoiseNode )

	PerlinNoiseNode() : CustomComputeNode( 0, 1 )
	{
		m_ResolutionReference = Res_GlobalSetting;

		SetUIName( "Perlin noise" );

		AddInput( "DistortionX", IOType::Float, true );
		AddInput( "DistortionY", IOType::Float, true );

		int iParam = AddParam( "Main", "Type", IOType::Integer, ParamEdition::ComboBox, 1, 0, 2, true );
		ParamSlot* pParam = &m_ParameterSlots[ iParam ];
		pParam->AddEnum( "Fractal" );
		pParam->AddEnum( "Ridged" );
		pParam->AddEnum( "MultiFractal" );
		//Billowy
		//TODO noise generators https://engineering.purdue.edu/~ebertd/texture/1stEdition/musgrave/musgrave.c

		AddParam( "Main", "Frequency", IOType::Float, ParamEdition::Slider, 0.2f, 0.0001f, 3.0f, false );
		AddParam( "Main", "Lacunarity", IOType::Float, ParamEdition::Slider, 2.0f, 0.0f, 4.0f, false );
		AddParam( "Main", "Persistence", IOType::Float, ParamEdition::Slider, 0.4f, 0.0f, 1.0f, false );
		AddParam( "Main", "Shape", IOType::Float, ParamEdition::Slider, 0.7f, 0.05f, 8.0f, false );
		AddParam( "Main", "Octaves", IOType::Integer, ParamEdition::Slider, 11, 1, 30, true );
		AddParam( "Main", "Distortion", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 10.0f, false );
		AddParam( "Main", "Scale", IOType::Float, ParamEdition::Slider, 1.0f, -4.0f, 4.0f, false );
		AddParam( "Main", "Bias", IOType::Float, ParamEdition::Slider, 0.0f, -4.0f, 4.0f, false );
		AddParam( "Main", "Clamp", IOType::Bool, ParamEdition::CheckBox, true, true );

		SetHLSLPrefix( "#include \"Shaders/Noise.h\"" );
		SetHLSLBody(
			"float2 p = _wsPos * (Frequency * 0.001f);\n"
			"if( Distortion > 0.0f )\n"
			"{\n"
			"	p.x += (_input0[ _pos ] * 2.0f - 1.0f) * Distortion;\n"
			"	p.y += (_input1[ _pos ] * 2.0f - 1.0f) * Distortion;\n"
			"}\n"
			"float noise;\n"
			"switch( Type )\n"
			"{\n"
			"case 0: noise = FractalNoise2D( p, Octaves, Lacunarity, Persistence, Shape, NOISE_HASH_INIGO_QUILEZ, HIGH_QUALITY_NOISE_INTERP ); break;\n"
			"case 1: noise = RidgedNoise2D( p, Octaves, Lacunarity, Persistence, Shape, NOISE_HASH_INIGO_QUILEZ, HIGH_QUALITY_NOISE_INTERP ); break;\n"
			"case 2: noise = MultiFractalNoise2D( p, Octaves, Lacunarity, Persistence, Shape, NOISE_HASH_INIGO_QUILEZ, HIGH_QUALITY_NOISE_INTERP ); break;\n"
			"}\n"
			"noise = noise * 0.5f + 0.5f;\n"
			"noise = noise * Scale + Bias;\n"
			"if( Clamp ) noise = saturate( noise );\n"
			"_output0[ _pos ] = noise;\n"
		);
	}

	virtual ~PerlinNoiseNode() {}
};

class VoronoiseNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( VoronoiseNode )

	VoronoiseNode() : CustomComputeNode( 0, 1 )
	{
		m_ResolutionReference = Res_GlobalSetting;

		SetUIName( "Voronoise" );

		AddInput( "DistortionX", IOType::Float, true );
		AddInput( "DistortionY", IOType::Float, true );

		AddParam( "Main", "Frequency", IOType::Float, ParamEdition::Slider, 0.5f, 0.001f, 4.0f, false );
		AddParam( "Main", "Chaos", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f, false );
		AddParam( "Main", "Smoothness", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f, false );
		AddParam( "Main", "Distortion", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 10.0f, false );
		AddParam( "Main", "Scale", IOType::Float, ParamEdition::Slider, 1.0f, -4.0f, 4.0f, false );
		AddParam( "Main", "Bias", IOType::Float, ParamEdition::Slider, 0.0f, -4.0f, 4.0f, false );
		AddParam( "Main", "Clamp", IOType::Bool, ParamEdition::CheckBox, true, true );

		SetHLSLPrefix( "#include \"Shaders/Noise.h\"" );

		SetHLSLBody(	"float2 p = _wsPos * (Frequency * 0.001f);\n"
						"if( Distortion > 0.0f )\n"
						"{\n"
						"	p.x += (_input0[ _pos ] * 2.0f - 1.0f) * Distortion;\n"
						"	p.y += (_input1[ _pos ] * 2.0f - 1.0f) * Distortion;\n"
						"}\n"
						"float noise = Voronoise( p, Chaos, Smoothness ); \n"
						"noise = noise * Scale + Bias;\n"
						"if( Clamp ) noise = saturate( noise );\n"
						"_output0[ _pos ] = noise;\n" );

	}

	virtual ~VoronoiseNode() {}
};


class VoronoiNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( VoronoiNode )

	VoronoiNode() : CustomComputeNode( 0, 2 )
	{
		m_ResolutionReference = Res_GlobalSetting;

		SetUIName( "Voronoi" );

		AddInput( "DistortionX", IOType::Float, true );
		AddInput( "DistortionY", IOType::Float, true );

		int iParam = AddParam( "Main", "Type", IOType::Integer, ParamEdition::ComboBox, 0, 0, 2, true );
		ParamSlot* pParam = &m_ParameterSlots[ iParam ];
		pParam->AddEnum( "F1" );
		pParam->AddEnum( "F2 - F1 Accurate" );
		pParam->AddEnum( "F1 Fractal" );

		AddParam( "Main", "Frequency", IOType::Float, ParamEdition::Slider, 0.5f, 0.001f, 4.0f, false );
		AddParam( "Main", "Chaos", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f, false );
		AddParam( "Main", "Distortion", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 10.0f, false );
		AddParam( "Main", "Shape", IOType::Float, ParamEdition::Slider, 0.7f, 0.001f, 4.0f, false );
		AddParam( "Main", "Scale", IOType::Float, ParamEdition::Slider, 1.0f, -4.0f, 4.0f, false );
		AddParam( "Main", "Bias", IOType::Float, ParamEdition::Slider, 0.0f, -4.0f, 4.0f, false );
		AddParam( "Fractal", "Octaves", IOType::Integer, ParamEdition::Slider, 11, 1, 30, true );
		AddParam( "Fractal", "Lacunarity", IOType::Float, ParamEdition::Slider, 2.0f, 0.0f, 4.0f, false );
		AddParam( "Fractal", "Persistence", IOType::Float, ParamEdition::Slider, 0.4f, 0.0f, 1.0f, false );
		AddParam( "Main", "Clamp", IOType::Bool, ParamEdition::CheckBox, true, true );

		SetHLSLPrefix( "#include \"Shaders/Noise.h\"" );

		SetHLSLBody(	"float2 p = _wsPos * (Frequency * 0.001f);\n"
						"if( Distortion > 0.0f )\n"
						"{\n"
						 "	p.x += (_input0[ _pos ] * 2.0f - 1.0f) * Distortion;\n"
						 "	p.y += (_input1[ _pos ] * 2.0f - 1.0f) * Distortion;\n"
						"}\n"

						"float2 voronoi;\n"
						"switch( Type )\n"
						"{\n"
						"case 0:\n"
						"	voronoi = Voronoi2D( p, Chaos );\n"
						"	voronoi.y *= 0.02f;\n"
						"	voronoi = pow( voronoi, Shape );\n"
						"	break;\n"
						"case 1:\n"
						"	voronoi = AccurateVoronoi2D( p, Chaos );\n"
						"	voronoi = pow( voronoi, Shape );\n"
						"	break;\n"
						"case 2:\n"
						"	voronoi.x = FractalVoronoi2D( p, Octaves, Lacunarity, Persistence, Chaos, Shape );\n"
						"	voronoi.y = 0.0f;\n"
						"	break;\n"
						"}\n"
						
						"float v = voronoi.x * Scale + Bias;\n"
						"if( Clamp ) v = saturate( v );\n"
						"_output0[ _pos ] = v;\n"
						"_output1[ _pos ] = voronoi.y;\n" );

	}

	virtual ~VoronoiNode() {}
};


class CheckerboardNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( CheckerboardNode )

	CheckerboardNode() : CustomComputeNode( 0, 1 )
	{
		m_ResolutionReference = Res_GlobalSetting;

		SetUIName( "CheckerBoard" );

		AddInput( "DistortionX", IOType::Float, true );
		AddInput( "DistortionY", IOType::Float, true );

		AddParam( "Main", "Frequency", IOType::Float, ParamEdition::Slider, 1.0f, 0.0001f, 4.0f, false );
		AddParam( "Main", "Distortion", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 10.0f, false );
		AddParam( "Main", "Scale", IOType::Float, ParamEdition::Slider, 1.0f, -4.0f, 4.0f, false );
		AddParam( "Main", "Bias", IOType::Float, ParamEdition::Slider, 0.0f, -4.0f, 4.0f, false );
		AddParam( "Main", "Clamp", IOType::Bool, ParamEdition::CheckBox, true, true );

#if 1
		SetHLSLBody(
			"float2 p = _wsPos + Extent * 0.5f;\n"
			"p = p * (Frequency * 0.001f);\n"
			"if( Distortion > 0.0f )\n"
			"{\n"
			"	p.x += (_input0[ _pos ] * 2.0f - 1.0f) * Distortion;\n"
			"	p.y += (_input1[ _pos ] * 2.0f - 1.0f) * Distortion;\n"
			"}\n"

			"int2 ip = (int2)p;\n"

			"int c = (ip.x % 2) ^ (ip.y % 2);\n"
			"float v = (float)c * Scale + Bias;\n"
			"if( Clamp ) v = saturate( v );\n"
			"_output0[ _pos ] = v;\n" );
#else
		SetHLSLBody(
			"int c = (_pos.x % 2) ^ (_pos.y % 2);\n"
			"float v = (float)c * Scale + Bias;\n"
			"if( Clamp ) v = saturate( v );\n"
			"_output0[ _pos ] = v;\n" );
#endif
	}

	virtual ~CheckerboardNode() {}
};




class DistortNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( DistortNode )

		DistortNode()
	{
		m_ResolutionReference = Res_MainInput;

		SetUIName( "Distort" );

		AddInput( "Input", IOType::FloatOrColor );
		AddInput( "DistortX", IOType::FloatOrColor );
		AddInput( "DistortY", IOType::FloatOrColor );
		AddOutput( "Output", IOType::FloatOrColor );

		int iParam = AddParam( "Main", "BorderMode", IOType::Integer, ParamEdition::ComboBox, 1, 0, 1, true );
		ParamSlot* pParam = &m_ParameterSlots[ iParam ];
		pParam->AddEnum( "Clamp" );
		pParam->AddEnum( "Mirror" );

		AddParam( "Main", "XScale", IOType::Float, ParamEdition::Slider, 1.0f, -2.0f, 2.0f, false );
		AddParam( "Main", "YScale", IOType::Float, ParamEdition::Slider, 1.0f, -2.0f, 2.0f, false );

		SetHLSLBody( "float2 disto;\n"
			"disto.x = _input1.SampleLevel( _bilinearClampSampler, _uv, 0.0f );\n"
			"disto.y = _input2.SampleLevel( _bilinearClampSampler, _uv, 0.0f );\n"

			"float2 uv = _uv + disto * float2( XScale, YScale );\n"
			
			"switch( BorderMode )\n"
			"{\n"
			"	case 0:\n"//Clamp
			"		_output0[ _pos ] = _input0.SampleLevel( _bilinearClampSampler, uv, 0.0f );\n"
			"		break;\n"
			"	case 1:\n" //Mirror
			"		_output0[ _pos ] = _input0.SampleLevel( _bilinearMirrorSampler, uv, 0.0f );\n"
			"		break;\n"
			"}\n" );


	}

	virtual ~DistortNode() {}
};

class DirectionalBlurNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( DirectionalBlurNode )

	DirectionalBlurNode()
	{
		AddInput( "Input", IOType::FloatOrColor );
		AddOutput( "Output", IOType::FloatOrColor );

		SetUIName( "Directional Blur" );

		AddParam( "Main", "Direction", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 360.0f, false );
		AddParam( "Main", "Distance",  IOType::Float, ParamEdition::Slider, 0.1f, 0.001f, 1.0f, false );

		SetHLSLBody(	"float2 dir;\n"
						"sincos( Direction * 3.14159f / 180.0f, dir.x, dir.y );\n"
			
						"dir /= Resolution;\n"
			
						"int numTaps = Distance / length( dir );\n"
			
						"float2 uv = _uv;\n"
						"float4 sum = float4( 0.0f, 0.0f, 0.0f, 0.0f );\n"
			
						"for( int i = 0; i < numTaps ; ++i )\n"
						"{\n"
						"	sum += _input0.SampleLevel( _bilinearClampSampler, uv, 0.0f );\n"
						"	uv += dir;\n"
						"}\n"
			
						"_output0[ _pos ] = sum / numTaps;\n" );
	}

	virtual ~DirectionalBlurNode() {}

	virtual void OnInputConnectionChanged( int _slot )
	{
		const Map* input1 = GetRemoteInputMap( 0 );

		if( !input1 )
			return;

		if( input1->GetFormat() == DXGI_FORMAT_R32_FLOAT )
			SetOutputFormat( 0, DXGI_FORMAT_R32_FLOAT );
		else
			SetOutputFormat( 0, DXGI_FORMAT_R16G16B16A16_FLOAT );

		InvalidateShader();
	}

	//virtual void OnOutputConnectionChanged( int _slot ) {}

};

class CombineNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( CombineNode )

	CombineNode()
	{
		AddInput( "A", IOType::FloatOrColor );
		AddInput( "B", IOType::FloatOrColor );
		AddOutput( "Output", IOType::FloatOrColor );

		SetUIName( "Combine" );

		int iParam = AddParam( "Main", "Type", IOType::Integer, ParamEdition::ComboBox, 0, 0, 7, true );
		ParamSlot* pParam = &m_ParameterSlots[ iParam ];
		pParam->AddEnum( "Add" );
		pParam->AddEnum( "Sub" );
		pParam->AddEnum( "Mul" );
		pParam->AddEnum( "Div" );
		pParam->AddEnum( "Min" );
		pParam->AddEnum( "Max" );
		pParam->AddEnum( "Avg" );
		pParam->AddEnum( "Pow" );

		AddParam( "Main", "Clamp", IOType::Bool, ParamEdition::CheckBox, true, true );

		SetHLSLBody( "float4 output = float4( 0.0f, 0.0f, 0.0f, 0.0f );\n"
			"switch( Type )\n{\n"
			"case 0: output = _input0[ _pos ] + _input1[ _pos ]; break;\n"
			"case 1: output = _input0[ _pos ] - _input1[ _pos ]; break;\n"
			"case 2: output = _input0[ _pos ] * _input1[ _pos ]; break;\n"
			"case 3: output = _input0[ _pos ] / _input1[ _pos ]; break;\n"
			"case 4: output = min( _input0[ _pos ], _input1[ _pos ] ); break;\n"
			"case 5: output = max( _input0[ _pos ], _input1[ _pos ] ); break;\n"
			"case 6: output = 0.5f * (_input0[ _pos ] + _input1[ _pos ]); break;\n"
			"case 7: output = pow(_input0[ _pos ], _input1[ _pos ]); break;\n"
			"}\n"
			"if( Clamp ) { output = saturate( output ); }\n"
			"_output0[ _pos ] = output;\n" );
	}

	virtual ~CombineNode() {}

	virtual void OnInputConnectionChanged( int _slot )
	{
		const Map* input1 = GetRemoteInputMap( 0 );
		const Map* input2 = GetRemoteInputMap( 1 );

		if( !input1 || !input2 )
			return;

		if( (input1->GetFormat() == DXGI_FORMAT_R32_FLOAT)
		 && (input2->GetFormat() == DXGI_FORMAT_R32_FLOAT) )
			SetOutputFormat( 0, DXGI_FORMAT_R32_FLOAT );
		else
			SetOutputFormat( 0, DXGI_FORMAT_R16G16B16A16_FLOAT );

		InvalidateShader();
	}

	//virtual void OnOutputConnectionChanged( int _slot ) {}

};

class AbsNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( AbsNode )

	AbsNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Absolute" );

		SetHLSLBody( "_output0[ _pos ] = abs( _input0[ _pos ] );\n" );
	}

	virtual ~AbsNode() {}
};

class PeriodicNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( PeriodicNode )

		PeriodicNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Periodic" );

		int iParam = AddParam( "Main", "Type", IOType::Integer, ParamEdition::ComboBox, 0, 0, 3, true );
		ParamSlot* pParam = &m_ParameterSlots[ iParam ];
		pParam->AddEnum( "Sine" );
		pParam->AddEnum( "Square" );
		pParam->AddEnum( "Sawtooth" );
		pParam->AddEnum( "Triangle" );

		AddParam( "Main", "Frequency", IOType::Float, ParamEdition::Slider, 10.000f, 0.000001f, 500.0f, false );
		AddParam( "Main", "Phase", IOType::Float, ParamEdition::Slider, 0.000f, 0.000001f, 2.0f * 3.14159f, false );

		SetHLSLBody(
			"float x = _input0[ _pos ] * Frequency + Phase;\n"
			"switch( Type )\n"
			"{\n"
			"	case 0: x = sin( x * 3.14159f ); break;\n"
			"	case 1: x = fmod( x, 2.0f ) < 1.0f ? 0.0f : 1.0f; break;\n"
			"	case 2: x = frac( x ); break;\n"
			"	case 3: x = abs( frac( x ) - 0.5f ) * 2.0f;	break;\n"
			"}\n"
			"_output0[ _pos ] = x;\n" );
	}

	virtual ~PeriodicNode() {}
};

class PowNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( PowNode )

	PowNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Power" );
		AddParam( "Main", "Power", IOType::Float, ParamEdition::Slider, 2.000f, 0.000001f, 10.0f, false );

		SetHLSLBody( "_output0[ _pos ] = pow( _input0[ _pos ], Power );\n" );
	}

	virtual ~PowNode() {}
};

class MixNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( MixNode )

	MixNode()
	{
		SetUIName( "Mix" );

		AddInput( "A", IOType::FloatOrColor, false );
		AddInput( "B", IOType::FloatOrColor, false );
		AddInput( "Mix", IOType::Float, false );

		AddOutput( "FloatMap", IOType::FloatOrColor );

		SetHLSLBody( "_output0[ _pos ] = lerp( _input0[ _pos ], _input1[ _pos ], saturate( _input2[ _pos ] ) );\n" );
	}


	virtual void OnInputConnectionChanged( int _slot )
	{
		const Map* input1 = GetRemoteInputMap( 0 );
		const Map* input2 = GetRemoteInputMap( 1 );

		if( !input1 || !input2 )
			return;

		if( (input1->GetFormat() == DXGI_FORMAT_R32_FLOAT) && (input2->GetFormat() == DXGI_FORMAT_R32_FLOAT) )
			SetOutputFormat( 0, DXGI_FORMAT_R32_FLOAT );
		else
			SetOutputFormat( 0, DXGI_FORMAT_R16G16B16A16_FLOAT );

		InvalidateShader();
	}

	virtual ~MixNode() {}
};
/*
class RGBMixNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( RGBMixNode )

	RGBMixNode()
	{
		SetUIName( "RGBMix" );

		AddInput( "A", IOType::Color, false );
		AddInput( "B", IOType::Color, false );
		AddInput( "Mix", IOType::Float, false );

		AddOutput( "ColorMap", IOType::Color );

		SetHLSLBody( "_output0[ _pos ] = lerp( _input0[ _pos ], _input1[ _pos ], saturate( _input2[ _pos ] ) );\n" );
	}

	virtual ~RGBMixNode() {}
};
*/
class ReRangeNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ReRangeNode )

	ReRangeNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Re-Range" );

		AddParam( "Main", "Min", IOType::Float, ParamEdition::Slider, 0.000f, -1.0f, 1.0f, false );
		AddParam( "Main", "Max", IOType::Float, ParamEdition::Slider, 1.000f, -1.0f, 1.0f, false );
		AddParam( "Main", "S_Curve", IOType::Bool, ParamEdition::CheckBox, false, true );

		SetHLSLPrefix( "#include \"Shaders/Noise.h\"" );
		SetHLSLBody(	"float output = saturate( (_input0[ _pos ] - Min) / (Max - Min) );\n"
						"if( S_Curve ) { output = SCurve5( output ); }\n"
						"_output0[ _pos ] = output;\n" );
	}

	virtual ~ReRangeNode() {}
};

class ScaleBiasNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ScaleBiasNode )

	ScaleBiasNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Scale && Bias" );

		AddParam( "Main", "Scale", IOType::Float, ParamEdition::Slider, 1.000f, -2.0f, 2.0f, false );
		AddParam( "Main", "Bias", IOType::Float, ParamEdition::Slider, 0.000f, -1.0f, 1.0f, false );
		AddParam( "Main", "Clamp", IOType::Bool, ParamEdition::CheckBox, true, true );

		SetHLSLBody(	"float output = _input0[ _pos ] * Scale + Bias;\n"
						"if( Clamp ) { output = saturate( output ); }\n"
						"_output0[ _pos ] = output;\n" );
	}

	virtual ~ScaleBiasNode() {}
};

class InvertNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( InvertNode )

	InvertNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Invert" );
		
		AddParam( "Main", "Clamp", IOType::Bool, ParamEdition::CheckBox, true, true );

		SetHLSLBody(	"float output = 1.0f - _input0[ _pos ];\n"
						"if( Clamp ) { output = saturate( output ); }\n"
						"_output0[ _pos ] = output;\n" );
	}

	virtual ~InvertNode() {}
};


class SharpenNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( SharpenNode )

	SharpenNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Sharpen" );

		AddParam( "Main", "Strength", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f, false );

		SetHLSLBody(	"float centerWeight = lerp( 15.0f, 6.1f, Strength );"
						"float v  = _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f ) * centerWeight;\n"
						"      v -= _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f, int2( -1,  0 ) );\n"
						"      v -= _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f, int2(  1,  0 ) );\n"
						"      v -= _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f, int2(  0, -1 ) );\n"
						"      v -= _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f, int2(  0,  1 ) );\n"

						"float corners  = _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f, int2( -1, -1 ) );\n"
						"      corners += _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f, int2(  1, -1 ) );\n"
						"      corners += _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f, int2( -1,  1 ) );\n"
						"      corners += _input0.SampleLevel( _bilinearClampSampler, _uv, 0.0f, int2(  1,  1 ) );\n"
						
						"v -= corners * 0.5f;\n"
						"v /= (centerWeight - 6.0f);\n"//Normalize kernel
						"_output0[ _pos ] = v;\n" );
	}

	virtual ~SharpenNode() {}
};

class HSVNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( HSVNode )

	HSVNode()
	{
		SetUIName( "HSV" );

		AddInput( "Input", IOType::Color );
		AddOutput( "Output", IOType::Color );

		AddParam( "Main", "Hue", IOType::Float, ParamEdition::Slider, 0.0f, -1.0f, 1.0f );
		AddParam( "Main", "Saturation", IOType::Float, ParamEdition::Slider, 0.0f, -1.0f, 1.0f );
		AddParam( "Main", "Value", IOType::Float, ParamEdition::Slider, 0.0f, -1.0f, 1.0f );

		SetHLSLPrefix(	"float3 rgb2hsv( float3 c )\n"
						"{\n"
						"	float4 K = float4( 0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f );\n"
						"	float4 p = lerp( float4( c.bg, K.wz ), float4( c.gb, K.xy ), step( c.b, c.g ) );\n"
						"	float4 q = lerp( float4( p.xyw, c.r ), float4( c.r, p.yzx ), step( p.x, c.r ) );\n"
		
						"	float d = q.x - min( q.w, q.y );\n"
						"	float e = 1.0e-10;\n"
						"	return float3( abs( q.z + (q.w - q.y) / (6.0f * d + e) ), d / (q.x + e), q.x );\n"
						"}\n"
		
						"float3 hsv2rgb( float3 c )\n"
						"{\n"
						"	float4 K = float4( 1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f );\n"
						"	float3 p = abs( frac( c.xxx + K.xyz ) * 6.0f - K.www );\n"
						"	return c.z * lerp( K.xxx, saturate( p - K.xxx ), c.y );\n"
						"}\n" );

		SetHLSLBody( "float4 rgba = _input0.SampleLevel( _pointSampler, _uv, 0.0f );\n"
						"float3 hsv = rgb2hsv( rgba.rgb );\n"
						"hsv.x += Hue;\n"
						"hsv.y += Saturation;\n"
						"hsv.z += Value;\n"
						"hsv.x = frac( hsv.x );"
						"hsv.y = saturate( hsv.y );"
						"hsv.z = saturate( hsv.z );"
						"rgba.rgb = hsv2rgb( hsv );\n"
						"_output0[ _pos ] = rgba;\n" );
	}

	virtual ~HSVNode() {}
};

class TerraceNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( TerraceNode )

	TerraceNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Terrace" );

		AddInput( "Perturbation", IOType::Float, true );

		AddParam( "Main", "Terraces", IOType::Integer, ParamEdition::Slider, 6, 1, 30, false );
		AddParam( "Main", "Shape", IOType::Float, ParamEdition::Slider, 2.5f, 1.0f, 50.0f, false );
		AddParam( "Main", "Perturbation", IOType::Float, ParamEdition::Slider, 0.0f, 0.0f, 1.0f, false );

		SetHLSLBody(	"float numTerraces = (float)Terraces;\n"
						"float h = _input0[ _pos ]; \n"
						"h *= numTerraces;\n"
						"float t = floor( h ); \n"
						"float f = h - t; \n"
						"t /= numTerraces; \n"
						"_output0[ _pos ] = _input1[ _pos ] * Perturbation + t + pow( f, Shape ) / numTerraces;\n" );
	}

	virtual ~TerraceNode() {}
};

class ConvexityMaskNode : public CustomComputeNode
{
	//Laplace matrix https://en.wikipedia.org/wiki/Discrete_Laplace_operator
public:
	COMPUTE_NODE_FACTORY( ConvexityMaskNode )

	ConvexityMaskNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Convexity Mask" );

		m_bPreviewAsHeightField = false;

	//	AddParam( "Main", "Method", IOType::Integer, ParamEdition::Slider, 0, 0, 2, true );
		AddParam( "Main", "Scale", IOType::Float, ParamEdition::Slider, 1.0f, -1000.0f, 1000.0f, false );

		SetHLSLBody(
			"float k;\n"
			"k  = 0.5f  * _input0[ int2( _pos.x - 1, _pos.y - 1 ) ]; \n"
			"k +=         _input0[ int2( _pos.x    , _pos.y - 1 ) ]; \n"
			"k += 0.5f  * _input0[ int2( _pos.x + 1, _pos.y - 1 ) ]; \n"
			"k +=         _input0[ int2( _pos.x - 1, _pos.y     ) ]; \n"
			"k += -6.0f * _input0[ _pos ]; \n"
			"k +=         _input0[ int2( _pos.x + 1, _pos.y     ) ]; \n"
			"k += 0.5f  * _input0[ int2( _pos.x - 1, _pos.y + 1 ) ]; \n"
			"k +=         _input0[ int2( _pos.x    , _pos.y + 1 ) ]; \n"
			"k += 0.5f  * _input0[ int2( _pos.x + 1, _pos.y + 1 ) ]; \n"
			"_output0[ _pos ] = saturate( k * Scale );\n" );
	}

	virtual ~ConvexityMaskNode() {}
};

class RadialNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( RadialNode )

	RadialNode() : CustomComputeNode()
	{
		m_ResolutionReference = Res_GlobalSetting;

		SetUIName( "Radial" );

		int iParam = AddParam( "Main", "Type", IOType::Integer, ParamEdition::ComboBox, 0, 0, 1, true );
		ParamSlot* pParam = &m_ParameterSlots[ iParam ];
		pParam->AddEnum( "Square" );
		pParam->AddEnum( "Round" );

		AddParam( "Main", "Size", IOType::Float, ParamEdition::Slider, 0.2f, 0.0000001f, 1.0f, false );
		AddParam( "Main", "Falloff", IOType::Float, ParamEdition::Slider, 0.25f, 0.0000001f, 1.0f, false );

		AddParam( "Main", "CenterX", IOType::Float, ParamEdition::Slider, 0.5f, -1.0f, 2.0f, false );
		AddParam( "Main", "CenterY", IOType::Float, ParamEdition::Slider, 0.5f, -1.0f, 2.0f, false );
		AddParam( "Main", "Scale", IOType::Float, ParamEdition::Slider, 1.0f, -4.0f, 4.0f, false );
		AddParam( "Main", "Bias", IOType::Float, ParamEdition::Slider, 0.0f, -4.0f, 4.0f, false );

		AddOutput( "FloatMap", IOType::Float );

		SetHLSLBody(	"_uv -= float2( CenterX, CenterY );\n"
						"float v;\n"
						"if( Type == 0 )\n"
						"{\n"
						"	v  = 1.0f - smoothstep( Size, Size + Falloff, _uv.x ); \n"
						"	v *= 1.0f - smoothstep( Size, Size + Falloff, -_uv.x );\n"
						"	v *= 1.0f - smoothstep( Size, Size + Falloff, _uv.y ); \n"
						"	v *= 1.0f - smoothstep( Size, Size + Falloff, -_uv.y );\n"
						"}\n"
						"else if( Type == 1 )\n"
						"{\n"
						"	float d = length( _uv );\n"
						"	v = 1.0f - smoothstep( Size, Size + Falloff, d );\n"
						"}\n"
						"v = v * Scale + Bias;\n"
						"_output0[ _pos ] = saturate( v );\n" );
	}

	virtual ~RadialNode() {}
};

class GradientNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( GradientNode )

	GradientNode() : CustomComputeNode()
	{
		m_ResolutionReference = Res_GlobalSetting;

		SetUIName( "Gradient" );

		AddParam( "Main", "Direction", IOType::Float, ParamEdition::Slider, -0.0f, 0.0f, 360.0f, false );
		AddParam( "Main", "Start", IOType::Float, ParamEdition::Slider, -0.5f, -1.0f, 1.0f, false );
		AddParam( "Main", "End", IOType::Float, ParamEdition::Slider, 0.5f, -1.0f, 1.0f, false );
		AddParam( "Main", "SCurve", IOType::Bool, ParamEdition::CheckBox, false, true );

		AddOutput( "FloatMap", IOType::Float );

		SetHLSLBody(	"float2 dir;\n"
						"sincos( Direction * 3.14159f / 180.0f, dir.x, dir.y );\n"
						"float g = dot( _uv - 0.5f, dir );\n"
						"if( SCurve )\n"
						"	g = smoothstep( Start, End, g );\n"
						"else\n"
						"	g = saturate( (g - Start) / (End - Start) );\n"
						"_output0[ _pos ] = g;\n" );
	}

	virtual ~GradientNode() {}
};


class AltitudeMaskNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( AltitudeMaskNode )

	AltitudeMaskNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Altitude Mask" );

		m_bPreviewAsHeightField = false;

		AddParam( "Main", "Min", IOType::Float, ParamEdition::Slider, 0.25f, 0.0f, 1.0f, false );
		AddParam( "Main", "Max", IOType::Float, ParamEdition::Slider, 0.75f, 0.0f, 1.0f, false );
		AddParam( "Main", "Falloff", IOType::Float, ParamEdition::Slider, 0.1f, 0.0f, 1.0f, false );

		SetHLSLBody( "float alt = _input0[ _pos ];\n"
					"_output0[ _pos ] = smoothstep( Min - Falloff, Min, alt ) * (1.0f - smoothstep( Max, Max + Falloff, alt )); \n" );
	}

	virtual ~AltitudeMaskNode() {}
};

class SlopeMaskNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( SlopeMaskNode )

	SlopeMaskNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Slope Mask" );

		m_bPreviewAsHeightField = false;

		AddParam( "Main", "Min", IOType::Float, ParamEdition::Slider, 45.0f, 0.0f, 90.0f, false );
		AddParam( "Main", "Max", IOType::Float, ParamEdition::Slider, 90.0f, 0.0f, 90.0f, false );
		AddParam( "Main", "Falloff", IOType::Float, ParamEdition::Slider, 5.0f, 0.0f, 90.0f, false );

		SetHLSLBody( "float x1 = Terrain( _input0[ uint2(_pos.x - 1, _pos.y) ] );\n"
			"float x2 = Terrain( _input0[ uint2(_pos.x + 1, _pos.y) ] );\n"
			"float y1 = Terrain( _input0[ uint2(_pos.x, _pos.y - 1) ] );\n"
			"float y2 = Terrain( _input0[ uint2(_pos.x, _pos.y + 1) ] );\n\n"
			"float quadLength = Extent / (float)Resolution;\n"
			"float3 normal = float3( x2 - x1, 2.0f * quadLength, y2 - y1 );\n"
			"normal = normalize( normal );\n"
			"float slope = acos( normal.y ) * (180.0f / 3.14159f);\n"
			"_output0[ _pos ] = smoothstep( Min - Falloff, Min, slope ) * (1.0f - smoothstep( Max, Max + Falloff, slope )); \n" );
	}

	virtual ~SlopeMaskNode() {}
};

class ConstantColorNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ConstantColorNode )

	ConstantColorNode() : CustomComputeNode()
	{
		m_ResolutionReference = Res_GlobalSetting;

		SetUIName( "Constant Color" );

		AddParam( "Main", "Color", IOType::Color, ParamEdition::ColorPickerControl, Color::Red, false );

		AddOutput( "ColorMap", IOType::Color );

		SetHLSLBody( "_output0[ _pos ] = Color;\n" );
	}

	virtual ~ConstantColorNode() {}

	virtual CDialogEx* GetCustomUI( CWnd* _pParent );

	void SetColor( const Color& c ) { m_ParameterSlots[ 0 ].m_Value.c = c.ToWin32COLORREF(); }
};

class ConstantValueNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ConstantValueNode )

	ConstantValueNode() : CustomComputeNode()
	{
		m_ResolutionReference = Res_GlobalSetting;

		SetUIName( "Constant Value" );

		AddParam( "Main", "Value", IOType::Float, ParamEdition::Slider, 0.5f, 0.0f, 1.0f );

		AddOutput( "FloatMap", IOType::Float );

		SetHLSLBody( "_output0[ _pos ] = Value;\n" );
	}

	virtual ~ConstantValueNode() {}
};

class ChannelSplitNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ChannelSplitNode )

	ChannelSplitNode() : CustomComputeNode()
	{
		m_ResolutionReference = Res_MainInput;

		SetUIName( "Channel Splt" );
				
		AddInput( "ColorMap", IOType::Color );
		
		AddOutput( "R", IOType::Float );
		AddOutput( "G", IOType::Float );
		AddOutput( "B", IOType::Float );
		AddOutput( "A", IOType::Float );

		SetHLSLBody( "float4 c = _input0[ _pos ];\n"
					"_output0[ _pos ] = c.r;\n"
					"_output1[ _pos ] = c.g;\n"
					"_output2[ _pos ] = c.b;\n"
					"_output3[ _pos ] = c.a;\n"	);
	}

	virtual ~ChannelSplitNode() {}
};

class ChannelMergeNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ChannelMergeNode )

	ChannelMergeNode() : CustomComputeNode()
	{
		m_ResolutionReference = Res_MainInput;

		SetUIName( "Channel Merge" );
		
		AddInput( "R", IOType::Float );
		AddInput( "G", IOType::Float );
		AddInput( "B", IOType::Float );
		AddInput( "A", IOType::Float );

		AddOutput( "ColorMap", IOType::Color );

		SetHLSLBody( "float4 c;\n"
					"c.r = _input0[ _pos ];\n"
					"c.g = _input1[ _pos ];\n"
					"c.b = _input2[ _pos ];\n"
					"c.a = _input3[ _pos ];\n"
					"_output0[ _pos ] = c;\n" );
	}

	virtual ~ChannelMergeNode() {}
};

class BrickNode : public CustomComputeNode //TODO https://www.shadertoy.com/view/lltGDM
{
public:
	COMPUTE_NODE_FACTORY( BrickNode )

	BrickNode()
	{
		m_bPreviewAsHeightField = false;

		SetUIName( "Bricks" );

		//AddParam( "Main", "Color", IOType::Color, ParamEdition::ColorPickerControl, Color::Red, false );

		AddOutput( "FloatMap", IOType::Float );

		SetHLSLBody( "_output0[ _pos ] = 0.0f;\n" );
	}

	virtual ~BrickNode() {}
};

class ExpanderNode : public CustomComputeNode
{
public:
	COMPUTE_NODE_FACTORY( ExpanderNode )

	ExpanderNode() : CustomComputeNode( 1, 1 )
	{
		SetUIName( "Expander" );

		//	AddParam( "Main", "Radius", IOType::Float, ParamEdition::Slider, 0.1f, 0.0f, 0.5f ); //TODO float radius
		AddParam( "Main", "Radius", IOType::Integer, ParamEdition::Slider, 16, 0, 64, true ); //TODO float radius

		int iParam = AddParam( "Main", "Type", IOType::Integer, ParamEdition::ComboBox, 0, 0, 1, true );
		ParamSlot* pParam = &m_ParameterSlots[ iParam ];
		pParam->AddEnum( "Min" );
		pParam->AddEnum( "Max" );

		SetHLSLBody(
			"float v = _input0[ _pos ];\n"
			"for( int y = -Radius ; y <= Radius ; ++y )\n"
			"{\n"
			"	for( int x = -Radius ; x <= Radius ; ++x )\n"
			"	{\n"
			"		if( x*x + y*y > Radius*Radius )\n"
			"			continue;\n"

			"		float2 uv2 = _uv + float2( x, y ) / (Resolution - 1);\n"

			"		switch( Type )\n"
			"		{\n"
			"			case 0:	v = min( v, _input0.SampleLevel( _bilinearClampSampler, uv2, 0.0f ) ); break; \n"
			"			case 1:	v = max( v, _input0.SampleLevel( _bilinearClampSampler, uv2, 0.0f ) ); break; \n"
			"		}\n"
			"	}\n"
			"}\n"
			"_output0[ _pos ] = v;\n" );
	}

	virtual ~ExpanderNode() {}
};

