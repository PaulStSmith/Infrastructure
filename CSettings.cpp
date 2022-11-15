/// ==========================================================================
// Name    :    CSettings.cpp
// Purpose :    simple wrapper class for handling app
//              settings by means of an .ini file

// includes
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <direct.h>
#include <string.h>

#include "CSettings.h"
#include "CError.h"
#include "resource.h"
#ifndef __PLATFORM
#include "OSDetector.h"
#endif

// ==========================================================================
// Initializes an instance of the CSettings class.
// ==========================================================================
CSettings::CSettings(LPCTSTR pszIniName)
{
    TCHAR * pszWalk         = NULL;

    /*
     * Initialize the private variables
     */
    m_bQuietMode            = FALSE;
    m_hAppInst              = NULL;
    *m_szIniName            = END_OF_STRING;
    *m_szBoolAux            = END_OF_STRING;

    /*
     * Application properties
     */
    *m_szMsi                = END_OF_STRING;
    *m_szProductName        = END_OF_STRING;
    *m_szCaptionText        = END_OF_STRING;
    *m_szErrorCaptionText   = END_OF_STRING;

    /*
     * .NET Properties
     */
    *m_szFxInstallerName    = END_OF_STRING;
    *m_szFxInstallerPath    = END_OF_STRING;
    *m_szFxInstallerParams  = END_OF_STRING;
    *m_szFxVersion          = END_OF_STRING;

    /*
     * MSI Properties
     */
    *m_szMSIInstallerName   = END_OF_STRING;
    *m_szMSIInstallerPath   = END_OF_STRING;
    *m_szMSIInstallerParams = END_OF_STRING;

    /*
     * IE Properties
     */
    *m_szIEName             = END_OF_STRING;
    *m_szIEPath             = END_OF_STRING;
    *m_szIEParams           = END_OF_STRING;
    *m_szIEVersion          = END_OF_STRING;

    /*
     * MDAC Properties
     */
    *m_szMDACPath           = END_OF_STRING;
    *m_szMDACParams         = END_OF_STRING;
    *m_szMDACVersion        = END_OF_STRING;

    /*
     * MSDE Properties
     */
    *m_szMSDEPath           = END_OF_STRING;
    *m_szMSDEParams         = END_OF_STRING;

    /*
     * Get the executable full path
     */
    GetModuleFileName( g_settings.GetHInstance(), m_szIniName, LENGTH(m_szIniName) );

    /*
     * Find the end of the path
     */
    pszWalk = _tcsrchr( m_szIniName, BACKSLASH );
    if (pszWalk)
    {
        pszWalk++; // preserve trailing '\'
        *pszWalk = END_OF_STRING;
    }

    /*
     * Change to the application directory
     */
    _chdir((const char *)m_szIniName);

    /*
     * Sets the ini file name "settings.ini"
     */
    strncat_s(m_szIniName, pszIniName, LENGTH(m_szIniName));
}

// ==========================================================================
// Loads the INI file and setup the respective properties.
// ==========================================================================
BOOL CSettings::Parse()
{
    BOOL  bSucceeded = TRUE;
    DWORD dwCount    = 0;

    /*
     *  Note that we continue to parse even
     *  if encountering a problem. This is
     *  because some values are initialized
     *  even in the absence of an ini file
     *  and are required for msg box captions, etc.
     */

    /*
     * Load the WhatInstall
     */
    ParseWhatInstall();

    /*
     * Parse the Application Keys
     */
    bSucceeded = (BOOL)(bSucceeded & ParseApplication());

    /*
     * Parse the Framework keys
     */
    bSucceeded = (BOOL)(bSucceeded & ParseFramework());

    /*
     * Parse the MSI keys
     */
    bSucceeded = (BOOL)(bSucceeded & ParseMSI());

    /*
     * Parse the IE keys
     */
    bSucceeded = (BOOL)(bSucceeded & ParseIE());

    /*
     * Parse the MDAC keys
     */
    bSucceeded = (BOOL)(bSucceeded & ParseMDAC());

    /*
     * Parse the MSDE keys
     */
    bSucceeded = (BOOL)(bSucceeded & ParseMSDE());

    /*
     * Return the result
     */
    return bSucceeded;
}

