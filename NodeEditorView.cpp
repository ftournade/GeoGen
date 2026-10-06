#include "stdafx.h"
#include "GeoGen.h"
#include "NodeEditorView.h"

#include "NodeEditor.h"
#include "NodeFactory.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BASE_WM_COMMAND_FOR_NODE_CREATION 10000

#define WM_SET_NODE_PREVIEW_AS_HEIGHTMAP 9000
#define WM_SET_NODE_PREVIEW_AS_MASK 9001

#define WM_SET_NODE_RESOLUTION_REF_GLOBAL_SETTING 9005
#define WM_SET_NODE_RESOLUTION_REF_FIRST_INPUT    9006
#define WM_SET_NODE_RESOLUTION_REF_FIXED          9007


#define WM_SET_NODE_RESOLUTION_MULTIPLIER_4X   9010
#define WM_SET_NODE_RESOLUTION_MULTIPLIER_2X   9011
#define WM_SET_NODE_RESOLUTION_MULTIPLIER_1X   9012
#define WM_SET_NODE_RESOLUTION_MULTIPLIER_1_2X 9013
#define WM_SET_NODE_RESOLUTION_MULTIPLIER_1_4X 9014
#define WM_SET_NODE_RESOLUTION_MULTIPLIER_1_8X 9015


extern Vec2 g_LastNodeEditorMousePos;

bool g_bDraggingView = false;

extern shared_ptr<ComputeNode> g_DragNode;
extern Vec2 g_ToolTipPos;
extern std::string g_ToolTipText;

extern shared_ptr<ComputeNode> g_HoveringNode;
extern uint32_t g_HoveringSlotIndex;
extern IOSlotCategory g_HoveringSlotCategory;

extern shared_ptr<ComputeNode> g_DragLink_Node;
extern uint32_t g_DragLink_SlotIndex;
extern IOSlotCategory g_DragLink_SlotCategory;

shared_ptr< ComputeNode > g_pNodeHavingPoppedUpOptionsMenu;

///////////////////////////////////////////////

NodeEditorView::NodeEditorView() :
	m_bPopupMenusInitialized(false),
	m_bDrawWithD3D11(true),
	m_pD3D11SwapChain(nullptr),
	m_pD3D11BackBuffer(nullptr)
{
}

NodeEditorView::~NodeEditorView()
{
}


