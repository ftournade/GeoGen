
template <class T>
inline TMat44<T> Inverse( const TMat44<T>& _m )
{
	//TODO optimize
	TMat44<T> inv( _m );
	inv.Inverse();
	return inv;
}

template <class T>
inline TMat44<T> Transpose( const TMat44<T>& _m )
{
	//TODO optimize
	TMat44<T> transposed( _m );
	transposed.Transpose();
	return transposed;
}

template <class T>
inline			TMat44<T>::TMat44()
{
}

template <class T>
inline			TMat44<T>::TMat44(	T f11,T f12,T f13,T f14,
									 T f21,T f22,T f23,T f24,
									 T f31,T f32,T f33,T f34,
									 T f41,T f42,T f43,T f44 )
{
	_11 = f11;  _12 = f12;  _13 = f13;  _14 = f14;  
	_21 = f21;  _22 = f22;  _23 = f23;  _24 = f24;  
	_31 = f31;  _32 = f32;  _33 = f33;  _34 = f34;  
	_41 = f41;  _42 = f42;  _43 = f43;  _44 = f44;
}

template <class T>
inline TMat44<T>::TMat44( const TMat33<T>& _rot, const TVec3<T>& _trans )
{
	_11 = _rot._11;  _12 = _rot._12;  _13 = _rot._13;  _14 = 0.0f;  
	_21 = _rot._21;  _22 = _rot._22;  _23 = _rot._23;  _24 = 0.0f;  
	_31 = _rot._31;  _32 = _rot._32;  _33 = _rot._33;  _34 = 0.0f;  
	_41 = _trans.x;  _42 = _trans.y;  _43 = _trans.z;  _44 = (T)1.0;
	
}

template <class T>
inline TMat44<T>& TMat44<T>::operator=( const T* ptr )
{
	_11 = *ptr++;	_12 = *ptr++;	_13 = *ptr++;	_14 = *ptr++;
	_21 = *ptr++;	_22 = *ptr++;	_23 = *ptr++;	_24 = *ptr++;
	_31 = *ptr++;	_32 = *ptr++;	_33 = *ptr++;	_34 = *ptr++;
	_41 = *ptr++;	_42 = *ptr++;	_43 = *ptr++;	_44 = *ptr;
	return (*this);
}

template <class T>
inline TVec3<T> TMat44<T>::GetTranslation() const
{
	return TVec3<T>( _41, _42, _43 );
}

template <class T>
inline void TMat44<T>::SetTranslation( const TVec3<T>& t )
{
	_41 = t.x;
	_42 = t.y;
	_43 = t.z;
	_44 = (T)1.0;
}


template <class T>
inline TVec3<T> TMat44<T>::GetXAxis() const
{
	return TVec3<T>( _11, _12, _13 );
}

template <class T>
inline TVec3<T> TMat44<T>::GetYAxis() const
{
	return TVec3<T>( _21, _22, _23 );
}

template <class T>
inline TVec3<T> TMat44<T>::GetZAxis() const
{
	return TVec3<T>( _31, _32, _33 );
}


template <class T>
inline void TMat44<T>::SetXAxis( const TVec3<T>& axis )
{
	_11 = axis.x;
	_12 = axis.y;
	_13 = axis.z;
	_14 = 0.0f;
}

template <class T>
inline void TMat44<T>::SetYAxis( const TVec3<T>& axis )
{
	_21 = axis.x;
	_22 = axis.y;
	_23 = axis.z;
	_24 = 0.0f;
}

template <class T>
inline void TMat44<T>::SetZAxis( const TVec3<T>& axis )
{
	_31 = axis.x;
	_32 = axis.y;
	_33 = axis.z;
	_34 = 0.0f;
}


template <class T>
inline T TMat44<T>::GetDeterminant() const
{
	//TODO optimize and SIMDize
	//TODO Doesnt pass unit testing !!!
	return	  _11 * _22 * _33 * _44 
			+ _12 * _23 * _34 * _41
			+ _13 * _24 * _31 * _42
			+ _14 * _21 * _32 * _43 
			- _14 * _23 * _32 * _41
			- _13 * _22 * _31 * _44
			- _12 * _21 * _34 * _43
			- _11 * _24 * _33 * _42;
}

template <class T>
inline TVec4<T> TMat44<T>::Transform( const TVec4<T>& v ) const
{
	return TVec4<T>(	v.x * _11 + v.y * _21 + v.z * _31 + v.w * _41,
					v.x * _12 + v.y * _22 + v.z * _32 + v.w * _42,
					v.x * _13 + v.y * _23 + v.z * _33 + v.w * _43,
					v.x * _14 + v.y * _24 + v.z * _34 + v.w * _44 );
}


