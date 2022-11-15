//
//   Copyright (c) Microsoft Corporation.  All rights reserved.
//
/// =======================================================================
// Name    :    main.cpp
// Purpose :    Windows bootstrap application that installs the
//              Microsoft .Net Framework redistributable files,
//              if necessary, and an additional application msi.

// includes
#include <windows.h>
#include <crtdbg.h>
#include <tchar.h>
#include <stdio.h>
#include <CorError.h>
#include <Shlwapi.h>
#include <WinError.h>
#include <string.h>
#include <direct.h>

#include "CSingleInstance.hxx"  // CSingleInstance implementation
#include "CError.h"             // CError definition
#include "resource.h"           // string defines
#include "SetupCodes.h"         // setup-related error codes
#include "CSettings.h"          // ini-based app globals/settings

#include "main.h"

#ifndef __PLATFORM
#include "OSDetector.h"
#endif

#ifdef _DEBUG
const bool test = true;
#else
const bool test = false;
#endif

CSettings   g_settings;
DWORD       g_dwThread  = 0;
HANDLE      g_hThread   = NULL;
TCHAR       szWindowsDir[MAX_PATH];
TCHAR       szWindowsSystemDir[MAX_PATH];
bool        g_isBillboardVisible = false;

// ==========================================================================
// WinMain()
//
// Purpose: application entry point
//
// ==========================================================================
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    UINT    uRetCode = 0;       // bootstrapper return code
    BOOL    bFxReboot = FALSE;  // reboot indicated due to fx install
    BOOL    bAppReboot = FALSE; // reboot indicated after host app install
    BOOL    bAppInstallSucceeded = TRUE;

    ShowBillboard(&g_dwThread, &g_hThread);

    // create single instance object
    //
    CSingleInstance si( g_tszBootstrapMutexName );


    // initialize hInstance in global settings
    g_settings.SetHInstance(hInstance);

    try
    {
        // parse our application settings
        if (!g_settings.Parse())
        {
            /*
             * Close the Billboard
             */
            TeardownBillboard(g_dwThread, g_hThread);

            CError se( IDS_SETTINGS_INIT_FAILURE,
                       0,
                       MB_ICONERROR,
                       FALSE,
                       g_settings.GetIniName());

            throw (se);
        }

        // validate single instance
        //
        // if we are not alone, throw an error
        if( !si.IsUnique() )
        {
            /*
             * Close the Billboard
             */
            TeardownBillboard(g_dwThread, g_hThread);

            CError se( IDS_NOT_SINGLE_INSTANCE,
                       0,
                       MB_ICONERROR,
                       COR_NOT_SINGLE_INSTANCE );

            throw( se );
        }
        // if there was a problem creating mutex, throw an error
        else if( !si.IsHandleOK() )
        {
            /*
             * Close the Billboard
             */
            TeardownBillboard(g_dwThread, g_hThread);

            CError se( IDS_SINGLE_INSTANCE_FAIL,
                       0,
                       MB_ICONERROR,
                       COR_SINGLE_INSTANCE_FAIL );

            throw( se );
        }

        /*
         * Check the existence of the MSI file to install
         */
        HANDLE handle = CreateFile( g_settings.GetMsi(),
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

        if (INVALID_HANDLE_VALUE == handle)
        {
            /*
             * Store the last error
             */
            DWORD dwResult = GetLastError();

            /*
             * Check the type of error
             */
            if (ERROR_FILE_NOT_FOUND == dwResult)
            {
                /*
                 * Close the Billboard
                 */
                TeardownBillboard(g_dwThread, g_hThread);

                CError se(IDS_MSI_NOT_FOUND,
                          0,
                          MB_ICONERROR,
                          ERROR_FILE_NOT_FOUND,
                          g_settings.GetMsi());

                throw (se);
            }
            else
            {
                /*
                 * Close the Billboard
                 */
                TeardownBillboard(g_dwThread, g_hThread);

                CError se( IDS_SETUP_FAILURE, 0, MB_ICONERROR, dwResult);
                throw (se);
            }
        }
        else
        {
            CloseHandle(handle);
        }

        int nResult= 0;

        if (!GetWindowsDirectory(szWindowsDir, LENGTH(szWindowsDir)))
            HandleResult(GetLastError());

        if (!GetSystemDirectory(szWindowsSystemDir, LENGTH(szWindowsSystemDir)))
            HandleResult(GetLastError());

        // put ourselves in install mode
        // if running on Terminal Server.
        SetTSInInstallMode();

        DWORD dwResult;

        // Check the OS Version
        PLATFORM platform = getPlatform();

        checkIE();
        checkMSI();
        checkMDAC();
        checkMSDE();

        FxNeedsInstalling = (FxInstallRequired() == ERROR_SUCCESS) ? false : true;

        if (!g_settings.InstallIE())        IENeedsInstalling   = false;
        if (!g_settings.InstallMSI())       MSINeedsInstalling  = false;
        if (!g_settings.InstallMDAC())      MDACNeedsInstalling = false;
        if (!g_settings.InstallMSDE())      MSDENeedsInstalling = false;
        if (!g_settings.InstallFramework()) FxNeedsInstalling   = false;

        // take down billboard
        TeardownBillboard(g_dwThread, g_hThread);

        // If one of the 3 needed components needs installing, show the install dialog
        if (test || (FxNeedsInstalling      ||
                     IENeedsInstalling      ||
                     MSINeedsInstalling     ||
                     MDACNeedsInstalling    ||
                     MSDENeedsInstalling))
        {
            if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_INSTALL), NULL, DlgProc) != TRUE)
                return 0;
        }

        if (IENeedsInstalling)
        {
            /*
             * Build the IE6Setup command
             */
            TCHAR szIEInstallCmd[MAX_PATH];
            sprintf_s(szIEInstallCmd,                   // IE Setup Command
                         MAX_PATH,
                         g_tszCommandPathBuilder,       // Format to concat
                         g_settings.GetPath(),          // The current path
                         g_settings.GetIEPath(),        // IE Setup relative path
                         g_settings.GetIEInstaller(),   // IE Setup installer name
                         g_settings.GetIEParams());     // IE setup parameters

            dwResult = ExecCmd(szIEInstallCmd);         // Install IE
            HandleResult(dwResult);                     // Handle the result
            bAppReboot = Reboot(dwResult) ? true : bAppReboot;
        }

        if (MDACNeedsInstalling)
        {
            /*
             * Build the MDAC setup command
             */
            TCHAR szMDACInstallCmd[MAX_PATH];
            sprintf_s(szMDACInstallCmd,
                         MAX_PATH,
                         g_tszCommandPathBuilder,
                         g_settings.GetPath(),
                         g_settings.GetMDACPath(),
                         g_tszMDAC,
                         g_settings.GetMDACParams());

            dwResult = ExecCmd(szMDACInstallCmd);
            HandleResult(dwResult);
            bAppReboot = Reboot(dwResult) ? true : bAppReboot;
        }


        if (MSDENeedsInstalling)
        {
            /*
             * Build the MSDE setup command
             */
            TCHAR szMSDEInstallCmd[MAX_PATH];
            sprintf_s(szMSDEInstallCmd,
                         MAX_PATH,
                         "%s%s",
                         g_settings.GetPath(),
                         g_settings.GetMSDEPath());

            sprintf_s(szMSDEInstallCmd,
                         MAX_PATH,
                         g_tszMSDECmdLine,
                         szMSDEInstallCmd,
                         g_settings.GetMSDEParams());

            dwResult = ExecCmd(szMSDEInstallCmd);
            HandleResult(dwResult);
            bAppReboot = Reboot(dwResult) ? true : bAppReboot;
        }

        if (MSINeedsInstalling)
        {
            /*
             * Build the Windows Installer setup command
             */
            TCHAR szMsiInstallCmd[MAX_PATH];

            sprintf_s(szMsiInstallCmd,
                         MAX_PATH,
                         g_tszCommandPathBuilder,
                         g_settings.GetPath(),
                         g_settings.GetMSIPath(),
                         g_settings.GetMSIName(),
                         g_settings.GetMSIParams());

            dwResult = ExecCmd(szMsiInstallCmd);
            HandleResult(dwResult);
            bAppReboot = Reboot(dwResult) ? true : bAppReboot;
        }

        // install Fx if required
        if (FxNeedsInstalling)
        {
            /*
             * Builds the .NET Framework setup command
             */
            TCHAR szFxInstallCmd[MAX_PATH];
            sprintf_s(szFxInstallCmd,
                         MAX_PATH,
                         _T(g_tszCommandPathBuilder),
                         g_settings.GetPath(),
                         g_settings.GetFxInstallerPath(),
                         g_settings.GetFxInstallerName(),
                         g_settings.GetFxInstallerParams());


            // execute dotnetfx.exe setup
            DWORD dwResult;
            dwResult = ExecCmd(szFxInstallCmd);

            if ( ERROR_SUCCESS_REBOOT_REQUIRED == dwResult ||
                ERROR_SUCCESS == dwResult )
            {
                bFxReboot = (dwResult == ERROR_SUCCESS_REBOOT_REQUIRED);
            }
            else
            {
                switch(dwResult)
                {
                    case (COR_INSUFFICIENT_PRIVILEGES):
                    {
                        /*
                         * Close the Billboard
                         */
                        TeardownBillboard(g_dwThread, g_hThread);

                        CError se( IDS_INSUFFICIENT_PRIVILEGES,
                                   0,
                                   MB_ICONERROR,
                                   COR_INSUFFICIENT_PRIVILEGES );

                        throw( se );
                        break;
                    }

                    default :
                    {
                        /*
                         * Close the Billboard
                         */
                        TeardownBillboard(g_dwThread, g_hThread);

                        CError se( IDS_SETUP_FAILURE,
                                   0,
                                   MB_ICONERROR,
                                   dwResult );

                        throw ( se );
                        break ;
                    }
                }
            }
        }

        /*
         * Get the full path of the MSI package
         */
        TCHAR szMsiLocation[MAX_PATH];
        sprintf_s(szMsiLocation,
                     MAX_PATH,
                     "%s%s",
                     g_settings.GetPath(),
                     g_settings.GetMsi());

        /*
         * Install the Main Application
         */
        TCHAR szMsiInstallCmd[MAX_PATH];
        sprintf_s(szMsiInstallCmd,
                     MAX_PATH,
                     g_tszMsiCmdLine,
                     szWindowsSystemDir,
                     szMsiLocation);

        dwResult = ExecCmd(szMsiInstallCmd);

        if ((ERROR_SUCCESS_REBOOT_REQUIRED == dwResult) || (ERROR_SUCCESS == dwResult))
            bAppReboot = (dwResult == ERROR_SUCCESS_REBOOT_REQUIRED) ? true : bAppReboot;
        else if ( dwResult == ERROR_INSTALL_USEREXIT)
            bAppInstallSucceeded = FALSE;
        else
        {
            /*
             * Close the Billboard
             */
            TeardownBillboard(g_dwThread, g_hThread);
            CError se( IDS_SETUP_FAILURE, 0, MB_ICONERROR, dwResult );
            se.ShowMessage();
            bAppInstallSucceeded = FALSE;
        }

        // now handle the reboot

        if (bFxReboot || bAppReboot)
        {
            /*
             * Close the Billboard
             */
            TeardownBillboard(g_dwThread, g_hThread);

            CError se( IDS_REBOOT_QUERY, IDS_DIALOG_CAPTION, MB_YESNO);
            nResult = se.ShowMessage();
            if (nResult == IDYES)
                InitiateReboot();
        }
        else
        {
            // we only throw up the setup succeeded dialog
            // if the app install was successful.
            if (bAppInstallSucceeded)
            {
                /*
                 * Close the Billboard
                 */
                TeardownBillboard(g_dwThread, g_hThread);
                CError se( IDS_SETUP_SUCCEEDED, IDS_DIALOG_CAPTION, MB_OK );
                se.ShowMessage();
            }
        }
    }

    catch (HRESULT)
    {
        // hresult exception msg display is handled
        // by the originator. the exception is rethrown
        // and caught here in order to exit.
    }

    catch( CError se )
    {
        /*
         * Close the Billboard
         */
        TeardownBillboard(g_dwThread, g_hThread);
        uRetCode = se.m_nRetCode;
        se.ShowMessage();
    }

    catch( ... )
    {
        /*
         * Close the Billboard
         */
        TeardownBillboard(g_dwThread, g_hThread);
        CError se( IDS_SETUP_FAILURE, 0, MB_ICONERROR, COR_EXIT_FAILURE );
        uRetCode = se.m_nRetCode;
        se.ShowMessage();
    }

    return uRetCode;
}

