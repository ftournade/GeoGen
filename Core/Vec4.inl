
//Constants
template <class T> const TVec4<T> TVec4<T>::XAxis( 1.0f, 0.0f, 0.0f, 0.0f );
template <class T> const TVec4<T> TVec4<T>::YAxis( 0.0f, 1.0f, 0.0f, 0.0f );
template <class T> const TVec4<T> TVec4<T>::ZAxis( 0.0f, 0.0f, 1.0f, 0.0f );
template <class T> const TVec4<T> TVec4<T>::Origin( 0.0f, 0.0f, 0.0f, 1.0f );
template <class T> const TVec4<T> TVec4<T>::Zero( 0.0f, 0.0f, 0.0f, 0.0f );
template <class T> const TVec4<T> TVec4<T>::One( 1.0f, 1.0f, 1.0f, 1.0f );
template <class T> const TVec4<T> TVec4<T>::Min( -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );
template <class T> const TVec4<T> TVec4<T>::Max(  FLT_MAX,  FLT_MAX,  FLT_MAX,  FLT_MAX );


//Code
template <class T>
inline T Dot(const TVec4<T>& vec1, const TVec4<T>& vec2)
{
	return vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z + vec1.w*vec2.w;
}

/*
inline TVec4<T> Cross(const TVec4<T>& vec1, const TVec4<T>& vec2)
{
	return TVec4<T>(	vec1.y*vec2.z - vec1.z*vec2.y,
					vec1.z*vec2.x - vec1.x*vec2.z,
					vec1.x*vec2.y - vec1.y*vec2.x);
}
*/

template <class T>
inline T DistanceSquared(const TVec4<T>& vec1, const TVec4<T>& vec2)
{
	//TODO Check assembly code
	TVec4<T> v( vec1 - vec2 );
	return v.LengthSquared();
}

template <class T>
inline T Distance(const TVec4<T>& vec1, const TVec4<T>& vec2)
{
	//TODO Check assembly code
	TVec4<T> v( vec1 - vec2 );
	return v.Length();
}

template <class T>
inline TVec4<T> Modulate(const TVec4<T>& a, const TVec4<T>& b)
{
	return TVec4<T>( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w );
}

template <class T>
inline TVec4<T> Pow( const TVec4<T>& _v, T _f )
{
	return TVec4<T>( Pow( _v.x, _f ), Pow( _v.y, _f ), Pow( _v.z, _f ), Pow( _v.w, _f ) );
}

template <class T>
inline TVec4<T> Abs( const TVec4<T>& _v )
{
	return TVec4<T>( Abs( _v.x ), Abs( _v.y ), Abs( _v.z ), Abs( _v.w ) );
}

//////////////

template <class T>
inline TVec4<T>::TVec4()
{}

template <class T>
inline TVec4<T>::TVec4(const T* pVec) :
	x( *pVec++ ),
	y( *pVec++ ),
	z( *pVec++ ),
	w( *pVec )
{

}

template <class T>
inline TVec4<T>::TVec4(T _x,T _y,T _z,T _w) :
x( _x ),
y( _y ),
z( _z ),
w( _w )
{
}

template <class T>
inline TVec4<T>::TVec4(const TVec4<T>& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	w = vec.w;
}

template <class T>
inline TVec4<T>::TVec4(const TVec3<T>& vec, T _w)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	w = _w;
}

template <class T>
inline TVec4<T>& TVec4<T>::operator=(const TVec4<T>& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	w = vec.w;
	return (*this);
}

template <class T>
inline TVec4<T>& TVec4<T>::operator=(const T* pVec)
{
	x = *pVec++; 
	y = *pVec++;
	z = *pVec++;
	w = *pVec;
	return (*this);
}

template <class T>
inline TVec3<T>	TVec4<T>::xyz() const
{
	return TVec3<T>( x, y, z );
}

template <class T>
inline void TVec4<T>::Set(T _x,T _y,T _z,T _w)
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

template <class T>
inline void TVec4<T>::Set(const T* pVec)
{
	x = *pVec++;
	y = *pVec++;
	z = *pVec++;
	w = *pVec;
}