template <class T>
inline TVec3<T> TMat44<T>::TransformDirection( const TVec3<T>& v ) const
{
	return TVec3<T>(	v.x * _11 + v.y * _21 + v.z * _31,
					v.x * _12 + v.y * _22 + v.z * _32,
					v.x * _13 + v.y * _23 + v.z * _33 );
}


template <class T>
inline TVec3<T> TMat44<T>::TransformPosition( const TVec3<T>& v ) const
{
	return TVec3<T>(	v.x * _11 + v.y * _21 + v.z * _31 + _41,
					v.x * _12 + v.y * _22 + v.z * _32 + _42,
					v.x * _13 + v.y * _23 + v.z * _33 + _43 );
}

template <class T>
inline TVec3<T> operator*( const TVec3<T>& v, const TMat44<T>& m )
{
	return m.TransformPosition( v );
}

template <class T>
inline TVec4<T> operator*( const TVec4<T>& v, const TMat44<T>& m )
{
	return m.Transform( v );
}


template <class T>
TMat44<T>::TMat44( const T* ptr )
{
	_11 = *ptr++;	_12 = *ptr++;	_13 = *ptr++;	_14 = *ptr++;
	_21 = *ptr++;	_22 = *ptr++;	_23 = *ptr++;	_24 = *ptr++;
	_31 = *ptr++;	_32 = *ptr++;	_33 = *ptr++;	_34 = *ptr++;
	_41 = *ptr++;	_42 = *ptr++;	_43 = *ptr++;	_44 = *ptr;
}
	
template <class T>
void TMat44<T>::MakeIdentity()
{
	_11 = (T)1.0;	_12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
	_21 = 0.0f; _22 = (T)1.0; _23 = 0.0f; _24 = 0.0f;
	_31 = 0.0f; _32 = 0.0f; _33 = (T)1.0; _34 = 0.0f;
	_41 = 0.0f; _42 = 0.0f; _43 = 0.0f; _44 = (T)1.0;
}

template <class T>
void TMat44<T>::MakeTranslation( const TVec3<T>& tr )
{
	_11 = (T)1.0;	_12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
	_21 = 0.0f; _22 = (T)1.0; _23 = 0.0f; _24 = 0.0f;
	_31 = 0.0f; _32 = 0.0f; _33 = (T)1.0; _34 = 0.0f;
	_41 = tr.x; _42 = tr.y; _43 = tr.z; _44 = (T)1.0;
}

template <class T>
void TMat44<T>::MakeScaling( const TVec3<T>& sc )
{
	_11 = sc.x;	_12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
	_21 = 0.0f; _22 = sc.y; _23 = 0.0f; _24 = 0.0f;
	_31 = 0.0f; _32 = 0.0f; _33 = sc.z; _34 = 0.0f;
	_41 = 0.0f; _42 = 0.0f; _43 = 0.0f; _44 = (T)1.0;
}

template <class T>
void TMat44<T>::MakeRotationX( T angle )
{
	T s, c;
	SinCos( angle, s, c );

	_11 = (T)1.0; _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
	_21 = 0.0f; _22 = c;	_23 = -s;   _24 = 0.0f;
	_31 = 0.0f; _32 = s;	_33 =  c;   _34 = 0.0f;
	_41 = 0.0f; _42 = 0.0f; _43 = 0.0f; _44 = (T)1.0;
}

template <class T>
void TMat44<T>::MakeRotationY( T angle )
{
	T s, c;
	SinCos( angle, s, c );

	_11 = c;	_12 = 0.0f; _13 = s;	_14 = 0.0f;
	_21 = 0.0f; _22 = (T)1.0; _23 = 0.0f;	_24 = 0.0f;
	_31 = -s;	_32 = 0.0f; _33 = c;	_34 = 0.0f;
	_41 = 0.0f; _42 = 0.0f; _43 = 0.0f;	_44 = (T)1.0;
}

template <class T>
void TMat44<T>::MakeRotationZ( T angle )
{
	T s, c;
	SinCos( angle, s, c );

	_11 = c;	_12 = -s;	_13 = 0.0f; _14 = 0.0f;
	_21 = s;	_22 = c;	_23 = 0.0f; _24 = 0.0f;
	_31 = 0.0f;	_32 = 0.0f; _33 = (T)1.0; _34 = 0.0f;
	_41 = 0.0f;	_42 = 0.0f; _43 = 0.0f; _44 = (T)1.0;
}

