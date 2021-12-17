#pragma once

class CFloatSliderProperty : public CMFCPropertyGridProperty
{
public:
	CFloatSliderProperty(	const CString& _strName,
							LPCTSTR lpszDescr = NULL,
							float _value = 0.0f, 
							float _min = 0.0f, 
							float _max = 1.0f, 
							int _numDecimals = 3,
							DWORD_PTR dwData = 0,
							COLORREF _clr = RGB(200,0,0) );

	virtual CWnd* CreateInPlaceEdit( CRect rectEdit, BOOL& bDefaultFormat );
	virtual BOOL OnUpdateValue();

	virtual void OnDrawValue( CDC* pDC, CRect rect );

private:
	int m_NumDecimals;
	float m_Min, m_Max;
	COLORREF m_clr;
};
