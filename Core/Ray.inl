
template <class T>
inline void Ray<T>::SetRayOnSphere( float u, float v )
{
	DBG_CHECK( ( u >= 0.0f ) && ( v <= 1.0f ) );

	float theta = 2.0f * ACos( Sqrt( 1.0f - u ) );
	float phi = 2.0f * XTM_PI * v;

	float sinTheta = Sin( theta );

	m_dir.x = sinTheta * Cos( phi );
	m_dir.y = sinTheta * Sin( phi );
	m_dir.z = Cos( theta );
}

template <class T>
inline void Ray<T>::SetRayOnHemisphere( float u, float v,
									const Vec3& hemisphereDir )
{
	SetRayOnSphere( u, v );

	if( Dot( hemisphereDir, ToVec3( m_dir ) ) < 0.0f )
		m_dir *= - 1.0f;
}

template <class T>
inline T Ray<T>::SquaredMinimumDistanceFromPoint( const TVec3<T>& point ) const
{
	//Ray origin   
	// o----+---> Ray direction
	//	\ b |
	//	 \  |c
	//	a \ | 
	//	   \|
	//	    * point

	TVec3<T> A( point - m_origin );

	T a2 = Dot( A, A ); 
	T b = Dot( A, m_dir );

	return ( b > 0.0f ) ? (a2 - b*b) : a2;		
}

template <class T>
inline T Ray<T>::MinimumDistanceFromPoint( const TVec3<T>& point ) const
{
	return Sqrt( SquaredMinimumDistanceFromPoint( point ) ); 
}


template <class T>
inline void Ray<T>::Transform( const TMat44<T>& mat )
{
	m_origin = mat.TransformPosition( m_origin );
	m_dir = mat.TransformDirection( m_dir );
}
