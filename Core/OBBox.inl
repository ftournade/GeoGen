template <typename T>
OBBox<T>::OBBox()
{
	m_Orientation = TMat33<T>::Identity;
	m_Center = TVec3<T>::Origin;
	m_HalfScale = TVec3<T>::Zero;

}

template <typename T>
OBBox<T>::OBBox( const OBBox& _obbox ) :
        m_Orientation( _obbox.m_Orientation ),
        m_Center( _obbox.m_Center ),
        m_HalfScale( _obbox.m_HalfScale ),
        m_CornersDirty( true )
{
}


template <typename T>
OBBox<T>::OBBox( const AABBox<T>& _aabbox, const TMat44<T>& _transform ) :
    m_CornersDirty( true )
{
    T scaleX = _transform.GetXAxis().Length();
    T scaleY = _transform.GetYAxis().Length();
    T scaleZ = _transform.GetZAxis().Length();
    TVec3<T> aabboxSize( _aabbox.Size() );

    m_Center = _aabbox.Center() * _transform;

    m_HalfScale.x = aabboxSize.x * T(0.5) * scaleX;
    m_HalfScale.y = aabboxSize.y * T(0.5) * scaleY;
    m_HalfScale.z = aabboxSize.z * T(0.5) * scaleZ;

    m_Orientation.SetXAxis( _transform.GetXAxis() / (float)scaleX );
    m_Orientation.SetYAxis( _transform.GetYAxis() / (float)scaleY );
    m_Orientation.SetZAxis( _transform.GetZAxis() / (float)scaleZ );
}

template <typename T>
PlaneClassify OBBox<T>::Classify( const Plane<T>& _plane ) const
{
	char classify;

    const Vec3d* pCorners = GetCorners();

	classify = _plane.SignedDistance( pCorners[0] ) >= 0.0f ?
		POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

	classify |= _plane.SignedDistance( pCorners[1] ) >= 0.0f ?
		POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

	classify |= _plane.SignedDistance( pCorners[2] ) >= 0.0f ?
		POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

	classify |= _plane.SignedDistance( pCorners[3] ) >= 0.0f ?
		POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

	classify |= _plane.SignedDistance( pCorners[4] ) >= 0.0f ?
		POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

	classify |= _plane.SignedDistance( pCorners[5] ) >= 0.0f ?
		POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

	classify |= _plane.SignedDistance( pCorners[6] ) >= 0.0f ?
		POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

	classify |= _plane.SignedDistance( pCorners[7] ) >= 0.0f ?
		POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

	return (PlaneClassify)classify;
}


/*
	bool OBBox<T>::RayHitTest( const Ray& _ray ) const
	{
		//TODO find a fast algorithm

		return false;
	}
*/

#define OBBOXRayHitTest( axis, AXIS ) \
    e = Dot( m_Orientation.Get##AXIS##Axis(), p ); \
    f = Dot( m_Orientation.Get##AXIS##Axis(), _ray.m_dir ); \
    if( !IsEpsilonNull( f ) ) \
    { \
        oneOverF = (T)1.0 / f; \
        t1 = ( e + m_HalfScale.axis ) * oneOverF; \
        t2 = ( e - m_HalfScale.axis ) * oneOverF; \
        if( t1 < t2 ) \
        { \
            t_min = Max( t1, t_min ); \
            t_max = Min( t2, t_max ); \
        } \
        else \
        { \
            t_min = Max( t2, t_min ); \
            t_max = Min( t1, t_max ); \
        } \
        if( ( t_max < 0.0f ) || ( t_min > t_max ) ) \
            return false; \
    } \
    else if( ( e + m_HalfScale.axis < 0.0f ) || ( e - m_HalfScale.axis > 0.0f ) ) \
    { \
        return false; \
    }


template <typename T>
bool OBBox<T>::RayHitTest( const Ray<T>& _ray, T& _dist ) const
{

    //Slabs Method ( by Haines - Real-Time Rendering p. 572 )

    T   t_min = - FLT_MAX,
            t_max = FLT_MAX,
			oneOverF, e, f, t1, t2;

    TVec3<T> p( m_Center - _ray.m_origin );

    _dist = FLT_MAX;

    OBBOXRayHitTest( x, X )
    OBBOXRayHitTest( y, Y )
    OBBOXRayHitTest( z, Z )

    _dist = ( t_min > (T)0.0 ) ? t_min : t_max;

    return true;
}


template <class T>
inline const TMat33<T>& OBBox<T>::GetOrientation() const
{
    return m_Orientation; 
}

template <typename T>
inline void OBBox<T>::SetOrientation( const TMat33<T>& _orientation )
{
    m_Orientation = _orientation;
    m_CornersDirty = true;
}

template <typename T>
inline const TVec3<T>& OBBox<T>::GetCenter() const
{
    return m_Center;
}

template <typename T>
inline void OBBox<T>::SetCenter( const TVec3<T>& _center )
{
    m_Center = _center;
    m_CornersDirty = true;
}

template <typename T>
inline const TVec3<T>& OBBox<T>::GetHalfScale() const
{
    return m_HalfScale;
}

template <typename T>
inline TVec3<T> OBBox<T>::GetScale() const
{
    return m_HalfScale * (T)2.0;
}

template <typename T>
inline void OBBox<T>::SetScale( const TVec3<T>& _scale )
{
    m_HalfScale = _scale * (T)0.5;
    m_CornersDirty = true;
}

template <typename T>
inline const TVec3<T>* OBBox<T>::GetCorners() const
{
    if( m_CornersDirty )
    {

        //TODO thread safety

        m_Corners[0] = m_Center + TVec3<T>( - m_HalfScale.x, - m_HalfScale.y, - m_HalfScale.z ) * m_Orientation;
        m_Corners[1] = m_Center + TVec3<T>( - m_HalfScale.x, - m_HalfScale.y,   m_HalfScale.z ) * m_Orientation;
        m_Corners[2] = m_Center + TVec3<T>( - m_HalfScale.x,   m_HalfScale.y, - m_HalfScale.z ) * m_Orientation;
        m_Corners[3] = m_Center + TVec3<T>( - m_HalfScale.x,   m_HalfScale.y,   m_HalfScale.z ) * m_Orientation;
        m_Corners[4] = m_Center + TVec3<T>(   m_HalfScale.x, - m_HalfScale.y, - m_HalfScale.z ) * m_Orientation;
        m_Corners[5] = m_Center + TVec3<T>(   m_HalfScale.x, - m_HalfScale.y,   m_HalfScale.z ) * m_Orientation;
        m_Corners[6] = m_Center + TVec3<T>(   m_HalfScale.x,   m_HalfScale.y, - m_HalfScale.z ) * m_Orientation;
        m_Corners[7] = m_Center + TVec3<T>(   m_HalfScale.x,   m_HalfScale.y,   m_HalfScale.z ) * m_Orientation;

        m_CornersDirty = false;
    }

    return m_Corners;
}

template <typename T>
inline T OBBox<T>::ComputeVolume() const
{
	return m_HalfScale.x * m_HalfScale.y * m_HalfScale.z * (T)8.0;
}
