#include "stdafx.h"
#include "OutputBitmapNode.h"


OutputBitmapNode::OutputBitmapNode()
{
	SetUIName( "Output Bitmap" );

	AddInput( "FloatMap", IOType::FloatOrColor );

	AddParam( "Main", "Filename", IOType::String, ParamEdition::FilePicker, "" );
}


OutputBitmapNode::~OutputBitmapNode()
{
}

const std::string& OutputBitmapNode::GetFilename()
{
	return m_ParameterSlots[ 0 ].m_ValueString;
}

void OutputBitmapNode::SetFilename( const char* _filename )
{
	m_ParameterSlots[ 0 ].m_ValueString = _filename;
}

bool OutputBitmapNode::OnResolutionChanged()
{

	return true;
}

const Map* OutputBitmapNode::GetOutput( uint32_t _idx ) const
{
	return nullptr;
}

void OutputBitmapNode::InternalCompute()
{
#if 1
	const Map* inputMap = GetRemoteInputMap( 0 );

	if( !inputMap || GetFilename().empty() )
		return; //TODO error code

	vector<float> rawPixels;
	
	inputMap->CopyFromGPU( rawPixels );

	Bitmap bmp;

	DataFormat fmt;
	if( inputMap->GetFormat() == DXGI_FORMAT_R32_FLOAT )
	{
		fmt = DataFormat::R32_FLOAT;
	}
	else
	{
		fmt = DataFormat::R8G8B8A8_UNORM;
	}

	bmp.Create( GetResolution(), GetResolution(), 1, 1, fmt );
	
	byte* pBitmapData;
	bmp.GetMipMapData( 0, 0, 0, &pBitmapData );

	for( uint32_t y = 0 ; y < GetResolution() ; ++y )
	{
		for( uint32_t x = 0 ; x < GetResolution() ; ++x )
		{
			uint32_t i = y * GetResolution() + x;

			if( inputMap->GetFormat() == DXGI_FORMAT_R32_FLOAT )
			{
				float v = rawPixels[ i ];
				((float*)pBitmapData)[ i ] = v;
			}
			else
			{
				assert( inputMap->GetFormat() == DXGI_FORMAT_R16G16B16A16_FLOAT );

				float* v = &rawPixels[ ( y * GetResolution() + x ) * 4 ];

				pBitmapData[ 0 ] = (u8)(Saturate( v[0] ) * 255.0f);
				pBitmapData[ 1 ] = (u8)(Saturate( v[1] ) * 255.0f);
				pBitmapData[ 2 ] = (u8)(Saturate( v[2] ) * 255.0f);
				pBitmapData[ 3 ] = (u8)(Saturate( v[3] ) * 255.0f);

				pBitmapData += 4;
			}

		}
	}

	if( !bmp.Save( GetFilename().c_str() ) )
	{
		assert( false ); //TODO handle error
	}
#else
	::AfxMessageBox( _T( "Bitmap output not available on demo version !" ) );
#endif
}