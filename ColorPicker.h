#pragma once
#include "afxwin.h"


#define WM_COLORPICKER_CHANGED (WM_USER + 24)

Vec2 PointUV( const CRect& r, CPoint p );

class ColorPickerControl :	public CWnd
{
public:
	ColorPickerControl();
	virtual ~ColorPickerControl();

protected:
	BOOL RegisterWindowClass();
	BOOL UpdateLayout( int w, int h );

	void DrawSlider( CDC* _pDC, const CRect& _r, COLORREF _c, float _v );

	int MouseHitTest( const CPoint& p );

	void OnColorModified();

public:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	Win32BackBuffer m_backBuffer;
	CRect m_SliderRect[6], m_HueSatPickerRect;

public:
	Color m_Color;

	int m_LeftButtonDownArea;
};

