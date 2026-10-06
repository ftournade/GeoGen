#pragma once
#ifndef XTM_MAT33_H
#define XTM_MAT33_H

#include "Vec3.h"

#include <Core/Utility.h> //For TSwap

	template <class T>
    class TMat33
    {
    public:
        inline			TMat33();
        inline			TMat33(	T f11,T f12,T f13,
                                T f21,T f22,T f23,
                                T f31,T f32,T f33 );

						TMat33( const T* ptr );

		void	Dump() const;

        void	MakeIdentity();

        void	MakeTranslation( const TVec3<T>& trans );

        void	MakeScaling( const TVec3<T>& scale );
        void	RemoveScaling();

        void	MakeRotationX( T angle );
        void	MakeRotationY( T angle );
        void	MakeRotationZ( T angle );

        inline	void	Transpose();

				bool	Inverse();

        inline	TVec3<T>	Transform( const TVec3<T>& v ) const;
		inline	TVec2<T>	TransformDirection( const TVec2<T>& v ) const;
		inline	TVec2<T>	TransformPosition( const TVec2<T>& v ) const;

        inline	T	GetDeterminant() const;

        inline	TVec3<T>	GetXAxis() const;
        inline	TVec3<T>	GetYAxis() const;
        inline	TVec3<T>	GetZAxis() const;

        inline	void	SetXAxis( const TVec3<T>& axis );
        inline	void	SetYAxis( const TVec3<T>& axis );
        inline	void	SetZAxis( const TVec3<T>& axis );

        inline	TMat33&	operator=( const T* ptr );
        TMat33	operator*( const TMat33& ) const;
        void	operator*=( const TMat33& );


    public:
        union
        {
            struct
            {
                T	_11,_12,_13,
                        _21,_22,_23,
                        _31,_32,_33;
            };


            T m[3][3];
        };

		
        static const TMat33 Zero;
        static const TMat33 Identity;

    };

	template <class T>
	inline TMat33<T> Inverse( const TMat33<T>& _m );

	template <class T>
	inline TMat33<T> Transpose( const TMat33<T>& _m );

    //Transform 'position' vector by matrix
	template <class T>
    inline TVec3<T> operator*( const TVec3<T>& v, const TMat33<T>& m );

	template <class T> const TMat33<T> TMat33<T>::Zero(	0, 0, 0,
														0, 0, 0,
														0, 0, 0 );

    template <class T> const TMat33<T> TMat33<T>::Identity(	1, 0, 0,
															0, 1, 0,
															0, 0, 1 );


    #include "Mat33.inl"

	typedef TMat33<float> Mat33;
	typedef TMat33<double> Mat33d;

#endif