BEGIN_MESSAGE_MAP(NodeEditorView, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_COMMAND_RANGE( BASE_WM_COMMAND_FOR_NODE_CREATION, BASE_WM_COMMAND_FOR_NODE_CREATION + 700, &NodeEditorView::OnAddNode )

	ON_COMMAND_RANGE( WM_SET_NODE_PREVIEW_AS_HEIGHTMAP, WM_SET_NODE_PREVIEW_AS_MASK, &NodeEditorView::OnSetNodePreviewMode )
	ON_COMMAND_RANGE( WM_SET_NODE_RESOLUTION_REF_GLOBAL_SETTING, WM_SET_NODE_RESOLUTION_REF_FIXED, &NodeEditorView::OnSetNodeResolutionReference )
	ON_COMMAND_RANGE( WM_SET_NODE_RESOLUTION_MULTIPLIER_4X, WM_SET_NODE_RESOLUTION_MULTIPLIER_1_8X, &NodeEditorView::OnSetNodeResolutionModifier )
END_MESSAGE_MAP()

// NodeEditorView message handlers

BOOL NodeEditorView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void NodeEditorView::InitPopupMenus()
{
	ASSERT( !m_bPopupMenusInitialized );

	m_GenNodeCreationSubMenu.CreatePopupMenu();
	m_ModifierNodeCreationSubMenu.CreatePopupMenu();
	m_CombinerNodeCreationSubMenu.CreatePopupMenu();
	m_MaskNodeCreationSubMenu.CreatePopupMenu();
	m_NaturalNodeCreationSubMenu.CreatePopupMenu();
	m_IONodeCreationSubMenu.CreatePopupMenu();

	const NodeFactory::FactoryMap& factoryMap = g_NodeFactory.GetFactories();

	for( const auto & nodeInfo : factoryMap )
	{
		CMenu* pSubMenu;

		switch( nodeInfo.second.m_Category )
		{
			case NodeCategory::Generator:   pSubMenu = &m_GenNodeCreationSubMenu; break;
			case NodeCategory::Modifier:    pSubMenu = &m_ModifierNodeCreationSubMenu; break;
			case NodeCategory::Combiner:    pSubMenu = &m_CombinerNodeCreationSubMenu; break;
			case NodeCategory::Mask:		pSubMenu = &m_MaskNodeCreationSubMenu; break;
			case NodeCategory::Natural:		pSubMenu = &m_NaturalNodeCreationSubMenu; break;
			case NodeCategory::InputOutput: pSubMenu = &m_IONodeCreationSubMenu; break;
		}

		pSubMenu->AppendMenu( MF_STRING,
			BASE_WM_COMMAND_FOR_NODE_CREATION + nodeInfo.first, 
			CString( nodeInfo.second.m_UIName.c_str() ) );

	}

	m_MainContextMenu.CreatePopupMenu();
	m_MainContextMenu.AppendMenu( MF_POPUP, (UINT_PTR)m_GenNodeCreationSubMenu.m_hMenu, _T( "Generators" ) );
	m_MainContextMenu.AppendMenu( MF_POPUP, (UINT_PTR)m_ModifierNodeCreationSubMenu.m_hMenu, _T( "Modifiers" ) );
	m_MainContextMenu.AppendMenu( MF_POPUP, (UINT_PTR)m_CombinerNodeCreationSubMenu.m_hMenu, _T( "Combiners" ) );
	m_MainContextMenu.AppendMenu( MF_POPUP, (UINT_PTR)m_MaskNodeCreationSubMenu.m_hMenu, _T( "Masks" ) );
	m_MainContextMenu.AppendMenu( MF_POPUP, (UINT_PTR)m_NaturalNodeCreationSubMenu.m_hMenu, _T( "Natural" ) );
	m_MainContextMenu.AppendMenu( MF_POPUP, (UINT_PTR)m_IONodeCreationSubMenu.m_hMenu, _T( "IO" ) );

	m_bPopupMenusInitialized = true;
}

void NodeEditorView::InitNodeOptionsPopupMenus( const CPoint& _menuPos )
{
	CMenu nodeOptionMenu;
	CMenu previewModeOptionMenu;
	CMenu resolutionOptionMenu;

	nodeOptionMenu.CreatePopupMenu();
	previewModeOptionMenu.CreatePopupMenu();
	resolutionOptionMenu.CreatePopupMenu();
	
	previewModeOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_PREVIEW_AS_HEIGHTMAP, _T( "Heightmap" ) );
	previewModeOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_PREVIEW_AS_MASK, _T( "Mask" ) );

	previewModeOptionMenu.CheckMenuRadioItem(	WM_SET_NODE_PREVIEW_AS_HEIGHTMAP,
												WM_SET_NODE_PREVIEW_AS_MASK,
												g_pNodeHavingPoppedUpOptionsMenu->m_bPreviewAsHeightField ? WM_SET_NODE_PREVIEW_AS_HEIGHTMAP : WM_SET_NODE_PREVIEW_AS_MASK,
												MF_BYCOMMAND );

	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_REF_GLOBAL_SETTING, _T( "Relative to global resolution" ) );
	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_REF_FIRST_INPUT, _T( "Relative to first input" ) );
	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_REF_FIXED, _T( "Fixed" ) );
	
	UINT currentResReference;

	switch( g_pNodeHavingPoppedUpOptionsMenu->GetResolutionReference() )
	{
		case Res_GlobalSetting: currentResReference = WM_SET_NODE_RESOLUTION_REF_GLOBAL_SETTING; break;
		case Res_MainInput: currentResReference = WM_SET_NODE_RESOLUTION_REF_FIRST_INPUT; break;
		case Res_Fixed: currentResReference = WM_SET_NODE_RESOLUTION_REF_FIXED; break;
	}

	resolutionOptionMenu.CheckMenuRadioItem(	WM_SET_NODE_RESOLUTION_REF_GLOBAL_SETTING,
												WM_SET_NODE_RESOLUTION_REF_FIXED,
												currentResReference,
												MF_BYCOMMAND );

	resolutionOptionMenu.AppendMenu( MF_SEPARATOR );
	
	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_MULTIPLIER_4X, _T( "4x" ) );
	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_MULTIPLIER_2X, _T( "2x" ) );
	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_MULTIPLIER_1X, _T( "1x" ) );
	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_MULTIPLIER_1_2X, _T( "1/2x" ) );
	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_MULTIPLIER_1_4X, _T( "1/4x" ) );
	resolutionOptionMenu.AppendMenu( MF_STRING, WM_SET_NODE_RESOLUTION_MULTIPLIER_1_8X, _T( "1/8x" ) );


	int32_t resModifier = g_pNodeHavingPoppedUpOptionsMenu->GetResolutionModifier();
	UINT currentResModifier = WM_SET_NODE_RESOLUTION_MULTIPLIER_1X;

	switch( resModifier )
	{
		case  4: currentResModifier = WM_SET_NODE_RESOLUTION_MULTIPLIER_4X; break;
		case  2: currentResModifier = WM_SET_NODE_RESOLUTION_MULTIPLIER_2X; break;
		case  1: currentResModifier = WM_SET_NODE_RESOLUTION_MULTIPLIER_1X; break;
		case -2: currentResModifier = WM_SET_NODE_RESOLUTION_MULTIPLIER_1_2X; break;
		case -4: currentResModifier = WM_SET_NODE_RESOLUTION_MULTIPLIER_1_4X; break;
		case -8: currentResModifier = WM_SET_NODE_RESOLUTION_MULTIPLIER_1_8X; break;
	}

	resolutionOptionMenu.CheckMenuRadioItem(	WM_SET_NODE_RESOLUTION_MULTIPLIER_4X, 
												WM_SET_NODE_RESOLUTION_MULTIPLIER_1_8X,
												currentResModifier, MF_BYCOMMAND );

	nodeOptionMenu.AppendMenu( MF_POPUP, (UINT_PTR)previewModeOptionMenu.m_hMenu, _T( "Preview mode" ) );
	nodeOptionMenu.AppendMenu( MF_POPUP, (UINT_PTR)resolutionOptionMenu.m_hMenu, _T( "Resolution modifier" ) );

	nodeOptionMenu.TrackPopupMenu( TPM_CENTERALIGN + TPM_LEFTBUTTON, _menuPos.x, _menuPos.y, this, NULL );
}


