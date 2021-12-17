Texture2D g_NoisePermutationTex : register(t8);
Texture2D g_NoiseGradientsTex : register(t9);
//Texture2D<float> g_PerlinNoiseTex : register(t10);
SamplerState g_NoisePermutationSampler : register(s8);
SamplerState g_NoiseGradientsSampler : register(s9);

//Interpolation methods
///////////////////////

#define LOW_QUALITY_NOISE_INTERP 0
#define MED_QUALITY_NOISE_INTERP 1
#define HIGH_QUALITY_NOISE_INTERP 2

float SCurve3( float t )
{
	return (t * t * (3.0f - 2.0f * t));
}

float SCurve5( float t )
{
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float SCurve( float _t, int _quality )
{
	[flatten]
	switch( _quality )
	{
		case LOW_QUALITY_NOISE_INTERP: return _t;
		case MED_QUALITY_NOISE_INTERP: return SCurve3( _t );
		case HIGH_QUALITY_NOISE_INTERP: return SCurve5( _t );
		default: return 0.0f;
	}
}

float2 SCurve( float2 _t, int _quality )
{
	[flatten]
	switch( _quality )
	{
		case LOW_QUALITY_NOISE_INTERP: return _t;
		case MED_QUALITY_NOISE_INTERP: return float2( SCurve3( _t.x ), SCurve3( _t.y ) );
		case HIGH_QUALITY_NOISE_INTERP: return float2( SCurve5( _t.x ), SCurve5( _t.y ) );
		default: return float2( 0, 0 );
	}
}

float3 SCurve( float3 _t, int _quality )
{
	[flatten]
	switch( _quality )
	{
		case LOW_QUALITY_NOISE_INTERP: return _t;
		case MED_QUALITY_NOISE_INTERP: return float3( SCurve3( _t.x ), SCurve3( _t.y ), SCurve3( _t.z ) );
		case HIGH_QUALITY_NOISE_INTERP: return float3( SCurve5( _t.x ), SCurve5( _t.y ), SCurve5( _t.z ) );
		default: return float3( 0, 0, 0 );
	}
}

//Hash methods
//////////////

#define NOISE_HASH_INIGO_QUILEZ 0
#define NOISE_HASH_LIBNOISE 1
#define NOISE_HASH_GPU_GEMS 2

float NoiseHash_InigoQuilez( float2 p ) 
{
	float h = dot( p, float2( 127.1f, 311.7f ) );
    return frac( sin(h) * 43758.5453123f );
}

float2 NoiseHash2_InigoQuilez( float2 p ) 
{
	p = float2( dot( p, float2( 127.1f, 311.7f ) ), 
				dot( p, float2( 269.5f, 183.3 ) ) );

    return frac( sin(p) * 43758.5453123f );
}

float3 NoiseHash3_InigoQuilez( float2 p )
{
	float3 q = float3(	dot( p, float2( 127.1f, 311.7f ) ),
						dot( p, float2( 269.5f, 183.3f ) ),
						dot( p, float2( 419.2f, 371.9f ) ) );

	return frac( sin( q ) * 43758.5453f );
}

float3 NoiseHash3_InigoQuilez( float3 p ) 
{
	p = float3( dot( p, float3( 127.1f, 311.7f, 74.7f ) ),
				dot( p, float3( 269.5f, 183.3f, 246.1f ) ),
				dot( p, float3( 113.5f, 271.9f, 124.6f ) ) );

    return frac( sin(p) * 43758.5453123f );
}

float NoiseHash_InigoQuilez( float3 p ) 
{
	float h = dot( p, float3( 127.1f, 311.7f, 74.7f ) );
    return frac( sin(h) * 43758.5453123f );
}

float NoiseHash_LibNoise( float2 p )
{
	const int X_NOISE_GEN = 1619;
	const int Y_NOISE_GEN = 31337;
	const int Z_NOISE_GEN = 6971;
	const int SEED_NOISE_GEN = 1013;
	const int SHIFT_NOISE_GEN = 8;

	// All constants are primes and must remain prime in order for this noise
	// function to work correctly.
	int n = ( X_NOISE_GEN * (int)p.x 
			+ Y_NOISE_GEN * (int)p.y ) & 0x7fffffff;

	n = (n >> 13) ^ n;
	n = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;

	return (float)n / 1073741824.0f;
}

float NoiseHash_LibNoise( float3 p )
{
	const int X_NOISE_GEN = 1619;
	const int Y_NOISE_GEN = 31337;
	const int Z_NOISE_GEN = 6971;
	const int SEED_NOISE_GEN = 1013;
	const int SHIFT_NOISE_GEN = 8;

	// All constants are primes and must remain prime in order for this noise
	// function to work correctly.
	int n = (	X_NOISE_GEN      * (int)p.x
				+ Y_NOISE_GEN    * (int)p.y
				+ Z_NOISE_GEN    * (int)p.z ) & 0x7fffffff;

	n = (n >> 13) ^ n;
	n = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;

	return (float)n / 1073741824.0f;
}

float NoiseHash( float2 p, int _hashMethod )
{
	[flatten]
	switch( _hashMethod )
	{
		case NOISE_HASH_INIGO_QUILEZ: return NoiseHash_InigoQuilez( p );
		case NOISE_HASH_LIBNOISE: return NoiseHash_LibNoise( p );
		default : return 0;
	}
}

float NoiseHash( float3 p, int _hashMethod )
{
	[flatten]
	switch( _hashMethod )
	{
		case NOISE_HASH_INIGO_QUILEZ: return NoiseHash_InigoQuilez( p );
		case NOISE_HASH_LIBNOISE: return NoiseHash_LibNoise( p );
		default : return 0;
	}
}

float2 NoiseHash2( float2 p, int _hashMethod )
{
	[flatten]
	switch( _hashMethod )
	{
		case NOISE_HASH_INIGO_QUILEZ: return NoiseHash2_InigoQuilez( p );
		default : return 0;
	}
}

float3 NoiseHash3( float3 p, int _hashMethod )
{
	[flatten]
	switch( _hashMethod )
	{
		case NOISE_HASH_INIGO_QUILEZ: return NoiseHash3_InigoQuilez( p );
		default : return 0;
	}
}

//Real perlin noise (gradient based, high quality) see GPU Gems 2
///////////////////

float4 perm2d( float2 p )
{
	return g_NoisePermutationTex.SampleLevel( g_NoisePermutationSampler, p, 0 );
}

float gradperm( float x, float3 p )
{
	return dot( g_NoiseGradientsTex.SampleLevel( g_NoiseGradientsSampler, x, 0 ).rgb, p );
}

float GradNoise3D( float3 p, int _interpQuality = MED_QUALITY_NOISE_INTERP )
{
	float3 floor_p = floor( p );
	float3 P = fmod( floor_p, 256.0f );	// FIND UNIT CUBE THAT CONTAINS POINT
  	p -= floor_p;                      // FIND RELATIVE X,Y,Z OF POINT IN CUBE.
	float3 f;
	f.x = SCurve( p.x, _interpQuality );
	f.y = SCurve( p.y, _interpQuality );
	f.z = SCurve( p.z, _interpQuality );
	
	P = P / 256.0f;
	const float one = 1.0f / 256.0f;
	
    // HASH COORDINATES OF THE 8 CUBE CORNERS
	float4 AA = perm2d(P.xy) + P.z;
 
	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
  	return lerp( lerp( lerp( gradperm(AA.x, p ),  
                             gradperm(AA.z, p + float3(-1.0f, 0.0f, 0.0f) ), f.x),
                       lerp( gradperm(AA.y, p + float3(0.0f, -1.0f, 0.0f) ),
                             gradperm(AA.w, p + float3(-1.0f, -1.0f, 0.0f) ), f.x), f.y),
                             
                 lerp( lerp( gradperm(AA.x+one, p + float3(0.0f, 0.0f, -1.0f) ),
                             gradperm(AA.z+one, p + float3(-1.0f, 0.0f, -1.0f) ), f.x),
                       lerp( gradperm(AA.y+one, p + float3(0.0f, -1.0f, -1.0f) ),
                             gradperm(AA.w+one, p + float3(-1.0f, -1.0f, -1.0f) ), f.x), f.y), f.z);
}

//Noise functions
/////////////////

float Noise2D( float2 p, int _hashMethod = NOISE_HASH_INIGO_QUILEZ, int _interpQuality = HIGH_QUALITY_NOISE_INTERP ) {

//	if( _hashMethod == NOISE_HASH_GPU_GEMS )
//	{
//		return GradNoise2D( p, _interpQuality ); //TODO implement
//	}

    float2 i = floor( p );
    float2 f = frac( p );

	float2 u = SCurve( f, _interpQuality );

    float n = lerp(	lerp(	NoiseHash( i + float2(0.0f,0.0f), _hashMethod ), 
							NoiseHash( i + float2(1.0f,0.0f), _hashMethod ), u.x ),
					lerp(	NoiseHash( i + float2(0.0f,1.0f), _hashMethod ), 
							NoiseHash( i + float2(1.0f,1.0f), _hashMethod ), u.x ), u.y );

	if( _hashMethod == NOISE_HASH_INIGO_QUILEZ )
	{
		n = n * 2.0f - 1.0f;
	}

	return n;
}

float Noise3D( float3 p, int _hashMethod = NOISE_HASH_INIGO_QUILEZ, int _interpQuality = HIGH_QUALITY_NOISE_INTERP ) {

	if( _hashMethod == NOISE_HASH_GPU_GEMS )
	{
		return GradNoise3D( p, _interpQuality );
	}

    float3 i = floor( p );
    float3 f = frac( p );

	float3 u = SCurve( f, _interpQuality );

    float n =	lerp(	lerp(	lerp(	NoiseHash( i + float3(0.0f, 0.0f, 0.0f), _hashMethod ), 
										NoiseHash( i + float3(1.0f, 0.0f, 0.0f), _hashMethod ), u.x ),
								lerp(	NoiseHash( i + float3(0.0f, 1.0f, 0.0f), _hashMethod ), 
										NoiseHash( i + float3(1.0f, 1.0f, 0.0f), _hashMethod ), u.x ), u.y ),
						lerp(	lerp(	NoiseHash( i + float3(0.0f, 0.0f, 1.0f), _hashMethod ), 
										NoiseHash( i + float3(1.0f, 0.0f, 1.0f), _hashMethod ), u.x ),
								lerp(	NoiseHash( i + float3(0.0f, 1.0f, 1.0f), _hashMethod ), 
										NoiseHash( i + float3(1.0f, 1.0f, 1.0f), _hashMethod ), u.x ), u.y ), u.z );

	if( _hashMethod == NOISE_HASH_INIGO_QUILEZ )
	{
		n = n * 2.0f - 1.0f;
	}

	return n;
}

//returns float2( dist * dist, id )
float2 Voronoi2D( float2 p, float _chaos = 1.0f )
{
    float2 n = floor( p );
    float2 f = frac( p );

	float3 m = float3( 8.0f, 8.0f, 8.0f );

	[unroll]
	for( int j = -1; j <= 1; j++ )
	{
		[unroll]
		for( int i = -1; i <= 1; i++ )
		{
			float2  g = float2( i, j );
			float2  o = NoiseHash2( n + g, NOISE_HASH_INIGO_QUILEZ );
			float2  r = g - f + o * _chaos;
			float d = dot( r, r );

			if( d < m.x )
				m = float3( d, o );
		}
	}
	float id = dot( m.yz, float2( 1.0f, 57.0f ) );

    return float2( m.x, id );
}

//returns float2( dist1 * dist1, dist2 * dist2, id )
//dist1 is distance to nearest cell, and dist 2 is distance to 2nd nearest cell
//see http://www.iquilezles.org/www/articles/voronoilines/voronoilines.htm
float3 F1F2Voronoi2D( float2 p, float _chaos = 1.0f )
{
    float2 n = floor( p );
    float2 f = frac( p );

	float4 m = float4( 8.0f, 8.0f, 8.0f, 8.0f );
	
	[unroll]
	for( int j = -1; j <= 1; j++ )
	{
		[unroll]
		for( int i = -1; i <= 1; i++ )
		{
			float2  g = float2( i, j );
			float2  o = NoiseHash2( n + g, NOISE_HASH_INIGO_QUILEZ );
			float2  r = g - f + o * _chaos;
			float d = dot( r, r );

			if( d < m.x )
			{
				m.y = m.x;
				m.x = d;
				m.zw = o;
			}
			else if( d < m.y )
			{
				m.y = d;
			}
		}
	}
	float id = dot( m.zw, float2( 1.1952f, 57.243f ) );

    return float3( m.xy, id );
}

//http://www.iquilezles.org/www/articles/voronoilines/voronoilines.htm
float2 AccurateVoronoi2D( float2 p, float _chaos = 1.0f )
{
	float2 n = floor( p );
	float2 f = frac( p );

	// first pass: regular voronoi
	float2 mg, mr;

	float md = 8.0f;

	[unroll]
	for( int j = -1; j <= 1; j++ )
	{
		[unroll]
		for( int i = -1; i <= 1; i++ )
		{
			float2 g = float2( float( i ), float( j ) );
			float2 o = NoiseHash2( n + g, NOISE_HASH_INIGO_QUILEZ );
			float2 r = g - f + o * _chaos;
			float d = dot( r, r );

			if( d < md )
			{
				md = d;
				mr = r;
				mg = g;
			}
		}
	}

	// second pass: distance to borders
	md = 8.0f;

	float2 mo;

	[unroll]
	for( j = -2; j <= 2; j++ )
	{
		[unroll]
		for( int i = -2; i <= 2; i++ )
		{
			float2 g = mg + float2( float( i ), float( j ) );
			float2 o = NoiseHash2( n + g, NOISE_HASH_INIGO_QUILEZ );
			float2 r = g - f + o * _chaos;

			if( dot( mr - r, mr - r ) > 0.00001f )
			{
				md = min( md, dot( 0.5f*(mr + r), normalize( r - mr ) ) );
				mo = o;
			}
		}
	}

	float id = frac( dot( mo, float2( 1.1952f, 57.243f ) ) );

	return float2( md, id );
}

//returns float2( dist * dist, id )
float2 Voronoi3D( float3 p, float _chaos = 1.0f )
{
    float3 n = floor( p );
    float3 f = frac( p );

	float4 m = float4( 100.0f, 100.0f, 100.0f, 100.0f );

	[unroll]
	for( int k=-1; k<=1; k++ )
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        float3  g = float3( i, j, k );
        float3  o = NoiseHash3( n + g, NOISE_HASH_INIGO_QUILEZ );
		float3  r = g - f + o * _chaos;
		float d = dot( r, r );

        if( d < m.x )
            m = float4( d, o );
    }

	float id = dot( m.yzw, float3( 1.0f, 57.0f, 113.0f ) );

    return float2( m.x, id );
}