BOOL Reboot(DWORD dwResult)
{
    if ( ERROR_SUCCESS_REBOOT_REQUIRED == dwResult ||
        ERROR_SUCCESS == dwResult )
    {
        return (dwResult == ERROR_SUCCESS_REBOOT_REQUIRED);
    }
    return false;
}
BOOL HandleResult(DWORD dwResult)
{
    if ( ERROR_SUCCESS_REBOOT_REQUIRED == dwResult ||
        ERROR_SUCCESS == dwResult )
    {
        return true;
    }
    else
    {
        // we display the error msg here and do not rethrow
        // this is because we need to continue with a system
        // reboot in the event that fx was installed
        // successfully before msi-install failure
        CError se( IDS_SETUP_FAILURE, 0, MB_ICONERROR, dwResult );
        se.ShowMessage();
        return false;
    }
    return true;
}

// ==========================================================================
// ExecCmd()
//
// Purpose:
//  Executes command-line
// Inputs:
//  LPCTSTR pszCmd: command to run
// Outputs:
//  DWORD dwExitCode: exit code from the command
// Notes: This routine does a CreateProcess on the input cmd-line
//        and waits for the launched process to exit.
// ==========================================================================
DWORD ExecCmd( LPCTSTR pszCmd )
{
    if (test)
    {
        MessageBox(NULL, pszCmd, "Command", MB_OK);
        return ERROR_SUCCESS;
    }

    BOOL  bReturnVal   = false ;
    STARTUPINFO  si ;
    DWORD  dwExitCode ;
    SECURITY_ATTRIBUTES saProcess, saThread ;
    PROCESS_INFORMATION process_info ;

    ZeroMemory(&si, sizeof(si)) ;
    si.cb = sizeof(si) ;

    saProcess.nLength = sizeof(saProcess) ;
    saProcess.lpSecurityDescriptor = NULL ;
    saProcess.bInheritHandle = TRUE ;

    saThread.nLength = sizeof(saThread) ;
    saThread.lpSecurityDescriptor = NULL ;
    saThread.bInheritHandle = FALSE ;

    bReturnVal = CreateProcess(NULL,
                               (LPTSTR)pszCmd,
                               &saProcess,
                               &saThread,
                               FALSE,
                               DETACHED_PROCESS,
                               NULL,
                               NULL,
                               &si,
                               &process_info) ;

    if (bReturnVal)
    {
        CloseHandle( process_info.hThread ) ;
        WaitForSingleObject( process_info.hProcess, INFINITE ) ;
        GetExitCodeProcess( process_info.hProcess, &dwExitCode ) ;
        CloseHandle( process_info.hProcess ) ;
    }
    else
    {
        CError se( IDS_CREATE_PROCESS_FAILURE,
                   0,
                   MB_ICONERROR,
                   COR_EXIT_FAILURE,
                   pszCmd );

        throw( se );
    }

    return dwExitCode;
}

