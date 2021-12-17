
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "GeoGen.h"

#include "ComputeNode.h"
#include "FloatSliderProperty.h"
#include "BoolProperty.h"
//#include "NodeEditor.h"

#include <Core/Log.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd() : m_pCustomUI(nullptr), m_PropertyGridWidth(400)
{
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChanged )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyPropList = 1;
	int cyTlb = 1;

	shared_ptr<ComputeNode> pNode = m_pComputeNode.lock();

	if( pNode && (pNode->m_ParameterSlots.empty() || pNode->m_ParameterSlots[0].m_DataType == IOType::Color ) )
	{
		cyPropList = 1; //hide prop list
		cyTlb = 1;
	}
	else
	{
		cyPropList = 400;
		cyTlb = m_wndToolBar.CalcFixedLayout( FALSE, TRUE ).cy;
	}

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);


	if( m_pCustomUI )
	{
		m_wndPropList.SetWindowPos( NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), cyPropList, SWP_NOACTIVATE | SWP_NOZORDER );
		m_pCustomUI->SetWindowPos( NULL, rectClient.left, rectClient.top + cyTlb + cyPropList, rectClient.Width(), rectClient.Height() - (cyPropList + cyTlb), SWP_NOACTIVATE | SWP_NOZORDER );
	}
	else
	{
		m_wndPropList.SetWindowPos( NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER );
	}
	
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();


	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	if( !m_pCustomUI )
	{
		m_PropertyGridWidth = cx;
	}

	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();
/*
	m_wndPropList.AddProperty( new CMFCPropertyGridProperty(_T("Appearance"), _T("toto"), nullptr, 1212 ) );

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("Border"), _T("Dialog Frame"), _T("One of: None, Thin, Resizable, or Dialog Frame"));
	pProp->AddOption(_T("None"));
	pProp->AddOption(_T("Thin"));
	pProp->AddOption(_T("Resizable"));
	pProp->AddOption(_T("Dialog Frame"));
	pProp->AllowEdit(FALSE);
	

	m_wndPropList.AddProperty( new CFloatSliderProperty( _T( "slider" ), _T( "zob" ), 50.0f, 0.0f, 250.0f, 1, 69280 ) );
	m_wndPropList.AddProperty( new CBoolProperty( _T("bool"), TRUE, _T( "zob" ), 974 ) );


	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Window Color"), RGB(210, 192, 254), NULL, _T("Specifies the default window color"));
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	m_wndPropList.AddProperty(pColorProp);
*/

}

