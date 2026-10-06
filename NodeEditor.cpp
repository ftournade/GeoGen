#include "stdafx.h"

#include "Resource.h"

#include "NodeEditor.h"

#include "BasicComputeNodes.h"
#include "MinstrelNoiseNode.h"
#include "MountainNode.h"
#include "InputBitmapNode.h"
#include "OutputBitmapNode.h"
#include "ErosionNode.h"
#include "SnowNode.h"
#include "MonteCarloErosionNode.h"
#include "MonteCarloErosionNodeCPU.h"
#include "CurveNode.h"
#include "ColorGradientNode.h"
#include "BlurNode.h"
#include "ExtractDetailNode.h"
#include "ScatterMapNode.h"
#include "PreviewNode.h"
#include "FakeErosionV2Node.h"
#include "MountainColoringNode.h"

#include "NodeFactory.h"

#include "GeoGen.h" //to trigger preview redraw

#define GEOGEN_VERSION_MAJOR 0
#define GEOGEN_VERSION_MINOR 1

NodeEditor g_NodeEditor;

Vec2 g_LastNodeEditorMousePos;

shared_ptr<ComputeNode> g_DragNode;
Vec2 g_ToolTipPos;
std::string g_ToolTipText;

shared_ptr<ComputeNode> g_HoveringNode;
uint32_t g_HoveringSlotIndex = 0;
IOSlotCategory g_HoveringSlotCategory = IOSlotCategory::Input;

shared_ptr<ComputeNode> g_DragLink_Node;
uint32_t g_DragLink_SlotIndex = 0;
IOSlotCategory g_DragLink_SlotCategory = IOSlotCategory::Input;

//////////////////////////////////////////

NodeEditor::NodeEditor() :
	m_bLockPreview(false),
	m_LinksPen( PS_SOLID, 3, RGB( 127, 30, 30 ) ),
	m_RGBLinksPen( PS_SOLID, 3, RGB( 30, 127, 30 ) ),
	m_NodesPen( PS_SOLID, 2, RGB( 30, 127, 30 ) ),
	m_ViewCenter(300.0f, 300.0f ),
	m_viewZoom(1.0f)
{
	ResizeFont();
}


NodeEditor::~NodeEditor()
{
}

