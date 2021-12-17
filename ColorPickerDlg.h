#pragma once

#include "ColorPicker.h"

// ColorPickerDlg dialog

class ConstantColorNode;

class ColorPickerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ColorPickerDlg)

public:
	ColorPickerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ColorPickerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_COLOR_PICKER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnColorPickerChanged( WPARAM wParam, LPARAM lParam );

public:
	ColorPickerControl m_ColorPicker;

	ConstantColorNode* m_pComputeNode;
};
