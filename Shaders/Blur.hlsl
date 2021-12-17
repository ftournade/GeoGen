Texture2D<float> _input0 : register(t0);
Texture2D<float> _input1 : register(t1);
RWTexture2D<float> _output0 : register(u0);

SamplerState _bilinearClampSampler : register(s0);

cbuffer BlurNodeConstants
{
	int KernelSize;
	float BlurRadius;
	float2 BlurDir;
}


float GaussianBlur(float2 _uv, Texture2D<float> _inputTex )
{
	uint2 texRes;
	_output0.GetDimensions( texRes.x, texRes.y );


	float sum = 0.0f;
	float weightSum = 0.0f;

	for( int i = -KernelSize; i <= KernelSize; ++i )
	{
		float2 duv = BlurDir * (float)i;
		float2 uv2 = _uv + duv;

		float weight = exp( -5.0f * (duv.x * duv.x + duv.y * duv.y) / (BlurRadius * BlurRadius) );
		sum += _inputTex.SampleLevel( _bilinearClampSampler, uv2, 0.0f ) * weight;
		weightSum += weight;
	}

	return sum / weightSum;
}

[ numthreads( 32, 32, 1 ) ]
void Blur( uint2 _pos : SV_DispatchThreadID )
{
	uint2 texRes;
	_output0.GetDimensions( texRes.x, texRes.y );

	float2 uv;
	uv.x = (float)_pos.x / (texRes.x - 1);
	uv.y = (float)_pos.y / (texRes.y - 1);

	_output0[ _pos ] = GaussianBlur( uv, _input0 );
}


[ numthreads( 32, 32, 1 ) ]
void BlurAndSubtractFromOriginal( uint2 _pos : SV_DispatchThreadID )
{
	uint2 texRes;
	_output0.GetDimensions( texRes.x, texRes.y );

	float2 uv;
	uv.x = (float)_pos.x / (texRes.x - 1);
	uv.y = (float)_pos.y / (texRes.y - 1);

	float blurred = GaussianBlur( uv, _input0 );
	float original = _input1.SampleLevel( _bilinearClampSampler, uv, 0.0f );

	_output0[ _pos ] = original - blurred;
}