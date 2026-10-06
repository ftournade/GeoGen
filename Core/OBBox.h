#pragma once
#ifndef XTM_OBBOX_H
#define XTM_OBBOX_H

#include "Mat33.h"
#include "Mat44.h"
#include "Plane.h"
#include "AABBox.h"
#include "Ray.h"

namespace xtm
{
	template <class T> class AABBox;


	template <class T>
	class OBBox
	{
	public:
        inline          OBBox();
        inline			OBBox( const OBBox<T>& _obbox );
                        OBBox( const AABBox<T>& _aabbox, const TMat44<T>& _transform );

        inline const    TMat33<T>&      GetOrientation() const;
        inline          void            SetOrientation( const TMat33<T>& _orientation );
        
        inline const    TVec3<T>&       GetCenter() const;
        inline          void            SetCenter( const TVec3<T>& _center );

        inline			TVec3<T>		GetScale() const;
        inline          void            SetScale( const TVec3<T>& _scale );

		inline const    TVec3<T>&		GetHalfScale() const;

        inline          const TVec3<T>* GetCorners() const;

        inline          void            Transform( const TMat44<T>& _transform );

		inline			T				ComputeVolume() const;

		                PlaneClassify	Classify( const Plane<T>& _plane ) const;

                        bool			RayHitTest( const Ray<T>& _ray ) const;
                        bool			RayHitTest( const Ray<T>& _ray, T& _dist ) const;

    private:
        TMat33<T>				m_Orientation;
        TVec3<T>				m_Center;
        TVec3<T>				m_HalfScale;
        mutable TVec3<T>		m_Corners[8];
        mutable bool			m_CornersDirty;
        
    };

    #include "OBBox.inl"

}

#endif
