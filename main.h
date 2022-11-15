// ==========================================================================
// Name    :    main.h
// Purpose :    #includes, prototypes, globals, and const
//              definitions for setup sample

#ifdef _DEBUG
#define __TEST__ 1
#endif

// prototypes
HRESULT         GetFileVersion              ( LPTSTR filename, VS_FIXEDFILEINFO *pvsf );
HRESULT         LastError                   ();

DWORD           ExecCmd                     ( LPCTSTR pszCmd );
DWORD WINAPI    StaticThreadProc            ( LPVOID lpParameter );

BOOL            FxInstallRequired           ();
BOOL            HandleResult                (DWORD dwResult);
BOOL            InitiateReboot              ();
BOOL            IsTerminalServicesEnabled   ();
BOOL            Reboot                      (DWORD dwResult);
BOOL            ValidateProductSuite        (LPSTR lpszSuiteToValidate);
BOOL CALLBACK   BillboardProc               (HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
void            SetTSInInstallMode          ();
void            ShowBillboard               (DWORD * pdwThreadId, HANDLE * phThread);
void            TeardownBillboard           (DWORD dwThreadId, HANDLE hThread);
size_t          getStringIndex              (char *searchStr, char *searchFor);

void checkIE();
void checkMSI();
void checkMDAC();
void checkMSDE();

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//    Private message to tell the thread to destroy the window
const UINT  PWM_THREADDESTROYWND = WM_USER;

// single instance data
const TCHAR g_tszBootstrapMutexName[] = _T( "FxApp Install Bootstrapper" );

// msi install cmd-line
const TCHAR g_tszMsiCmdLine[] = _T("%s\\Msiexec.exe /I \"%s\" REBOOT=ReallySuppress");

// MDAC
const TCHAR g_tszMDAC[]        = _T("MDAC_TYP.EXE");

// MSDE
const TCHAR g_tszMSDECmdLine[] = _T("%s\\setup.exe %s");

// Command Path Builder
const TCHAR g_tszCommandPathBuilder[] = _T("\"%s%s\\%s\" %s");

// reg key for fx policy info
// used to detect if fx is installed
// this key resides in HKEY_LOCAL_MACHINE
const TCHAR g_tszFxRegKey[] = _T("SOFTWARE\\Microsoft\\.NETFramework\\");

// reg key for IE
const TCHAR g_tszIERegKey[]      = _T("SOFTWARE\\Microsoft\\Internet Explorer");
const TCHAR g_tszIERegKeyValue[] = _T("Version");

// reg key for Windows Installer
const TCHAR g_tszMSIRegKey[]      = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer");
const TCHAR g_tszMSIRegKeyValue[] = _T("InstallerLocation");

//MDAC
const TCHAR g_tszMDACRegKey[]      = _T("SOFTWARE\\Microsoft\\DataAccess");
const TCHAR g_tszMDACRegKeyValue[] = _T("FullInstallVer");

//MSDE
const TCHAR g_tszMSDERegKey[]      = _T("SOFTWARE\\Microsoft\\Microsoft SQL Server");
const TCHAR g_tszMSDERegKeyValue[] = _T("InstalledInstances");

const TCHAR   g_tszTSChangeUserToInstall[] = _T("change user /INSTALL");
// ini-based application settings
//

bool IENeedsInstalling      = true;
bool MSINeedsInstalling     = true;
bool FxNeedsInstalling      = true;
bool MDACNeedsInstalling    = true;
bool MSDENeedsInstalling    = true;
