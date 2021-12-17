
// GeoGen.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "GeoGen.h"
#include "MainFrm.h"
#include "DEMGrabFrm.h"
#include "SettingsDlg.h"
#include "NodeEditor.h"
#include "HTTPConnection.h"

#include <Core/Log.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGeoGenApp theApp;

// CGeoGenApp

BEGIN_MESSAGE_MAP(CGeoGenApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CGeoGenApp::OnAppAbout)
	ON_COMMAND( ID_SETTINGS, &CGeoGenApp::OnOpenSettings )
	ON_COMMAND( ID_FILE_NEW, &CGeoGenApp::OnFileNew )
	ON_COMMAND( ID_FILE_OPEN, &CGeoGenApp::OnFileLoad )
	ON_COMMAND( ID_FILE_SAVE, &CGeoGenApp::OnFileSave )
	ON_COMMAND( ID_FILE_SAVE_AS, &CGeoGenApp::OnFileSaveAs )
	ON_COMMAND( ID_DEM_GRABBER, &CGeoGenApp::OnOpenDEMGrabber )
END_MESSAGE_MAP()


// CGeoGenApp construction

CGeoGenApp::CGeoGenApp() : 
#ifdef _DEBUG
	m_Resolution(512),
#else
	m_Resolution(2048),
