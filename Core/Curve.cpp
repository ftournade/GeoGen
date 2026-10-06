//#include "stdafx.h"
#include "Curve.h"


	void Curve::EvaluateAll( u32 _numSubdiv, Vec3* _result ) const
	{
		_numSubdiv += 2;

		float r = 1.0f / (float)(_numSubdiv - 1);

		for( u32 i = 0 ; i < _numSubdiv ; ++i )
		{
			float t = (float)i * r;
			_result[i] = Evaluate( t );
		}
	}

	Vec3 Curve::EvaluateTangent( float t ) const
	{
		//TODO better ...
		const float smallDelta = 0.05f;

		if( t > smallDelta )
		{
			return Normalized( Evaluate( t ) - Evaluate( t - smallDelta ) );
		}
		else
		{
			return Normalized( Evaluate( t + smallDelta ) - Evaluate( t ) );
		}
	}

	CubicBezierCurve::CubicBezierCurve()
	{
	}

	CubicBezierCurve::~CubicBezierCurve()
	{
	}

	void CubicBezierCurve::SetNumCurveSegment( u32 n )
	{
		m_Segments.resize( n );
		m_ControlPoints.resize( 3 * n + 1 );
	}

	void CubicBezierCurve::SetControlPoint( u32 i, const Vec3& p )
	{
		m_ControlPoints[i] = p;
		UpdatePolynomials();
	}

	void CubicBezierCurve::UpdatePolynomials()
	{
		u32 numSegments = (u32)m_Segments.size();

		for( u32 i=0 ; i < numSegments ; ++i )
		{
			Segment& s = m_Segments[i];
			u32 base = i * 3;

			s.c = (m_ControlPoints[base + 1] - m_ControlPoints[base + 0]) * 3.0f;
			s.b = (m_ControlPoints[base + 2] - m_ControlPoints[base + 1]) * 3.0f - s.c;
			s.a =  m_ControlPoints[base + 3] - m_ControlPoints[base + 0] - s.c - s.b;
		}
	}

	void CubicBezierCurve::ComputeAutoTangents( float _smoothness )
	{
		u32 numSegment = (u32)m_Segments.size();

		m_ControlPoints[1] = Lerp( m_ControlPoints[0], m_ControlPoints[3], _smoothness );

		for( u32 i=1 ; i < numSegment ; ++i )
		{
			//m_ControlPoints[ i * 3 + 1 ] = m_ControlPoints[ i * 3 ];
			//m_ControlPoints[ i * 3 + 2 ] = m_ControlPoints[ i * 3 + 3 ];

			Vec3 tangentDir = m_ControlPoints[ (i + 1) * 3 ] - m_ControlPoints[ (i - 1) * 3 ];
			tangentDir.Normalize();
			float L1 = (m_ControlPoints[ i * 3 ] - m_ControlPoints[ (i - 1) * 3 ]).Length();
			float L2 = (m_ControlPoints[ i * 3 ] - m_ControlPoints[ (i + 1) * 3 ]).Length();
			float LMin = Min( L1, L2 );
			tangentDir *= ( LMin * _smoothness );

			m_ControlPoints[ i * 3 - 1 ] = m_ControlPoints[ i * 3 ] - tangentDir;
			m_ControlPoints[ i * 3 + 1 ] = m_ControlPoints[ i * 3 ] + tangentDir;
		}

		m_ControlPoints[ numSegment * 3 - 1 ] = Lerp(	m_ControlPoints[ numSegment * 3 ],
														m_ControlPoints[ (numSegment - 1) * 3 ],
														_smoothness );

		UpdatePolynomials();
	}

	Vec3 CubicBezierCurve::Evaluate( float t ) const
	{
		t = Clamp( t, 0.0f, 0.9999999f );

		u32 numSegments = (u32)m_Segments.size();

		float oneOverNumSeg = 1.0f / (float)numSegments;

		u32 iSegment = (u32)( t * numSegments );

		t = (t - (float)iSegment * oneOverNumSeg) * (float)numSegments;
		//t = t * (float)numSegments - (float)iSegment;

		const Segment& seg = m_Segments[iSegment];

		float t2 = t * t;
		float t3 = t2 * t;

		const Vec3& firstSegPoint = m_ControlPoints[ iSegment * 3 ];

		return Vec3( seg.a * t3 + seg.b * t2 + seg.c * t + firstSegPoint );
	}


	void CubicBezierCurve::EvaluateAll( u32 _numSubdiv, Vec3* _result ) const
	{

		_numSubdiv += 2;

		float r = 1.0f / (float)(_numSubdiv - 1);

		u32 numSegments = (u32)m_Segments.size();

		float oneOverNumSeg = 1.0f / (float)numSegments;

		for( u32 i = 0 ; i < _numSubdiv ; ++i )
		{
			float t = (float)i * r;
			t = Clamp( t, 0.0f, 0.9999999f );

			u32 iSegment = (u32)( t * numSegments );
			const Segment& seg = m_Segments[iSegment];

			t = (t - (float)iSegment * oneOverNumSeg) * (float)numSegments;

			float t2 = t * t;
			float t3 = t2 * t;

			const Vec3& firstSegPoint = m_ControlPoints[ iSegment * 3 ];

			_result[i] = Vec3( seg.a * t3 + seg.b * t2 + seg.c * t + firstSegPoint );
		}

	}

