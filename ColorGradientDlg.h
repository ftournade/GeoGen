#pragma once

#include "ColorGradientNode.h"
#include "ColorPicker.h"

class ColorGradientWnd : public CWnd
{
public:
	ColorGradientWnd();
	virtual ~ColorGradientWnd();

protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	BOOL RegisterWindowClass();
	BOOL UpdateLayout();

	void DrawSlider( CDC* _pDC, const CRect& _r, COLORREF _c, float _v );

	int MouseHitTestKeyframe( const CPoint& p ); //-1 if not found, keyframe index otherwise

	void OnColorGradientModified();

public:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg LRESULT OnColorPickerChanged( WPARAM wParam, LPARAM lParam );
	
	virtual void PreSubclassWindow();


public:
	Win32BackBuffer m_backBuffer;

	Keyframer< Color > m_ColorGradient;

	ColorPickerControl m_wndColorPicker;

	CRect m_ClearRect, m_RampRect, m_ColorPickerRect;

	int m_SelectedKeyFrame;
	bool m_bDraggingKey;
};

// ColorGradientDlg dialog

class ColorGradientDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ColorGradientDlg)

public:
	ColorGradientDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ColorGradientDlg();

	virtual BOOL OnInitDialog();
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_COLOR_RAMP };
#endif

	afx_msg void OnColorGradientModified( NMHDR*, LRESULT* );
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:

public:
	//	weak_ptr< ColorGradientNode > m_pComputeNode;
	ColorGradientWnd m_ColorGradientCtrl;
	ColorGradientNode* m_pComputeNode;
};
