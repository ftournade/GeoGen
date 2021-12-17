// ColorPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoGen.h"
#include "ColorPickerDlg.h"
#include "afxdialogex.h"

#include "BasicComputeNodes.h"

// ColorPickerDlg dialog

IMPLEMENT_DYNAMIC(ColorPickerDlg, CDialogEx)

ColorPickerDlg::ColorPickerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_COLOR_PICKER, pParent),
	m_pComputeNode( nullptr )

{

}

ColorPickerDlg::~ColorPickerDlg()
{
}

void ColorPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COLOR_PICKER_CTRL, m_ColorPicker );
}


BEGIN_MESSAGE_MAP(ColorPickerDlg, CDialogEx)
	ON_MESSAGE( WM_COLORPICKER_CHANGED, OnColorPickerChanged )
END_MESSAGE_MAP()


// ColorPickerDlg message handlers

LRESULT ColorPickerDlg::OnColorPickerChanged( WPARAM wParam, LPARAM lParam )
{
	//	shared_ptr< ColorGradientNode > pNode = m_pComputeNode.lock();
	ConstantColorNode* pNode = m_pComputeNode;

	if( !pNode )
		return TRUE;

	pNode->SetColor( m_ColorPicker.m_Color );
	
	pNode->SetDirty();
	theApp.RedrawPreview();

	return TRUE;
}