template <class T>
void TMat44<T>::MakeRotation( const TVec3<T>& axis, T angle )
{
	//TODO unit test agains D3DXMatrixRotationAxis

	T s, c;
	SinCos( angle, s, c );

	T omc = (T)1.0 - c;

	T xomc = axis.x * omc;
	T yomc = axis.y * omc;
	T zomc = axis.z * omc;

	T xxomc = axis.x * xomc;
	T xyomc = axis.x * yomc;
	T xzomc = axis.x * zomc;
	T yyomc = axis.y * yomc;
	T yzomc = axis.y * zomc;
	T zzomc = axis.z * zomc;

	T xs = axis.x * s;
	T ys = axis.y * s;
	T zs = axis.z * s;

	_11 = xxomc + c;  _12 = xyomc + zs;	_13 = xzomc - ys; _14 = 0.0f;
	_21 = xyomc - zs; _22 = yyomc + c;	_23 = yzomc + xs; _24 = 0.0f;
	_31 = xzomc + ys; _32 = yzomc - xs;	_33 = zzomc + c;  _34 = 0.0f;
	_41 = 0.0f;	      _42 = 0.0f;       _43 = 0.0f;       _44 = (T)1.0;

}

template <class T>
void TMat44<T>::MakeRotationYawPitchRoll( T _yaw, T _pitch, T _roll )
{
	T yc,ys, pc, ps, rc, rs;
	SinCos( _yaw, ys, yc );
	SinCos( _pitch, ps, pc );
	SinCos( _roll, rs, rc );

	_11 = rc * yc - rs * ps * ys;
	_12 = -rs * pc;
	_13 = rc * ys + rs * ps * yc;
	_14 = (T)0.0;

	_21 = rs * yc + rc * ps * ys;
	_22 = rc * pc;
	_23 = rs * ys - rc * ps * yc;
	_24 = (T)0.0;

	_31 = -pc * ys;
	_32 = ps;
	_33 = pc * yc;
	_34 = (T)0.0;

	_41 = 0.0f;
	_42 = 0.0f;
	_43 = 0.0f;
	_44 = (T)1.0;

}

template <class T>
void TMat44<T>::GetYawPitchRoll( T& _yaw, T& _pitch, T& _roll ) const
{

/*	_yaw = _13;
	_pitch = - atan2f( _23, Sqrt( _21*_21 + _22*_22 ) );
	_roll = atan2f( -_21, _22 );
	*/
	/*
	//See http://planning.cs.uiuc.edu/node103.html

	_yaw = atan2f( _12, _11 );
	_pitch = atan2f( -_13, Sqrt( _23*_23 + _33*_33 ) );
	_roll = atan2f( _23, _33 );
		*/
	if( GetYAxis().y > 0.0f )
	{
		_pitch = ASin( GetZAxis().y );
		_roll = ASin( GetXAxis().y );
		_yaw = 0.0f;
	}
	else
	{
		_pitch = ASin( GetZAxis().y );
		_roll = - ASin( GetXAxis().y );
		_yaw = 0.0f;
	}
}


template <class T>
void TMat44<T>::MakePerspectiveProjection( T _verticalFov, T _aspectRatio, T _zNear, T _zFar )
{
	T yScale = (T)1.0 / Tan( _verticalFov * 0.5f );
	T Q = _zFar / ( _zFar - _zNear );

#if 1//def XTM_GL
	T xScale = - yScale / _aspectRatio;
#else
	T xScale = yScale / _aspectRatio;
#endif
	_11 = xScale;	_12 = 0.0f;		_13 = 0.0f;			_14 = 0.0f;
	_21 = 0.0f;		_22 = yScale;	_23 = 0.0f;			_24 = 0.0f;
#if 1 //Left Handed
	_31 = 0.0f;		_32 = 0.0f;		_33 = Q;			_34 = (T)1.0;
#else //Right Handed
	_31 = 0.0f;		_32 = 0.0f;		_33 = -Q;			_34 = (T)-1.0;
#endif
	_41 = 0.0f;		_42 = 0.0f;		_43 = -_zNear * Q;	_44 = 0.0f;
}

template <class T>
bool TMat44<T>::RetrieveProjectionParameters( bool& _isOrthographic, T& _verticalFov, T& _aspectRatio, T& _zNear, T& _zFar ) const
{
	T Q = _33;

	if( Abs( Q ) < XTM_EPSILON ) //Avoid division by 0
		return false;

	_zNear = - _43 / Q;

		

	if( _44 == 0.0f ) //Note this is volontary to test equality without epsilon here
	{
		_isOrthographic = false;
		DBG_CHECK( Abs( Q - (T)1.0 ) > 0.00000001f ); //Avoid division by 0
		_zFar = Q * _zNear / ( Q - (T)1.0 );
	}
	else
	{
		_isOrthographic = true;
		DBG_CHECK( Abs( Q ) > 0.00000001f ); //Avoid division by 0
		_zFar = ( (T)1.0 + Q *_zNear) / Q;	
	}

#if 1//def XTM_GL
	_aspectRatio = - _22 / _11;
#else
	_aspectRatio = _22 / _11;
#endif

	_verticalFov = (T)2.0 * ATan( (T)1.0 / _22 );

	return true;
}

