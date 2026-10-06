// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoGen.h"
#include "SettingsDlg.h"
#include "afxdialogex.h"

#include "TerrainTileProviderSetupDlg.h"

// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS, pParent)
	, m_Resolution( 0 )
	, m_MinAltitude( 0 )
	, m_MaxAltitude( 0 )
	, m_SeaLevel( 0 )
	, m_TerrainExtentKM( 0 )
	, m_CamFOV( 0 )
	, m_CamNearClip( 0 )
	, m_CamFarClip( 0 )
{

}

CSettingsDlg::~CSettingsDlg()
{
}

bool g_bValidateSettings = true;

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_RESOLUTION, m_Resolution );
	DDX_Text( pDX, IDC_EDIT_MIN_ALT, m_MinAltitude );
	DDX_Text( pDX, IDC_EDIT_MAX_ALT, m_MaxAltitude );
	DDX_Text( pDX, IDC_EDIT_SEA_LEVEL, m_SeaLevel );
	DDX_Text( pDX, IDC_EDIT_TERRAIN_EXTENT, m_TerrainExtentKM );

	if( g_bValidateSettings )
	{
		DDV_MinMaxUInt( pDX, m_Resolution, 64, 16384 );
		DDV_MinMaxInt( pDX, m_MinAltitude, -10000, 10000 );
		DDV_MinMaxInt( pDX, m_MaxAltitude, -10000, 10000 );
		DDV_MinMaxInt( pDX, m_SeaLevel, -10000, 10000 );
		DDV_MinMaxFloat( pDX, m_TerrainExtentKM, 0, 100000000 );
	}

	DDX_Control( pDX, IDC_STATIC_RESOLUTION_STATISTICS, m_ResolutionStats );
	DDX_Text( pDX, IDC_EDIT_CAM_FOV, m_CamFOV );
	DDV_MinMaxFloat( pDX, m_CamFOV, 0.01f, 85.0f );
	DDX_Text( pDX, IDC_EDIT_CAM_NEAR_CLIP, m_CamNearClip );
	DDV_MinMaxFloat( pDX, m_CamNearClip, 0.01f, 100000.0f );
	DDX_Text( pDX, IDC_EDIT_CAM_FAR_CLIP, m_CamFarClip );
	DDV_MinMaxFloat(pDX, m_CamFarClip, 10.0f,1000000000.0f );
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_EN_CHANGE( IDC_EDIT_RESOLUTION, &CSettingsDlg::UpdateResolutionStats )
	ON_EN_CHANGE( IDC_EDIT_TERRAIN_EXTENT, &CSettingsDlg::UpdateResolutionStats )
	ON_BN_CLICKED( IDC_BUTTON_SETUP_GEODATA_PROVIDER, &CSettingsDlg::OnBnClickedButtonSetupGeodataProvider )
END_MESSAGE_MAP()


// CSettingsDlg message handlers


void CSettingsDlg::UpdateResolutionStats()
{
	g_bValidateSettings = false; //only validate when user presses OK
	UpdateData();
	g_bValidateSettings = true;

	float terrainResolution = m_TerrainExtentKM * 1000.0f / (float)m_Resolution;

	CString str;
	str.Format( _T("Resolution: %.3fm/pixel"), terrainResolution );
	m_ResolutionStats.SetWindowText( str );
	
}


BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	UpdateResolutionStats();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CSettingsDlg::OnBnClickedButtonSetupGeodataProvider()
{
	TerrainTileProviderSetupDlg dlg;
	dlg.m_MapTilerAPIKey = TerrainTileProviderSetupDlg::LoadMapTilerAPIKey();

	if( dlg.DoModal() == IDOK )
		TerrainTileProviderSetupDlg::SaveMapTilerAPIKey( dlg.m_MapTilerAPIKey ); //an empty key clears it
}