void NodeEditorView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect r;
	GetClientRect( &r );

	if( false ) //m_bDrawWithD3D11
	{
		//g_Renderer.Init()

		//g_NodeEditor.DrawWithD3D11( r );

	}
	else
	{
		//Draw with MFC/Win32

		if( (r.Width() != m_backBuffer.GetWidth()) ||
			(r.Height() != m_backBuffer.GetHeight()) )
		{
			if( !m_backBuffer.Init( GetSafeHwnd(), r.Width(), r.Height() ) )
			{
				//TODO error handling
				ASSERT( false );
			}
		}

		CDC* pMemDC = CDC::FromHandle( m_backBuffer.GetBackBufferDC() );

		g_NodeEditor.DrawWithMFC( r, pMemDC );

		uint32_t freeGPUMemory, totalGPUMemory;
		if( g_Renderer.GetMemoryStatistics( freeGPUMemory, totalGPUMemory ) )
		{
			CString str;
			str.Format( _T( "free GPU Memory: %d/%d (Mb)" ), freeGPUMemory / 1024, totalGPUMemory / 1024 );
			pMemDC->SetTextColor( RGB( 155, 155, 0 ) );
			pMemDC->DrawText( str, &r, DT_LEFT );
		}

		m_backBuffer.Blit( dc.GetSafeHdc() );

	}
}