// ==========================================================================
// Gets the full path of the current application.
// ==========================================================================
TCHAR * CSettings::GetPath()
{
    TCHAR * pszWalk = NULL;
    static TCHAR szPath[MAX_PATH + 1];

    GetModuleFileName( m_hAppInst,
                       szPath,
                       LENGTH(szPath) );

    pszWalk = _tcsrchr( szPath, BACKSLASH );

    if (pszWalk)
    {
        pszWalk++; // preserve trailing '\'
        *pszWalk = END_OF_STRING;
   }

    return (szPath);
}

// ==========================================================================
// Parses the [WhatInstall] group.
// ==========================================================================
void CSettings::ParseWhatInstall()
{
    DWORD dwCount = 0;

    /*
     * Check if we need to install IE6
     */
    dwCount = GetPrivateProfileString(g_szWhatInstall,
                                      g_szInstallIE6,
                                      NULL,
                                      m_szBoolAux,
                                      LENGTH(m_szBoolAux),
                                      m_szIniName);
    if (dwCount == 0)
        m_bInstallIE = FALSE;
    else
        m_bInstallIE = (*m_szBoolAux != '0');

    /*
     * Check if we need to install Windows Installer
     */
    dwCount = GetPrivateProfileString(g_szWhatInstall,
                                      g_szInstallMSI,
                                      NULL,
                                      m_szBoolAux,
                                      LENGTH(m_szBoolAux),
                                      m_szIniName);
    if (dwCount == 0)
        m_bInstallMSI = FALSE;
    else
        m_bInstallMSI = (*m_szBoolAux != '0');

    /*
     * Check if we need to install MDAC
     */
    dwCount = GetPrivateProfileString(g_szWhatInstall,
                                      g_szInstallMDAC,
                                      NULL,
                                      m_szBoolAux,
                                      LENGTH(m_szBoolAux),
                                      m_szIniName);
    if (dwCount == 0)
        m_bInstallMDAC = FALSE;
    else
        m_bInstallMDAC = (*m_szBoolAux != '0');

    /*
     * Check if we need to install MSDE
     */
    dwCount = GetPrivateProfileString(g_szWhatInstall,
                                      g_szInstallMSDE,
                                      NULL,
                                      m_szBoolAux,
                                      LENGTH(m_szBoolAux),
                                      m_szIniName);
    if (dwCount == 0)
        m_bInstallMSDE = FALSE;
    else
        m_bInstallMSDE = (*m_szBoolAux != '0');

    /*
     * Check if we need to install .NET Framework
     */
    dwCount = GetPrivateProfileString(g_szWhatInstall,
                                      g_szInstallFramework,
                                      NULL,
                                      m_szBoolAux,
                                      LENGTH(m_szBoolAux),
                                      m_szIniName);
    if (dwCount == 0)
        m_bInstallFramework = FALSE;
    else
        m_bInstallFramework = (*m_szBoolAux != '0');

}