void CPropertiesWnd::Populate( shared_ptr<ComputeNode> _pNode )
{
	m_wndPropList.RemoveAll();

	m_pComputeNode = _pNode;

	if( m_pCustomUI )
		delete m_pCustomUI;

	m_pCustomUI = _pNode->GetCustomUI( this );


	CRect clientRect;
	GetClientRect( &clientRect );

	CSize minPaneSize;

	if( m_pCustomUI )
	{
		m_pCustomUI->UpdateWindow();

		CRect r;
		m_pCustomUI->GetWindowRect( &r );

		clientRect.right = clientRect.left + r.Width();

		minPaneSize = r.Size();
	}
	else
	{
		clientRect.right = clientRect.left + m_PropertyGridWidth;
		minPaneSize = CSize( 100, 100 );
	}

	SetMinSize( minPaneSize );

	HDWP hdwp;
//	MovePane( clientRect, TRUE, hdwp );

	AdjustLayout();

	if( m_pCustomUI )
		m_pCustomUI->ShowWindow( SW_SHOW );

	u32 paramIndex = 0;

	map< Str, CMFCPropertyGridProperty* > categories;

	for( const ComputeNode::ParamSlot& param : _pNode->m_ParameterSlots )
	{
		CMFCPropertyGridProperty* pProp = nullptr;
		
		//TODO consider param.m_Edition and param.m_CategoryName
		switch( param.m_DataType )
		{
		case IOType::Float:
			pProp = new CFloatSliderProperty( CString( param.m_Name.c_str() ),
												_T( "TODO: description" ),
												param.m_Value.f, param.m_Min, param.m_Max,
												3, paramIndex );
			break;

		case IOType::Integer:
			if( param.m_Edition == ParamEdition::ComboBox )
			{
				pProp = new CMFCPropertyGridProperty( CString( param.m_Name.c_str() ),
												CString( param.m_Enum[ param.m_Value.i ].c_str() ),
												_T( "TODO: description" ),
												paramIndex );

				for( const Str& option : param.m_Enum )
				{
					pProp->AddOption( CString( option.c_str() ) );
				}

				pProp->AllowEdit( FALSE );
			}
			else
			{
				pProp = new CFloatSliderProperty( CString( param.m_Name.c_str() ),
					_T( "TODO: description" ),
					(float)param.m_Value.i, param.m_Min, param.m_Max,
					0, paramIndex );
			}
			break;

		case IOType::Bool:
			pProp = new CBoolProperty(	CString( param.m_Name.c_str() ), 
										param.m_Value.b,
										_T( "TODO: description" ), 
										paramIndex );
			break;

		case IOType::Color:
			pProp = new CMFCPropertyGridColorProperty(	CString( param.m_Name.c_str() ),
														param.m_Value.c,
														NULL, 
														_T( "TODO: description" ), 
														paramIndex );

			((CMFCPropertyGridColorProperty*)pProp)->EnableOtherButton( _T( "Other..." ) );
			((CMFCPropertyGridColorProperty*)pProp)->EnableAutomaticButton( _T( "Default" ), ::GetSysColor( COLOR_3DFACE ) );
			break;

		case IOType::String:
			pProp = new CMFCPropertyGridFileProperty( CString( param.m_Name.c_str() ), TRUE, CString( param.m_ValueString.c_str() ), 
					_T("bmp"),0,_T( "*.rawfp32|*.rawfp32|*.bmp|*.bmp|"), _T( "TODO: description" ),	paramIndex );
			break;

		}

		if( pProp )
		{
			auto itCategoryProp = categories.find( param.m_CategoryName );

			if( itCategoryProp != categories.end() )
			{
				itCategoryProp->second->AddSubItem( pProp );
			}
			else
			{
				CMFCPropertyGridProperty* pCategoryProp = new CMFCPropertyGridProperty( CString( param.m_CategoryName.c_str() ) );
				pCategoryProp->AddSubItem( pProp );
				m_wndPropList.AddProperty( pCategoryProp );

				categories[ param.m_CategoryName ] = pCategoryProp;
			}

		}

		++paramIndex;
	}

	m_wndPropList.ExpandAll();
	m_wndPropList.RedrawWindow();
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

bool operator==( const Str& _strA, BSTR _strB )
{
	return CString( _strA.c_str() ) == _strB;
}

LRESULT CPropertiesWnd::OnPropertyChanged( WPARAM wparam, LPARAM lparam )
{
	shared_ptr<ComputeNode> pNode = m_pComputeNode.lock();

	ASSERT( pNode );
	//TODO if( !pNode ) { RemoveAll(); } ???

	CMFCPropertyGridProperty * pProperty = (CMFCPropertyGridProperty *)lparam;
	DWORD_PTR id = pProperty->GetData();

	const COleVariant & val = pProperty->GetValue();

	ComputeNode::ParamSlot & param = pNode->m_ParameterSlots[ id ];

	switch( param.m_DataType )
	{
		case IOType::Float:
			ASSERT( val.vt == VT_R4 );
			param.m_Value.f = val.fltVal;
			break;

		case IOType::Integer:
			if( param.m_Edition == ParamEdition::ComboBox )
			{
				ASSERT( val.vt == VT_BSTR );
				int value = 0;
				for( const Str& option : param.m_Enum )
				{
					if( option == val.bstrVal )
					{
						param.m_Value.i = value;
						break;
					}

					++value;
				}

			}
			else
			{
				ASSERT( val.vt == VT_R4 );
				param.m_Value.i = (int)val.fltVal;
			}
			break;

		case IOType::Bool:
			ASSERT( val.vt == VT_INT );
			param.m_Value.b = (bool)val.intVal;
			break;

		case IOType::Color:
			ASSERT( val.vt == VT_I4 );
			param.m_Value.c = val.intVal;
			break;

		case IOType::String:
		{
			ASSERT( val.vt == VT_BSTR );
			param.m_ValueString = CT2A( CString( val.bstrVal ) ).m_psz;		}
			break;
	}

	if( param.m_bCompileTimeShaderConstant )
	{
		pNode->OnCompileTimeShaderConstantChanged();
	}

	//TODO compute nodes (set dirty bits) & refresh 3D view
	pNode->SetDirty();
	theApp.RedrawPreview();

	// Note: the return value is not used.
	return(0);
}