void NodeEditorView::OnLButtonDown( UINT nFlags, CPoint point )
{
//	CWnd::OnLButtonDown( nFlags, point );

	UIRect screenRect = GetUIRect();

	SetFocus();

	Vec2 mousePos( (float)point.x, (float)point.y );

	int inputSlot, outputSlot, paramSlot;
	shared_ptr<ComputeNode> pHitNode = g_NodeEditor.MouseHitTest( screenRect, mousePos, inputSlot, outputSlot, paramSlot );


	if( g_DragLink_Node )
	{
		if( pHitNode )
		{
			//TODO more safety checks (e.g. bind input/input, output/output or input on same node output)

			if( ( inputSlot >= 0 ) && ( g_DragLink_SlotCategory == IOSlotCategory::Output ) )
			{
				if( g_NodeEditor.CreateLink( g_DragLink_Node, g_DragLink_SlotIndex, pHitNode, inputSlot ) )
				{
					g_DragLink_Node = nullptr;
					theApp.RedrawPreview();
				}
			}
			else if( ( outputSlot >= 0 ) && ( g_DragLink_SlotCategory == IOSlotCategory::Input ) )
			{
				if( g_NodeEditor.CreateLink( pHitNode, outputSlot, g_DragLink_Node, g_DragLink_SlotIndex ) )
				{
					g_DragLink_Node = nullptr;
					theApp.RedrawPreview();
				}
			}
			else
			{
				g_DragLink_Node = nullptr;
			}
		}
		else
		{
			//Cancel link creation
			g_DragLink_Node = nullptr;

			Invalidate( FALSE );
		}

	}
	else
	{
		if( pHitNode )
		{
			if( inputSlot >= 0 )
			{
				g_DragLink_Node = pHitNode;
				g_DragLink_SlotIndex = inputSlot;
				g_DragLink_SlotCategory = IOSlotCategory::Input;
			}
			else if( outputSlot >= 0 )
			{
				g_DragLink_Node = pHitNode;
				g_DragLink_SlotIndex = outputSlot;
				g_DragLink_SlotCategory = IOSlotCategory::Output;
			}
			else
			{
				g_NodeEditor.SetPreviewNode( pHitNode );

				bool bAddToSelection = GetAsyncKeyState( VK_CONTROL ) != 0;
				g_NodeEditor.SelectNode( pHitNode, bAddToSelection );
				
				theApp.UpdatePropertyGrid( pHitNode );
				g_DragNode = pHitNode;

				Invalidate( FALSE );
			}
		}
		else
		{
			g_bDraggingView = true;
		}
	}

	g_LastNodeEditorMousePos = mousePos;

}


void NodeEditorView::OnLButtonUp( UINT nFlags, CPoint point )
{
//	CWnd::OnLButtonUp( nFlags, point );
	g_bDraggingView = false;
	g_DragNode = nullptr;
}


void NodeEditorView::OnRButtonDown( UINT nFlags, CPoint point )
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnRButtonDown( nFlags, point );
}


