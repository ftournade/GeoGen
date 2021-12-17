// TerrainTileProviderSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoGen.h"
#include "TerrainTileProviderSetupDlg.h"
#include "afxdialogex.h"


// TerrainTileProviderSetupDlg dialog

IMPLEMENT_DYNAMIC(TerrainTileProviderSetupDlg, CDialogEx)

TerrainTileProviderSetupDlg::TerrainTileProviderSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_APIKEY, pParent)
{

}

TerrainTileProviderSetupDlg::~TerrainTileProviderSetupDlg()
{
}

void TerrainTileProviderSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_APIKEY, m_MapZenAPIKey );
	DDX_Text( pDX, IDC_EDIT_HERE_APPCODE, m_HereAppCode );
	DDX_Text( pDX, IDC_EDIT_HERE_APPID, m_HereAppId );
	DDV_MaxChars( pDX, m_MapZenAPIKey, 32 );
	DDV_MaxChars( pDX, m_HereAppCode, 32 );
	DDV_MaxChars( pDX, m_HereAppId, 32 );
}


BEGIN_MESSAGE_MAP(TerrainTileProviderSetupDlg, CDialogEx)
	ON_BN_CLICKED( IDC_BUTTON_REGISTER_MAPZEN_APIKEY, &TerrainTileProviderSetupDlg::OnClickedButtonRegisterMapZenAPIkey )
	ON_BN_CLICKED( IDC_BUTTON_REGISTER_HERE_APIKEY, &TerrainTileProviderSetupDlg::OnClickedButtonRegisterHereAPIkey )
END_MESSAGE_MAP()


// TerrainTileProviderSetupDlg message handlers


void TerrainTileProviderSetupDlg::OnClickedButtonRegisterMapZenAPIkey()
{
	ShellExecute( NULL, _T("open"), _T("https://developers.nextzen.org"), NULL, NULL, SW_SHOWNORMAL );
}

void TerrainTileProviderSetupDlg::OnClickedButtonRegisterHereAPIkey()
{
	ShellExecute( NULL, _T( "open" ), _T( "https://developer.here.com/plans" ), NULL, NULL, SW_SHOWNORMAL );
}