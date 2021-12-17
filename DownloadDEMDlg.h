#pragma once


// CDownloadDEM dialog

class CDownloadDEMDlg : public CDialogEx
{
	DECLARE_DYNAMIC( CDownloadDEMDlg )

public:
	CDownloadDEMDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDownloadDEMDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DEM_CAPTURE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedButtonBrowseFile();

	BOOL m_bAlsoCaptureColorSat;
	UINT m_Resolution;
	UINT m_ColorSatResolution;
	BOOL m_bResizeWorld;
	CString m_Filename;
	BOOL m_bClipAtSeaLevel;
	BOOL m_bUnderwater;


	virtual void OnOK();
};