// ==========================================================================
// FxInstallRequired()
//
// Purpose:
//  Checks whether the provided Microsoft .Net Framework redistributable
//  files should be installed to the local machine
//
// ==========================================================================
BOOL FxInstallRequired()
{
    BOOL bResult = TRUE;

    /*
     * Try to locate the .NET Framework directory for the specified version.
     */
    strncat_s(szWindowsDir, MAX_PATH, _T("\\Microsoft.Net\\Framework\\"), LENGTH(szWindowsDir));
    strncat_s(szWindowsDir, MAX_PATH, g_settings.GetFxVersion(), LENGTH(szWindowsDir));
    DWORD dwResult = GetFileAttributes(szWindowsDir);

    /*
     * Check if the directory version exists,
     * Return FALSE if the path was not found.
     */
    if ((dwResult != INVALID_FILE_ATTRIBUTES) && (dwResult & FILE_ATTRIBUTE_DIRECTORY))
        bResult = FALSE;

    if (test)
        return TRUE;

    return bResult;
}

// ==========================================================================
// GetFileVersion()
//
// Purpose: retrieves a file version info structure for the specified file
//
// ==========================================================================
HRESULT GetFileVersion (LPTSTR filename, VS_FIXEDFILEINFO *pvsf)
{
    DWORD dwHandle;
    HRESULT hrReturn = S_OK;
    char* pver = NULL;

    try
    {
        DWORD cchver = GetFileVersionInfoSize(filename,&dwHandle);
        if (cchver == 0)
            throw LastError();
        pver = new char[cchver];

        if (!pver)
            throw E_OUTOFMEMORY;

        BOOL bret = GetFileVersionInfo(filename,dwHandle,cchver,pver);
        if (!bret)
            throw LastError();

        UINT uLen;
        void *pbuf;
        bret = VerQueryValue(pver,_T("\\"),&pbuf,&uLen);
        if (!bret)
            throw LastError();

        memcpy(pvsf,pbuf,sizeof(VS_FIXEDFILEINFO));
    }
    catch (HRESULT hr)
    {
        hrReturn = hr;
    }

    delete[] pver;
    return hrReturn;
}

