#pragma once


#define WM_RIVER_CHANGED (WM_USER + 37)

class MountainNode;
struct RiverNode;

class RiverEditorControl : public CWnd
{
public:
	RiverEditorControl();
	virtual ~RiverEditorControl();

	BOOL RegisterWindowClass();

public:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnPaint();
	afx_msg void OnDeleteKey();
	DECLARE_MESSAGE_MAP()

	void OnRiverModified();
private:
	void RecDraw( CDC* _pDC, const CRect& _rect, const shared_ptr< RiverNode >& _node );
	bool RecHitTestRiverNodes( const CRect& _rect, const CPoint& _p, const shared_ptr< RiverNode >& _node, shared_ptr< RiverNode >& _hitNode );

	xtm::Win32BackBuffer m_backBuffer;

public:
	shared_ptr< RiverNode >* m_ppRiverRoot;
	weak_ptr< RiverNode > m_pSelectedRiverNode;
};

// RiverEditorDlg dialog

class RiverEditorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(RiverEditorDlg)

public:
	RiverEditorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~RiverEditorDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CURVE_EDITOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	afx_msg void OnRiverModified( NMHDR*, LRESULT* );
	DECLARE_MESSAGE_MAP()

public:
	RiverEditorControl m_RiverControl;

	MountainNode* m_pComputeNode;
};