bool NodeEditor::Init()
{

	//TODO maybe not the best place for registering nodes
	REGISTER_COMPUTE_NODE( PerlinNoiseNode, Generator, "Perlin Noise", 10 );
	REGISTER_COMPUTE_NODE( VoronoiseNode, Generator, "Voronoise", 11 );
	REGISTER_COMPUTE_NODE( VoronoiNode, Generator, "Voronoi", 12 );
	REGISTER_COMPUTE_NODE( MountainNode, Generator, "Mountain", 13 );
//	REGISTER_COMPUTE_NODE( CustomComputeNode, "Custom HLSL", 3 );
	REGISTER_COMPUTE_NODE( RadialNode, Generator, "Radial", 14 );
	REGISTER_COMPUTE_NODE( GradientNode, Generator, "Gradient", 15 );
	REGISTER_COMPUTE_NODE( ConstantColorNode, Generator, "Constant Color", 16 );
	REGISTER_COMPUTE_NODE( ConstantValueNode, Generator, "Constant Value", 17 );
	REGISTER_COMPUTE_NODE( ScatterMapNode, Generator, "Scatter Map", 18 );
	REGISTER_COMPUTE_NODE( CheckerboardNode, Generator, "CheckerBoard", 30 );
//	REGISTER_COMPUTE_NODE( BrickNode, Generator, "Bricks", 20 );
	
	REGISTER_COMPUTE_NODE( CombineNode, Combiner, "Combine", 110 );
	REGISTER_COMPUTE_NODE( MixNode, Combiner, "Mix", 111 );
	REGISTER_COMPUTE_NODE( ChannelSplitNode, Combiner, "Channel Split", 114 );
	REGISTER_COMPUTE_NODE( ChannelMergeNode, Combiner, "Channel Merge", 115 );
	

	REGISTER_COMPUTE_NODE( TerraceNode, Modifier, "Terrace", 210 );
	REGISTER_COMPUTE_NODE( ReRangeNode, Modifier, "Re-Range", 214 );
	REGISTER_COMPUTE_NODE( ScaleBiasNode, Modifier, "Scale && Bias", 218 );
	REGISTER_COMPUTE_NODE( InvertNode, Modifier, "Invert", 222 );
	REGISTER_COMPUTE_NODE( CurveNode, Modifier, "Curve", 223 );
	REGISTER_COMPUTE_NODE( DistortNode, Modifier, "Distort", 224 );
	REGISTER_COMPUTE_NODE( BlurNode, Modifier, "Blur", 226 );
	REGISTER_COMPUTE_NODE( DirectionalBlurNode, Modifier, "Directional Blur", 227 );
	REGISTER_COMPUTE_NODE( SharpenNode, Modifier, "Sharpen", 229 );
	REGISTER_COMPUTE_NODE( ColorGradientNode, Modifier, "Color Gradient", 230 );
	REGISTER_COMPUTE_NODE( ExtractDetailNode, Modifier, "Extract detail", 234 );
	REGISTER_COMPUTE_NODE( ExpanderNode, Modifier, "Expander", 238 );
	REGISTER_COMPUTE_NODE( PowNode, Modifier, "Power", 242 );
	REGISTER_COMPUTE_NODE( AbsNode, Modifier, "Absolute", 246 );
	REGISTER_COMPUTE_NODE( PeriodicNode, Modifier, "Periodic", 250 );
	REGISTER_COMPUTE_NODE( HSVNode, Modifier, "HSV", 254 );

	REGISTER_COMPUTE_NODE( AltitudeMaskNode, Mask, "Altitude Mask", 310 );
	REGISTER_COMPUTE_NODE( SlopeMaskNode, Mask, "Slope Mask", 311 );
	REGISTER_COMPUTE_NODE( ConvexityMaskNode, Mask, "Convexity Mask", 315 ); 

	REGISTER_COMPUTE_NODE( MinstrelNoiseNode, Natural, "Fake Erosion", 410 );
	REGISTER_COMPUTE_NODE( ErosionNode, Natural, "Erosion (GPU)", 411 );
	REGISTER_COMPUTE_NODE( MonteCarloErosionNode, Natural, "Erosion", 412 );
	REGISTER_COMPUTE_NODE( MonteCarloErosionNodeCPU, Natural, "Erosion (CPU)", 413 );
	REGISTER_COMPUTE_NODE( SnowNode, Natural, "Snow", 414 );
	REGISTER_COMPUTE_NODE( FakeErosionV2Node, Natural, "Fake Erosion V2", 415 );
	REGISTER_COMPUTE_NODE( MountainColoringNode, Natural, "Mountain Coloring", 416 );
	
	REGISTER_COMPUTE_NODE( PreviewNode, InputOutput, "Preview", 510 );
	REGISTER_COMPUTE_NODE( InputBitmapNode, InputOutput, "Input Bitmap", 511 );
	REGISTER_COMPUTE_NODE( OutputBitmapNode, InputOutput, "Output Bitmap", 512 );
	REGISTER_COMPUTE_NODE( NormalNode, InputOutput, "Normal", 513 );

	
	m_pNormalNode = make_shared<NormalNode>();

	m_pNormalNode->UpdateInternalResolution();
	m_pNormalNode->OneTimeInit();
	m_pNormalNode->OnResolutionChanged();

	return true;
}

void NodeEditor::AddNode( shared_ptr<ComputeNode> _pNode )
{
	m_AllNodes.push_back( _pNode );
}

void NodeEditor::SetPreviewNode( shared_ptr<ComputeNode> _pNode )
{
	if( m_bLockPreview )
		return;

	if( strcmp( _pNode->GetNodeClassName(), "PreviewNode" ) == 0 )
	{
	//	 pPreviewNode = std::static_pointer_cast<PreviewNode, ComputeNode>(_pNode);
		const ComputeNode::RemoteSlot& remoteSlot = _pNode->m_InputSlots[ 0 ].m_RemoteOutputSlot;
		
		shared_ptr<ComputeNode> pRemoteNode = remoteSlot.m_pNode.lock();

		if( !pRemoteNode )
			return;

		CreateLink( pRemoteNode, remoteSlot.m_SlotIndex, m_pNormalNode, 0 );
	}
	else
	{
		if( _pNode->m_OutputSlots.empty() )
		{
			//Special case for output nodes
			_pNode->Compute();
			return;
		}

		CreateLink( _pNode, 0, m_pNormalNode, 0 );
	}

	m_pPreviewedNode = _pNode;

	m_pNormalNode->SetDirty();
	m_pNormalNode->Compute();

	theApp.RedrawPreview();
	theApp.RedrawNodeEditor();
}

void NodeEditor::LockUnlockPreview()
{
	m_bLockPreview = !m_bLockPreview;
}

