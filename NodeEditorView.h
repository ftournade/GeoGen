#pragma once


class NodeEditorView : public CWnd
{
// Construction
public:
	NodeEditorView();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~NodeEditorView();

	// Generated message map functions
protected:
	void InitPopupMenus();
	void InitNodeOptionsPopupMenus( const CPoint& _menuPos );

	UIRect GetUIRect() const;

	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnAddNode( UINT _id );
	afx_msg void OnSetNodePreviewMode( UINT _id );
	afx_msg void OnSetNodeResolutionReference( UINT _id );
	afx_msg void OnSetNodeResolutionModifier( UINT _id );

private:

	xtm::Win32BackBuffer m_backBuffer;

	bool m_bDrawWithD3D11;
	IDXGISwapChain* m_pD3D11SwapChain;
	RenderTarget* m_pD3D11BackBuffer;

	bool m_bPopupMenusInitialized;
	CMenu m_MainContextMenu;
	CMenu m_GenNodeCreationSubMenu;
	CMenu m_ModifierNodeCreationSubMenu;
	CMenu m_CombinerNodeCreationSubMenu;
	CMenu m_MaskNodeCreationSubMenu;
	CMenu m_NaturalNodeCreationSubMenu;
	CMenu m_IONodeCreationSubMenu;

	CPoint m_PopupMenuPos;

};

