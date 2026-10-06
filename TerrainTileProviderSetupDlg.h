#pragma once


// TerrainTileProviderSetupDlg dialog
// Elevation tiles (AWS Terrain Tiles) need no key; satellite imagery (MapTiler) needs a free API key.

class TerrainTileProviderSetupDlg : public CDialogEx
{
	DECLARE_DYNAMIC(TerrainTileProviderSetupDlg)

public:
	TerrainTileProviderSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~TerrainTileProviderSetupDlg();

	//MapTiler API key persisted in HKCU\Software\GeoGen (empty if not set)
	static CString LoadMapTilerAPIKey();
	static void SaveMapTilerAPIKey( const CString& _key );

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_APIKEY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnClickedButtonGetMapTilerKey();
	DECLARE_MESSAGE_MAP()
public:
	CString m_MapTilerAPIKey;

};
