#pragma once
#ifndef XTM_VEC4_H
#define XTM_VEC4_H


#include <Core/StandardMath.h>
#include <Core/Vec3.h>

namespace xtm
{

	template <class T> class TVec4;

	template <class T> inline T Dot(				const TVec4<T>& _a, const TVec4<T>& _b);
	//inline TVec4	 Cross(							const TVec4<T>& _a, const TVec4<T>& _b);
	template <class T> inline T DistanceSquared(	const TVec4<T>& _a, const TVec4<T>& _b);
	template <class T> inline T Distance(			const TVec4<T>& _a, const TVec4<T>& _b);
	template <class T> inline TVec4<T>  Modulate(	const TVec4<T>& _a, const TVec4<T>& _b);
	template <class T> inline TVec4<T>  Abs(		const TVec4<T>& _v );
	template <class T> inline TVec4<T>  Pow(		const TVec4<T>& _v, T _f );

	template <class T> 
	class TVec4
	{
	public:
		T x, y, z, w;

		static const TVec4 XAxis;
		static const TVec4 YAxis;
		static const TVec4 ZAxis;
		static const TVec4 Origin;
		static const TVec4 Zero;
		static const TVec4 One;
		static const TVec4 Min;
		static const TVec4 Max;

		inline			TVec4();
		inline			TVec4(const T* _pVec);
		inline			TVec4(T _x,T _y,T _z,T _w);
		inline			TVec4(const TVec4& _vec);
		explicit inline	TVec4(const TVec3<T>& _vec, T _w = 1.0f);

		inline	TVec3<T>	xyz() const;

		inline	void	Set(T _x,T _y,T _z,T _w);
		inline	void	Set(const T* _pVec);

		inline	void	Normalize();

		inline	T	Dot(const TVec4& _vec) const;

		//inline	TVec4 Cross(const TVec4& _vec) const;


		//inline bool	IsNull() {return ((x);}

		inline	T	LengthSquared() const;

		inline	T	Length() const;

		/*
		inline void Transform(const CMatrix & mat);

		inline TVec4 Transform(const CMatrix & mat) const;
		*/

		inline	TVec4&	operator=(const TVec4& _vec);

		inline	TVec4&	operator=(const T* _pVec);

		inline	void	operator*=(T _scalar);

		inline	void	operator/=(T _scalar);
		inline	void	operator/=(const TVec4& _vec);

		inline	TVec4	operator*(T _scalar) const;
		inline	TVec4	operator*(const TVec4& _vec) const;

		inline	TVec4	operator/(T _scalar) const;

		inline	void	operator+=(const TVec4& _vec);

		inline	void	operator-=(const TVec4& _vec);

		inline	TVec4	operator+(const TVec4& _vec) const;

		inline	TVec4	operator-(const TVec4& _vec) const;

		inline	TVec4	operator-() const;

		inline	bool	operator==(const TVec4& _vec) const;
	};

	#include "Vec4.inl"

	//#pragma warning (disable : 4231)

	template class TVec4<float>;
	template class TVec4<double>;

	typedef TVec4<float>	Vec4;
	typedef TVec4<double>	Vec4d;
}

#endif
