#pragma once


// TerrainTileProviderSetupDlg dialog

class TerrainTileProviderSetupDlg : public CDialogEx
{
	DECLARE_DYNAMIC(TerrainTileProviderSetupDlg)

public:
	TerrainTileProviderSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~TerrainTileProviderSetupDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_APIKEY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnClickedButtonRegisterMapZenAPIkey();
	afx_msg void OnClickedButtonRegisterHereAPIkey();
	DECLARE_MESSAGE_MAP()
public:
	CString m_MapZenAPIKey, m_HereAppId, m_HereAppCode;

};
