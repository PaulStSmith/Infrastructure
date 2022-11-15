//
//   Copyright (c) Microsoft Corporation.  All rights reserved.
//
/// ==========================================================================
// Name    :    CSettings.h
// Purpose :    simple wrapper class for handling app
//              settings by means of an .ini

#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>
#include <tchar.h>

//defines
//
#define EMPTY_BUFFER  { _T( '\0' ) }
#define END_OF_STRING   _T( '\0' )
#define BACKSLASH       _T( '\\' )

#define MAX_MSG 4096

const TCHAR g_szSettingsFile[]           = _T("settings.ini");
const TCHAR g_szBootstrap[]              = _T("Bootstrap");
const TCHAR g_szMsiKey[]                 = _T("Package");
const TCHAR g_szProductNameKey[]         = _T("ProductName");
const TCHAR g_szCaptionTextKey[]         = _T("CaptionText");
const TCHAR g_szErrorCaptionTextKey[]    = _T("ErrorCaptionText");
/*
 * .NET Constants
 */
const TCHAR g_szFxInstallerNameKey[]     = _T("FxInstallerName");
const TCHAR g_szFxInstallerPathKey[]     = _T("FxInstallerPath");
const TCHAR g_szFxInstallerParamsKey[]   = _T("FxInstallerParams");
const TCHAR g_szFxVersionKey[]           = _T("FxVersion");
/*
 * Windows Installer Constants
 */
const TCHAR g_szMSIInstallerNameKey[]    = _T("MSIInstallerName");
const TCHAR g_szMSIInstallerPathKey[]    = _T("MSIInstallerPath");
const TCHAR g_szMSIInstallerParamsKey[]  = _T("MSIInstallerParams");
/*
 * Internet Explorer Constants
 */
const TCHAR g_szIEInstallerNameKey[]     = _T("IEInstallerName");
const TCHAR g_szIEInstallerPathKey[]     = _T("IEInstallerPath");
const TCHAR g_szIEInstallerParamsKey[]   = _T("IEInstallerParams");
const TCHAR g_szIEVersionKey[]           = _T("IEVersion");
/*
 * MDAC Constants
 */
const TCHAR g_szMDACInstallerPathKey[]   = _T("MDACInstallerPath");
const TCHAR g_szMDACInstallerParamsKey[] = _T("MDACInstallerParams");
const TCHAR g_szMDACVersionKey[]         = _T("MDACVersion");
/*
 * MSDE Constants
 */
const TCHAR g_szMSDEInstallerPathKey[]   = _T("MSDEInstallerPath");
const TCHAR g_szMSDEParamKey[]           = _T("MSDEInstallerParams");

/*
 * WhatInstall Constants
 */
const TCHAR g_szWhatInstall[]            = _T("WhatInstall");
const TCHAR g_szInstallIE6[]             = _T("IE");
const TCHAR g_szInstallMSI[]             = _T("WindowsInstaller");
const TCHAR g_szInstallMDAC[]            = _T("MDAC");
const TCHAR g_szInstallMSDE[]            = _T("MSDE");
const TCHAR g_szInstallFramework[]       = _T(".NETFramework");

// msi 3.0 installer (XP and NT2000 only)
const TCHAR g_tszMSI30[] = _T("InstMsi.exe");

// msi 2.0 installer
const TCHAR g_tszMSINT[] = _T("InstMsiW.exe");
const TCHAR g_tszMSI9x[] = _T("InstMsiA.exe");

// ==========================================================================
// class CSettings
//
// Purpose:
//  This class wraps an ini file specific to this sample
// ==========================================================================
class CSettings
{
public:
    // Constructors
    CSettings(LPCTSTR pszIniName = g_szSettingsFile);

    BOOL Parse();

    // Getters/Setters
           TCHAR * GetPath();
    /*
     * Application Properties
     */
    inline LPCTSTR GetMsi()                   { return m_szMsi; }
    inline LPCTSTR GetIniName()               { return m_szIniName; }
    inline LPCTSTR GetProductName()           { return m_szProductName; }
    inline LPCTSTR GetCaptionText()           { return m_szCaptionText; }
    inline LPCTSTR GetErrorCaptionText()      { return m_szErrorCaptionText; }

    /*
     * .NET Framework Properties
     */
    inline LPCTSTR GetFxInstallerName()       { return m_szFxInstallerName; }
    inline LPCTSTR GetFxInstallerPath()       { return m_szFxInstallerPath; }
    inline LPCTSTR GetFxInstallerParams()     { return m_szFxInstallerParams; }
    inline LPCTSTR GetFxVersion()             { return m_szFxVersion; }

