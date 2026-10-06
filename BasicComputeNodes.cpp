#include "stdafx.h"
#include "BasicComputeNodes.h"

#include "ColorPickerDlg.h"

#include "resource.h"


CDialogEx* ConstantColorNode::GetCustomUI( CWnd* _pParent )
{
	Color c;
	c.FromWin32COLORREF( m_ParameterSlots[ 0 ].m_Value.c );
	ColorPickerDlg* pUI = new ColorPickerDlg; 
	
	pUI->m_pComputeNode = this;
	pUI->m_ColorPicker.m_Color = c;

	if( !pUI->Create( IDD_DLG_COLOR_PICKER, _pParent ) )
	{
		LOG_R( "Failed to create custom UI for node %s", GetNodeClassName() );
		assert( false );
		delete pUI;
		return nullptr;
	}

	pUI->UpdateWindow();

	return pUI;
}