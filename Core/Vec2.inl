template <class T>
inline T Dot(const TVec2<T>& vec1, const TVec2<T>& vec2)
{
	return vec1.x*vec2.x + vec1.y*vec2.y;
}

template <class T>
inline T DistanceSquared(const TVec2<T>& vec1, const TVec2<T>& vec2)
{
	//TODO Check assembly code
	TVec2<T> v( vec1 - vec2 );
	return v.LengthSquared();
}

template <class T>
inline T Distance(const TVec2<T>& vec1, const TVec2<T>& vec2)
{
	//TODO Check assembly code
	TVec2<T> v( vec1 - vec2 );
	return v.Length();
}

template <class T>
inline TVec2<T> Normalized(const TVec2<T>& _v)
{
	TVec2<T> r(_v);
	r.Normalize();
	return r;
}

template <class T>
inline TVec2<T> Modulate(const TVec2<T>& a, const TVec2<T>& b)
{
	return TVec2<T>( a.x * b.x, a.y * b.y );
}

template <class T>
inline TVec2<T> Reflect(const TVec2<T>& _v, const TVec2<T>& _normal )
{
	DBG_CHECK( _v.IsNormalized() && _normal.IsNormalized() );

	return _v - _normal * ( 2.0f * Dot( _v, _normal ) );
}

template <class T>
inline TVec2<T>	Floor( const TVec2<T>& _v )
{
	return TVec2<T>( floor( _v.x ), floor( _v.y ) );
}


template <class T>
inline TVec2<T>::TVec2()
{}

template <class T>
inline TVec2<T>::TVec2(const T* pVec) :
	x( *pVec++ ),
	y( *pVec )
{

}

template <class T>
inline TVec2<T>::TVec2(T _x,T _y) :
x( _x ),
y( _y )
{
}

template <class T>
inline TVec2<T>::TVec2(const TVec2<T>& vec)
{
	x = vec.x;
	y = vec.y;
}

template <class T>
inline TVec2<T>::TVec2(const Vec2i& vec)
{
	x = (T)vec.x;
	y = (T)vec.y;
}

template <class T>
inline TVec2<T>& TVec2<T>::operator=(const TVec2<T>& vec)
{
	x = vec.x;
	y = vec.y;
	return (*this);
}

template <class T>
inline TVec2<T>& TVec2<T>::operator=(const T* pVec)
{
	x = *pVec++; 
	y = *pVec;
	return (*this);
}


template <class T>
inline void TVec2<T>::Set(T _x,T _y)
{
	x = _x;
	y = _y;
}

template <class T>
inline void TVec2<T>::Set(const T* pVec)
{
	x = *pVec++;
	y = *pVec;
}

template <class T>
inline void TVec2<T>::Normalize()
{
	T oneoverlength = (T)1.0 / Sqrt(x*x + y*y);
	x *= oneoverlength;
	y *= oneoverlength;
}


//	inline bool IsNull() {return ((x);}
template <class T>
inline T TVec2<T>::LengthSquared() const
{
	return x*x + y*y;
}

template <class T>
inline T TVec2<T>::Length() const
{
	return Sqrt( x*x + y*y ); 
}

template <class T>
inline bool TVec2<T>::IsNormalized() const
{
	return IsEpsilonEqual( LengthSquared(), (T)1.0 ); 
}

/*	inline void Transform(const CMatrix & mat)
{
TVec2<T> vec(	x*mat._11 + y*mat._12 + z*mat._12 + 1.0f*mat._14,
x*mat._21 + y*mat._22 + z*mat._22 + 1.0f*mat._24,
x*mat._21 + y*mat._22 + z*mat._22 + 1.0f*mat._24);
x=vec.x;
y=vec.y;
z=vec.z;
}


inline TVec2<T> Transform(const CMatrix & mat) const
{
return TVec2<T>(x*mat._11 + y*mat._12 + z*mat._12 + 1.0f*mat._14,
x*mat._21 + y*mat._22 + z*mat._22 + 1.0f*mat._24,
x*mat._21 + y*mat._22 + z*mat._22 + 1.0f*mat._24);
//x*mat._41 + y*mat._42 + z*mat._42 + 1.0f*mat._44,
}
*/
template <class T>
inline void TVec2<T>::operator*=(T scalar)
{
	x *= scalar;
	y *= scalar;
}

template <class T>
inline void TVec2<T>::operator/=(T scalar)
{
	T oneOverScalar = (T)1.0 / scalar;
	x *= oneOverScalar;
	y *= oneOverScalar;
}

template <class T>
inline TVec2<T> TVec2<T>::operator*(T scalar) const
{
	return TVec2<T>(x*scalar,y*scalar);
}

template <class T>
inline TVec2<T> TVec2<T>::operator*(const TVec2<T>& vec) const
{
	return TVec2<T>(x*vec.x,y*vec.y);
}

template <class T>
inline TVec2<T> TVec2<T>::operator/(T scalar) const
{
	T oneOverScalar = (T)1.0 / scalar;

	return TVec2<T>(x*oneOverScalar,y*oneOverScalar);
}

template <class T>
inline void TVec2<T>::operator+=(const TVec2<T>& vec)
{
	x += vec.x;
	y += vec.y;
}

template <class T>
inline void TVec2<T>::operator-=(const TVec2<T>& vec)
{
	x -= vec.x;
	y -= vec.y;
}

template <class T>
inline TVec2<T> TVec2<T>::operator+(const TVec2<T>& vec) const
{
	return TVec2<T>(x+vec.x,y+vec.y);
}

template <class T>
inline TVec2<T> TVec2<T>::operator+(T scalar) const
{
	return TVec2<T>(x+scalar,y+scalar);
}

template <class T>
inline TVec2<T> TVec2<T>::operator-(const TVec2<T>& vec) const
{
	return TVec2<T>(x-vec.x,y-vec.y);
}

template <class T>
inline TVec2<T> TVec2<T>::operator-(T scalar) const
{
	return TVec2<T>(x-scalar,y-scalar);
}

template <class T>
inline TVec2<T> TVec2<T>::operator-() const
{
	return TVec2<T>(-x,-y);
}

template <class T>
inline bool TVec2<T>::operator==(const TVec2<T>& vec) const
{
	return ( (x==vec.x) && (y==vec.y) );
}

template <class T>
inline T TVec2<T>::Dot(const TVec2<T>& vec) const
{
	return x*vec.x + y*vec.y;
}
