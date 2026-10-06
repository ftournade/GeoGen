#pragma once

#include <Core/Vec3.h>
#include <Core/TVector.h>

namespace xtm
{

	class Curve
	{
	public:
		virtual ~Curve() {}
		
		virtual Vec3 Evaluate( float t ) const=0;
		virtual Vec3 EvaluateTangent( float t ) const;
		//_result will receive (_numSubdiv+2) points
		//override this for optimisation
		virtual void EvaluateAll( u32 _numSubdiv, Vec3* _result ) const;
	};




	class CubicBezierCurve : public Curve
	{
	public:
		CubicBezierCurve();
		~CubicBezierCurve();

						void	SetNumCurveSegment( u32 n );
		inline			u32		GetNumCurveSegment() const					{ return (u32)m_Segments.size(); }

		inline			u32		GetNumControlPoint() const					{ return GetNumCurveSegment() * 3 + 1; }
		inline const	Vec3&	GetControlPoint( u32 i ) const				{ return m_ControlPoints[i]; }
						void	SetControlPoint( u32 i, const Vec3& p );

						void	ComputeAutoTangents( float _smoothness = 1.0f );

		virtual Vec3 Evaluate( float t ) const;
		virtual void EvaluateAll( u32 _numSubdiv, Vec3* _result ) const;

	private:
		void UpdatePolynomials();

		struct Segment
		{
			Vec3 a, b, c;
		};

		TVector< Vec3 >		m_ControlPoints;
		TVector< Segment >	m_Segments;


	};

}