shared_ptr<ComputeNode> NodeEditor::MouseHitTest( const UIRect& _screenRect, const Vec2& _pos, int& _inputSlot, int& _outputSlot, int& _paramSlot ) const
{
	Vec2  p = ScreenToLogicalPoint( _screenRect, _pos );

	_inputSlot = -1;
	_outputSlot = -1;
	_paramSlot = -1;

	for( const auto& pNode : m_AllNodes )
	{
		if( pNode->m_UIRect.PointInRect( p ) )
		{
			Vec2 relativePos = p - pNode->m_UIRect.Pos;

			for( uint32_t i=0 ; i < pNode->m_InputSlots.size() ; ++i )
			{
				const ComputeNode::InputSlot& slot = pNode->m_InputSlots[ i ];

				if( slot.m_UIRect.PointInRect( relativePos ) )
				{
					_inputSlot = i;
					break;
				}
			}

			for( uint32_t i = 0 ; i < pNode->m_OutputSlots.size() ; ++i )
			{
				const ComputeNode::OutputSlot& slot = pNode->m_OutputSlots[ i ];

				if( slot.m_UIRect.PointInRect( relativePos ) )
				{
					_outputSlot = i;
					break;
				}
			}

			//TODO same for paramSlots
			return pNode;
		}
	}

	return nullptr;
}

ID3D11ShaderResourceView* NodeEditor::GetPreviewHeight() const
{
	shared_ptr<ComputeNode> pPreviewedNode = m_pPreviewedNode.lock();

	if( !pPreviewedNode )
		return nullptr;
	
	if( strcmp( pPreviewedNode->GetNodeClassName(), "PreviewNode" ) == 0 )
	{
		shared_ptr<PreviewNode> pPreviewNode = std::static_pointer_cast<PreviewNode, ComputeNode>( pPreviewedNode );
		return pPreviewNode->GetHeightMap()->GetSRV();
	}

	if( pPreviewedNode->IsDirty() )
		pPreviewedNode->Compute();

	return pPreviewedNode->GetOutput( 0 )->GetSRV();
}

ID3D11ShaderResourceView* NodeEditor::GetPreviewAlbedo() const
{
	shared_ptr<ComputeNode> pPreviewedNode = m_pPreviewedNode.lock();

	if( !pPreviewedNode )
		return nullptr;

	if( strcmp( pPreviewedNode->GetNodeClassName(), "PreviewNode" ) != 0 )
		return nullptr;
	
	shared_ptr<PreviewNode> pPreviewNode = std::static_pointer_cast<PreviewNode, ComputeNode>(pPreviewedNode);

	if( pPreviewNode->IsDirty() )
		pPreviewNode->Compute();

	const Map* pAlbedoMap = pPreviewNode->GetAlbedoMap();

	return pAlbedoMap ? pAlbedoMap->GetSRV() : nullptr;
}

ID3D11ShaderResourceView* NodeEditor::GetPreviewWater() const
{
	shared_ptr<ComputeNode> pPreviewedNode = m_pPreviewedNode.lock();

	if( !pPreviewedNode )
		return nullptr;

	if( strcmp( pPreviewedNode->GetNodeClassName(), "PreviewNode" ) != 0 )
		return nullptr;

	shared_ptr<PreviewNode> pPreviewNode = std::static_pointer_cast<PreviewNode, ComputeNode>(pPreviewedNode);
	
	if( pPreviewNode->IsDirty() )
		pPreviewNode->Compute();

	const Map* pWaterMap = pPreviewNode->GetWaterMap();
		
	return pWaterMap ? pWaterMap->GetSRV() : nullptr;
}

ID3D11ShaderResourceView* NodeEditor::GetPreviewNormal()
{
	shared_ptr<ComputeNode> pPreviewNode = m_pPreviewedNode.lock();

	if( !pPreviewNode )
		return nullptr;

	if( m_pNormalNode->IsDirty() )
		m_pNormalNode->Compute();

	return m_pNormalNode->GetOutput( 0 )->GetSRV();
}

bool NodeEditor::IsPreviewingColorMap() const
{
	shared_ptr<ComputeNode> pPreviewedNode = m_pPreviewedNode.lock();

	if( !pPreviewedNode )
		return nullptr;

	if( strcmp( pPreviewedNode->GetNodeClassName(), "PreviewNode" ) == 0 )
		return false;

	const Map* pPreviewedMap = pPreviewedNode->GetOutput( 0 );
	
	if( !pPreviewedMap )
		return false;

	D3D11_TEXTURE2D_DESC desc;
		pPreviewedMap->GetTex()->GetDesc( &desc ); //TODO store resolution and format in Map class
	
	return desc.Format != DXGI_FORMAT_R32_FLOAT;
}


