// DownloadDEM.cpp : implementation file
//

#include "stdafx.h"
#include "GeoGen.h"
#include "DownloadDEMDlg.h"
#include "afxdialogex.h"


// CDownloadDEMDlg dialog

IMPLEMENT_DYNAMIC(CDownloadDEMDlg, CDialogEx)

CDownloadDEMDlg::CDownloadDEMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_DEM_CAPTURE, pParent)
	, m_bResizeWorld( TRUE )
	, m_Filename( _T( "" ) )
	, m_Resolution( 4096 )
	, m_ColorSatResolution( 4096 )
	, m_bClipAtSeaLevel( FALSE )
	, m_bUnderwater( FALSE )
	, m_bAlsoCaptureColorSat( TRUE )
{

}

CDownloadDEMDlg::~CDownloadDEMDlg()
{
}

void CDownloadDEMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_FILENAME, m_Filename );
	DDX_Text( pDX, IDC_EDIT_RESOLUTION, m_Resolution );
	DDV_MinMaxUInt( pDX, m_Resolution, 8, 1024 * 512 );
	DDX_Text( pDX, IDC_EDIT_COLORSAT_RESOLUTION, m_ColorSatResolution );
	DDV_MinMaxUInt( pDX, m_ColorSatResolution, 8, 1024 * 512 );
	DDX_Check( pDX, IDC_CHECK_EXPORT_COLORSAT, m_bAlsoCaptureColorSat );
	DDX_Check( pDX, IDC_CHECK_RESIZE_TERRAIN, m_bResizeWorld );
	DDX_Check( pDX, IDC_CHECK_CLIP_AT_SEA_LEVEL, m_bClipAtSeaLevel );
	DDX_Check( pDX, IDC_CHECK_UNDERWATER, m_bUnderwater );
}


BEGIN_MESSAGE_MAP(CDownloadDEMDlg, CDialogEx)
	ON_BN_CLICKED( IDC_BUTTON_BROWSE_FILE, &CDownloadDEMDlg::OnClickedButtonBrowseFile )
END_MESSAGE_MAP()


// CDownloadDEMDlg message handlers


void CDownloadDEMDlg::OnClickedButtonBrowseFile()
{
//	CFileDialog dlg( FALSE, _T( "bmp" ), nullptr, OFN_OVERWRITEPROMPT, _T( "bmp (*.bmp)|*.bmp|" ), this );
	CFileDialog dlg( FALSE, _T( "rawfp32" ), nullptr, OFN_OVERWRITEPROMPT, _T( "raw fp32 (*.rawfp32)|*.rawfp32|" ), this );

	if( dlg.DoModal() != IDOK )
		return;

	UpdateData( TRUE );

	m_Filename = dlg.GetPathName();

	UpdateData( FALSE );
}


void CDownloadDEMDlg::OnOK()
{
	UpdateData( TRUE );

	if( !m_Filename.IsEmpty() )
	{
		//if( GetFileExtension( m_Filename ).empty() )
		//{
		//	m_Filename += ".bmp";
		//}

		CDialogEx::OnOK();
	}
}
