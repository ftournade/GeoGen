#include "stdafx.h"
#include "InputBitmapNode.h"

#include <Core/Bitmap.h>

InputBitmapNode::InputBitmapNode()
{
	SetResolutionReference( Res_GlobalSetting );

	SetUIName( "Input Bitmap" );

	AddOutput( "FloatMap", IOType::Float );

	AddParam( "Main", "Filename", IOType::String, ParamEdition::FilePicker, "" );
}


InputBitmapNode::~InputBitmapNode()
{
}

const Str& InputBitmapNode::GetFilename()
{
	return m_ParameterSlots[ 0 ].m_ValueString;
}

void InputBitmapNode::SetFilename( const char* _filename )
{
	m_ParameterSlots[ 0 ].m_ValueString = _filename;
}

bool InputBitmapNode::OnResolutionChanged()
{
	if( !m_Bitmap.Init( GetResolution(), IOType::Float ) ) //TODO fix here for color maps
	{
		return false;
	}

	return true;
}

const Map* InputBitmapNode::GetOutput( u32 _idx ) const
{
	return &m_Bitmap;
}

void InputBitmapNode::InternalCompute()
{
	if( GetFilename().empty() )
		return;

	Bitmap bmp;

	if( !bmp.Load( GetFilename().c_str() ) )
	{
		DBG_CHECK( false );
		return;
	}

	u32 n = GetResolution() * GetResolution();
	float rcpRes = 1.0f / (float)(GetResolution() - 1);

	if( bmp.GetFormat().m_Fields.m_Layout == DataFormat::Layout::Layout_8_8_8_8 )
	{
		m_Bitmap.Init( GetResolution(), GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM );
		m_OutputSlots[ 0 ].m_DataType = IOType::Color;

		vector<u32> pixels( n );

		for( u32 y = 0 ; y < GetResolution() ; ++y )
		{
			for( u32 x = 0 ; x < GetResolution() ; ++x )
			{
				Color c = bmp.BilinearSample( Vec2( x, y ) * rcpRes ); //TODO make configurable
				pixels[ y * GetResolution() + x ] = c.ToWin32COLORREF();
			}
		}

		m_Bitmap.CopyToGPU( (rgba8_t*)&pixels[ 0 ] );

	}
	else
	{
		m_Bitmap.Init( GetResolution(), IOType::Float );
		m_OutputSlots[ 0 ].m_DataType = IOType::Float;

		vector<float> pixels( n );

		for( u32 y = 0 ; y < GetResolution() ; ++y )
		{
			for( u32 x = 0 ; x < GetResolution() ; ++x )
			{
				Color c = bmp.BilinearSample( Vec2( x, y ) * rcpRes ); //TODO make configurable
				pixels[ y * GetResolution() + x ] = c.r;
			}
		}

		m_Bitmap.CopyToGPU( &pixels[ 0 ] );

	}

}