#pragma once

class CBoolProperty : public CMFCPropertyGridProperty
{
public:
	CBoolProperty( const CString& _strName,	BOOL _value, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0 );
	virtual ~CBoolProperty();

	virtual void OnDrawValue( CDC* pDC, CRect rect );
	virtual CWnd* CreateInPlaceEdit( CRect rectEdit, BOOL& bDefaultFormat );
private:
	CImageList m_images;
};