// More info here: http://iquilezles.org/www/articles/voronoise/voronoise.htm
float Voronoise( float2 x, float _chaos, float _smoothness )
{
	float2 p = floor( x );
	float2 f = frac( x );

	float k = 1.0f + 63.0f*pow( 1.0f - _smoothness, 4.0f );

	float va = 0.0f;
	float wt = 0.0f;
	for( int j = -2; j <= 2; j++ )
		for( int i = -2; i <= 2; i++ )
		{
			float2 g = float2( float( i ), float( j ) );
			float3 o = NoiseHash3_InigoQuilez( p + g ) * float3( _chaos, _chaos, 1.0f );
			float2 r = g - f + o.xy;
			float d = dot( r, r );
			float ww = pow( 1.0 - smoothstep( 0.0f, 1.414f, sqrt( d ) ), k );
			va += o.z*ww;
			wt += ww;
		}

	return va / wt;
}

float FractalNoise2D(	float2		_in, 
						int			_octaves,
						float		_lacunarity,
						float		_persistence,
						float		_shape,
						int			_hashMethod = NOISE_HASH_INIGO_QUILEZ,
						int			_interpQuality = HIGH_QUALITY_NOISE_INTERP )
{
	float noise = 0.0f;
	float amplitude = 1.0f;
	
	float normalization = 0;

	for( int i=0 ; i < _octaves ; ++i )
	{
		float signal = Noise2D( _in, _hashMethod, _interpQuality );
		signal = pow( signal * 0.5f + 0.5f, _shape ) * 2.0f - 1.0f;
		noise += signal * amplitude;

		normalization += amplitude;
			
		_in *= _lacunarity;
		amplitude *= _persistence;
	}
		
	return noise / normalization;
}