void DrawRectOutline( CDC* _dc, const RECT& r )
{
	_dc->MoveTo( r.left, r.top );
	_dc->LineTo( r.right, r.top );
	_dc->LineTo( r.right, r.bottom );
	_dc->LineTo( r.left, r.bottom );
	_dc->LineTo( r.left, r.top );
}



void GradientRect( CDC* _dc, const RECT& rect, COLORREF col1, COLORREF col2 )
{	
	#define COLORREF_TO_COLOR16( c, o ) (COLOR16)((((c)>>o) & 0xFF) << 8)
	#define COLORREF_R_TO_COLOR16( c ) COLORREF_TO_COLOR16( c, 0 )
	#define COLORREF_G_TO_COLOR16( c ) COLORREF_TO_COLOR16( c, 8 )
	#define COLORREF_B_TO_COLOR16( c ) COLORREF_TO_COLOR16( c, 16 )

	TRIVERTEX tv[ 2 ] =
	{
		{ rect.left, rect.top,       COLORREF_R_TO_COLOR16( col1 ), COLORREF_G_TO_COLOR16( col1 ), COLORREF_B_TO_COLOR16( col1 ), 65280 },
		{ rect.right, rect.bottom, COLORREF_R_TO_COLOR16( col2 ), COLORREF_G_TO_COLOR16( col2 ), COLORREF_B_TO_COLOR16( col2 ), 65280 }
	};
	GRADIENT_RECT gr = { 0, 1 };

	_dc->GradientFill( tv, 2, &gr, 1, GRADIENT_FILL_RECT_V );
}

void DrawShadowedText( const char* _text, CDC* _dc, RECT _rect, UINT _format, UINT _shadowOffset )
{
	_shadowOffset = Max( _shadowOffset, 1U );

	CString str( _text );

	_dc->SetBkMode( TRANSPARENT );

	_dc->SetTextColor( RGB( 0, 0, 0 ) );
	_dc->DrawText( str, &_rect, _format );

	_rect.top -= _shadowOffset; _rect.bottom -= _shadowOffset; _rect.left -= _shadowOffset; _rect.right -= _shadowOffset;
	_dc->SetTextColor( RGB( 255, 255, 255 ) );
	_dc->DrawText( str, &_rect, _format );

}

void RelativeToAbsoluteRect( const UIRect& _rect, const UIRect& _parentRect, RECT& _out )
{
	_out.left   = (LONG)( _parentRect.Pos.x + _rect.Pos.x );
	_out.right  = (LONG)( _out.left + _rect.Size.x );
	_out.top    = (LONG)( _parentRect.Pos.y + _rect.Pos.y );
	_out.bottom = (LONG)( _out.top + _rect.Size.y );
}

void NodeEditor::DrawLink( CDC* _dc, const UIRect& _screenRect, const Vec2& _linkA, const Vec2& _linkB )
{
	Vec2 LA = LogicalToScreenPoint( _screenRect, _linkA );
	Vec2 LB = LogicalToScreenPoint( _screenRect, _linkB );

#if 0
	//TODO splines etc
	MoveToEx( _dc, _linkA.x, _linkA.y, nullptr );
	LineTo( _dc, _linkB.x, _linkB.y );
#else
	//bezier curve
	uint32_t detail = 24;

	Vec2 P1 = LA;
	Vec2 P2 = LA + Vec2( 70.0f * m_viewZoom, 0.0f );
	Vec2 P3 = LB - Vec2( 70.0f * m_viewZoom, 0.0f );
	Vec2 P4 = LB;

	_dc->MoveTo( (int)P1.x, (int)P1.y );

	for( uint32_t i = 1 ; i < detail ; ++i )
	{
		float t = (float)i / (float)(detail - 1);

		Vec2 p = CubicBezier( P1, P2, P3, P4, t );

		_dc->LineTo( (int)p.x, (int)p.y );
	
	}
	

#endif
}

void NodeEditor::ResizeFont()
{
	m_Font.DeleteObject();

	int fontSize = (int)(m_viewZoom * 90.0f);
	fontSize = Max( fontSize, 80 );

	BOOL res = m_Font.CreatePointFont( fontSize, _T( "Arial" ) );
	ASSERT( res );
}

