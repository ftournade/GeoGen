#pragma once

#include "Plane.h"

#include <Core/TVector.h>

	template <class T>
	class ConvexHull
	{
	 public:
		TVector< Plane<T> > m_Planes;
				
		//bool PointInHull( const TVec3<T>& p );
		
	};


	template < u32 N, class T >
	class PointSetConvexHull
	{
		public:
			TVec3<T> m_Points[ N ]; //TODO TVector
		
			inline PlaneClassify Classify( const Plane<float>& _plane ) const
			{
				u32 result = (PlaneClassify)0;
				
				for( u32 i= 0 ; i < N ; ++i )
				{
					float d = _plane.SignedDistance( m_Points[i] );
					result |= (d > 0.0f) ? POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;
				}
				
				return (PlaneClassify)result;
			}

			inline PlaneClassify Classify( const Plane<double>& _plane ) const
			{
				u32 result = (PlaneClassify)0;
				
				for( u32 i= 0 ; i < N ; ++i )
				{
					double d = _plane.SignedDistance( ToVec3d( m_Points[i] ) );
					result |= (d > 0.0) ? POSITIVE_PLANE_SIDE : NEGATIVE_PLANE_SIDE;
				}
				
				return (PlaneClassify)result;
			}

	};
		
			

	enum ConvexHullClassify
	{
		IN_CONVEX_HULL = 1,
		OUTOF_CONVEX_HULL = 2,
		CROSSING_CONVEX_HULL = 3
	};

	//Important note : This method isn't perfect,
	//in some cases it could return CROSSING_CONVEX_HULL
	//where it should return OUTOF_CONVEX_HULL
	//(There certainly exist more correct algos, but maybe not as fast as this one)
	//(In the case of camera frustum culling, this 'approximation' is acceptable)

	template < class T, class S >
	ConvexHullClassify ClassifyBoundAgainstConvexHull( const T& bound, const ConvexHull<S>& ch )
	{
		ConvexHullClassify result = IN_CONVEX_HULL;

		TVector< Plane<S> >::const_iterator it,
										end = ch.m_Planes.end();

		for( it = ch.m_Planes.begin() ; it != end ; ++it )
		{
			const Plane<S>& plane = *it;

			switch( bound.Classify( plane ) )
			{
			case NEGATIVE_PLANE_SIDE:
				return OUTOF_CONVEX_HULL;
			case CROSSING_PLANE:
				result = CROSSING_CONVEX_HULL;
				break;
			}			
		}

		return result;
	}