HRESULT LastError ()
{
   HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
   if (SUCCEEDED(hr))
   {
      hr = E_FAIL;
   }
   return hr;
}

// ==========================================================================
// InitiateReboot()
//
// Purpose: initiates a system reboot
//
// ==========================================================================
BOOL InitiateReboot()
{
    HANDLE hToken;              // handle to process token
    TOKEN_PRIVILEGES tkp;       // pointer to token structure

    try
    {
        // Get the current process token handle so we can get shutdown
        // privilege.

        if (!OpenProcessToken(GetCurrentProcess(),
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            return FALSE;
        }

        // Get the LUID for shutdown privilege.

        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
                &tkp.Privileges[0].Luid);

        tkp.PrivilegeCount = 1;  // one privilege to set
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        // Get shutdown privilege for this process.

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
            (PTOKEN_PRIVILEGES) NULL, 0);

        // Cannot test the return value of AdjustTokenPrivileges.

        if (GetLastError() != ERROR_SUCCESS)
        {
            return FALSE;
        }
    }

    catch (...)
    {
    }
    return ExitWindowsEx( EWX_REBOOT, 0);
}

// ==========================================================================
// SetTSInInstallMode()
//
// Purpose: checks if Terminal Services is enabled and if so
//          switches machine to INSTALL mode
// ==========================================================================
void SetTSInInstallMode()
{
    if (IsTerminalServicesEnabled())
    {
        ExecCmd(g_tszTSChangeUserToInstall);
    }
}

//Detecting If Terminal Services is Installed
// code is taken directly from
// http://msdndevstg/library/psdk/termserv/termserv_7mp0.htm


/* -------------------------------------------------------------
   Note that the ValidateProductSuite and IsTerminalServices
   functions use ANSI versions of Win32 functions to maintain
   compatibility with Windows 95/98.
   ------------------------------------------------------------- */

