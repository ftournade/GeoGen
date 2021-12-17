
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "DEMGrabView.h"

class DEMGrabFrame : public CFrameWndEx
{
	
public:
	DEMGrabFrame();
protected: 
	DECLARE_DYNAMIC( DEMGrabFrame )

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~DEMGrabFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
	CMFCToolBar       m_wndToolBar;
	CMFCToolBarImages m_UserImages;

	DEMGrabView		m_wndView;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnClose();
};


