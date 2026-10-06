
	template <class T>
	inline TVec3<T> AABBox<T>::Size() const
	{
		return m_max - m_min;
	}


	template <class T>
	inline TVec3<T> AABBox<T>::Center() const
	{
		return ( m_min + m_max ) * (T)0.5;
	}


	template <class T>
	inline void AABBox<T>::operator+=( const TVec3<T>& _point )
	{
		m_min.x = Min( m_min.x, _point.x );
		m_min.y = Min( m_min.y, _point.y );
		m_min.z = Min( m_min.z, _point.z );

		m_max.x = Max( m_max.x, _point.x );
		m_max.y = Max( m_max.y, _point.y );
		m_max.z = Max( m_max.z, _point.z );
	}

	template <class T>
	void AABBox<T>::Set( const Vec3* points, u32 numPoints )
	{
		if( numPoints == 0 )
			return;

		Vec3 min, max;
		min = max = points[0];

		++points;

		for( u32 i=1 ; i < numPoints ; ++i )
		{
			min.x = Min( points->x, min.x );
			min.y = Min( points->y, min.y );
			min.z = Min( points->z, min.z );

			max.x = Max( points->x, max.x );
			max.y = Max( points->y, max.y );
			max.z = Max( points->z, max.z );

			++points;
		}

		m_min = ToVec3d( min );
		m_max = ToVec3d( max );
	}

	template <class T>
	void AABBox<T>::Set( const Vec3d* points, u32 numPoints )
	{
		if( numPoints == 0 )
			return;

		m_min = m_max = points[0];

		++points;

		for( u32 i=1 ; i < numPoints ; ++i )
		{
			m_min.x = Min( points->x, m_min.x );
			m_min.y = Min( points->y, m_min.y );
			m_min.z = Min( points->z, m_min.z );

			m_max.x = Max( points->x, m_max.x );
			m_max.y = Max( points->y, m_max.y );
			m_max.z = Max( points->z, m_max.z );

			++points;
		}

	}

	template <class T>
	void AABBox<T>::Clear()
	{
		m_min = TVec3<T>(  FLT_MAX,  FLT_MAX,  FLT_MAX );
		m_max = TVec3<T>( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	}

	template <class T>
	inline bool AABBox<T>::IsNull() const
	{
		return	( m_min == TVec3<T>(  FLT_MAX,  FLT_MAX,  FLT_MAX ) ) &&
				( m_max == TVec3<T>( -FLT_MAX, -FLT_MAX, -FLT_MAX ) );
		
	}

	template <class T>
	inline void AABBox<T>::operator=( const TVec3<T>& _point )
	{
		m_min = _point;
		m_max = _point;
	}

	template <class T>
	void AABBox<T>::operator+=( const AABBox& bbox )
	{
		if( IsNull() )
		{
			*this = bbox;
			return;
		}

		m_min.x = Min( m_min.x, bbox.m_min.x );
		m_min.y = Min( m_min.y, bbox.m_min.y );
		m_min.z = Min( m_min.z, bbox.m_min.z );
		m_max.x = Max( m_max.x, bbox.m_max.x );
		m_max.y = Max( m_max.y, bbox.m_max.y );
		m_max.z = Max( m_max.z, bbox.m_max.z );
	}

	template <class T>
	AABBox<T> AABBox<T>::operator*( T _scale ) const
	{
		//TODO could be optimized
		AABBox<T> box;

		box.m_min = m_min * _scale;
		box.m_max = m_max * _scale;

		return box;
	}


	template <class T>
	AABBox<T> AABBox<T>::operator*( const TVec3<T>& _scale ) const
	{
		//TODO could be optimized
		AABBox<T> box;

		box.m_min = Modulate( m_min, _scale );
		box.m_max = Modulate( m_max, _scale );

		return box;
	}

	template <class T>
	PlaneClassify AABBox<T>::Classify( const Plane<T>& plane ) const
	{
		char classify;

		classify = plane.SignedDistance( m_min ) >= (T)0.0 ?
			POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

		classify |= plane.SignedDistance( TVec3<T>( m_min.x, m_min.y, m_max.z ) ) >= (T)0.0 ?
			POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

		classify |= plane.SignedDistance( TVec3<T>( m_min.x, m_max.y, m_min.z ) ) >= (T)0.0 ?
			POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

		classify |= plane.SignedDistance( TVec3<T>( m_min.x, m_max.y, m_max.z ) ) >= (T)0.0 ?
			POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

		classify |= plane.SignedDistance( TVec3<T>( m_max.x, m_min.y, m_min.z ) ) >= (T)0.0 ?
			POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

		classify |= plane.SignedDistance( TVec3<T>( m_max.x, m_min.y, m_max.z ) ) >= (T)0.0 ?
			POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

		classify |= plane.SignedDistance( TVec3<T>( m_max.x, m_max.y, m_min.z ) ) >= (T)0.0 ?
			POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

		classify |= plane.SignedDistance( m_max ) >= (T)0.0 ?
			POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;

		return (PlaneClassify)classify;
	}


	template <class T>
	T AABBox<T>::ShortestDistanceToPoint( const TVec3<T>& p ) const
	{
		Plane<T> planes[6];

		// - X
		planes[0].m_Dist = - p.x + m_min.x;
		planes[0].m_Normal = - TVec3<T>::XAxis;
		// + X
		planes[1].m_Dist = p.x - m_max.x;
		planes[1].m_Normal = TVec3<T>::XAxis;
		// - Y
		planes[2].m_Dist = - p.y + m_min.y;
		planes[2].m_Normal = - TVec3<T>::YAxis;
		// + Y
		planes[3].m_Dist = p.y - m_max.y;
		planes[3].m_Normal = TVec3<T>::YAxis;
		// - Z
		planes[4].m_Dist = - p.z + m_min.z;
		planes[4].m_Normal = - TVec3<T>::ZAxis;
		// + Z
		planes[5].m_Dist = p.z - m_max.z;
		planes[5].m_Normal = TVec3<T>::ZAxis;

		//sort by distance


		#define SWAP_IF_GREATER( a, b )	\
			if( (planes[a].m_Dist) > (planes[b].m_Dist) ) \
			{	tmp = planes[a]; \
				planes[a] = planes[b];	\
				planes[b] = tmp; \
				bSorted = false; }

		//Unrolled dumb bubble sort

		bool bSorted;

		Plane<T> tmp;

		do
		{
			bSorted = true;

			SWAP_IF_GREATER(0,1)
			SWAP_IF_GREATER(1,2)
			SWAP_IF_GREATER(2,3)
			SWAP_IF_GREATER(3,4)
			SWAP_IF_GREATER(4,5)

		}
		while( !bSorted );

		#undef SWAP_IF_GREATER

		u8 numPositiveDistances = 0;

		if( planes[0].m_Dist >= 0.0f )
			++numPositiveDistances;
		if( planes[1].m_Dist >= 0.0f )
			++numPositiveDistances;
		if( planes[2].m_Dist >= 0.0f )
			++numPositiveDistances;
		if( planes[3].m_Dist >= 0.0f )
			++numPositiveDistances;
		if( planes[4].m_Dist >= 0.0f )
			++numPositiveDistances;
		if( planes[5].m_Dist >= 0.0f )
			++numPositiveDistances;

		switch( numPositiveDistances )
		{
			case 0:
			case 4:
			case 5:
			case 6:
				//Inside the box
				return -1.0f;
			case 1:
				//Closest point on a box face
				return planes[5].m_Dist;
			case 2:
				//Closest point on a box edge

				/*
				    |   B
					| - - x Point
					|	/ |
					|  /  | C
					| /D  |
			P1	____o/____|______
					|
					| P2

				o : box edge
				B = Dist( P1, Point )
				C = Dist( P2, Point )

				P1 and P2 being orthogonal the Pythagor theorem apply
				D≤ = B≤ + C≤

				D = sqrt( B≤ + C≤ )
				*/

				return sqrtf( (float)( planes[4].m_Dist * planes[4].m_Dist + planes[5].m_Dist * planes[5].m_Dist ) );

			case 3:
				//Closest point on a box corner

				//Idem but in 3D

				/*
				A = Dist( P1, Point )
				B = Dist( P2, Point )
				C = Dist( P3, Point )

				D1≤ = A≤ + B≤
				D2≤ = D1≤ + C≤ = A≤ + B≤ + C≤

				D2 = sqrt( A≤ + B≤ + C≤ )
				*/

				return sqrtf( float( planes[3].m_Dist * planes[3].m_Dist
								+	 planes[4].m_Dist * planes[4].m_Dist
								+	 planes[5].m_Dist * planes[5].m_Dist ) );

		}

		return 696969.0f; //Shouldn't reach here
	}
	
	template <class T>
	TVec3<T> AABBox<T>::GetCorner( u32 i ) const
	{
		DBG_CHECK( i < 8 );
		
		switch( i )
		{
			case 0: return TVec3<T>( m_min.x, m_min.y, m_min.z );
			case 1: return TVec3<T>( m_min.x, m_min.y, m_max.z );
			case 2: return TVec3<T>( m_min.x, m_max.y, m_min.z );
			case 3: return TVec3<T>( m_min.x, m_max.y, m_max.z );
			case 4: return TVec3<T>( m_max.x, m_min.y, m_min.z );
			case 5: return TVec3<T>( m_max.x, m_min.y, m_max.z );
			case 6: return TVec3<T>( m_max.x, m_max.y, m_min.z );
			case 7: return TVec3<T>( m_max.x, m_max.y, m_max.z );
			default: return TVec3<T>( 0, 0, 0 );
		}
	}

	template <class T>
	void AABBox<T>::GetCorners( TVec3<T>* _pCorners ) const
	{
		DBG_CHECK( _pCorners );
		
		*_pCorners++ = TVec3<T>( m_min.x, m_min.y, m_min.z );
		*_pCorners++ = TVec3<T>( m_min.x, m_min.y, m_max.z );
		*_pCorners++ = TVec3<T>( m_min.x, m_max.y, m_min.z );
		*_pCorners++ = TVec3<T>( m_min.x, m_max.y, m_max.z );
		*_pCorners++ = TVec3<T>( m_max.x, m_min.y, m_min.z );
		*_pCorners++ = TVec3<T>( m_max.x, m_min.y, m_max.z );
		*_pCorners++ = TVec3<T>( m_max.x, m_max.y, m_min.z );
		*_pCorners   = TVec3<T>( m_max.x, m_max.y, m_max.z );
	}


	template <class T>
	inline T AABBox<T>::Volume() const
	{
		TVec3<T> s = Size();
		return s.x * s.y * s.z;
	}

	template <class T>
	inline T AABBox<T>::SurfaceArea() const
	{
		TVec3<T> s = Size();
		return ( s.x * s.y + s.x * s.z + s.y * s.z ) * 2.0f;
	}

	template <class T>
	inline u32 AABBox<T>::MaxDimension() const
	{
		TVec3<T> s = Size();
		if( s.x > s.y )
		{
			if( s.x > s.z )
				return 0;
			else
				return 2; 
		}
		else
		{
			if( s.y > s.z )
				return 1;
			else
				return 2; 
		}
	}


	template <class T>
	bool AABBox<T>::IntersectRay(	const Ray<T>& _ray, const TVec3<T>& _invDir, u32 _dirIsNeg[3], 
									T _minRayLength, T _maxRayLength ) const
	{
		//Slabs method
#define GET_BOX_ELEM( i ) (((TVec3<T>*)this)[i])

	    T tmin =  (GET_BOX_ELEM(  _dirIsNeg[0]).x - _ray.m_origin.x) * _invDir.x;
		T tmax =  (GET_BOX_ELEM(1-_dirIsNeg[0]).x - _ray.m_origin.x) * _invDir.x;
		T tymin = (GET_BOX_ELEM(  _dirIsNeg[1]).y - _ray.m_origin.y) * _invDir.y;
		T tymax = (GET_BOX_ELEM(1-_dirIsNeg[1]).y - _ray.m_origin.y) * _invDir.y;
		
		if ((tmin > tymax) || (tymin > tmax))
			return false;

		if (tymin > tmin) tmin = tymin;
		if (tymax < tmax) tmax = tymax;

		// Check for ray intersection against $z$ slab
		T tzmin = (GET_BOX_ELEM(  _dirIsNeg[2]).z - _ray.m_origin.z) * _invDir.z;
		T tzmax = (GET_BOX_ELEM(1-_dirIsNeg[2]).z - _ray.m_origin.z) * _invDir.z;
		
		if ((tmin > tzmax) || (tzmin > tmax))
			return false;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;
		return (tmin < _maxRayLength) && (tmax > _minRayLength);
#undef GET_BOX_ELEM
	}

/*
	void AABBox::Set( const OBBox& obbox )
	{
        const Vec3* pCorners = obbox.GetCorners();

		m_min.x =	Min( pCorners[0].x, Min( pCorners[1].x, Min( pCorners[2].x,
					Min( pCorners[3].x, Min( pCorners[4].x, Min( pCorners[5].x,
					Min( pCorners[6].x, pCorners[7].x ) ) ) ) ) ) );

		m_min.y =	Min( pCorners[0].y, Min( pCorners[1].y, Min( pCorners[2].y,
					Min( pCorners[3].y, Min( pCorners[4].y, Min( pCorners[5].y,
					Min( pCorners[6].y, pCorners[7].y ) ) ) ) ) ) );

		m_min.z =	Min( pCorners[0].z, Min( pCorners[1].z, Min( pCorners[2].z,
					Min( pCorners[3].z, Min( pCorners[4].z, Min( pCorners[5].z,
					Min( pCorners[6].z, pCorners[7].z ) ) ) ) ) ) );

		m_max.x =	Max( pCorners[0].x, Max( pCorners[1].x, Max( pCorners[2].x,
					Max( pCorners[3].x, Max( pCorners[4].x, Max( pCorners[5].x,
					Max( pCorners[6].x, pCorners[7].x ) ) ) ) ) ) );

		m_max.y =	Max( pCorners[0].y, Max( pCorners[1].y, Max( pCorners[2].y,
					Max( pCorners[3].y, Max( pCorners[4].y, Max( pCorners[5].y,
					Max( pCorners[6].y, pCorners[7].y ) ) ) ) ) ) );

		m_max.z =	Max( pCorners[0].z, Max( pCorners[1].z, Max( pCorners[2].z,
					Max( pCorners[3].z, Max( pCorners[4].z, Max( pCorners[5].z,
					Max( pCorners[6].z, pCorners[7].z ) ) ) ) ) ) );


	}
*/

		/* AABB-triangle overlap test code                      */
		/* by Tomas Akenine-Möller                              */

	#define FINDMINMAX(x0,x1,x2,min,max) \
		min = max = x0;   \
		if( x1<min ) min = x1; \
		if( x1>max ) max = x1; \
		if( x2<min ) min = x2; \
		if( x2>max ) max = x2;

	template <class T>
	int PlaneBoxOverlap( const TVec3<T>& _normal, const TVec3<T>& _vert, const TVec3<T>& _maxbox )
	{
		int q;
		TVec3<T> vmin, vmax;
		T v;

		for( q = 0; q <= 2; q++ )
		{
			v = _vert[ q ];

			if( _normal[ q ]>0.0f )
			{
				vmin[ q ] = -_maxbox[ q ] - v;
				vmax[ q ] = _maxbox[ q ] - v;
			}
			else
			{
				vmin[ q ] = _maxbox[ q ] - v;
				vmax[ q ] = -_maxbox[ q ] - v;
			}
		}

		if( Dot( _normal, vmin ) > 0.0f )
			return 0;

		if( Dot( _normal, vmax ) >= 0.0f ) 
			return 1;

		return 0;
	}


#define AXISTEST_X01(a, b, fa, fb)			\
	p0 = a*v0.y - b*v0.z;					\
	p2 = a*v2.y - b*v2.z;					\
	if( p0<p2 ) { min = p0; max = p2; }		\
	else { min = p2; max = p0; }			\
	rad = fa * boxhalfsize.y + fb * boxhalfsize.z;   \
	if( min>rad || max<-rad ) return 0;

#define AXISTEST_X2(a, b, fa, fb)			\
	p0 = a*v0.y - b*v0.z;					\
	p1 = a*v1.y - b*v1.z;					\
	if( p0<p1 ) { min = p0; max = p1; }		\
	else { min = p1; max = p0; }			\
	rad = fa * boxhalfsize.y + fb * boxhalfsize.z;   \
	if( min>rad || max<-rad ) return 0;

#define AXISTEST_Y02(a, b, fa, fb)			\
	p0 = -a*v0.x + b*v0.z;					\
	p2 = -a*v2.x + b*v2.z;					\
	if( p0<p2 ) { min = p0; max = p2; }		\
	else { min = p2; max = p0; }			\
	rad = fa * boxhalfsize.x + fb * boxhalfsize.z;   \
	if( min>rad || max<-rad ) return 0;

#define AXISTEST_Y1(a, b, fa, fb)			\
	p0 = -a*v0.x + b*v0.z;					\
	p1 = -a*v1.x + b*v1.z;					\
	if( p0<p1 ) { min = p0; max = p1; }		\
	else { min = p1; max = p0; }			\
	rad = fa * boxhalfsize.x + fb * boxhalfsize.z;   \
	if( min>rad || max<-rad ) return 0;

#define AXISTEST_Z12(a, b, fa, fb)			\
	p1 = a*v1.x - b*v1.y;					\
	p2 = a*v2.x - b*v2.y;					\
	if( p2<p1 ) { min = p2; max = p1; }		\
	else { min = p1; max = p2; }			\
	rad = fa * boxhalfsize.x + fb * boxhalfsize.y;   \
	if( min>rad || max<-rad ) return 0;

#define AXISTEST_Z0(a, b, fa, fb)			\
	p0 = a*v0.x - b*v0.y;					\
	p1 = a*v1.x - b*v1.y;					\
	if( p0<p1 ) { min = p0; max = p1; }		\
	else { min = p1; max = p0; }			\
	rad = fa * boxhalfsize.x + fb * boxhalfsize.y;   \
	if( min>rad || max<-rad ) return 0;

	template <class T>
	bool AABBox<T>::IntersectTriangle( const TVec3<T>& _a, const TVec3<T>& _b, const TVec3<T>& _c ) const
	{

		/*    use separating axis theorem to test overlap between triangle and box */
		/*    need to test for overlap in these directions: */
		/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
		/*       we do not even need to test these) */
		/*    2) normal of the triangle */
		/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
		/*       this gives 3x3=9 more tests */

		TVec3<T> v0, v1, v2;

		//   float axis[3];

		T min, max, p0, p1, p2, rad, fex, fey, fez;		// -NJMP- "d" local variable removed

		TVec3<T> normal, e0, e1, e2;

		TVec3<T> boxcenter = (m_min + m_max) * ( T )0.5;
		TVec3<T> boxhalfsize = (m_max - m_min) * ( T )0.5;

		/* This is the fastest branch on Sun */
		/* move everything so that the boxcenter is in (0,0,0) */

		v0 = _a - boxcenter;
		v1 = _b - boxcenter;
		v2 = _c - boxcenter;

		/* compute triangle edges */
		e0 = v1 - v0;
		e1 = v2 - v1;
		e2 = v0 - v2;

		/* Bullet 3:  */
		/*  test the 9 tests first (this was faster) */

		fex = Abs( e0.x );
		fey = Abs( e0.y );
		fez = Abs( e0.z );

		AXISTEST_X01( e0.z, e0.y, fez, fey );
		AXISTEST_Y02( e0.z, e0.x, fez, fex );
		AXISTEST_Z12( e0.y, e0.x, fey, fex );

		fex = fabsf( e1.x );
		fey = fabsf( e1.y );
		fez = fabsf( e1.z );

		AXISTEST_X01( e1.z, e1.y, fez, fey );
		AXISTEST_Y02( e1.z, e1.x, fez, fex );
		AXISTEST_Z0( e1.y, e1.x, fey, fex );

		fex = fabsf( e2.x );
		fey = fabsf( e2.y );
		fez = fabsf( e2.z );

		AXISTEST_X2( e2.z, e2.y, fez, fey );
		AXISTEST_Y1( e2.z, e2.x, fez, fex );
		AXISTEST_Z12( e2.y, e2.x, fey, fex );

		/* Bullet 1: */
		/*  first test overlap in the {x,y,z}-directions */
		/*  find min, max of the triangle each direction, and test for overlap in */
		/*  that direction -- this is equivalent to testing a minimal AABB around */
		/*  the triangle against the AABB */

		/* test in X-direction */

		FINDMINMAX( v0.x, v1.x, v2.x, min, max );

		if( min>boxhalfsize.x || max<-boxhalfsize.x )
			return false;

		/* test in Y-direction */

		FINDMINMAX( v0.y, v1.y, v2.y, min, max );

		if( min>boxhalfsize.y || max<-boxhalfsize.y )
			return false;

		/* test in Z-direction */

		FINDMINMAX( v0.z, v1.z, v2.z, min, max );

		if( min>boxhalfsize.z || max<-boxhalfsize.z )
			return false;

		/* Bullet 2: */

		/*  test if the box intersects the plane of the triangle */
		/*  compute plane equation of triangle: normal*x+d=0 */

		normal = Cross( e0, e1 );

		if( !PlaneBoxOverlap( normal, v0, boxhalfsize ) )
			return false;

		return true;   /* box and triangle overlaps */
	}

	template <class T>
	bool AABBox<T>::IntersectAABBox( const AABBox<T>& _box ) const
	{
		if( ( m_min.x > _box.m_max.x ) || ( m_max.x < _box.m_min.x ) ||
			( m_min.y > _box.m_max.y ) || ( m_max.y < _box.m_min.y ) ||
			( m_min.z > _box.m_max.z ) || ( m_max.z < _box.m_min.z ) )
			return false;

		return true;
	}