void NodeEditorView::OnRButtonUp( UINT nFlags, CPoint point )
{
	UIRect screenRect = GetUIRect();

	Vec2 mousePos( (float)point.x, (float)point.y );
	
	int inputSlot, outputSlot, paramSlot;
	shared_ptr<ComputeNode> pHitNode = g_NodeEditor.MouseHitTest( screenRect, mousePos, inputSlot, outputSlot, paramSlot );

	CPoint menuPos = point;
	ClientToScreen( &menuPos );

	if( pHitNode )
	{
		//Node options
		g_pNodeHavingPoppedUpOptionsMenu = pHitNode;

		InitNodeOptionsPopupMenus( menuPos );
		return;
	}
	else
	{
		//Node creation

		if( !m_bPopupMenusInitialized )
			InitPopupMenus();

		m_PopupMenuPos = point;
	}

	
	BOOL ret = m_MainContextMenu.TrackPopupMenu( TPM_CENTERALIGN + TPM_LEFTBUTTON, menuPos.x, menuPos.y, this, NULL );

	//CWnd::OnRButtonUp( nFlags, point );
}


UIRect NodeEditorView::GetUIRect() const
{
	CRect r;
	GetClientRect( &r );

	UIRect screenRect;
	screenRect.Pos.x = r.left;
	screenRect.Pos.y = r.top;
	screenRect.Size.x = r.Width();
	screenRect.Size.y = r.Height();
	return screenRect;
}

void NodeEditorView::OnMouseMove( UINT nFlags, CPoint point )
{
//	CWnd::OnMouseMove( nFlags, point );

	UIRect screenRect = GetUIRect();

	const float mouseSpeed = 1.0f;

	Vec2 mousePos( (float)point.x, (float)point.y );
	Vec2 mouseMovement = (mousePos - g_LastNodeEditorMousePos) * mouseSpeed;

	int inputSlot, outputSlot, paramSlot;
	shared_ptr<ComputeNode> pHitNode = g_NodeEditor.MouseHitTest( screenRect, mousePos, inputSlot, outputSlot, paramSlot );

	bool bRedraw = true;

	if( inputSlot >= 0 )
	{
		g_HoveringNode = pHitNode;
		g_HoveringSlotIndex = inputSlot;
		g_HoveringSlotCategory = IOSlotCategory::Input;
		const ComputeNode::InputSlot& slot = pHitNode->m_InputSlots[ inputSlot ];
		g_ToolTipText = slot.m_Name;
		g_ToolTipPos = mousePos;
	}
	else if( outputSlot >= 0 )
	{
		g_HoveringNode = pHitNode;
		g_HoveringSlotIndex = outputSlot;
		g_HoveringSlotCategory = IOSlotCategory::Output;
		const ComputeNode::OutputSlot& slot = pHitNode->m_OutputSlots[ outputSlot ];
		g_ToolTipText = slot.m_Name;
		g_ToolTipPos = mousePos;
	}
	else if( g_bDraggingView )
	{
		Vec2 viewCenter = g_NodeEditor.GetViewCenter();
		viewCenter -= mouseMovement / g_NodeEditor.GetZoom();
		g_NodeEditor.SetViewCenter( viewCenter );
	}
	else
	{
		g_HoveringNode = nullptr;
		
		if( !g_DragNode && !g_DragLink_Node && g_ToolTipText.empty() )
			bRedraw = false;

		g_ToolTipText.clear();
		if( g_DragNode )
		{
			g_DragNode->m_UIRect.Pos += mouseMovement / g_NodeEditor.GetZoom();
		}
	}
	g_LastNodeEditorMousePos = mousePos;

	if( bRedraw )
		Invalidate( FALSE );
}


BOOL NodeEditorView::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	float zoom = g_NodeEditor.GetZoom();
	zoom *= 1.0f + 0.003f * zDelta;
	zoom = Max( zoom, 0.001f );
	g_NodeEditor.SetZoom( zoom );

	Invalidate( FALSE );

//	return CWnd::OnMouseWheel( nFlags, zDelta, pt );
	return TRUE;
}

