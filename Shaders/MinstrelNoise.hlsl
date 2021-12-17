RWTexture2D<float> _output0 : register(u0);

cbuffer Constants : register(b0)
{
	float Radius; float3 pad0;
}

[ numthreads( 32, 32, 1 ) ]
void SetupGrid( uint2 _pos : SV_DispatchThreadID )
{


	_output0[ _pos ] = exp( -d2 );

	AllMemoryBarrierWithGroupSync();


}


[numthreads( 32, 32, 1 )]
void Main( uint2 _pos : SV_DispatchThreadID )
{
	_output0[ _pos ] = _pos.x / 1024.0f;
}