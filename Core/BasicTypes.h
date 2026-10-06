#pragma once
#ifndef XTM_BASIC_TYPES_H
#define XTM_BASIC_TYPES_H

#ifndef NULL
	#define NULL 0
#endif


/* Usage:
 
 class MyStupidManager
 {
	DECLARE_SINGLETON( MyStupidManager )

	//Note: This is not part of the DECLARE_SINGLETON macro
	//because most code completion tools work badly with macros
 
	inline static MyStupidManager& Get(); 
 
 public:
	~MyStupidManager();
	
	...
 };
 
 IMPLEMENT_SINGLETON( MyStupidManager )
 
*/

#define DECLARE_SINGLETON( Class ) \
	private: \
		Class(); \
		Class( const Class& o ); \
		Class& operator=( const Class& o ); \
		public:

//NOTE: Don't implement operator=() or copy constructor !! 

 #define IMPLEMENT_SINGLETON( Class ) \
	inline Class& Class::Get() \
	{ \
		static Class singleton; \
		return singleton; \
	}

#if defined(XTM_WIN32) || defined(XTM_IPHONE) || defined(XTM_MACOSX) || defined(XTM_LINUX)

	typedef   unsigned	char	  byte;
	typedef   signed    char      s8;
	typedef   unsigned  char      u8;
	typedef   signed    short     s16;
	typedef   signed    long      s32;
	typedef   unsigned  short     u16;
	typedef   unsigned  long      u32;
	typedef   float               f32;
	typedef   double              f64;

	#ifdef XTM_USE_DOUBLE_FLOAT_PRECISION
		typedef   double				scalar;
	#else
		typedef   float					scalar;
	#endif

	#ifdef XTM_WIN32
		typedef   unsigned  __int64   u64;                          // ! Microsoft specific
		typedef   signed    __int64   s64;                          // ! Microsoft specific
	#else
		typedef unsigned long long u64;
		typedef long long s64;
	#endif

#else
	#error "not implemented on target platform"
#endif

struct rgb8_t
{
	u8 r, g, b;
};

struct rgba8_t
{
	u8 r, g, b, a;
};

#ifdef XTM_WIN32
	#define FORCE_INLINE __forceinline
#else
	#define FORCE_INLINE inline
#endif


#ifdef XTM_MACOSX
	#define TCHAR u8
#endif

#define BIT( b )	( 1U << (b) )
#define BIT64( b )	( 1ULL << (b) )

#endif