// ==========================================================================
// Parses the application keys of the [Bootstrap] group.
// ==========================================================================
BOOL CSettings::ParseApplication()
{
    DWORD dwCount    = 0;
    BOOL  bSucceeded = TRUE;

    /*
     * Retrieve the general application name.
     * If this key is not present, we will load
     * a resource-based default value
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szProductNameKey,
                                      NULL,
                                      m_szProductName,
                                      LENGTH(m_szProductName),
                                      m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Load the default name "Application"
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_DEFAULT_PRODUCTNAME,
                      m_szProductName,
                      LENGTH(m_szProductName) ) ;

        /*
         * If fail to load the default
         */
        if ( *m_szProductName == END_OF_STRING)
            sprintf_s(m_szProductName, MAX_PATH, "%s", "Application");
    }

    /*
     * Retrieve the default messagebox caption text.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                     g_szCaptionTextKey,
                                     NULL,
                                     m_szCaptionText,
                                     LENGTH(m_szCaptionText),
                                     m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Load the default name "<ProductName> Setup"
         */
        TCHAR szCaption[MAX_PATH];
        ::LoadString( g_settings.GetHInstance(),
                      IDS_DIALOG_CAPTION,
                      szCaption,
                      LENGTH(szCaption) ) ;

        /*
         * If fail to load the default
         */
        if ( *szCaption == END_OF_STRING)
            sprintf_s( m_szCaptionText, MAX_PATH, "%s Setup", g_settings.GetProductName() );
        else
            sprintf_s( m_szCaptionText, MAX_PATH, szCaption, g_settings.GetProductName() );
    }

    // Retrieve the default error caption text.
    // This is not a required key
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szErrorCaptionTextKey,
                                      NULL,
                                      m_szErrorCaptionText,
                                      LENGTH(m_szErrorCaptionText),
                                      m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Load the default "<ProductName> Setup Error"
         */
        TCHAR szErrorCaption[MAX_PATH];
        ::LoadString( g_settings.GetHInstance(),
                      IDS_ERROR_CAPTION,
                      szErrorCaption,
                      LENGTH(szErrorCaption) ) ;

        /*
         * If fail to load the default
         */
        if ( *szErrorCaption == END_OF_STRING )
            sprintf_s( m_szErrorCaptionText, MAX_PATH, "%s Setup Error", g_settings.GetProductName() );
        else
            sprintf_s( m_szErrorCaptionText, MAX_PATH, szErrorCaption, g_settings.GetProductName() );
    }

    /*
     * Retrieve the .MSI name that we will install
     * This is a required key
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMsiKey,
                                      NULL,
                                      m_szMsi,
                                      LENGTH(m_szMsi),
                                      m_szIniName);

    /*
     * MSI is a required key, fail if not present
     */
    if (dwCount == 0)
    {
        CError se(IDS_INI_KEY_NOT_FOUND, 0, MB_ICONERROR, 0, g_szMsiKey );
        bSucceeded = FALSE;
        throw (se);
    }

    return bSucceeded;
}

