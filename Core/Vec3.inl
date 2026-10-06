template <class T>
inline T Dot(const TVec3<T>& vec1, const TVec3<T>& vec2)
{
	return vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;
}

template <class T>
inline T AngleBetweenNormalizedVectors( const TVec3<T>& vec1, const TVec3<T>& vec2 )
{
	return ACos( Dot( vec1, vec2 ) );
}

template <class T>
inline TVec3<T> Cross(const TVec3<T>& vec1, const TVec3<T>& vec2)
{
	return TVec3<T>(	vec1.y*vec2.z - vec1.z*vec2.y,
					vec1.z*vec2.x - vec1.x*vec2.z,
					vec1.x*vec2.y - vec1.y*vec2.x);
}

template <class T>
inline T DistanceSquared(const TVec3<T>& vec1, const TVec3<T>& vec2)
{
	//TODO Check assembly code
	TVec3<T> v( vec1 - vec2 );
	return v.LengthSquared();
}

template <class T>
inline T Distance(const TVec3<T>& vec1, const TVec3<T>& vec2)
{
	//TODO Check assembly code
	TVec3<T> v( vec1 - vec2 );
	return v.Length();
}

template <class T>
inline TVec3<T> Modulate(const TVec3<T>& a, const TVec3<T>& b)
{
	return TVec3<T>( a.x * b.x, a.y * b.y, a.z * b.z );
}

template <class T>
inline TVec3<T> Pow( const TVec3<T>& _v, T _f )
{
	return TVec3<T>( Pow( _v.x, _f ), Pow( _v.y, _f ), Pow( _v.z, _f ) );
}

template <class T>	
inline TVec3<T> Abs( const TVec3<T>& _v )
{
	return TVec3<T>( Abs( _v.x ), Abs( _v.y ), Abs( _v.z ) );
}


template <class T>
inline TVec3<T> Sign( const TVec3<T>& _v )
{
	return TVec3<T>(	_v.x < 0.0f ? -1.0f : 1.0f,
						_v.y < 0.0f ? -1.0f : 1.0f,
						_v.z < 0.0f ? -1.0f : 1.0f );
}

template <class T>
inline TVec3<T> Normalized(const TVec3<T>& _v)
{
	TVec3<T> r(_v);
	r.Normalize();
	return r;
}


template <class T>
inline TVec3<T> Reflect(const TVec3<T>& _v, const TVec3<T>& _normal )
{
	DBG_CHECK( _v.IsNormalized() && _normal.IsNormalized() );

	return _v - _normal * ( (T)2.0 * Dot( _v, _normal ) );
}

template <class T>	
inline bool	Refract( const TVec3<T>& _lightDir, const TVec3<T>& _normal, T _ior, TVec3<T>& _refractedDir )
{
	T c1 = -Dot( _lightDir, _normal );
	T c2 = (T)1.0 - _ior * _ior * ( (T)1.0 - c1 * c1 );

	if( c2 < (T)0.0 )
		return false; //total internal reflection

	_refractedDir = _lightDir * _ior + _normal * ( _ior * c1 - Sqrt( c2 ) );

	return true;
}

template <class T>
inline TVec3<T> Min( const TVec3<T>& _a, const TVec3<T>& _b )
{
	return TVec3<T>( Min( _a.x, _b.x ), Min( _a.y, _b.y ), Min( _a.z, _b.z ) );
}

template <class T>
inline TVec3<T> Max( const TVec3<T>& _a, const TVec3<T>& _b )
{
	return TVec3<T>( Max( _a.x, _b.x ), Max( _a.y, _b.y ), Max( _a.z, _b.z ) );
}

template <class T>
inline TVec3<T> Floor( const TVec3<T>& _v )
{
	return TVec3<T>( Floor( _v.x ), Floor( _v.y ), Floor( _v.z ) );
}

template <class T>
inline TVec3<T> Ceil( const TVec3<T>& _v )
{
	return TVec3<T>( Ceil( _v.x ), Ceil( _v.y ), Ceil( _v.z ) );
}

template <class T>
inline TVec3<T> Exp( const TVec3<T>& _v )
{
	return TVec3<T>( exp( _v.x ), exp( _v.y ), exp( _v.z ) );
}

/////////////////////////////////////////////



template <class T>
inline TVec3<T>::TVec3()
{}

