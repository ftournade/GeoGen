
template <class T>
inline TMat33<T> Inverse( const TMat33<T>& _m )
{
	//TODO optimize
	TMat33<T> inv( _m );
	inv.Inverse();
	return inv;
}

template <class T>
inline TMat33<T> Transpose( const TMat33<T>& _m )
{
	//TODO optimize
	TMat33<T> transposed( _m );
	transposed.Transpose();
	return transposed;
}

template <class T>
inline TMat33<T>::TMat33()
{
}

template <class T>
inline TMat33<T>::TMat33(	T f11,T f12,T f13,
                        T f21,T f22,T f23,
                        T f31,T f32,T f33 )
{
    _11 = f11;  _12 = f12;  _13 = f13;
    _21 = f21;  _22 = f22;  _23 = f23;
    _31 = f31;  _32 = f32;  _33 = f33;
}

template <class T>
inline TMat33<T>& TMat33<T>::operator=( const T* ptr )
{
    _11 = *ptr++;	_12 = *ptr++;	_13 = *ptr++;
    _21 = *ptr++;	_22 = *ptr++;	_23 = *ptr++;
    _31 = *ptr++;	_32 = *ptr++;	_33 = *ptr;
    return (*this);
}


template <class T>
inline void TMat33<T>::Transpose()
{
    TSwap(_12,_21);
    TSwap(_13,_31);

    TSwap(_23,_32);
}


template <class T>
inline TVec3<T> TMat33<T>::GetXAxis() const
{
    return TVec3<T>( _11, _12, _13 );
}

template <class T>
inline TVec3<T> TMat33<T>::GetYAxis() const
{
    return TVec3<T>( _21, _22, _23 );
}

template <class T>
inline TVec3<T> TMat33<T>::GetZAxis() const
{
    return TVec3<T>( _31, _32, _33 );
}


template <class T>
inline void TMat33<T>::SetXAxis( const TVec3<T>& axis )
{
    _11 = axis.x;
    _12 = axis.y;
    _13 = axis.z;
}

template <class T>
inline void TMat33<T>::SetYAxis( const TVec3<T>& axis )
{
    _21 = axis.x;
    _22 = axis.y;
    _23 = axis.z;
}

template <class T>
inline void TMat33<T>::SetZAxis( const TVec3<T>& axis )
{
    _31 = axis.x;
    _32 = axis.y;
    _33 = axis.z;
}


template <class T>
inline T TMat33<T>::GetDeterminant() const
{
    //TODO optimize and SIMDize
    //TODO Doesnt pass unit testing !!!
    return	  _11 * _22 * _33  
            + _12 * _23 
            + _13 * _31 
            + _21 * _32  
            - _23 * _32 
            - _13 * _22 * _31 
            - _12 * _21 
            - _11 * _33;
}


template <class T>
inline TVec3<T> TMat33<T>::Transform( const TVec3<T>& v ) const
{
    return TVec3<T>(	v.x * _11 + v.y * _21 + v.z * _31,
						v.x * _12 + v.y * _22 + v.z * _32,
						v.x * _13 + v.y * _23 + v.z * _33 );
}

template <class T>
inline TVec2<T> TMat33<T>::TransformDirection( const TVec2<T>& v ) const
{
	return TVec2<T>(	v.x * _11 + v.y * _21,
						v.x * _12 + v.y * _22 );
}


template <class T>
inline TVec2<T> TMat33<T>::TransformPosition( const TVec2<T>& v ) const
{
	return TVec2<T>(	v.x * _11 + v.y * _21 + _31,
						v.x * _12 + v.y * _22 + _32 );
}



template <class T>
inline TVec3<T> operator*( const TVec3<T>& v, const TMat33<T>& m )
{
    return m.Transform( v );
}


template <class T>
TMat33<T>::TMat33( const T* ptr )
{
    _11 = *ptr++;	_12 = *ptr++;	_13 = *ptr++;
    _21 = *ptr++;	_22 = *ptr++;	_23 = *ptr++;
    _31 = *ptr++;	_32 = *ptr++;	_33 = *ptr++;
}

template <class T>
void TMat33<T>::Dump() const
{
	LOG( "%f %f %f\n%f %f %f\n%f %f %f",
		_11, _12, _13,
		_21, _22, _23,
		_31, _32, _33 );
}