void NodeEditorView::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	switch( nChar )
	{
	case 'L':
		g_NodeEditor.LockUnlockPreview();
		theApp.RedrawNodeEditor();
		break;
	case 'S':
	{
		shared_ptr<ComputeNode> pNode = g_NodeEditor.GetPreviewedNode().lock();
		
		if( pNode && pNode->IsIterativeComputation() )
			theApp.StartIterativeSim( pNode );

		break;
	}
	case VK_ESCAPE:
		theApp.StopIterativeSim();
		break;
	case VK_DELETE:
		g_NodeEditor.DeleteSelectedNodes();
		theApp.RedrawNodeEditor();
		theApp.RedrawPreview();
		theApp.ClearPropertyList();
		break;
	}

	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void NodeEditorView::OnAddNode( UINT _cmdID )
{
	UINT nodeID = _cmdID - BASE_WM_COMMAND_FOR_NODE_CREATION;

	shared_ptr<ComputeNode> pNewNode( g_NodeFactory.CreateNode( nodeID ) );

	UIRect screenRect = GetUIRect();

	pNewNode->UpdateInternalResolution();

	if( !pNewNode->OneTimeInit() )
	{
		assert( false );
		LOG_R( "Node %s failed in OneTimeInit()", pNewNode->GetName() );
		return;
	}
	
	pNewNode->OnResolutionChanged();

	pNewNode->m_UIRect.Pos = g_NodeEditor.ScreenToLogicalPoint( screenRect, Vec2( m_PopupMenuPos.x, m_PopupMenuPos.y ) );

	g_NodeEditor.AddNode( pNewNode );
	g_NodeEditor.SelectNode( pNewNode, false );
	g_NodeEditor.SetPreviewNode( pNewNode );

	theApp.UpdatePropertyGrid( pNewNode );
	theApp.RedrawPreview();

	Invalidate( FALSE ); //Redraw itself
}

void NodeEditorView::OnSetNodePreviewMode( UINT _id )
{
	ASSERT( g_pNodeHavingPoppedUpOptionsMenu );

	g_pNodeHavingPoppedUpOptionsMenu->m_bPreviewAsHeightField = ( _id == WM_SET_NODE_PREVIEW_AS_HEIGHTMAP );

	theApp.RedrawPreview();
}

void NodeEditorView::OnSetNodeResolutionReference( UINT _id )
{
	ASSERT( g_pNodeHavingPoppedUpOptionsMenu );

	ResolutionReference resolutionRef;

	switch( _id )
	{
		case WM_SET_NODE_RESOLUTION_REF_GLOBAL_SETTING: resolutionRef = Res_GlobalSetting; break;
		case WM_SET_NODE_RESOLUTION_REF_FIRST_INPUT:	resolutionRef = Res_MainInput; break;
		case WM_SET_NODE_RESOLUTION_REF_FIXED:			resolutionRef = Res_Fixed; break;
	}

	g_pNodeHavingPoppedUpOptionsMenu->SetResolutionReference( resolutionRef );
}

void NodeEditorView::OnSetNodeResolutionModifier( UINT _id )
{
	ASSERT( g_pNodeHavingPoppedUpOptionsMenu );

	int32_t resModifier = 1;

	switch( _id )
	{
		case WM_SET_NODE_RESOLUTION_MULTIPLIER_4X:		resModifier = 4; break;	
		case WM_SET_NODE_RESOLUTION_MULTIPLIER_2X:  	resModifier = 2; break;	
		case WM_SET_NODE_RESOLUTION_MULTIPLIER_1X:		resModifier = 1; break;
		case WM_SET_NODE_RESOLUTION_MULTIPLIER_1_2X:	resModifier = -2; break;
		case WM_SET_NODE_RESOLUTION_MULTIPLIER_1_4X:	resModifier = -4; break;
		case WM_SET_NODE_RESOLUTION_MULTIPLIER_1_8X:	resModifier = -8; break;
	}

	g_pNodeHavingPoppedUpOptionsMenu->SetResolutionModifier( resModifier );

	Invalidate( FALSE );

	g_pNodeHavingPoppedUpOptionsMenu->SetDirty();

	theApp.RedrawPreview();
}
