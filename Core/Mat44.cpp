//#include "stdafx.h"
#include "Mat44.h"

//#include <Core/Plane.h>
//#include <Core/Log.h>

namespace xtm
{
	/*
	template <class T>
	TMat44<T>::TMat44(const char* _text)
	{
		Xsscanf(_text, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
			&_11, &_12, &_13, &_14,
			&_21, &_22, &_23, &_24,
			&_31, &_32, &_33, &_34,
			&_41, &_42, &_43, &_44);

	}
	*/
	Mat44::Mat44(const char* _text)
	{
		Xsscanf(_text, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
			&_11, &_12, &_13, &_14,
			&_21, &_22, &_23, &_24,
			&_31, &_32, &_33, &_34,
			&_41, &_42, &_43, &_44);

	}
		
	Mat44d::Mat44d(const char* _text)
	{
		Xsscanf(_text, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
			&_11, &_12, &_13, &_14,
			&_21, &_22, &_23, &_24,
			&_31, &_32, &_33, &_34,
			&_41, &_42, &_43, &_44);

	}
}