template <class T>
void TMat44<T>::MakeOrthoProjection( T _width, T _height, T _zNear, T _zFar )
{
	T Q = (T)1.0 / ( _zFar - _zNear );
		
	_11 = (T)2.0 / _width;	_12 = 0.0f;				_13 = 0.0f;			_14 = 0.0f;
	_21 = 0.0f;				_22 = (T)-2.0 / _height;_23 = 0.0f;			_24 = 0.0f;
	_31 = 0.0f;				_32 = 0.0f;				_33 = Q;			_34 = 0.0f;
	_41 = 0.0f;				_42 = 0.0f;				_43 = -_zNear * Q;	_44 = (T)1.0;		
}

template <class T>
void TMat44<T>::MakeOrthoProjection( T l, T r, T t, T b, T _zNear, T _zFar )
{
	//TODO y may need to be inverted

	T Q = (T)1.0 / ( _zFar - _zNear );
		
	_11 = (T)2.0 / (r - l);	_12 = 0.0f;				_13 = 0.0f;			_14 = 0.0f;
	_21 = 0.0f;				_22 = (T)2.0 / (t - b);	_23 = 0.0f;			_24 = 0.0f;
	_31 = 0.0f;				_32 = 0.0f;				_33 = Q;			_34 = 0.0f;
	_41 = (l + r)/(l - r);	_42 = (t + b)/(b - t);	_43 = -_zNear * Q;	_44 = (T)1.0;		
}


template <class T>
void TMat44<T>::MakeLookAt( const TVec3<T>& pos, const TVec3<T>& target, const TVec3<T>& up )
{
#if 1 //def XTM_GL
	TVec3<T> zaxis = Normalized( target - pos );
	TVec3<T> xaxis = Normalized( Cross( up, zaxis ) );
	TVec3<T> yaxis = Cross( zaxis, xaxis );

	_11 = xaxis.x;	_12 = yaxis.x;	_13 = zaxis.x;	_14 = 0.0f;
	_21 = xaxis.y;	_22 = yaxis.y;	_23 = zaxis.y;	_24 = 0.0f;
	_31 = xaxis.z;	_32 = yaxis.z;	_33 = zaxis.z;	_34 = 0.0f;
	_41 = -Dot(xaxis, pos);
	_42 = -Dot(yaxis, pos);
	_43 = -Dot(zaxis, pos);
	_44 = (T)1.0;
#else
#pragma error("test TMat44<T>::MakeLookAt")
#endif
/*
#ifdef XTM_D3D
	D3DXMatrixLookAtLH(	(D3DXMATRIX*)this,
						(const D3DXVECTOR3*)&pos,
						(const D3DXVECTOR3*)&target,
						(const D3DXVECTOR3*)&up );
#endif
*/
}

