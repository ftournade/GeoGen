#pragma once
#include "afxwin.h"


// CSettingsDlg dialog

class CSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void UpdateResolutionStats();
	DECLARE_MESSAGE_MAP()
public:
	UINT m_Resolution;
	int m_MinAltitude;
	int m_MaxAltitude;
	int m_SeaLevel;
	float m_TerrainExtentKM;
	CStatic m_ResolutionStats;
	virtual BOOL OnInitDialog();
	float m_CamFOV;
	float m_CamNearClip;
	float m_CamFarClip;
	afx_msg void OnBnClickedButtonSetupGeodataProvider();
};
