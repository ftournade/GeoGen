#include "stdafx.h"
#include "CPUFloatMap.h"


CPUFloatMap::CPUFloatMap() :
	m_SX(0), m_SY(0), m_pData(nullptr)
{
}


CPUFloatMap::CPUFloatMap( const CPUFloatMap& _rhs ) :
	m_SX( 0 ), m_SY( 0 ), m_pData( nullptr )
{
	Resize( _rhs.m_SX, _rhs.m_SY );

	memcpy( m_pData, _rhs.m_pData, m_SX * m_SY * sizeof( float ) );
}

void CPUFloatMap::operator=( const CPUFloatMap& _rhs )
{
	Resize( _rhs.m_SX, _rhs.m_SY );

	memcpy( m_pData, _rhs.m_pData, m_SX * m_SY * sizeof( float ) );
}

CPUFloatMap::~CPUFloatMap()
{
	SAFE_DELETE( m_pData )
}

void CPUFloatMap::Resize( u32 _sx, u32 _sy )
{
	SAFE_DELETE( m_pData )

	m_pData = new float[ _sx * _sy ];

	m_SX = _sx;
	m_SY = _sy;
}

void CPUFloatMap::Fill( float _v )
{
	DBG_CHECK( m_pData );

	u32 n = m_SX * m_SY;

	for( u32 i = 0 ; i < n ; ++i )
		m_pData[ i ] = _v;
}