BOOL IsTerminalServicesEnabled()
{
  BOOL    bResult = FALSE;
  DWORD   dwVersion;
  OSVERSIONINFOEXA osVersion;
  DWORDLONG dwlCondition = 0;
  HMODULE hmodK32 = NULL;
  HMODULE hmodNtDll = NULL;
  typedef ULONGLONG (WINAPI *PFnVerSetCondition) (ULONGLONG, ULONG, UCHAR);
  typedef BOOL (WINAPI *PFnVerifyVersionA) (POSVERSIONINFOEXA, DWORD, DWORDLONG);
  PFnVerSetCondition pfnVerSetCondition;
  PFnVerifyVersionA pfnVerifyVersionA;

  dwVersion = GetVersion();

  // Are we running Windows NT?

  if (!(dwVersion & 0x80000000))
  {
    // Is it Windows 2000 or greater?

    if (LOBYTE(LOWORD(dwVersion)) > 4)
    {
      // In Windows 2000, use the VerifyVersionInfo and
      // VerSetConditionMask functions. Don't static link because
      // it won't load on earlier systems.

      hmodNtDll = GetModuleHandleA( "ntdll.dll" );
      if (hmodNtDll)
      {
        pfnVerSetCondition = (PFnVerSetCondition) GetProcAddress(
            hmodNtDll, "VerSetConditionMask");
        if (pfnVerSetCondition != NULL)
        {
          dwlCondition = (*pfnVerSetCondition) (dwlCondition,
              VER_SUITENAME, VER_AND);

          // Get a VerifyVersionInfo pointer.

          hmodK32 = GetModuleHandleA( "KERNEL32.DLL" );
          if (hmodK32 != NULL)
          {
            pfnVerifyVersionA = (PFnVerifyVersionA) GetProcAddress(
               hmodK32, "VerifyVersionInfoA") ;
            if (pfnVerifyVersionA != NULL)
            {
              ZeroMemory(&osVersion, sizeof(osVersion));
              osVersion.dwOSVersionInfoSize = sizeof(osVersion);
              osVersion.wSuiteMask = VER_SUITE_TERMINAL;
              bResult = (*pfnVerifyVersionA) (&osVersion,
                  VER_SUITENAME, dwlCondition);
            }
          }
        }
      }
    }
    else  // This is Windows NT 4.0 or earlier.

      bResult = ValidateProductSuite( "Terminal Server" );
  }

  return bResult;
}


// ==========================================================================
// ValidateProductSuite()
//
// Purpose:
//  Terminal Services detection code for systems running
//  Windows NT 4.0 and earlier.
// ==========================================================================
BOOL ValidateProductSuite (LPSTR lpszSuiteToValidate)
{
  BOOL fValidated = FALSE;
  LONG lResult;
  HKEY hKey = NULL;
  DWORD dwType = 0;
  DWORD dwSize = 0;
  LPSTR lpszProductSuites = NULL;
  LPSTR lpszSuite;

  // Open the ProductOptions key.

  lResult = RegOpenKeyA(
      HKEY_LOCAL_MACHINE,
      "System\\CurrentControlSet\\Control\\ProductOptions",
      &hKey
  );
  if (lResult != ERROR_SUCCESS)
      goto exit;

  // Determine required size of ProductSuite buffer.

  lResult = RegQueryValueExA( hKey, "ProductSuite", NULL, &dwType,
      NULL, &dwSize );
  if (lResult != ERROR_SUCCESS || !dwSize)
      goto exit;

  // Allocate buffer.

  lpszProductSuites = (LPSTR) LocalAlloc( LPTR, dwSize );
  if (!lpszProductSuites)
      goto exit;

  // Retrieve array of product suite strings.

  lResult = RegQueryValueExA( hKey, "ProductSuite", NULL, &dwType,
      (LPBYTE) lpszProductSuites, &dwSize );
  if (lResult != ERROR_SUCCESS || dwType != REG_MULTI_SZ)
      goto exit;

  // Search for suite name in array of strings.

  lpszSuite = lpszProductSuites;
  while (*lpszSuite)
  {
      if (lstrcmpA( lpszSuite, lpszSuiteToValidate ) == 0)
      {
          fValidated = TRUE;
          break;
      }
      lpszSuite += (lstrlenA( lpszSuite ) + 1);
  }

exit:
  if (lpszProductSuites)
      LocalFree( lpszProductSuites );

  if (hKey)
      RegCloseKey( hKey );

  return fValidated;
}

// ==========================================================================
// ShowBillboard()
//
// Purpose:
//  Display billboard on created thread
// ==========================================================================
void ShowBillboard(DWORD * pdwThreadId, HANDLE * phThread)
{
    if (!g_isBillboardVisible)
    {
        *phThread = CreateThread(NULL, 0L, StaticThreadProc, (LPVOID)NULL, 0, pdwThreadId );
        g_isBillboardVisible = true;
    }
}

// ==========================================================================
// TeardownBillboard()
//
// Purpose:
//  Take down billboard
// ==========================================================================
void TeardownBillboard(DWORD dwThreadId, HANDLE hThread)
{
    if (g_isBillboardVisible)
    {
        //    Tell the thread to destroy the modeless dialog
        PostThreadMessage( dwThreadId, PWM_THREADDESTROYWND, 0, 0 );
        WaitForSingleObject( hThread, INFINITE );
        CloseHandle( hThread );
        g_isBillboardVisible = false;
    }
}

// ==========================================================================
// StaticThreadProc()
//
// Purpose:
//  Thread proc that creates our billboard dialog
// ==========================================================================
DWORD WINAPI StaticThreadProc( LPVOID lpParameter )
{
    MSG msg;

    HWND hwnd;

    hwnd = CreateDialog(g_settings.GetHInstance(),
                        MAKEINTRESOURCE(IDD_BILLBOARD),
                        GetDesktopWindow(),
                        BillboardProc);

    ShowWindow(hwnd, SW_SHOW);

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        if (!::IsDialogMessage( hwnd, &msg ))
        {
                if (msg.message == PWM_THREADDESTROYWND)
                {
                    //    Tell the dialog to destroy itself
                    DestroyWindow(hwnd);

                    //    Tell our thread to break out of message pump
                    PostThreadMessage( g_dwThread, WM_QUIT, 0, 0 );
                }
        }
    }

    return( 0L );
}