template <class T>
void TMat33<T>::MakeIdentity()
{
    _11 = 1.0f;	_12 = 0.0f; _13 = 0.0f;
    _21 = 0.0f; _22 = 1.0f; _23 = 0.0f;
    _31 = 0.0f; _32 = 0.0f; _33 = 1.0f;
}


template <class T>
void TMat33<T>::MakeTranslation( const TVec3<T>& tr )
{
	_11 = (T)1.0; _12 = 0.0f;   _13 = 0.0f;
	_21 = 0.0f;   _22 = (T)1.0; _23 = 0.0f;
	_31 = tr.x;   _32 = tr.y;   _33 = tr.z;
}

template <class T>
void TMat33<T>::MakeScaling( const TVec3<T>& sc )
{
    _11 = sc.x;	_12 = 0.0f; _13 = 0.0f;
    _21 = 0.0f; _22 = sc.y; _23 = 0.0f;
    _31 = 0.0f; _32 = 0.0f; _33 = sc.z;
}

template <class T>
void TMat33<T>::MakeRotationX( T angle )
{
    _11 = 1.0f; _12 = 0.0f;        _13 = 0.0f;
    _21 = 0.0f; _22 = Cos(angle);	_23 = -Sin(angle);
    _31 = 0.0f; _32 = Sin(angle);	_33 =  Cos(angle);
}

template <class T>
void TMat33<T>::MakeRotationY( T angle )
{
    _11 =  Cos(angle);	_12 = 0.0f; _13 = Sin(angle);
    _21 = 0.0f;         _22 = 1.0f; _23 = 0.0f;
    _31 = -Sin(angle);	_32 = 0.0f; _33 = Cos(angle);
}

template <class T>
void TMat33<T>::MakeRotationZ( T angle )
{
    _11 = Cos(angle);	_12 = -Sin(angle);	_13 = 0.0f;
    _21 = Sin(angle);	_22 =  Cos(angle);	_23 = 0.0f;
    _31 = 0.0f;			_32 = 0.0f;         _33 = 1.0f;
}

template <class T>
void TMat33<T>::RemoveScaling()
{
    //TODO optimal ?

    TVec3<T> axis( GetXAxis() );
    axis.Normalize();
    SetXAxis( axis );

    axis = GetYAxis();
    axis.Normalize();
    SetYAxis( axis );

    axis = GetZAxis();
    axis.Normalize();
    SetZAxis( axis );
}


template <class T>
TMat33<T> TMat33<T>::operator*( const TMat33<T>& _right ) const
{
	//TODO unit-test against D3DXMatrixMultiply !!
	#define _ROWCOL(i,j) m[i][0]*_right.m[0][j] + m[i][1]*_right.m[1][j] + m[i][2]*_right.m[2][j]
	return TMat33<T>(
	_ROWCOL(0,0), _ROWCOL(0,1), _ROWCOL(0,2),
	_ROWCOL(1,0), _ROWCOL(1,1), _ROWCOL(1,2),
	_ROWCOL(2,0), _ROWCOL(2,1), _ROWCOL(2,2) );
	#undef _ROWCOL
}

template <class T>
bool TMat33<T>::Inverse()
{
	//from www.geometrictools.com

	TMat33<T> old( *this );

	T fC11 = old._22 * old._33 - old._23 * old._32;
	T fC21 = old._23 * old._31 - old._21 * old._33;
	T fC31 = old._21 * old._32 - old._22 * old._31;

	T fDet = old._11 * fC11 + old._12 * fC21 + old._13 * fC31;

	if ( !IsEpsilonNull(fDet) )
	{
		_11 = fC11;
		_12 = old._13 * old._32 - old._12 * old._33;
		_13 = old._12 * old._23 - old._13 * old._22;
		_21 = fC21;
		_22 = old._11 * old._33 - old._13 * old._31;
		_23 = old._13 * old._21 - old._11 * old._23;
		_31 = fC31;
		_32 = old._12 * old._31 - old._11 * old._32;
		_33 = old._11 * old._22 - old._12 * old._21;

		T fInvDet = (T)1.0 / fDet;
		_11 *= fInvDet;
		_12 *= fInvDet;
		_13 *= fInvDet;
		_21 *= fInvDet;
		_22 *= fInvDet;
		_23 *= fInvDet;
		_31 *= fInvDet;
		_32 *= fInvDet;
		_33 *= fInvDet;
	}
	else
	{
		return false;
	}

	return true;
}