template <class T>
inline void TVec4<T>::Normalize()
{
	T oneoverlength = 1.0f / (T)Sqrt(x*x + y*y + z*z + w*w);
	x *= oneoverlength;
	y *= oneoverlength;
	z *= oneoverlength;
	w *= oneoverlength;
}


//	inline bool IsNull() {return ((x);}
template <class T>
inline T TVec4<T>::LengthSquared() const
{
	return x*x + y*y + z*z + w*w;
}

template <class T>
inline T TVec4<T>::Length() const
{
	return (T)Sqrt( x*x + y*y + z*z + w*w ); 
}

/*	inline void Transform(const CMatrix & mat)
{
TVec4<T> vec(	x*mat._11 + y*mat._12 + z*mat._12 + 1.0f*mat._14,
x*mat._21 + y*mat._22 + z*mat._22 + 1.0f*mat._24,
x*mat._21 + y*mat._22 + z*mat._22 + 1.0f*mat._24);
x=vec.x;
y=vec.y;
z=vec.z;
}


inline TVec4<T> Transform(const CMatrix & mat) const
{
return TVec4<T>(x*mat._11 + y*mat._12 + z*mat._12 + 1.0f*mat._14,
x*mat._21 + y*mat._22 + z*mat._22 + 1.0f*mat._24,
x*mat._21 + y*mat._22 + z*mat._22 + 1.0f*mat._24);
//x*mat._41 + y*mat._42 + z*mat._42 + 1.0f*mat._44,
}
*/
template <class T>
inline void TVec4<T>::operator*=(T scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
}

template <class T>
inline void TVec4<T>::operator/=(T scalar)
{
	T oneOverScalar=1.0f/scalar;
	x *= oneOverScalar;
	y *= oneOverScalar;
	z *= oneOverScalar;
	w *= oneOverScalar;
}

template <class T>
inline void TVec4<T>::operator/=(const TVec4<T>& _vec)
{
	x /= _vec.x;
	y /= _vec.y;
	z /= _vec.z;
	w /= _vec.w;
}

template <class T>
inline TVec4<T> TVec4<T>::operator*(T scalar) const
{
	return TVec4<T>( x*scalar, y*scalar, z*scalar, w*scalar );
}

template <class T>
inline TVec4<T> TVec4<T>::operator*(const TVec4<T>& _vec) const
{	
	return TVec4<T>( x * _vec.x, y * _vec.y, z * _vec.z, w * _vec.w );
}

template <class T>
inline TVec4<T> TVec4<T>::operator/(T scalar) const
{
	T oneOverScalar=1.0f/scalar;

	return TVec4<T>(x*oneOverScalar,y*oneOverScalar,z*oneOverScalar,w*oneOverScalar);
}

template <class T>
inline void TVec4<T>::operator+=(const TVec4<T>& vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
	w += vec.w;
}


template <class T>
inline void TVec4<T>::operator-=(const TVec4<T>& vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	w -= vec.w;
}

template <class T>
inline TVec4<T> TVec4<T>::operator+(const TVec4<T>& vec) const
{
	return TVec4<T>(x+vec.x,y+vec.y,z+vec.z,w+vec.w);
}

template <class T>
inline TVec4<T> TVec4<T>::operator-(const TVec4<T>& vec) const
{
	return TVec4<T>(x-vec.x,y-vec.y,z-vec.z,w-vec.w);
}

template <class T>
inline TVec4<T> TVec4<T>::operator-() const
{
	return TVec4<T>(-x,-y,-z,-w);
}

template <class T>
inline bool TVec4<T>::operator==(const TVec4<T>& vec) const
{
	return ( (x==vec.x) && (y==vec.y) && (z==vec.z) && (w==vec.w) );
}

template <class T>
inline T TVec4<T>::Dot(const TVec4<T>& vec) const
{
	return x*vec.x + y*vec.y + z*vec.z + w*vec.w;
}
/*
template <class T>
inline TVec4<T> TVec4<T>::Cross(const TVec4<T>& vec) const
{
	return TVec4<T>(	y*vec.z-z*vec.y,
					z*vec.x-x*vec.z,
					x*vec.y-y*vec.x);
}
*/