// ==========================================================================
// Parses the .NET Framework keys of the [Bootstrap] group.
// ==========================================================================
BOOL CSettings::ParseFramework()
{
    DWORD dwCount    = 0;
    BOOL  bSucceeded = TRUE;

    /*
     * Retrieve path to "dotnetfx.exe".
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                     g_szFxInstallerPathKey,
                                     NULL,
                                     m_szFxInstallerPath,
                                     LENGTH(m_szFxInstallerPath),
                                     m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Defaults to the SETUP.EXE path
         */
        sprintf_s(m_szFxInstallerPath, MAX_PATH, "%s", GetPath());
    }

    /*
     * Retrieve the required .NET Framework Setup name.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szFxInstallerNameKey,
                                      NULL,
                                      m_szFxInstallerName,
                                      LENGTH(m_szFxInstallerName),
                                      m_szIniName);
    if (dwCount == 0)
    {
        /*
         * Load the default "dotnetfx.exe"
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_DOTNET_INSTALLER,
                      m_szFxInstallerName,
                      LENGTH(m_szFxInstallerName) ) ;

        /*
         * If we fail to load the text from the resource, that's bad.
         */
        if ( *m_szFxInstallerName == END_OF_STRING )
            sprintf_s(m_szFxInstallerName, MAX_PATH, "%s", "dotnetfx.exe");
    }

    /*
     * Retrieve the .NET Framework Setup parameter.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szFxInstallerParamsKey,
                                      NULL,
                                      m_szFxInstallerParams,
                                      LENGTH(m_szFxInstallerParams),
                                      m_szIniName);
    if (dwCount == 0)
    {
        /*
         * Load the default /q:a /c:"install /l /q"
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_DOTNET_PARAMS,
                      m_szFxInstallerParams,
                      LENGTH(m_szFxInstallerParams) ) ;

        /*
         * If we fail to load the text from the resource, that's bad.
         */
        if ( *m_szFxInstallerParams == END_OF_STRING )
            sprintf_s(m_szFxInstallerParams, MAX_PATH, "%s", "/q:a /c:\"install /l /q\"");
    }

    /*
     * Retrieve the required .Net Version.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szFxVersionKey,
                                      NULL,
                                      m_szFxVersion,
                                      LENGTH(m_szFxVersion),
                                      m_szIniName);

    if ((dwCount == 0) && g_settings.InstallFramework())
    {
        CError se(IDS_INI_KEY_NOT_FOUND, 0, MB_ICONERROR, 0, g_szFxVersionKey );
        bSucceeded = FALSE;
        throw (se);
    }

    return bSucceeded;
}

// ==========================================================================
// Parses the Internet Explorer keys of the [Bootstrap] group.
// ==========================================================================
BOOL CSettings::ParseIE()
{
    DWORD dwCount    = 0;
    BOOL  bSucceeded = TRUE;

    /*
     * Retrieve path to IE Installer.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szIEInstallerPathKey,
                                      NULL,
                                      m_szIEPath,
                                      LENGTH(m_szIEPath),
                                      m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Defaults to the Local Path
         */
        sprintf_s(m_szIEPath, MAX_PATH, "%s", GetPath());
    }

    /*
     * Retrieve IE setup installer parameters
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szIEInstallerParamsKey,
                                      NULL,
                                      m_szIEParams,
                                      LENGTH(m_szIEParams),
                                      m_szIniName);
    if (dwCount == 0)
    {
        /*
         * Load the default /passive
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_IE_PARAMS,
                      m_szIEParams,
                      LENGTH(m_szIEParams) ) ;

        /*
         * If we fail to load the text from the resource, that's bad.
         */
        if ( *m_szIEParams == END_OF_STRING )
            sprintf_s(m_szIEParams, MAX_PATH, "%s", "/passive");
    }

    /*
     * Retrieve IE setup installer name
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szIEInstallerNameKey,
                                      NULL,
                                      m_szIEName,
                                      LENGTH(m_szIEName),
                                      m_szIEName);
    if (dwCount == 0)
    {
        /*
         * Load the default "ie6setup.exe"
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_IE_INSTALLER,
                      m_szIEName,
                      LENGTH(m_szIEName) ) ;

        /*
         * If fail to load the default
         */
        if ( *m_szIEName == END_OF_STRING )
            sprintf_s(m_szIEName, MAX_PATH, "%s", "ie6setup.exe");
    }

    /*
     * Retrieve the required IE version
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szIEVersionKey,
                                      NULL,
                                      m_szIEVersion,
                                      LENGTH(m_szIEVersion),
                                      m_szIniName);

    if ((dwCount == 0) && g_settings.InstallIE())
    {
        CError se(IDS_INI_KEY_NOT_FOUND, 0, MB_ICONERROR, 0, g_szIEVersionKey );
        bSucceeded = FALSE;
        throw (se);
    }
    else if (dwCount == 0)
    {
        /*
         * Load the default 6.0
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_IE_VERSION,
                      m_szIEVersion,
                      LENGTH(m_szIEVersion) ) ;

        /*
         * If fail to load the default
         */
        if ( *m_szIEVersion == END_OF_STRING )
            sprintf_s(m_szIEVersion, MAX_PATH, "%s", "6.0");
    }

    return bSucceeded;
}

