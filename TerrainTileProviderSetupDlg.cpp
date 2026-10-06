// TerrainTileProviderSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoGen.h"
#include "TerrainTileProviderSetupDlg.h"
#include "afxdialogex.h"


static LPCTSTR s_RegistryKey = _T( "Software\\GeoGen" );
static LPCTSTR s_MapTilerAPIKeyValue = _T( "MapTilerAPIKey" );

// TerrainTileProviderSetupDlg dialog

IMPLEMENT_DYNAMIC(TerrainTileProviderSetupDlg, CDialogEx)

TerrainTileProviderSetupDlg::TerrainTileProviderSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_APIKEY, pParent)
{

}

TerrainTileProviderSetupDlg::~TerrainTileProviderSetupDlg()
{
}

CString TerrainTileProviderSetupDlg::LoadMapTilerAPIKey()
{
	CString key;
	CRegKey cKey;

	if( cKey.Open( HKEY_CURRENT_USER, s_RegistryKey, KEY_READ ) == ERROR_SUCCESS )
	{
		TCHAR buffer[ 128 ];
		ULONG len = _countof( buffer );

		if( cKey.QueryStringValue( s_MapTilerAPIKeyValue, buffer, &len ) == ERROR_SUCCESS )
			key = buffer;
	}

	key.Trim();
	return key;
}

void TerrainTileProviderSetupDlg::SaveMapTilerAPIKey( const CString& _key )
{
	CRegKey cKey;

	if( cKey.Create( HKEY_CURRENT_USER, s_RegistryKey ) == ERROR_SUCCESS )
	{
		CString key( _key );
		key.Trim();
		cKey.SetStringValue( s_MapTilerAPIKeyValue, key );
	}
}

void TerrainTileProviderSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_APIKEY, m_MapTilerAPIKey );
	DDV_MaxChars( pDX, m_MapTilerAPIKey, 64 );
}


BEGIN_MESSAGE_MAP(TerrainTileProviderSetupDlg, CDialogEx)
	ON_BN_CLICKED( IDC_BUTTON_GET_MAPTILER_KEY, &TerrainTileProviderSetupDlg::OnClickedButtonGetMapTilerKey )
END_MESSAGE_MAP()


// TerrainTileProviderSetupDlg message handlers


void TerrainTileProviderSetupDlg::OnClickedButtonGetMapTilerKey()
{
	ShellExecute( NULL, _T( "open" ), _T( "https://cloud.maptiler.com/account/keys/" ), NULL, NULL, SW_SHOWNORMAL );
}