Vec2 NodeEditor::ScreenToLogicalPoint( const UIRect& _screenRect, const Vec2& _p ) const
{
	Vec2 center( _screenRect.Center().x, _screenRect.Center().y );

	return m_ViewCenter + (_p - center) / m_viewZoom;
}

Vec2 NodeEditor::LogicalToScreenPoint( const UIRect& _screenRect, const Vec2& _p ) const
{
	Vec2 center( _screenRect.Center().x, _screenRect.Center().y );

	return  (_p - m_ViewCenter) * m_viewZoom + center;
}


void NodeEditor::DrawWithD3D11( const CRect& _screenRect )
{
	m_D3D11Data.m_pSwapChain->Present( 1, 0 );
}

void NodeEditor::DrawWithMFC( const CRect& _screenRect, CDC* _dc )
{
	UIRect screenRect;
	screenRect.Pos.x = (float)_screenRect.left;
	screenRect.Pos.y = (float)_screenRect.top;
	screenRect.Size.x = (float)_screenRect.Width();
	screenRect.Size.y = (float)_screenRect.Height();

	Vec2 center( (float)_screenRect.CenterPoint().x, (float)_screenRect.CenterPoint().y );

	_dc->FillSolidRect( &_screenRect, RGB( 50, 50, 50 ) );

	CPen* oldPen = _dc->SelectObject( &m_NodesPen );
	CFont* pOldFont = _dc->SelectObject( &m_Font );
	
	for( const auto pNode : m_AllNodes )
	{
		UIRect nodeRect = ( pNode->m_UIRect - m_ViewCenter ) * m_viewZoom + center;

		CRect rect;
		rect.left = (LONG)nodeRect.Pos.x;
		rect.top = (LONG)nodeRect.Pos.y;
		rect.right = (LONG)(nodeRect.Pos.x + nodeRect.Size.x);
		rect.bottom = (LONG)(nodeRect.Pos.y + nodeRect.Size.y);

		COLORREF nodeColor1, nodeColor2;

		if( pNode == m_pPreviewedNode.lock() )
		{
			if( m_bLockPreview )
			{
				nodeColor1 = RGB( 255, 50, 50 );
				nodeColor2 = RGB( 255, 200, 200 );
			}
			else
			{
				nodeColor1 = RGB( 50, 255, 50 );
				nodeColor2 = RGB( 200, 250, 200 );
			}
		}
		else
		{
			nodeColor1 = RGB( 50, 50, 255 );
			nodeColor2 = RGB( 200, 200, 255 );
		}

		GradientRect( _dc, rect, nodeColor1, nodeColor2 );

		DrawShadowedText( pNode->m_UIName.c_str(), _dc, rect, DT_CENTER, (UINT)(1.0f * m_viewZoom) );
		

		std::string resInfoTxt( Format( "%d ", pNode->GetResolution() ) );
		
		if( ( pNode->GetResolutionModifier() != 1 ) && ( pNode->GetResolutionReference() != Res_Fixed ) )
		{
			if( pNode->GetResolutionModifier() > 0 )
				resInfoTxt += Format( "(x%d)", pNode->GetResolutionModifier() );
			else if( pNode->GetResolutionModifier() < 0 )
				resInfoTxt += Format( "(1/%d)", -pNode->GetResolutionModifier() );
		}

		CRect resInfoRect( rect );
		resInfoRect.top    += (LONG)(nodeRect.Size.y + 0.001f);
		resInfoRect.bottom += (LONG)(nodeRect.Size.y + 0.001f);

		DrawShadowedText( resInfoTxt.c_str(), _dc, resInfoRect, DT_CENTER, (UINT)( 1.0f * m_viewZoom ) );

		DrawRectOutline( _dc, rect );

		if( Find( m_SelectedNodes, pNode ) != m_SelectedNodes.end() )
		{
			//Node is selected

			rect.InflateRect( 4, 4 );
			DrawRectOutline( _dc, rect );
		}


		uint32_t slotIndex = 0;
		for( const ComputeNode::InputSlot& slot : pNode->m_InputSlots )
		{ 
			RECT slotRect;
			RelativeToAbsoluteRect( slot.m_UIRect * m_viewZoom, nodeRect, slotRect );

			COLORREF slotColor;
			if( ( pNode == g_HoveringNode ) && ( g_HoveringSlotCategory == IOSlotCategory::Input ) && ( slotIndex == g_HoveringSlotIndex ) )
				slotColor = RGB( 255, 50, 50 );
			else
				slotColor = RGB( 50, 255, 50 );

			GradientRect( _dc, slotRect, slotColor, RGB( 200, 255, 200 ) );
			DrawRectOutline( _dc, slotRect );
			++slotIndex;
		}

		slotIndex = 0;
		for( const ComputeNode::OutputSlot& slot : pNode->m_OutputSlots )
		{
			RECT slotRect;
			RelativeToAbsoluteRect( slot.m_UIRect * m_viewZoom, nodeRect, slotRect );

			COLORREF slotColor;
			if( (pNode == g_HoveringNode) && (g_HoveringSlotCategory == IOSlotCategory::Output) && (slotIndex == g_HoveringSlotIndex) )
				slotColor = RGB( 255, 50, 50 );
			else
				slotColor = RGB( 50, 255, 50 );

			GradientRect( _dc, slotRect, slotColor, RGB( 200, 255, 200 ) );
			DrawRectOutline( _dc, slotRect );
			++slotIndex;
		}
	}

	_dc->SelectObject( pOldFont );


	//LINKS
	///////

	for( const auto pNode : m_AllNodes )
	{
		for( const ComputeNode::InputSlot& slot : pNode->m_InputSlots )
		{
			shared_ptr<ComputeNode> inputNode = slot.m_RemoteOutputSlot.m_pNode.lock();

			if( !inputNode )
				continue;

			const ComputeNode::OutputSlot & connectedSlot = inputNode->m_OutputSlots[ slot.m_RemoteOutputSlot.m_SlotIndex ];

			Vec2 linkA = pNode->m_UIRect.Pos + slot.m_UIRect.Center();
			Vec2 linkB = inputNode->m_UIRect.Pos + connectedSlot.m_UIRect.Center();
		
			_dc->SelectObject( connectedSlot.m_DataType == IOType::Color ? &m_RGBLinksPen : &m_LinksPen );

			DrawLink( _dc, screenRect, linkB, linkA );
		}
	}

	if( g_DragLink_Node )
	{
		Vec2 linkA = g_DragLink_Node->m_UIRect.Pos;

		Vec2 lastMousePos = ScreenToLogicalPoint( screenRect, g_LastNodeEditorMousePos );

		switch( g_DragLink_SlotCategory )
		{
		case IOSlotCategory::Input:
			linkA += g_DragLink_Node->m_InputSlots[ g_DragLink_SlotIndex ].m_UIRect.Center(); 
			DrawLink( _dc, screenRect, lastMousePos, linkA );
			break;
		case IOSlotCategory::Output:
			linkA += g_DragLink_Node->m_OutputSlots[ g_DragLink_SlotIndex ].m_UIRect.Center(); 
			DrawLink( _dc, screenRect, linkA, lastMousePos );
			break;
	//	case IOSlotCategory::Param:
		//	linkA = g_DragLink_Node->m_ParamSlots[ g_DragLink_SlotIndex ].m_UIRect.Center(); break;
		}

	}

	_dc->SelectObject( oldPen );

	//TOOLTIP
	/////////

	if( !g_ToolTipText.empty() )
	{
		RECT rect;
		rect.left = (LONG)g_ToolTipPos.x;
		rect.right = (LONG)g_ToolTipPos.x + 100;
		rect.top = (LONG)g_ToolTipPos.y;
		rect.bottom = (LONG)g_ToolTipPos.y + 20;

		_dc->SetTextColor( RGB( 0,0,0 ) );
		_dc->DrawText( CString( g_ToolTipText.c_str() ), &rect, DT_CENTER );
	}
}