template <class T>
void TMat44<T>::MakePlanarProjectionOmni( const Plane<T>& _plane, const TVec3<T>& _pos )
{
	//See http://www.devmaster.net/articles/shadowprojection/

	// L light pos
	// V vertex to be projected
	// P projected vertex
	// N plane normal
	// d plane dist
	//
	// (1) Line Equation:  P = L + (V - L) * t
	// (2) Plane equation: P.N + d = 0
	//
	// (1) & (2) =>  (L + (V - L) * t).N + d = 0
	// N.(V - L) * t + d + N.L = 0
	// t = - (d + N.L) / N.(V - L)
	// (note if t < 0 we are back firing)
	//
	// let z = N.L
	// let a = - (d + N.L) = - (d + z)

	// t = a / N.(V - L)
	// P = L + a * (V - L) / N.(V - L)
	// P = L + a * (V - L) / ( N.V - z )
	//
	// Let's develop ... (Calculations are similar for Py & Pz)
	//
	// Px = Lx + a * (Vx - Lx) / ( Nx*Vx + Ny*Vy + Nz*Vz - z )
	// Px = ( Lx * ( Nx*Vx + Ny*Vy + Nz*Vz - z ) + a * (Vx - Lx)     ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z ) 
	// Px = ( Lx*Nx*Vx + Lx*Ny*Vy + Lx*Nz*Vz - Lx*z + a*Vx - a*Lx)   ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z ) 
	// Px = ( (Lx*Nx + a)*Vx + (Lx*Ny)*Vy + (Lx*Nz)*Vz - Lx *(z + a) ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z )
	// Px = ( (Lx*Nx + a)*Vx + (Lx*Ny)*Vy + (Lx*Nz)*Vz + Lx * d      ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z )
		
	// Py = ( Ly * ( Nx*Vx + Ny*Vy + Nz*Vz - z ) + a * (Vy - Ly)     ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z ) 
	// Py = ( Ly*Nx*Vx + Ly*Ny*Vy + Ly*Nz*Vz - Ly*z + a*Vy - a*Ly)   ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z ) 
	// Py = ( (Ly*Nx)*Vx + (Ly*Ny + a)*Vy + (Ly*Nz)*Vz - Ly *(z + a) ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z )
	// Py = ( (Ly*Nx)*Vx + (Ly*Ny + a)*Vy + (Ly*Nz)*Vz + Ly * d      ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z )
		
	//...
	// Px = ( (Lx*Nx + a)*Vx + (Lx*Ny    )*Vy + (Lx*Nz    )*Vz + Lx * d  ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z )
	// Py = ( (Ly*Nx    )*Vx + (Ly*Ny + a)*Vy + (Ly*Nz    )*Vz + Ly * d  ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z )
	// Pz = ( (Lz*Nx    )*Vx + (Lz*Ny    )*Vy + (Lz*Nz + a)*Vz + Lz * d  ) / ( Nx*Vx + Ny*Vy + Nz*Vz - z )
	//
	// This translates into the matrix M where P = V * M
	//		|	Lx*N.x + a		Lx*Ny		L.x*N.z		L.x * d		|   
	// M =  |	Ly*N.x			Ly*Ny + a	L.y*N.z		L.y * d		|
	//   	|	Lz*N.x			Lz*Ny		L.z*N.z + a	L.z * d		|
	//		|	N.x				N.y			N.z			-z			|

	T d = (T)_plane.m_Dist;
	T z = Dot( _plane.m_Normal, _pos );
	T a = - d - z;
 
	const TVec3<T>& L = _pos;
	const TVec3<T>& N = _plane.m_Normal;

	_11 = L.x*N.x + a;		_21 = L.x*N.y;		_31 = L.x*N.z;		_41 = L.x * d;
	_12 = L.y*N.x;			_22 = L.y*N.y + a;	_32 = L.y*N.z;		_42 = L.y * d;
	_13 = L.z*N.x;			_23 = L.z*N.y;		_33 = L.z*N.z + a;	_43 = L.z * d;
	_14 = N.x;				_24 = N.y;			_34 = N.z;			_44 = -z;
		
}

template <class T>
void TMat44<T>::MakePlanarProjectionDir( const Plane<T>& _plane, const TVec3<T>& _dir )
{
	// D light dir
	// V vertex to be projected
	// P projected vertex
	// N plane normal
	// d plane dist
	//
	// (1) Line Equation:  P = V + D * t
	// (2) Plane equation: P.N + d = 0
	//
	// (1) & (2) =>  (V + D * t).N + d = 0
	// N.V + N.D * t + d = 0
	// t = - (d + N.V) / N.D
	// (note if t < 0 we are back firing)
	//
	// let a = 1 / N.D

	// P = V + D * t = V - D * (d + N.V) * a
	//
	// Let's develop ... (Calculations are similar for Py & Pz)
	//
	// Px = Vx - Dx * ( d + Nx*Vx + Ny*Vy + Nz*Vz ) * a
	// Px = Vx*(1-Dx*Nx*a) + Vy*( -Dx*Ny*a) + Vz*( -Dx*Nz*a) - Dx*d*a
	//...
	// Py = Vx*( -Dy*Nx*a) + Vy*(1-Dy*Ny*a) + Vz*( -Dy*Nz*a) - Dy*d*a
	// Pz = Vx*( -Dz*Nx*a) + Vy*( -Dz*Ny*a) + Vz*(1-Dz*Nz*a) - Dz*d*a
		

	T dotND = Dot( _plane.m_Normal, _dir );

	if( IsEpsilonNull( dotND ) )
	{
		//TODO ...
		CHECK( false );
		return;
	}

	T a = (T)1.0 / dotND;
		
	const TVec3<T>& D = _dir;
	TVec3<T> N = _plane.m_Normal * a;
	T d = (T)_plane.m_Dist * a;

	_11 = (T)1.0 - D.x*N.x;	_21 =        - D.x*N.y;	_31 =        - D.x*N.z;		_41 = -D.x * d;
	_12 =        - D.y*N.x;	_22 = (T)1.0 - D.y*N.y;	_32 =        - D.y*N.z;		_42 = -D.y * d;
	_13 =        - D.z*N.x;	_23 =        - D.z*N.y;	_33 = (T)1.0 - D.z*N.z;		_43 = -D.z * d;
	_14 = 0.0f;				_24 = 0.0f;				_34 = 0.0f;					_44 = (T)1.0;
}


