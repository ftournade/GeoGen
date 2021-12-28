RWTexture2D<float> _output0 : register(u0);
Texture2D<float> _input0 : register(t1);
Texture2D<float> _input1 : register(t2);

struct Segment
{
	float3 A, B;
};

StructuredBuffer<Segment> Segments : register(t0);

cbuffer Constants : register(b0)
{
	uint NumSegments;
	uint Resolution;
	float MountainSlope;
	float ValleyWidth;
	float ValleyShape;
	float Distortion;
	float2 pad;
}

float DistanceSquared( float2 A, float2 B )
{
	float2 v = B - A;
	return dot( v, v );
}

float NearestPointOn2DSegment( float2 A, float2 B, float2 P )
{
	float l2 = DistanceSquared( A, B );

	if( l2 == 0.0f )
		return distance( P, A ); //degenerate segment

	float t = dot( B - A, P - A ) / l2;

	t = saturate( t );

	return t;
}

float Valley( float x ) //This is computed here instead of precomputed on CPU because I intend maybe to have per segment valley settings
{
	//The resulting curve is a piecewise combination of 2 curves:
	//y1 = pow(x/ValleyWidth, ValleyShape)
	//y2 = MountainSlope * x + B
	//We switch from y1 to y2 at Xsplit where their derivative is equal:
	float Xsplit = pow( MountainSlope * pow( ValleyWidth, ValleyShape ) / ValleyShape, 1.0f / (ValleyShape - 1.0f) );
	
	if( x <= Xsplit )
	{
		return pow( x / ValleyWidth, ValleyShape );
	}
	else
	{
		float B = pow( Xsplit / ValleyWidth, ValleyShape ) - MountainSlope * Xsplit;
		return MountainSlope * x + B;
	}
}

[numthreads( 32, 32, 1 )]
void Main( uint2 _pos : SV_DispatchThreadID )
{
	float2 P = (float2)_pos / (float)Resolution;
	
	if( Distortion > 0.0f )
	{
		P.x += (_input0[ _pos ] * 2.0f - 1.0f) * Distortion;
		P.y += (_input1[ _pos ] * 2.0f - 1.0f) * Distortion;
	}

	float altitude = 9999999999.0f;

	for( uint i = 0 ; i < NumSegments ; ++i )
	{
		Segment seg = Segments[i];

		float t = NearestPointOn2DSegment( seg.A.xy, seg.B.xy, P );

		float3 nearestPoint = seg.A + (seg.B - seg.A) * t;

		float dist2D = distance( P, nearestPoint.xy );

		float alt = nearestPoint.z + Valley( dist2D );

		altitude = min( altitude, alt );
	}
	
	_output0[_pos] = altitude;

}