template <class T>
inline TVec3<T>::TVec3(const T* pVec) :
	x( *pVec++ ),
	y( *pVec++ ),
	z( *pVec )
{
	
}

template <class T>
inline TVec3<T>::TVec3(T _x,T _y,T _z) :
	x( _x ),
	y( _y ),
	z( _z )
{
}
	/*
template <class T>
inline TVec3<T>::TVec3(const TVec3<T>& vec) :
	x(vec.x),
	y(vec.y),
	z(vec.z)
{
}
	*/

template <class T>
inline TVec3<T>::TVec3(const TVec3<float>& vec) :
	x((T)vec.x),
	y((T)vec.y),
	z((T)vec.z)
{
}

template <class T>
inline TVec3<T>::TVec3(const TVec3<double>& vec) :
	x((T)vec.x),
	y((T)vec.y),
	z((T)vec.z)
{
}
	
template <class T>
inline TVec3<T>::TVec3(const TVec2<T>& vec, T _z) :
	x(vec.x),
	y(vec.y),
	z(_z)
{
}

/*
template <class T>
inline TVec3<T>::TVec3(const Vec4& vec) :
	x(vec.x),
	y(vec.y),
	z(vec.z)
{
}
*/

template <class T>
inline TVec3<T>& TVec3<T>::operator=(const TVec3<T>& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return (*this);
}

template <class T>
inline TVec3<T>& TVec3<T>::operator=(const T* pVec)
{
	x = *pVec++; 
	y = *pVec++;
	z = *pVec;
	return (*this);
}


template <class T>
inline void TVec3<T>::Set(T _x,T _y,T _z)
{
	x = _x;
	y = _y;
	z = _z;
}

template <class T>
inline void TVec3<T>::Set(const T* pVec)
{
	x = *pVec++;
	y = *pVec++;
	z = *pVec;
}

template <class T>
inline T& TVec3<T>::operator[]( u32 _index )
{
	DBG_CHECK( _index < 3 );
	return ((T*)this)[ _index ];
}

template <class T>
inline T TVec3<T>::operator[]( u32 _index ) const
{
	DBG_CHECK( _index < 3 );
	return ( ( const T* )this )[ _index ];
}

template <class T>
inline void TVec3<T>::Normalize()
{
	T sqLength = x*x + y*y + z*z;
	DBG_CHECK( sqLength > (T)0.0 );
	T oneoverlength = (T)1.0 / Sqrt( sqLength );
	x *= oneoverlength;
	y *= oneoverlength;
	z *= oneoverlength;
}


template <class T>
inline void TVec3<T>::CheckForNullAndNormalize()
{
	T sqLength = x*x + y*y + z*z;
	if( sqLength > (T)0.0001 )
	{
		T oneoverlength = (T)1.0 / Sqrt( sqLength );
		x *= oneoverlength;
		y *= oneoverlength;
		z *= oneoverlength;
	}
}

//	inline bool IsNull() {return ((x);}
template <class T>
inline T TVec3<T>::LengthSquared() const
{
	return x*x + y*y + z*z;
}

template <class T>
inline T TVec3<T>::Length() const
{
	return Sqrt( x*x + y*y + z*z ); 
}

template <class T>
inline bool TVec3<T>::IsNormalized() const
{
	return IsEpsilonEqual( LengthSquared(), (T)1.0 ); 
}

template <class T>
inline void TVec3<T>::operator*=(T scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
}

template <class T>
inline void TVec3<T>::operator*=(const TVec3<T>& vec)
{
	x *= vec.x;
	y *= vec.y;
	z *= vec.z;
}

template <class T>
inline void TVec3<T>::operator/=(T scalar)
{
	T oneOverScalar = (T)1.0 / scalar;
	x *= oneOverScalar;
	y *= oneOverScalar;
	z *= oneOverScalar;
}

template <class T>
inline TVec3<T> TVec3<T>::operator*(T scalar) const
{
	return TVec3<T>(x*scalar,y*scalar,z*scalar);
}

template <class T>
inline TVec3<T> TVec3<T>::operator*(const TVec3<T>& vec) const
{
	return TVec3<T>( x*vec.x, y*vec.y, z*vec.z );
}

template <class T>
inline TVec3<T> TVec3<T>::operator/(T scalar) const
{
	T oneOverScalar = (T)1.0 / scalar;

	return TVec3<T>(x*oneOverScalar,y*oneOverScalar,z*oneOverScalar);
}