// ==========================================================================
// BillboardProc()
//
// Purpose:
//  Callback proc used to set HWND_TOPMOST on billboard
// ==========================================================================
BOOL CALLBACK BillboardProc(HWND hwndDlg,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            SetWindowPos( hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
            return TRUE;
    }
    return FALSE;
}

// Return the index of the search string
size_t getStringIndex(char *searchStr, char *searchFor)
{
    size_t searchLen = strlen(searchStr);
    size_t forLen = strlen(searchFor);
    for (size_t i=0; i < searchLen; i++)
    {
        if (searchStr[i] != searchFor[0])
            continue;
        bool found = false;
        for (size_t j=0; j < forLen; j++)
        {
            found = false;
            if (i+j > searchLen)
                break;
            if (searchStr[i+j] != searchFor[j])
                break;
            found = true;
        }
        if (found)
            return i;
    }
    return -1;
}

void checkIE()
{
    // now we'll check the registry for the version of the Internet Explorer
    //
    LONG lResult;
    HKEY hkey = NULL;

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, // HKLM
                            g_tszIERegKey,      // name of subkey to open
                            NULL,
                            KEY_READ,
                            &hkey               // handle to open key
                            );

    // we don't proceed unless the call above succeeds
    if ((ERROR_SUCCESS != lResult) && (ERROR_FILE_NOT_FOUND != lResult))
        throw HRESULT_FROM_WIN32(lResult);

    if (ERROR_SUCCESS == lResult)
    {
        TCHAR szVersion[MAX_PATH];
        DWORD dwBufLen = LENGTH(szVersion);

        lResult = RegQueryValueEx( hkey,
                                g_tszIERegKeyValue,
                                NULL,
                                NULL,
                                (LPBYTE)szVersion,
                                &dwBufLen);

        // Close the Registry
        RegCloseKey(hkey);

        if (ERROR_SUCCESS == lResult)
        {
            char majorVersionStr[2], minorVersionStr[2];
            majorVersionStr[0] = szVersion[0];
            majorVersionStr[1] = '\0';
            minorVersionStr[0] = szVersion[2];
            minorVersionStr[1] = '\0';
            double ieVersion = (atof(majorVersionStr) * 10) + atof(minorVersionStr);
            IENeedsInstalling = (ieVersion < atof(g_settings.GetIEVersion()));
        }
        // if we receive an error other than 0x2, throw
        else if (ERROR_FILE_NOT_FOUND != lResult)
            throw HRESULT_FROM_WIN32(lResult);
        else
            IENeedsInstalling = (g_settings.InstallIE() == TRUE);
    }
    if (test)
        IENeedsInstalling = true;
}

void checkMSI()
{
    // now we'll check the registry for the Microsoft Setup Installer
    //
    LONG lResult;
    HKEY hkey = NULL;

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,  // HKLM
                            g_tszMSIRegKey,      // name of subkey to open
                            NULL,
                            KEY_READ,
                            &hkey                // handle to open key
                            );

    // we don't proceed unless the call above succeeds
    if ((ERROR_SUCCESS != lResult) && (ERROR_FILE_NOT_FOUND != lResult))
    {
        throw HRESULT_FROM_WIN32(lResult);
    }

    TCHAR szLocation[MAX_PATH];
    if (ERROR_SUCCESS == lResult)
    {
        DWORD dwBufLen = LENGTH(szLocation);

        lResult = RegQueryValueEx( hkey,
                                   g_tszMSIRegKeyValue,
                                   NULL,
                                   NULL,
                                   (LPBYTE)szLocation,
                                   &dwBufLen);

        // Close the Registry
        RegCloseKey(hkey);

        if (ERROR_SUCCESS == lResult)
        {
            strcat_s(szLocation, MAX_PATH, "\\msiexec.exe");

            // Find out how much space we need to store the version information block.
            DWORD dwVersionInfoSize;
            DWORD dwZero;   // Dummy variable.
            dwVersionInfoSize = GetFileVersionInfoSize(szLocation, &dwZero);
            if (dwVersionInfoSize)
            {
                //Allocate space to store the version info.
                void* pVersionInfo = malloc(dwVersionInfoSize);

                // Use GetFileVersionInfo to copy the version info block into pVersion info.
                if (!GetFileVersionInfo(szLocation, 0, dwVersionInfoSize, pVersionInfo))
                {
                    free(pVersionInfo);
                    return;
                }

                /*
                 * Use VerQueryValue to parse the version information
                 * data block and get a pointer to the VS_FIXEDFILEINFO
                 * structure.
                 */
                VS_FIXEDFILEINFO* pFixedFileInfo;
                UINT nBytesReturned;
                if (!VerQueryValue(pVersionInfo, "\\", (void**)&pFixedFileInfo, &nBytesReturned))
                {
                    free(pVersionInfo);
                    return;
                }

                WORD major = HIWORD(pFixedFileInfo->dwFileVersionMS);
                WORD minor = LOWORD(pFixedFileInfo->dwFileVersionMS);
                MSINeedsInstalling = (major < 2);

                // Cleanup
                free(pVersionInfo);
            }
        }
        // if we receive an error other than 0x2, throw
        else if (ERROR_FILE_NOT_FOUND != lResult)
            throw HRESULT_FROM_WIN32(lResult);
        else
            MSINeedsInstalling = (g_settings.InstallMSI() == TRUE);
    }

    if (test)
        MSINeedsInstalling = true;
}


