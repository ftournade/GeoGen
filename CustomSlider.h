#pragma once

#define NOTIFY_CUSTOM_SLIDER_CHANGED 69280

class CCustomSlider : public CWnd
{
public:
	CCustomSlider( COLORREF clr, float _min=0.0f, float _max=1.0f, int _numDecimals=3 );
	virtual ~CCustomSlider();

	inline float GetValue() const { return m_Value; }
	void SetValue( float v );

protected:
	//{{AFX_MSG(CCustomSlider)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

	virtual void OnValueChanged() {}
private:
	void ChangeSliderValue( const CPoint& point );

private:
	COLORREF m_clr;
	float m_Value, m_Min, m_Max;
	int m_NumDecimals;
	bool m_bMouseButtonDown;
};