template <class T>
bool TMat44<T>::IsOrthonormal() const
{
	if( !IsEpsilonEqual( GetXAxis().LengthSquared(), (T)1.0 ) ||
		!IsEpsilonEqual( GetYAxis().LengthSquared(), (T)1.0 ) ||
		!IsEpsilonEqual( GetZAxis().LengthSquared(), (T)1.0 ) )
		return false;

	T d1 = Dot( Cross( GetXAxis(), GetYAxis() ), GetZAxis() );
	T d2 = Dot( Cross( GetYAxis(), GetZAxis() ), GetXAxis() );

	if( !IsEpsilonEqual( d1, (T)1.0 ) ||
		!IsEpsilonEqual( d2, (T)1.0 ) )
		return false;

	return true;
}
	
template <class T>
bool TMat44<T>::Orthonormalize()
{
	//TODO enforce orthogonality (left hand ? right hand ?)
	//TODO optimize
	SetXAxis( Normalized( GetXAxis() ) );
	SetYAxis( Normalized( GetYAxis() ) );
	SetZAxis( Cross( GetXAxis(), GetYAxis() ) );
		
	return true;
}
	

template <class T>
void TMat44<T>::RemoveScaling()
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
void TMat44<T>::Mirror( const Plane<T>& _plane )
{
	SetXAxis( _plane.MirrorPoint( GetXAxis() ) );
	SetYAxis( _plane.MirrorPoint( GetYAxis() ) );
	SetZAxis( _plane.MirrorPoint( GetZAxis() ) );
	SetTranslation( _plane.MirrorPoint( GetTranslation() ) );
}

template <class T>
bool TMat44<T>::operator==( const TMat44& _right ) const
{
	#define _ROWCOL(i,j) IsEpsilonEqual( m[i][j], _right.m[i][j] )

	return	_ROWCOL(0,0)&&_ROWCOL(0,1)&&_ROWCOL(0,2)&&_ROWCOL(0,3)&&
			_ROWCOL(1,0)&&_ROWCOL(1,1)&&_ROWCOL(1,2)&&_ROWCOL(1,3)&&
			_ROWCOL(2,0)&&_ROWCOL(2,1)&&_ROWCOL(2,2)&&_ROWCOL(2,3)&&
			_ROWCOL(3,0)&&_ROWCOL(3,1)&&_ROWCOL(3,2)&&_ROWCOL(3,3);

	#undef _ROWCOL
}

template <class T>
TMat44<T> TMat44<T>::operator*( const TMat44<T>& _right ) const
{
#if 1
	//TODO unit-test against D3DXMatrixMultiply !!
	#define _ROWCOL(i,j) m[i][0]*_right.m[0][j] + m[i][1]*_right.m[1][j] + m[i][2]*_right.m[2][j] + m[i][3]*_right.m[3][j]
	return TMat44(
	_ROWCOL(0,0), _ROWCOL(0,1), _ROWCOL(0,2), _ROWCOL(0,3),
	_ROWCOL(1,0), _ROWCOL(1,1), _ROWCOL(1,2), _ROWCOL(1,3),
	_ROWCOL(2,0), _ROWCOL(2,1), _ROWCOL(2,2), _ROWCOL(2,3),
	_ROWCOL(3,0), _ROWCOL(3,1), _ROWCOL(3,2), _ROWCOL(3,3) );
	#undef _ROWCOL
#else
	TMat44 res;
	D3DXMatrixMultiply( (D3DXMATRIX*)&res, (const D3DXMATRIX*)this, (const D3DXMATRIX*)&_right );
	return res;
#endif
/*
#ifdef _DEBUG
	TMat44 res2;
	D3DXMatrixMultiply( (D3DXMATRIX*)&res2, (const D3DXMATRIX*)this, (const D3DXMATRIX*)&_right );
	DBG_CHECK( res2 == res1 );
#endif
	return res1;
*/
}


template <class T>
void TMat44<T>::operator*=( const TMat44<T>& _right )
{
	//TODO unit-test against D3DXMatrixMultiply !!
	TMat44<T> c( *this );

	#define _ROWCOL(i,j) m[i][j] = c.m[i][0]*_right.m[0][j] + c.m[i][1]*_right.m[1][j] + c.m[i][2]*_right.m[2][j] + c.m[i][3]*_right.m[3][j]
	
	_ROWCOL(0,0); _ROWCOL(0,1); _ROWCOL(0,2); _ROWCOL(0,3);
	_ROWCOL(1,0); _ROWCOL(1,1); _ROWCOL(1,2); _ROWCOL(1,3);
	_ROWCOL(2,0); _ROWCOL(2,1); _ROWCOL(2,2); _ROWCOL(2,3);
	_ROWCOL(3,0); _ROWCOL(3,1); _ROWCOL(3,2); _ROWCOL(3,3);

	#undef _ROWCOL

}

template <class T>
void TMat44<T>::Transpose()
{
	TSwap(_12,_21);
	TSwap(_13,_31);
	TSwap(_14,_41);

	TSwap(_23,_32);
	TSwap(_24,_42);

	TSwap(_34,_43);
}