float MultiFractalNoise2D(	float2		_in,
							int			_octaves,
							float		_lacunarity,
							float		_persistence,
							float		_shape,
							int			_hashMethod = NOISE_HASH_INIGO_QUILEZ,
							int			_interpQuality = HIGH_QUALITY_NOISE_INTERP )
{
	float noise = 0.0f;
	float amplitude = 1.0f;

	float normalization = 0;

	_persistence = 2.0f * (1.0f - _persistence);

	for( int i = 0 ; i < _octaves ; ++i )
	{
		float signal = Noise2D( _in, _hashMethod, _interpQuality ) * 0.5f + 0.5f;

		signal = pow( signal, _shape );

		noise += signal * amplitude;

		normalization += amplitude;

		_in *= _lacunarity;
		amplitude *= pow( signal, _persistence );
	}

	noise /= normalization;
	noise = noise * 2.0f - 1.0f;
	return noise;
}

float FractalNoise3D(	float3		_in, 
						int			_octaves,
						float		_lacunarity = 1.9879f,
						float		_persistence = 0.5f,
						int			_hashMethod = NOISE_HASH_INIGO_QUILEZ,
						int			_interpQuality = HIGH_QUALITY_NOISE_INTERP )
{
	float noise = 0.0f;
	float amplitude = 1.0f;
	
	float normalization = 0;

	for( int i=0 ; i < _octaves ; ++i )
	{
		float signal = Noise3D( _in, _hashMethod, _interpQuality );
		noise += signal * amplitude;

		normalization += amplitude;
			
		_in *= _lacunarity;
		amplitude *= _persistence;
	}
		
	return noise / normalization;
}