bool NodeEditor::CreateLink(	shared_ptr<ComputeNode> _nodeA, uint32_t _slotIndexA, //source
								shared_ptr<ComputeNode> _nodeB, uint32_t _slotIndexB )//dest
{
	assert( _nodeA && _nodeB );

	ComputeNode::OutputSlot& slotA = _nodeA->m_OutputSlots[ _slotIndexA ];
	ComputeNode::InputSlot& slotB = _nodeB->m_InputSlots[ _slotIndexB ];

	if( ((slotA.m_DataType == IOType::FloatOrColor) && ((slotB.m_DataType == IOType::Float) || (slotB.m_DataType == IOType::Color)) ) ||
		((slotB.m_DataType == IOType::FloatOrColor) && ((slotA.m_DataType == IOType::Float) || (slotA.m_DataType == IOType::Color)) ) )
	{
	
	}
	else if( slotA.m_DataType != slotB.m_DataType )
		return false;

	shared_ptr<ComputeNode> pPreviousInputNode = slotB.m_RemoteOutputSlot.m_pNode.lock();

	if( pPreviousInputNode )
	{
		//break old link

		ComputeNode::OutputSlot& slotToDisconnect = pPreviousInputNode->m_OutputSlots[ slotB.m_RemoteOutputSlot.m_SlotIndex ];
		
		for( auto it = slotToDisconnect.m_RemoteInputSlots.begin() ;
			it != slotToDisconnect.m_RemoteInputSlots.end(); ++it )
		{
			if( it->m_pNode.lock() == _nodeB )
			{
				slotToDisconnect.m_RemoteInputSlots.erase( it );
				break;
			}
		}

		pPreviousInputNode->OnOutputConnectionChanged( slotB.m_RemoteOutputSlot.m_SlotIndex );
	}


	ComputeNode::RemoteSlot remoteSlot;
	remoteSlot.m_pNode = _nodeB;
	remoteSlot.m_SlotIndex = _slotIndexB;

	slotA.m_RemoteInputSlots.push_back( remoteSlot );

	slotB.m_RemoteOutputSlot.m_pNode = _nodeA;
	slotB.m_RemoteOutputSlot.m_SlotIndex = _slotIndexA;

	_nodeB->SetDirty(); //it's input has changed...

	_nodeA->OnOutputConnectionChanged( _slotIndexA );
	_nodeB->OnInputConnectionChanged( _slotIndexB );

	return true;
}