template <class T>
bool TMat44<T>::GetFrustumCorners( TVec3<T>* _pCorners ) const
{
	TMat44 invMat( *this );

	if( !invMat.Inverse() )
		return false;

	T w[8];

	_pCorners[0].x = - invMat._11 - invMat._21              + invMat._41;
	_pCorners[0].y = - invMat._12 - invMat._22              + invMat._42;
	_pCorners[0].z = - invMat._13 - invMat._23              + invMat._43;
	w[0]           = - invMat._14 - invMat._24              + invMat._44;

	_pCorners[1].x =   invMat._11 - invMat._21              + invMat._41;
	_pCorners[1].y =   invMat._12 - invMat._22              + invMat._42;
	_pCorners[1].z =   invMat._13 - invMat._23              + invMat._43;
	w[1]           =   invMat._14 - invMat._24              + invMat._44;

	_pCorners[2].x =   invMat._11 + invMat._21              + invMat._41;
	_pCorners[2].y =   invMat._12 + invMat._22              + invMat._42;
	_pCorners[2].z =   invMat._13 + invMat._23              + invMat._43;
	w[2]           =   invMat._14 + invMat._24              + invMat._44;

	_pCorners[3].x = - invMat._11 + invMat._21              + invMat._41;
	_pCorners[3].y = - invMat._12 + invMat._22              + invMat._42;
	_pCorners[3].z = - invMat._13 + invMat._23              + invMat._43;
	w[3]           = - invMat._14 + invMat._24              + invMat._44;

	_pCorners[4].x = - invMat._11 - invMat._21 + invMat._31 + invMat._41;
	_pCorners[4].y = - invMat._12 - invMat._22 + invMat._32 + invMat._42;
	_pCorners[4].z = - invMat._13 - invMat._23 + invMat._33 + invMat._43;
	w[4]           = - invMat._14 - invMat._24 + invMat._34 + invMat._44;

	_pCorners[5].x =   invMat._11 - invMat._21 + invMat._31 + invMat._41;
	_pCorners[5].y =   invMat._12 - invMat._22 + invMat._32 + invMat._42;
	_pCorners[5].z =   invMat._13 - invMat._23 + invMat._33 + invMat._43;
	w[5]           =   invMat._14 - invMat._24 + invMat._34 + invMat._44;

	_pCorners[6].x =   invMat._11 + invMat._21 + invMat._31 + invMat._41;
	_pCorners[6].y =   invMat._12 + invMat._22 + invMat._32 + invMat._42;
	_pCorners[6].z =   invMat._13 + invMat._23 + invMat._33 + invMat._43;
	w[6]           =   invMat._14 + invMat._24 + invMat._34 + invMat._44;

	_pCorners[7].x = - invMat._11 + invMat._21 + invMat._31 + invMat._41;
	_pCorners[7].y = - invMat._12 + invMat._22 + invMat._32 + invMat._42;
	_pCorners[7].z = - invMat._13 + invMat._23 + invMat._33 + invMat._43;
	w[7]           = - invMat._14 + invMat._24 + invMat._34 + invMat._44;


	/*
	_pCorners[0] = invMat.Transform( TVec4<T>( -(T)1.0, -(T)1.0, 0.0f, (T)1.0 ) );
	_pCorners[1] = invMat.Transform( TVec4<T>(  (T)1.0, -(T)1.0, 0.0f, (T)1.0 ) );
	_pCorners[2] = invMat.Transform( TVec4<T>(  (T)1.0,  (T)1.0, 0.0f, (T)1.0 ) );
	_pCorners[3] = invMat.Transform( TVec4<T>( -(T)1.0,  (T)1.0, 0.0f, (T)1.0 ) );

	_pCorners[4] = invMat.Transform( TVec4<T>( -(T)1.0, -(T)1.0, (T)1.0, (T)1.0 ) );
	_pCorners[5] = invMat.Transform( TVec4<T>(  (T)1.0, -(T)1.0, (T)1.0, (T)1.0 ) );
	_pCorners[6] = invMat.Transform( TVec4<T>(  (T)1.0,  (T)1.0, (T)1.0, (T)1.0 ) );
	_pCorners[7] = invMat.Transform( TVec4<T>( -(T)1.0,  (T)1.0, (T)1.0, (T)1.0 ) );
	*/

	//Homogeneous divide

	for( int i=0 ; i < 8 ; ++i )
	{
		T rcpW = (T)1.0 / w[i];
		_pCorners[i] *= rcpW;
	}

	return true;
}

template <class T>
T TMat44<T>::UnitTestDiff( const TMat44& a, const TMat44& b )
{
	T diff = 0.0f;

	for( u32 j=0 ; j < 4 ; ++j )
		for( u32 i=0 ; i < 4 ; ++i )
			diff += Abs( a.m[i][j] - b.m[i][j] );

	return diff;
}

