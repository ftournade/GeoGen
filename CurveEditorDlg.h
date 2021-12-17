#pragma once

#include <Core/Win32BackBuffer.h>
#include <Core/Keyframer.h>

#define WM_CURVE_CHANGED (WM_USER + 35)

class CurveNode;

class CurveEditorControl : public CWnd
{
public:
	CurveEditorControl();
	virtual ~CurveEditorControl();

	BOOL RegisterWindowClass();

public:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnPaint();
	afx_msg void OnSetKeyTypeCorner();
	afx_msg void OnSetKeyTypeBezierCorner();
	afx_msg void OnSetKeyTypeBezierSmooth();
	afx_msg void OnDeleteKey();
	DECLARE_MESSAGE_MAP()

	void OnCurveModified();
private:
	xtm::Win32BackBuffer m_backBuffer;

public:
	Keyframer< Vec2 > m_Curve;
	int m_DraggedKeyframe, m_RightClickedKeyframe;
	int m_DraggedHandle; //-1 left 0 control point 1 right
};

// CurveEditorDlg dialog

class CurveEditorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CurveEditorDlg)

public:
	CurveEditorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CurveEditorDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CURVE_EDITOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	afx_msg void OnCurveModified( NMHDR*, LRESULT* );
	DECLARE_MESSAGE_MAP()

public:
	CurveEditorControl m_CurveControl;

	CurveNode* m_pComputeNode;
};
