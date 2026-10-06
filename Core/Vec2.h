#pragma once
#ifndef XTM_VEC2_H
#define XTM_VEC2_H


#include <Core/StandardMath.h>

#include <Core/Utility.h>
#include <Core/Debug.h>
#include <Core/Vec2i.h>

	template <class T>
	class TVec2;

	template <class T>	inline T		Dot(				const TVec2<T>& _a, const TVec2<T>& _b);
	template <class T>	inline T		DistanceSquared(	const TVec2<T>& _a, const TVec2<T>& _b);
	template <class T>	inline TVec2<T>	Normalized(			const TVec2<T>& _v);
	template <class T>	inline T		Distance(			const TVec2<T>& _a, const TVec2<T>& _b);
	template <class T>	inline TVec2<T>	Modulate(			const TVec2<T>& _a, const TVec2<T>& _b);
	template <class T>	inline TVec2<T>	Reflect(			const TVec2<T>& _v, const TVec2<T>& _normal );

	template <class T>	inline TVec2<T>	Floor(				const TVec2<T>& _v );

	template <class T>
	class TVec2
	{
	public:
		T x, y;

		static const TVec2 XAxis;
		static const TVec2 YAxis;
		static const TVec2 Origin;
		static const TVec2 Zero;
		static const TVec2 One;

		inline			TVec2();
		inline			TVec2(const T* pVec);
		inline			TVec2(T _x,T _y);
		inline			TVec2(const TVec2& vec);
		inline			TVec2(const Vec2i& vec);

		inline	void	Set(T _x,T _y);
		inline	void	Set(const T* pVec);

		inline	void	Normalize();

		inline	T		Dot(const TVec2& vec) const;


		//inline bool	IsNull() {return ((x);}

		inline	T		LengthSquared() const;

		inline	T		Length() const;

		inline	bool	IsNormalized() const;

		/*
		inline void Transform(const CMatrix & mat);

		inline TVec2 Transform(const CMatrix & mat) const;
		*/

		inline	TVec2&	operator=(const TVec2& vec);

		inline	TVec2&	operator=(const T* pVec);

		inline	void	operator*=(T scalar);

		inline	void	operator/=(T scalar);

		inline	TVec2	operator*(T scalar) const;

		inline	TVec2	operator*(const TVec2& vec) const;

		inline	TVec2	operator/(T scalar) const;

		inline	void	operator+=(const TVec2& vec);

		inline	void	operator-=(const TVec2& vec);

		inline	TVec2	operator+(const TVec2& vec) const;
		inline	TVec2	operator+(T scalar) const;

		inline	TVec2	operator-(const TVec2& vec) const;
		inline	TVec2	operator-(T scalar) const;

		inline	TVec2	operator-() const;

		inline	bool	operator==(const TVec2& vec) const;

	};

	#include "Vec2.inl"

	template <class T> const TVec2<T> TVec2<T>::XAxis( 1, 0 );
	template <class T> const TVec2<T> TVec2<T>::YAxis( 0, 1 );
	template <class T> const TVec2<T> TVec2<T>::Origin( 0, 0 );
	template <class T> const TVec2<T> TVec2<T>::Zero( 0, 0 );
	template <class T> const TVec2<T> TVec2<T>::One( 1, 1 );

	//#pragma warning (disable : 4231)

	template class TVec2<float>;
	template class TVec2<double>;

	typedef TVec2<float>	Vec2;
	typedef TVec2<double>	Vec2d;


#endif