//This version allows blending of the last octave, it is designed to work like trilinear texture filtering to smoothly blend between fractal LODs
float FractalNoise3DBlendLastOctave(	float3		_in, 
										int			_octaves,
										float		_lastOctaveMultiplier,
										float3		_lacunarity,
										float		_persistence,
										int			_hashMethod = NOISE_HASH_INIGO_QUILEZ,
										int			_interpQuality = HIGH_QUALITY_NOISE_INTERP )
{
	float noise = 0.0f;
	float amplitude = 1.0f;

	float normalization = 0;

	for( int i=0 ; i < _octaves - 1 ; ++i )
	{
		float signal = Noise3D( _in, _hashMethod, _interpQuality );
		noise += signal * amplitude;

		normalization += amplitude;

		_in *= _lacunarity;
		amplitude *= _persistence;
	}

	normalization += amplitude;
	float signal = Noise3D( _in, _hashMethod, _interpQuality );
	noise += signal * amplitude * _lastOctaveMultiplier;

	return noise / normalization;
}		

float RidgedNoise2D(	float2		_in, 
						int			_octaves,
						float		_lacunarity,
						float		_persistence,
						float		_shape,
						int			_hashMethod = NOISE_HASH_INIGO_QUILEZ,
						int			_interpQuality = HIGH_QUALITY_NOISE_INTERP )
{
	float noise = 0.0f;
	float amplitude = 1.0f;

	float normalization = 0;

	for( int i=0 ; i < _octaves ; ++i )
	{
		float signal = Noise2D( _in, _hashMethod, _interpQuality );

		signal = 1.0f - abs( signal );
		signal = pow( signal, _shape );
		signal = signal * 2.0f - 1.0f;

		noise += signal * amplitude;

		normalization += amplitude;

		_in *= _lacunarity;
		amplitude *= _persistence;
	}

	return noise / normalization;
}