// ==========================================================================
// Parses the MDAC keys of the [Bootstrap] group.
// ==========================================================================
BOOL CSettings::ParseMDAC()
{
    DWORD dwCount    = 0;
    BOOL  bSucceeded = TRUE;

    /*
     * Retrieve path to MDAC Installer.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMDACInstallerPathKey,
                                      NULL,
                                      m_szMDACPath,
                                      LENGTH(m_szMDACPath),
                                      m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Defaults to the Local Path
         */
        sprintf_s(m_szMDACPath, MAX_PATH, "%s", GetPath());
    }

    /*
     * Retrieve the MDAC parameters.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMDACInstallerParamsKey,
                                      NULL,
                                      m_szMDACParams,
                                      LENGTH(m_szMDACParams),
                                      m_szIniName);
    if (dwCount == 0)
    {
        /*
         * Load the default /q
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_MDAC_PARAMS,
                      m_szMDACParams,
                      LENGTH(m_szMDACParams) ) ;

        /*
         * If fail to load the default
         */
        if ( *m_szMDACParams == END_OF_STRING )
            sprintf_s(m_szMDACParams, MAX_PATH, "%s", "/Q");
    }

    /*
     * Get the required MDAC Version
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMDACVersionKey,
                                      NULL,
                                      m_szMDACVersion,
                                      LENGTH(m_szMDACVersion),
                                      m_szIniName);

    if ((dwCount == 0) && g_settings.InstallMDAC())
    {
        CError se(IDS_INI_KEY_NOT_FOUND, 0, MB_ICONERROR, 0, g_szMDACVersionKey );
        bSucceeded = FALSE;
        throw (se);
    }
    else if (dwCount == 0)
    {
        /*
         * Defaults to 2.7
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_MDAC_VERSION,
                      m_szMDACVersion,
                      LENGTH(m_szMDACVersion) ) ;

        if (*m_szMDACVersion == END_OF_STRING)
            sprintf_s(m_szMDACVersion, MAX_PATH, "%s", "2.7");
    }

    return bSucceeded;
}

// ==========================================================================
// Parses the MSDE keys of the [Bootstrap] group.
// ==========================================================================
BOOL CSettings::ParseMSDE()
{
    DWORD dwCount    = 0;
    BOOL  bSucceeded = TRUE;

    /*
     * Retrieve path to MSDE Installer.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMSDEInstallerPathKey,
                                      NULL,
                                      m_szMSDEPath,
                                      LENGTH(m_szMSDEPath),
                                      m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Defaults to the Local Path
         */
        sprintf_s(m_szMSDEPath, MAX_PATH, "%s", GetPath());
    }

    /*
     * Retrieve the MSDE Parameters.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMSDEParamKey,
                                      NULL,
                                      m_szMSDEParams,
                                      LENGTH(m_szMSDEParams),
                                      m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Default to '/setup.ini /SAPWD="password"'
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_MSDE_PARAMS,
                      m_szMSDEParams,
                      LENGTH(m_szMSDEParams) ) ;

        // if we fail to load caption text, use default
        if (*m_szMSDEParams == END_OF_STRING)
            sprintf_s( m_szMSDEParams, MAX_PATH, "%s", "/setup.ini /SAPWD='password'" );
    }

    return bSucceeded;
}

// ==========================================================================
// Parses the Windows Installer keys of the [Bootstrap] group.
// ==========================================================================
BOOL CSettings::ParseMSI()
{
    DWORD dwCount    = 0;
    BOOL  bSucceeded = TRUE;

    /*
     * Retrieve path to MSI Installer.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMSIInstallerPathKey,
                                      NULL,
                                      m_szMSIInstallerPath,
                                      LENGTH(m_szMSIInstallerPath),
                                      m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Defaults to the Local Path
         */
        sprintf_s(m_szMSIInstallerPath, MAX_PATH, "%s", GetPath());
    }

    /*
     * Retrieve the parameters for the MSI installer
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMSIInstallerParamsKey,
                                      NULL,
                                      m_szMSIInstallerParams,
                                      LENGTH(m_szMSIInstallerParams),
                                      m_szIniName);

    if (dwCount == 0)
    {
        /*
         * Default to '/Q'
         */
        ::LoadString( g_settings.GetHInstance(),
                      IDS_MDAC_PARAMS,
                      m_szMSIInstallerParams,
                      LENGTH(m_szMSIInstallerParams) );

        /*
         * If fail to load the default
         */
        if (*m_szMSIInstallerParams == END_OF_STRING)
            sprintf_s(m_szMSIInstallerParams, MAX_PATH, "%s", "/Q");
    }

    /*
     * Retrieve path to MSI Name.
     */
    dwCount = GetPrivateProfileString(g_szBootstrap,
                                      g_szMSIInstallerNameKey,
                                      NULL,
                                      m_szMSIInstallerName,
                                      LENGTH(m_szMSIInstallerName),
                                      m_szIniName);

    if (dwCount == 0)
    {
        // Check the OS Version
        PLATFORM platform = getPlatform();

        /*
         * Set the default according to the Platform
         */
        if (platform == PLATFORM_WIN32)
            sprintf_s(m_szMSIInstallerName, MAX_PATH, "%s", g_tszMSI9x);
        else if (platform == PLATFORM_WIN32_NT)
            sprintf_s(m_szMSIInstallerName, MAX_PATH, "%s", g_tszMSINT);
        else if ((platform == PLATFORM_WIN32_XP) || (platform == PLATFORM_WIN32_2000))
            sprintf_s(m_szMSIInstallerName, MAX_PATH, "%s", g_tszMSI30);
    }

    return bSucceeded;
}