void checkMDAC()
{
    // now we'll check the registry for the MDAC version
    //
    LONG lResult;
    HKEY hkey = NULL;

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,   // HKLM
                            g_tszMDACRegKey,      // name of subkey to open
                            NULL,
                            KEY_READ,
                            &hkey                 // handle to open key
                            );

    // we don't proceed unless the call above succeeds
    if ((ERROR_SUCCESS != lResult) && (ERROR_FILE_NOT_FOUND != lResult))
        throw HRESULT_FROM_WIN32(lResult);

    TCHAR szVersion[MAX_PATH];
    if (ERROR_SUCCESS == lResult)
    {
        DWORD dwBufLen = LENGTH(szVersion);

        lResult = RegQueryValueEx( hkey,
                                g_tszMDACRegKeyValue,
                                NULL,
                                NULL,
                                (LPBYTE)szVersion,
                                &dwBufLen);

        // Close the Registry
        RegCloseKey(hkey);

        if (ERROR_SUCCESS == lResult)
        {
            int   i;
            int   iDot = 0;
            TCHAR szVersionAux[15];
            for (i = 0; (szVersion[i] && (iDot < 2)); i++)
            {
                szVersionAux[i] = szVersion[i];
                if (szVersion[i] == '.')
                    iDot++;
            }
            szVersionAux[i-1] = END_OF_STRING;

            MDACNeedsInstalling  = (atof(szVersionAux) < atof((char *)g_settings.GetMDACVersion()));
        }
        else if (ERROR_FILE_NOT_FOUND != lResult)
            throw HRESULT_FROM_WIN32(lResult);
        else
            MDACNeedsInstalling = (g_settings.InstallMDAC() == TRUE);
    }
    if (test)
        MDACNeedsInstalling = true;
}