float RidgedNoise3D(	float3		_in, 
						int			_octaves,
						float		_lacunarity,
						float		_persistence,
						float		_shape,
						int			_hashMethod = NOISE_HASH_INIGO_QUILEZ,
						int			_interpQuality = HIGH_QUALITY_NOISE_INTERP )
{
	float noise = 0.0f;
	float amplitude = 1.0f;

	float normalization = 0;

	for( int i=0 ; i < _octaves ; ++i )
	{
		float signal = Noise3D( _in, _hashMethod, _interpQuality );

		signal = 1.0f - abs( signal );
		signal = pow( signal, _shape );
		signal = signal * 2.0f - 1.0f;

		noise += signal * amplitude;

		normalization += amplitude;

		_in *= _lacunarity;
		amplitude *= _persistence;
	}

	return noise / normalization;
}

float FractalVoronoi2D(	float2		_in, 
						int			_octaves,
						float		_lacunarity,
						float		_persistence,
						float		_chaos,
						float		_shape )
{
	float noise = 0.0f;
	float amplitude = 1.0f;
	
	float normalization = 0;

	for( int i=0 ; i < _octaves ; ++i )
	{
		float signal = Voronoi2D( _in, _chaos ).x;
		signal = pow( signal, _shape );
		noise += signal * amplitude;

		normalization += amplitude;
			
		_in *= _lacunarity;
		amplitude *= _persistence;
	}
		
	return noise / normalization;
}