void NodeEditor::SelectNode( shared_ptr<ComputeNode> _pNode, bool _addToSelection )
{
	if( !_addToSelection )
	{
		m_SelectedNodes.clear();
	}
	else
	{
		if( Find( m_SelectedNodes, _pNode ) != m_SelectedNodes.end() )
			return;	//ensure it is not already selected
	}

	m_SelectedNodes.push_back( _pNode );
}

void NodeEditor::DeleteSelectedNodes()
{
	for( auto pNode : m_SelectedNodes )
	{
		Remove( m_AllNodes, pNode.lock() );

		if( pNode.lock() == m_pPreviewedNode.lock() )
		{
			m_pPreviewedNode.reset();
		}
	}

	m_SelectedNodes.clear();
}

void NodeEditor::DeleteAllNodes()
{
	m_AllNodes.clear();
	m_pPreviewedNode.reset();
	m_SelectedNodes.clear();
}

void NodeEditor::OnResolutionChange()
{
	for( auto pNode : m_AllNodes )
	{
		pNode->UpdateInternalResolution();
		pNode->SetDirty();
		pNode->OnResolutionChanged();
	}

	m_pNormalNode->UpdateInternalResolution();
	m_pNormalNode->SetDirty();
	m_pNormalNode->OnResolutionChanged();

}


bool NodeEditor::Load( const char* _filename )
{
	tinyxml2::XMLDocument xmlDoc;

	if( xmlDoc.LoadFile( _filename ) != tinyxml2::XML_SUCCESS )
		return false;

	tinyxml2::XMLElement* xmlHeader = xmlDoc.FirstChildElement( "Header" );

	if( !xmlHeader )
		return false;

	float version = xmlHeader->FloatAttribute( "Version" ); //TODO don't use float

//	if( version != GEOGEN_VERSION ) //TODO backward compat (and per node version, like world machine)
//		return false;

	
	theApp.m_Resolution		= xmlHeader->IntAttribute( "Resolution");
	theApp.m_MinAltitude	= xmlHeader->IntAttribute( "MinAltitude" );
	theApp.m_MaxAltitude	= xmlHeader->IntAttribute( "MaxAltitude" );
	theApp.m_SeaLevel		= xmlHeader->IntAttribute( "SeaLevel" );
	theApp.m_TerrainExtent	= xmlHeader->IntAttribute( "TerrainExtent" );
	
	//Load nodes
	////////////

	tinyxml2::XMLElement* xmlNodes = xmlDoc.FirstChildElement( "Nodes" );

	if( !xmlNodes )
		return false;

	m_AllNodes.clear();
	
	tinyxml2::XMLElement* xmlNode = xmlNodes->FirstChildElement();

	while( xmlNode )
	{
		shared_ptr<ComputeNode> pNode( g_NodeFactory.CreateNode( xmlNode->Name() ) );

		if( pNode )
		{ 
			if( !pNode->Load( xmlNode ) )
			{
				//LOG error
			}
		}
		else
		{
			//TODO log "unknown node"
		}

		m_AllNodes.push_back( pNode );

		xmlNode = xmlNode->NextSiblingElement();
	}

	//Init nodes
	////////////

	for( auto pNode : m_AllNodes )
	{
		pNode->UpdateInternalResolution();
		
		if( !pNode->OneTimeInit() )
		{
			assert( false );
			LOG_R( "Node %s failed in OneTimeInit()", pNode->GetName() );
			continue;
		}
	}

	//Load node links
	/////////////////


	tinyxml2::XMLElement* xmlNodeLinks = xmlDoc.FirstChildElement( "NodeLinks" );

	tinyxml2::XMLElement* xmlNodeLink = xmlNodeLinks->FirstChildElement();

	while( xmlNodeLink )
	{
		int nodeA = xmlNodeLink->IntAttribute( "NodeA" );
		int nodeA_slot = xmlNodeLink->IntAttribute( "NodeA_slotIdx" );
		int nodeB = xmlNodeLink->IntAttribute( "NodeB" );
		int nodeB_slot = xmlNodeLink->IntAttribute( "NodeB_slotIdx" );

		CreateLink( m_AllNodes[ nodeA ], nodeA_slot, m_AllNodes[ nodeB ], nodeB_slot );

		xmlNodeLink = xmlNodeLink->NextSiblingElement();
	}

	OnResolutionChange();

	for( auto pNode : m_AllNodes )
	{
		pNode->OnInputConnectionChanged(0); //kind of hacky ...
	}

	return true;
}