#endif
	m_MinAltitude(-1000),
	m_MaxAltitude(3000),
	m_SeaLevel(0),
	m_TerrainExtent(20480),
	m_CamFOV( 70.0f ),
	m_CamNearClip( 0.5f ),
	m_CamFarClip( 4000000.0f ),
	m_pDEMGrabberFrame(nullptr)
{
	HTTPConnection::InitHTTP();

	m_bHiColorIcons = TRUE;

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("GeoGen.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CGeoGenApp::~CGeoGenApp()
{ 
	HTTPConnection::ShutDownHTTP(); 
}



// CGeoGenApp initialization

BOOL CGeoGenApp::InitInstance()
{
	g_log.EnableHTMLOutput( "log.html" );

#ifdef _DEBUG
	g_log.EnableDebuggerOutput( true );
#endif

	LOG( "CGeoGenApp::InitInstance" );

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	LOG( "CGeoGenApp::InitInstance - Creating MainFrame" );

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;

	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	
	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}

int CGeoGenApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CGeoGenApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CGeoGenApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CGeoGenApp customization load/save methods

void CGeoGenApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CGeoGenApp::LoadCustomState()
{
}

void CGeoGenApp::SaveCustomState()
{
}

void CGeoGenApp::UpdatePropertyGrid( shared_ptr<ComputeNode> pHitNode )
{
	((CMainFrame*)GetMainWnd())->m_wndProperties.Populate( pHitNode );
}


void CGeoGenApp::RedrawPreview()
{
	((CMainFrame*)GetMainWnd())->m_wndPreview.Invalidate( FALSE );
}

void CGeoGenApp::RedrawNodeEditor()
{
	((CMainFrame*)GetMainWnd())->m_wndView.Invalidate( FALSE );
}

void CGeoGenApp::ClearPropertyList()
{
	((CMainFrame*)GetMainWnd())->m_wndProperties.Clear();
}

void CGeoGenApp::StartIterativeSim( shared_ptr<ComputeNode> _pErosionNode )
{
	((CMainFrame*)GetMainWnd())->m_wndPreview.StartIterativeSim( _pErosionNode );
}

void CGeoGenApp::StopIterativeSim()
{
	((CMainFrame*)GetMainWnd())->m_wndPreview.StopIterativeSim();
}

// CGeoGenApp message handlers



void CGeoGenApp::OnOpenSettings()
{
	CSettingsDlg dlg;

	dlg.m_Resolution = m_Resolution;
	dlg.m_MinAltitude = m_MinAltitude;
	dlg.m_MaxAltitude = m_MaxAltitude;
	dlg.m_SeaLevel = m_SeaLevel;
	dlg.m_TerrainExtentKM = (float)m_TerrainExtent / 1000.0f;

	dlg.m_CamFOV = m_CamFOV;
	dlg.m_CamNearClip = m_CamNearClip;
	dlg.m_CamFarClip = m_CamFarClip;

	if( dlg.DoModal() != IDOK )
		return;

	bool bResolutionChanged = (m_Resolution != dlg.m_Resolution);
	bool bNeedToCallOnResolutionChanged = ( m_Resolution != dlg.m_Resolution ) || ( m_MinAltitude != dlg.m_MinAltitude ) || ( m_MaxAltitude != dlg.m_MaxAltitude );

	m_Resolution = dlg.m_Resolution;
	m_MinAltitude = dlg.m_MinAltitude;
	m_MaxAltitude = dlg.m_MaxAltitude;
	m_SeaLevel = dlg.m_SeaLevel;
	m_TerrainExtent = (u32)(dlg.m_TerrainExtentKM * 1000.0f);

	m_CamFOV = dlg.m_CamFOV;
	m_CamNearClip = dlg.m_CamNearClip;
	m_CamFarClip = dlg.m_CamFarClip;

	if( bResolutionChanged )
	{
		((CMainFrame*)GetMainWnd())->m_wndPreview.OnNodesResolutionChanged();
	}

	if( bNeedToCallOnResolutionChanged )
	{
		g_NodeEditor.OnResolutionChange();
	}

	RedrawPreview();
	RedrawNodeEditor();
}

void CGeoGenApp::OnFileNew()
{
	//TODO if dirty ask for saving

	g_NodeEditor.DeleteAllNodes();

	RedrawNodeEditor();
	RedrawPreview();
	ClearPropertyList();

	//TODO reset resolution/extent to default ?
	//TODO clear Property list
}

void CGeoGenApp::OnFileLoad()
{
	CFileDialog dlg( TRUE, _T( "geo" ), nullptr, 0, _T( "GeoGen Files (*.geo)|*.geo|" ) );

	if( dlg.DoModal() != IDOK )
		return;

	CT2A filename( dlg.GetPathName() );
	if( g_NodeEditor.Load( filename.m_psz ) )
	{
		m_Filename = dlg.GetPathName();
	}
	else
	{
		::AfxMessageBox( _T( "Failed to load file !" ) );
	}

	((CMainFrame*)GetMainWnd())->m_wndPreview.OnNodesResolutionChanged();

	RedrawNodeEditor();
	RedrawPreview();

	//TODO clear Property list
}

void CGeoGenApp::OnFileSave()
{
	if( m_Filename.IsEmpty() )
	{
		OnFileSaveAs();
		return;
	}

	CT2A filename( m_Filename );
	if( !g_NodeEditor.Save( filename.m_psz ) )
	{
		::AfxMessageBox( _T( "Failed to save file !" ) );
	}

}


void CGeoGenApp::OnFileSaveAs()
{
	CFileDialog dlg( FALSE, _T( "geo" ), nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T( "GeoGen Files (*.geo)|*.geo|" ) );

	if( dlg.DoModal() != IDOK )
		return;

	CT2A filename( dlg.GetPathName() );
	if( g_NodeEditor.Save( filename.m_psz ) )
	{
		m_Filename = dlg.GetPathName();
	}
	else
	{
		::AfxMessageBox( _T( "Failed to save file !" ) );
	}
}


void CGeoGenApp::OnOpenDEMGrabber()
{
	if( m_pDEMGrabberFrame )
		return; //already open

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	m_pDEMGrabberFrame = new DEMGrabFrame;

	// create and load the frame with its resources
	m_pDEMGrabberFrame->LoadFrame( IDR_DEM_GRABBER, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL );
	
	// The one and only window has been initialized, so show and update it
	m_pDEMGrabberFrame->ShowWindow( SW_SHOW );
	m_pDEMGrabberFrame->UpdateWindow();
}
