//TODO separable gaussian (tradeoff mem vs speed) and CPU calculated weights
float BruteForceGaussian( Texture2D<float> _tex, SamplerState _sampler, float2 _uv, float _radius )
{
	const float threshold = 0.01f;
	int kernelRadius = 7;// sqrt( -2.0f * _radius * _radius * log( threshold ) );
	//kernelRadius = min( kernelRadius, 20 );
	//TODO make terrain scale independant

	float totalWeight = 0.0f;
	float f = 0.0f;

	const float s = 1.0f / (2.0f * _radius * _radius);

	for( int dy = -kernelRadius ; dy <= kernelRadius ; ++dy )
	{
		for( int dx = -kernelRadius ; dx <= kernelRadius ; ++dx )
		{
			float w = exp( -(float)(dx*dx + dy*dy) * s );
			totalWeight += w;

			float samp = _tex.SampleLevel( _sampler, _uv, 0.0f, int2( dx, dy ) );

			f += samp * w;
		}

	}

	return f / totalWeight;
}