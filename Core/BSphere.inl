	template <class T>
	inline void	BSphere<T>::Set( const TVec3<T>& _center, T _radius )
	{
		m_Center = _center;
		m_Radius = _radius;
		m_SquaredRadius = _radius * _radius;
	}
	
	template <class T>
	inline float BSphere<T>::ComputeVolume() const
	{
		return (float)(T((4.0 / 3.0) * XTM_PI) * m_SquaredRadius * m_Radius); 
	}

	template <class T>
	inline float BSphere<T>::ComputeSurfaceArea() const
	{
		return (float)( T(4.0 * XTM_PI) * m_SquaredRadius); 
	}


	template <class T>
	inline bool BSphere<T>::RayHitTest( const Ray<T>& _ray ) const
	{
		return ( _ray.SquaredMinimumDistanceFromPoint( m_Center ) < m_SquaredRadius );
	}

	template <class T>
	inline bool BSphere<T>::RayHitTest( const Ray<T>& _ray, float _maxRayLength ) const
	{
		T d = _maxRayLength + (float)m_Radius;
		
		if( DistanceSquared( _ray.m_origin, m_Center ) <= d * d )
			return ( _ray.SquaredMinimumDistanceFromPoint( m_Center ) < m_SquaredRadius );
		else
			return false;
	}
	
	template <class T>
	inline T BSphere<T>::RayHitDist( const Ray<T>& ray ) const
	{		
		TVec3<T> dst( m_Center - ray.m_origin );
		T B = Dot( dst, ray.m_dir );
		
		T A2 = Dot( dst, dst );
		T C2 = A2 - B*B;
		T N2 = m_SquaredRadius - C2;
		
		if( N2 <= T(0.0) )
			return (T)-1.0;

		T N = Sqrt( N2 );

		T dist = B - N;
	
		if( dist >= (T)0.0 )
		{
		//	_hitPos = ray.m_origin + ray.m_dir * _dist;
		//	_hitNormal = ToVec3( Normalized( _hitPos - m_Center ) );
		//	return true;
		}
		else if( B + N > (T)0.0 )
		{
			//The sphere originates inside the sphere
			dist = B + N;
		//	_hitPos = ray.m_origin;
		//	_hitNormal = ToVec3( Normalized( _hitPos - m_Center ) );
		//	return true;
			
		}

			
		return dist;
	}

	template <class T>
	inline bool BSphere<T>::RayHitTest( const Ray<T>& ray, float _maxRayLength, TVec3<T>& _hitPos, TVec3<float>& _hitNormal, T& _dist ) const
	{		
		TVec3<T> dst( m_Center - ray.m_origin );
		T B = Dot( dst, ray.m_dir );
		
		T A2 = Dot( dst, dst );
		T C2 = A2 - B*B;
		T N2 = m_SquaredRadius - C2;
		
		if( N2 <= (T)0.0 )
			return false;

		T N = Sqrt( N2 );

		_dist = B - N;
	
		if( ( _dist >= (T)0.0 ) && 
			( (_maxRayLength <= (T)0.0) || (_dist <= (T)_maxRayLength) ) )
		{
			_hitPos = ray.m_origin + ray.m_dir * _dist;
			_hitNormal = ToVec3( Normalized( _hitPos - m_Center ) );
			return true;
		}
		else if( B + N > (T)0.0 )
		{
			//The ray originates inside the sphere
		#if 0
			_dist = (T)0.0;
			_hitPos = ray.m_origin;
		#else
			_dist = B + N;
			_hitPos = ray.m_origin  + ray.m_dir * _dist;
		#endif

			_hitNormal = ToVec3( Normalized( _hitPos - m_Center ) );

			return true;
			
		}

			
		return false;
	}

	template <class T>
	inline bool BSphere<T>::RayHitTest( const Ray<T>& ray, float _maxRayLength, TVec3<T>& _hitPos, TVec3<double>& _hitNormal, T& _dist ) const
	{		
		TVec3<T> dst( m_Center - ray.m_origin );
		T B = Dot( dst, ray.m_dir );
		
		T A2 = Dot( dst, dst );
		T C2 = A2 - B*B;
		T N2 = m_SquaredRadius - C2;
		
		if( N2 <= (T)0.0 )
			return false;

		T N = Sqrt( N2 );

		_dist = B - N;
	
		if( ( _dist >= (T)0.0 ) && 
			( (_maxRayLength <= (T)0.0) || (_dist <= (T)_maxRayLength) ) )
		{
			_hitPos = ray.m_origin + ray.m_dir * _dist;
			_hitNormal = ToVec3( Normalized( _hitPos - m_Center ) );
			return true;
		}
		else if( B + N > (T)0.0 )
		{
			//The sphere originates inside the sphere
			_dist = (T)0.0;
			_hitPos = ray.m_origin;
			_hitNormal = ToVec3( Normalized( _hitPos - m_Center ) );
			return true;
			
		}

			
		return false;
	}

	template <class T>
	inline void BSphere<T>::Set( const BSphere<T>& sphere, const TMat44<T>& transform )
	{
		m_Center = sphere.m_Center * transform;

		//Assuming uniform scale
		TVec3<T> v( transform._11, transform._21, transform._31 );

		T scale = v.Length();

		m_Radius = sphere.m_Radius * scale;

		m_SquaredRadius = m_Radius * m_Radius;
	}

	template <class T>
	inline void BSphere<T>::Set( const OBBox<T>& obbox )
	{
		m_Center = obbox.GetCenter();
		m_Radius = Distance( obbox.GetCorners()[0], m_Center );
		m_SquaredRadius = m_Radius * m_Radius;
	}

	template <class T>
	inline void BSphere<T>::Set( const AABBox<T>& aabbox )
	{
		m_Center = aabbox.Center();
		m_Radius = Distance( aabbox.m_max, m_Center );
		m_SquaredRadius = m_Radius * m_Radius;
	}

	template <class T>
	inline PlaneClassify BSphere<T>::Classify( const Plane<T>& plane ) const
	{
		T d = plane.SignedDistance( m_Center );

		if( d < -m_Radius )
			return NEGATIVE_PLANE_SIDE;
		else if( d > m_Radius )
			return POSITIVE_PLANE_SIDE;
		else
			return CROSSING_PLANE;

	}