template <class T>
inline TVec3<T> TVec3<T>::operator/(const TVec3<T>& vec) const
{
	return TVec3<T>( x/vec.x, y/vec.y, z/vec.z );
}

template <class T>
inline void TVec3<T>::operator+=(const TVec3<T>& vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
}

template <class T>
inline void TVec3<T>::operator+=( T scalar )
{
	x += scalar;
	y += scalar;
	z += scalar;
}

template <class T>
inline void TVec3<T>::operator-=(const TVec3<T>& vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
}

template <class T>
inline TVec3<T> TVec3<T>::operator+(const TVec3<T>& vec) const
{
	return TVec3<T>(x+vec.x,y+vec.y,z+vec.z);
}

template <class T>
inline TVec3<T> TVec3<T>::operator+( T scalar ) const
{
	return TVec3<T>( x + scalar, y + scalar, z + scalar );
}

template <class T>
inline TVec3<T> TVec3<T>::operator-(const TVec3<T>& vec) const
{
	return TVec3<T>(x-vec.x,y-vec.y,z-vec.z);
}

template <class T>
inline TVec3<T> TVec3<T>::operator-( T scalar ) const
{
	return TVec3<T>( x - scalar, y - scalar, z - scalar );
}

template <class T>
inline TVec3<T> TVec3<T>::operator-() const
{
	return TVec3<T>(-x,-y,-z);
}

template <class T>
inline bool TVec3<T>::operator==(const TVec3<T>& vec) const
{
	return ( (x==vec.x) && (y==vec.y) && (z==vec.z) );
}

template <class T>
inline bool TVec3<T>::operator<( const TVec3<T>& rhs ) const
{
	//needed to have a vertex in a map.
	//must be an absolute sort so that we can reliably find the exact
	//position again, not a fuzzy compare for equality based on an epsilon.
	if ( x == rhs.x )
	{
		if ( y == rhs.y )
		{
			if ( z == rhs.z )
			{
				return false;
			}
			else
			{
				return ( z < rhs.z );
			}
		}
		else
		{
			return ( y < rhs.y );
		}
	}
	else
	{
		return ( x < rhs.x );
	}
}


template <class T>
inline T TVec3<T>::Dot(const TVec3<T>& vec) const
{
	return x*vec.x + y*vec.y + z*vec.z;
}

template <class T>
inline TVec3<T> TVec3<T>::Cross(const TVec3<T>& vec) const
{
	return TVec3<T>(	y*vec.z-z*vec.y,
					z*vec.x-x*vec.z,
					x*vec.y-y*vec.x);
}


template <class T>
inline void TVec3<T>::Saturate()
{
	x = Clamp( x, (T)0.0, (T)1.0 );
	y = Clamp( y, (T)0.0, (T)1.0 );
	z = Clamp( z, (T)0.0, (T)1.0 );
}


template <class T>
bool TVec3<T>::IsValid() const
{
	#define XTM_MAX_ALLOWABLE_FLOAT 10000000000.0f
	return IsFinite( x ) && IsFinite( y ) && IsFinite( z )
			&& (Abs(x) < XTM_MAX_ALLOWABLE_FLOAT)
			&& (Abs(y) < XTM_MAX_ALLOWABLE_FLOAT)
			&& (Abs(z) < XTM_MAX_ALLOWABLE_FLOAT);
}

template <class T>
void TVec3<T>::SetRandomDir()
{
	x = Random( (T)-1.0, (T)1.0 );
	y = Random( (T)-1.0, (T)1.0 );
	z = Random( (T)-1.0, (T)1.0 );
	Normalize();
}

template <class T>
TVec3<T> SphericalToCartesian( T _longitude, T _latitude, T _radius )
{
	T s1, c1, s2, c2;
	SinCos( _longitude, s1, c1 );
	SinCos( _latitude, s2, c2 );

	return TVec3<T>( _radius * c1 * c2, _radius * s2, _radius * s1 * c2 );
}

template <class T>
T SignedTriangleArea( const TVec3<T>& A, const TVec3<T>& B, const TVec3<T>& C )
{
	TVec3<T> ab( B - A ), ac( C - A );
	return Cross( ab, ac ).Length() / (T)2.0;
}