template <class T>
int Find( const vector< T > & _vec, const T& _val )
{
	for( uint32_t i = 0 ; i < _vec.size() ; ++i )
	{
		if( _vec[ i ] == _val )
			return i;
	}

	return -1;
}

bool NodeEditor::Save( const char* _filename ) const
{
	tinyxml2::XMLDocument xmlDoc;

	tinyxml2::XMLElement* xmlHeader = xmlDoc.NewElement( "Header" );
	xmlHeader->SetAttribute( "VersionMajor", 0 );
	xmlHeader->SetAttribute( "VersionMinor", 1 );

	xmlHeader->SetAttribute( "Resolution",    (int)theApp.GetResolution() );
	xmlHeader->SetAttribute( "MinAltitude",   theApp.GetMinAltitude() );
	xmlHeader->SetAttribute( "MaxAltitude",   theApp.GetMaxAltitude() );
	xmlHeader->SetAttribute( "SeaLevel",      theApp.GetSeaLevel() );
	xmlHeader->SetAttribute( "TerrainExtent", (int)theApp.GetTerrainExtent() );
	xmlDoc.InsertEndChild( xmlHeader );


	//Nodes

	tinyxml2::XMLElement* xmlNodes = xmlDoc.NewElement( "Nodes" );

	for( auto pNode : m_AllNodes )
	{
		tinyxml2::XMLElement* xmlNode = pNode->Save( xmlDoc );

		xmlNodes->InsertEndChild( xmlNode );
	}

	xmlDoc.InsertEndChild( xmlNodes );

	//Links

	tinyxml2::XMLElement* xmlNodeLinks = xmlDoc.NewElement( "NodeLinks" );

	int iNode = 0;

	for( auto pNode : m_AllNodes )
	{
		int iSlot = 0;

		for( const ComputeNode::InputSlot& inputSlot : pNode->m_InputSlots )
		{
			tinyxml2::XMLElement* xmlNodeLink = xmlDoc.NewElement( "NodeLink" );

			shared_ptr<ComputeNode> pRemoteNode = inputSlot.m_RemoteOutputSlot.m_pNode.lock();

			if( pRemoteNode )
			{
				int remoteNodeIndex = Find( m_AllNodes, pRemoteNode );

				xmlNodeLink->SetAttribute( "NodeA", remoteNodeIndex );
				xmlNodeLink->SetAttribute( "NodeA_slotIdx", (int)inputSlot.m_RemoteOutputSlot.m_SlotIndex );
				xmlNodeLink->SetAttribute( "NodeB", iNode );
				xmlNodeLink->SetAttribute( "NodeB_slotIdx", iSlot );

				xmlNodeLinks->InsertEndChild( xmlNodeLink );
			}

			++iSlot;
		}

		++iNode;
	}

	xmlDoc.InsertEndChild( xmlNodeLinks );


	//save to disk

	if( xmlDoc.SaveFile( _filename ) != tinyxml2::XML_SUCCESS )
		return false;

	return true;
}