template <class T>
bool TMat44<T>::UnitTest()
{
	//Test inversion
	TMat44 m1;
	m1.MakePerspectiveProjection( DegToRad(80.0f), 0.75f, (T)1.0, 5000.0f );
	TMat44 m2( xtm::Inverse( m1 ) );
	TMat44 m3( m1 * m2 );

	T error = UnitTestDiff( m3, TMat44<T>::Identity );
	if( error > 0.001f )
		return false;

	//Test look-at
	return true;
}

template <class T>
bool TMat44<T>::IsValid( bool _bStrict ) const
{
	if( _bStrict )
	{
		const T maxValue = 1000000.0f;

		return	IsFinite(_11) && IsFinite(_12) && IsFinite(_13) && IsFinite(_14) &&
				IsFinite(_21) && IsFinite(_22) && IsFinite(_23) && IsFinite(_24) &&
				IsFinite(_31) && IsFinite(_32) && IsFinite(_33) && IsFinite(_34) &&
				IsFinite(_41) && IsFinite(_42) && IsFinite(_43) && IsFinite(_44) &&
				(Abs(_11) < maxValue) && (Abs(_12) < maxValue) && (Abs(_13) < maxValue) && (Abs(_14) < maxValue) &&
				(Abs(_21) < maxValue) && (Abs(_22) < maxValue) && (Abs(_23) < maxValue) && (Abs(_24) < maxValue) &&
				(Abs(_31) < maxValue) && (Abs(_32) < maxValue) && (Abs(_33) < maxValue) && (Abs(_34) < maxValue) &&
				(Abs(_41) < maxValue) && (Abs(_42) < maxValue) && (Abs(_43) < maxValue) && (Abs(_44) < maxValue);
	}
	else
	{
		return	IsFinite(_11) && IsFinite(_12) && IsFinite(_13) && IsFinite(_14) &&
				IsFinite(_21) && IsFinite(_22) && IsFinite(_23) && IsFinite(_24) &&
				IsFinite(_31) && IsFinite(_32) && IsFinite(_33) && IsFinite(_34) &&
				IsFinite(_41) && IsFinite(_42) && IsFinite(_43) && IsFinite(_44);
	}
}
	
template <class T>
void TMat44<T>::Dump() const
{
	/*
	LOG( "%f %f %f %f", _11, _12, _13, _14 );
	LOG( "%f %f %f %f", _21, _22, _23, _24 );
	LOG( "%f %f %f %f", _31, _32, _33, _34 );
	LOG( "%f %f %f %f", _41, _42, _43, _44 );
*/
}

template <class T>
bool TMat44<T>::Inverse()
{
	T    tmp[12]; // temp array for pairs
	T    src[16]; // array of transpose source matrix 
	T    det;     
	T*	dst = (T*)this;

	//transpose matrix
	for (int i = 0; i < 4; i++)
	{
		src[i]        = dst[i*4];
		src[i + 4]    = dst[i*4 + 1];
		src[i + 8]    = dst[i*4 + 2];
		src[i + 12]   = dst[i*4 + 3];
	}

	// calculate pairs for first 8 elements (cofactors)
	tmp[0]  = src[10] * src[15];
	tmp[1]  = src[11] * src[14];
	tmp[2]  = src[9]  * src[15];
	tmp[3]  = src[11] * src[13];
	tmp[4]  = src[9]  * src[14];
	tmp[5]  = src[10] * src[13];
	tmp[6]  = src[8]  * src[15];
	tmp[7]  = src[11] * src[12];
	tmp[8]  = src[8]  * src[14];
	tmp[9]  = src[10] * src[12];
	tmp[10] = src[8]  * src[13];
	tmp[11] = src[9]  * src[12];

	// calculate first 8 elements (cofactors)
	dst[0]  = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1]  = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2]  = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3]  = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4]  = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5]  = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6]  = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7]  = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
		
	// calculate pairs for second 8 elements (cofactors)
	tmp[0]  = src[2]*src[7];
	tmp[1]  = src[3]*src[6];
	tmp[2]  = src[1]*src[7];
	tmp[3]  = src[3]*src[5];
	tmp[4]  = src[1]*src[6];
	tmp[5]  = src[2]*src[5];

	tmp[6]  = src[0]*src[7];
	tmp[7]  = src[3]*src[4];
	tmp[8]  = src[0]*src[6];
	tmp[9]  = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];

	// calculate second 8 elements (cofactors)
	dst[8]  = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9]  = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
		
	// calculate determinant
	det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
		
	// calculate matrix inverse
	det = 1/det;
	for (int j = 0; j < 16; j++)
		dst[j] *= det;

	return true;
}