    /*
     * MSI Properties
     */
    inline LPCTSTR GetMSIName()               { return m_szMSIInstallerName; }
    inline LPCTSTR GetMSIPath()               { return m_szMSIInstallerPath; }
    inline LPCTSTR GetMSIParams()             { return m_szMSIInstallerParams; }

    /*
     * IE Properties
     */
    inline LPCTSTR GetIEPath()                { return m_szIEPath; }
    inline LPCTSTR GetIEInstaller()           { return m_szIEName; }
    inline LPCTSTR GetIEParams()              { return m_szIEParams; }
    inline LPCTSTR GetIEVersion()             { return m_szIEVersion; }

    /*
     * MDAC Properties
     */
    inline LPCTSTR GetMDACPath()              { return m_szMDACPath; }
    inline LPCTSTR GetMDACParams()            { return m_szMDACParams; }
    inline LPCTSTR GetMDACVersion()           { return m_szMDACVersion; }

    /*
     * MDAC Properties
     */
    inline LPCTSTR GetMSDEPath()              { return m_szMSDEPath; }
    inline LPCTSTR GetMSDEParams()            { return m_szMSDEParams; }

    /*
     * WhatInstall Properties
     */
    inline BOOL    InstallIE()                { return m_bInstallIE; }
    inline BOOL    InstallMSI()               { return m_bInstallMSI; }
    inline BOOL    InstallFramework()         { return m_bInstallFramework; }
    inline BOOL    InstallMDAC()              { return m_bInstallMDAC; }
    inline BOOL    InstallMSDE()              { return m_bInstallMSDE; }

    /*
     * Quite Mode Properties
     */
    inline BOOL GetQuietMode()                { return m_bQuietMode; }
    inline void SetQuietMode(BOOL bQuiet)     { m_bQuietMode = bQuiet; }

    /*
     * Instance Mode
     */
    inline HINSTANCE GetHInstance()           { return m_hAppInst; }
    inline void SetHInstance(HINSTANCE hInst) { m_hAppInst = hInst; }

private:

    /*
     * Parsers
     */
    void        ParseWhatInstall();
    BOOL        ParseApplication();
    BOOL        ParseIE();
    BOOL        ParseMSI();
    BOOL        ParseFramework();
    BOOL        ParseMDAC();
    BOOL        ParseMSDE();

    /*
     * Instance
     */
    HINSTANCE   m_hAppInst;

    /*
     * Quiet Mode
     */
    BOOL        m_bQuietMode;

    /*
     * WhatInstall Properties
     */
    BOOL        m_bInstallIE;
    BOOL        m_bInstallMSI;
    BOOL        m_bInstallFramework;
    BOOL        m_bInstallMDAC;
    BOOL        m_bInstallMSDE;

    /*
     * Application Properties
     */
    TCHAR       m_szMsi                 [MAX_PATH+1];
    TCHAR       m_szProductName         [MAX_PATH+1];
    TCHAR       m_szCaptionText         [MAX_PATH+1];
    TCHAR       m_szErrorCaptionText    [MAX_PATH+1];

    /*
     * .NET Properties
     */
    TCHAR       m_szFxInstallerName     [MAX_PATH+1];
    TCHAR       m_szFxInstallerPath     [MAX_PATH+1];
    TCHAR       m_szFxInstallerParams   [MAX_PATH+1];
    TCHAR       m_szFxVersion           [15];

    /*
     * MSI Properties
     */
    TCHAR       m_szMSIInstallerName    [MAX_PATH+1];
    TCHAR       m_szMSIInstallerPath    [MAX_PATH+1];
    TCHAR       m_szMSIInstallerParams  [MAX_PATH+1];

    /*
     * IE Properties
     */
    TCHAR       m_szIEName              [MAX_PATH+1];
    TCHAR       m_szIEPath              [MAX_PATH+1];
    TCHAR       m_szIEParams            [MAX_PATH+1];
    TCHAR       m_szIEVersion           [MAX_PATH+1];

    /*
     * MDAC Properties
     */
    TCHAR       m_szMDACPath            [MAX_PATH+1];
    TCHAR       m_szMDACParams          [MAX_PATH+1];
    TCHAR       m_szMDACVersion         [15];

    /*
     * MSDE Properties
     */
    TCHAR       m_szMSDEPath            [MAX_PATH+1];
    TCHAR       m_szMSDEParams          [MAX_PATH+1];

    /*
     * Private Variables
     */
    TCHAR       m_szIniName             [MAX_PATH+1];
    TCHAR       m_szBoolAux             [15];
};

// global settings object
extern CSettings g_settings;

#endif // SETTINGS_H