void checkMSDE()
{
    // now we'll check the registry for MSDE
    //
    LONG lResult;
    HKEY hkey = NULL;

    MSDENeedsInstalling = true;
    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, // HKLM
                            g_tszMSDERegKey,    // name of subkey to open
                            NULL,
                            KEY_READ,
                            &hkey               // handle to open key
                            );

    // we don't proceed unless the call above succeeds
    if ((ERROR_SUCCESS != lResult) && (ERROR_FILE_NOT_FOUND != lResult))
    {
        RegCloseKey(hkey);
        throw HRESULT_FROM_WIN32(lResult);
    }

    TCHAR szVersion[MAX_PATH];
    if (ERROR_SUCCESS == lResult)
    {
        DWORD dwBufLen = LENGTH(szVersion);

        lResult = RegQueryValueEx( hkey,
                                g_tszMSDERegKeyValue,
                                NULL,
                                NULL,
                                (LPBYTE)szVersion,
                                &dwBufLen);

        // Close the Registry
        RegCloseKey(hkey);

        // if we receive an error other than 0x2, throw
        if (ERROR_SUCCESS == lResult)
            MSDENeedsInstalling = false;
        else if (ERROR_FILE_NOT_FOUND != lResult)
            throw HRESULT_FROM_WIN32(lResult);
        else
            MSDENeedsInstalling = (g_settings.InstallMSDE() == TRUE);

    }
    if (test)
        MSDENeedsInstalling = true;
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// the dialog procedure
{
    switch(msg)
    {
        case WM_INITDIALOG: // our dialog is being shown
            {
                HWND  desktop = GetDesktopWindow();     // Handle for the desktop
                HWND  control = NULL;                   // Handle for dialog controls
                TCHAR dummyMask[MAX_PATH];              // Temporary text
                TCHAR dummyText[MAX_PATH];              // Temporary text
                RECT  screenRect;                       // Screen Area
                RECT  dialogRect;                       // Dialog Area

                /*
                 * Get the Screen and Dialog
                 */
                GetClientRect(desktop, &screenRect);
                GetClientRect(hwnd, &dialogRect);

                /*
                 * Positions the window
                 */
                SetWindowPos(hwnd,
                    HWND_TOP,
                    (screenRect.right-screenRect.left)/2 - (dialogRect.right - dialogRect.left)/2,
                    (screenRect.bottom-screenRect.top)/2 - (dialogRect.bottom - dialogRect.top)/2,
                    (dialogRect.right - dialogRect.left),
                    (dialogRect.bottom - dialogRect.top),
                    SWP_NOSIZE);

                /*
                 * IE Checkbox
                 */
                control = GetDlgItem(hwnd, IDC_IE);
                GetDlgItemText(hwnd, IDC_IE, (LPSTR)dummyMask, MAX_PATH);
                sprintf_s(dummyText,
                             MAX_PATH,
                             dummyMask,
                             g_settings.GetIEVersion());
                SetDlgItemText(hwnd, IDC_IE, (LPCSTR)dummyText);
                if (!test)
                    EnableWindow(control, false);
                if (IENeedsInstalling)
                    CheckDlgButton(hwnd, IDC_IE, BST_CHECKED);
                if (!g_settings.InstallIE())
                    EnableWindow(control, false);
                /*
                 * MSI Checkbox
                 */
                control = GetDlgItem(hwnd, IDC_MSI);
                if (!test)
                    EnableWindow(control, false);
                if (MSINeedsInstalling)
                    CheckDlgButton(hwnd, IDC_MSI, BST_CHECKED);
                if (!g_settings.InstallMSI())
                    EnableWindow(control, false);
                /*
                 * .NET Framework checkbox
                 */
                control = GetDlgItem(hwnd, IDC_DOTNET);
                GetDlgItemText(hwnd, IDC_DOTNET, (LPSTR)dummyMask, MAX_PATH);
                sprintf_s(dummyText,
                             MAX_PATH,
                             dummyMask,
                             g_settings.GetFxVersion());
                SetDlgItemText(hwnd, IDC_DOTNET, (LPCSTR)dummyText);
                if (!test)
                    EnableWindow(control, false);
                if (FxNeedsInstalling)
                    CheckDlgButton(hwnd, IDC_DOTNET, BST_CHECKED);
                if (!g_settings.InstallFramework())
                    EnableWindow(control, false);
                /*
                 * MDAC Checkbox
                 */
                control = GetDlgItem(hwnd, IDC_MDAC);
                GetDlgItemText(hwnd, IDC_MDAC, (LPSTR)dummyMask, MAX_PATH);
                sprintf_s(dummyText,
                             MAX_PATH,
                             dummyMask,
                             g_settings.GetMDACVersion());
                SetDlgItemText(hwnd, IDC_MDAC, (LPCSTR)dummyText);
                if (MDACNeedsInstalling)
                    CheckDlgButton(hwnd, IDC_MDAC, BST_CHECKED);
                else // disable if it doesn't need to be installed
                    EnableWindow(control, false);
                if (!g_settings.InstallMDAC())
                    EnableWindow(control, false);
                /*
                 * MSDE Checkbox
                 */
                control = GetDlgItem(hwnd, IDC_MSDE);
                if (MSDENeedsInstalling)
                    CheckDlgButton(hwnd, IDC_MSDE, BST_CHECKED);
                else // disable if it doesn't need to be installed
                    EnableWindow(control, false);
                if (!g_settings.InstallMSDE())
                    EnableWindow(control, false);
            }
            break;

        case WM_COMMAND: // we got a message from a control/menu
            // in this case, a button
            switch(LOWORD(wParam))
            {
                case IDOK:
                    /*
                     * The Install Button
                     */
                    EndDialog(hwnd, TRUE);
                    break;

                case IDCANCEL:
                    /*
                     * The Cancel Button (or the "X", or Alt+F4)
                     */
                    EndDialog(hwnd, FALSE);
                    break;

                case IDC_IE:
                    /*
                     * IE Checkbox
                     */
                    {
                        HWND control = GetDlgItem(hwnd, IDC_IE);
                        IENeedsInstalling = (IsDlgButtonChecked(control, IDC_IE) == BST_CHECKED);
                        break;
                    }

                case IDC_MSI:
                    /*
                     * MSI Checkbox
                     */
                    {
                        HWND control = GetDlgItem(hwnd, IDC_MSI);
                        MSINeedsInstalling = (IsDlgButtonChecked(control, IDC_MSI) == BST_CHECKED);
                        break;
                    }

                case IDC_MDAC:
                    /*
                     * MDAC Checkbox
                     */
                    {
                        HWND control = GetDlgItem(hwnd, IDC_MDAC);
                        MDACNeedsInstalling = (IsDlgButtonChecked(control, IDC_MDAC) == BST_CHECKED);
                        break;
                    }

                case IDC_MSDE:
                    /*
                     * MSDE Checkbox
                     */
                    {
                        HWND control = GetDlgItem(hwnd, IDC_MSDE);
                        MSDENeedsInstalling = (IsDlgButtonChecked(control, IDC_MSDE) == BST_CHECKED);
                        break;
                    }

                case IDC_DOTNET:
                    /*
                     * .NET Framework Checkbox
                     */
                    {
                        HWND control = GetDlgItem(hwnd, IDC_DOTNET);
                        FxNeedsInstalling  = (IsDlgButtonChecked(control, IDC_DOTNET) == BST_CHECKED);
                        break;
                    }
                }
            break;

        case WM_DESTROY:
            /*
             * Dialog is off the screen by now...
             */
            break;

        default:
            /*
             * Any other message that we didn't processed...
             */
        return FALSE;
    }
    /*
     * OK, We've processed this message.
     */
    return TRUE;
}

