/// ==========================================================================
// Name    :    CError.cpp
// Purpose :    simple wrapper class for displaying (error) msgs

// includes
#include <windows.h>
#include <stdio.h>

#include "CError.h"
#include "CSettings.h"
#include "resource.h"

// Constructors
//
// ==========================================================================
// CError::CError()
//
// Purpose:
//  parameterless CError object construction
// ==========================================================================
CError::CError()
{
    m_nRetCode    = ERROR_SUCCESS;
    m_nMessage    = 0;
    m_nCaption    = 0;
    m_nIconType   = 0;
    m_pszArg      = NULL;
}

// ==========================================================================
// Initialize a new instance of the CError class.
// ==========================================================================
// Inputs:
//  UINT nMsg: resourceId for the message to display
//  UINT nCap: resourceId for the caption to display
//  UINT nIcon: icon to use
//  int nRetCode: return code to be returned to the caller of the wrapper
// ==========================================================================
CError::CError( UINT nMsg, UINT nCap, UINT nIcon, int nRetCode, LPCTSTR pszArg)
{
    m_nRetCode    = nRetCode;
    m_nMessage    = nMsg;
    m_nCaption    = nCap;
    m_nIconType   = nIcon;
    m_pszArg      = pszArg;
}

// Operations
// ==========================================================================
// CError::ShowMessage()
//
// Purpose:
//  displays messagebox w/resource-based content
// ==========================================================================
int CError::ShowMessage()
{
    TCHAR szCapFmt[MAX_PATH]  = EMPTY_BUFFER;
    TCHAR szMsgFmt[MAX_MSG]   = EMPTY_BUFFER;
    TCHAR szCaption[MAX_PATH] = EMPTY_BUFFER;
    TCHAR szMessage[MAX_MSG]  = EMPTY_BUFFER;

    int nResponse = IDCANCEL;

    // if a caption resource id has been specified, load it
    if (m_nCaption > 0)
    {
        ::LoadString( g_settings.GetHInstance(),
                      m_nCaption,
                      szCapFmt,
                      LENGTH(szCapFmt) );

        sprintf_s(szCaption, MAX_PATH, szCapFmt, g_settings.GetProductName() );
    }
    // otherwise use the global caption string
    else
    {
        sprintf_s(szCaption, MAX_PATH, g_settings.GetErrorCaptionText(), g_settings.GetProductName() );
    }

    ::LoadString( g_settings.GetHInstance(),
                  m_nMessage,
                  szMsgFmt,
                  LENGTH(szMsgFmt) );

    if ( NULL != m_pszArg )
        sprintf_s( szMessage, MAX_PATH, szMsgFmt, m_pszArg);
    else
        sprintf_s( szMessage, MAX_PATH, szMsgFmt, g_settings.GetProductName() );

    if ( !g_settings.GetQuietMode() )
    {
        nResponse = ::MessageBox( NULL,
                                  (LPCTSTR)szMessage,
                                  szCaption,
                                  MB_OK | m_nIconType ) ;
    }
    // else print message to stderr
    else
    {
        _ftprintf ( stderr, (LPCTSTR)szMessage ) ;
    }

    // for quiet mode, we assume 'yes' as response
    if (g_settings.GetQuietMode())
        nResponse = IDOK;

    return nResponse;
}

// Operations
// ==========================================================================
// CError::ShowMessage()
//
// Inputs: none
// Purpose:
//  displays messagebox w/argument-supplied content
// ==========================================================================
int CError::ShowMessage(LPCTSTR szMsg,
                        LPCTSTR szCaption,
                        UINT nIcon,
                        LPCTSTR szArg )
{
    TCHAR szMessage[MAX_MSG]  = EMPTY_BUFFER;

    int nResponse = IDCANCEL;

    sprintf_s( szMessage, MAX_PATH, szMsg, szArg != NULL ? szArg : _T(""));

    if ( !g_settings.GetQuietMode() )
    {
        nResponse = ::MessageBox( NULL, (LPCTSTR)szMessage, szCaption, nIcon ) ;
    }
    // else print message to stderr
    else
    {
        _ftprintf ( stderr, (LPCTSTR)szMessage ) ;
    }

    // for quiet mode, we assume 'yes' as response
    if (g_settings.GetQuietMode())
    {
        nResponse = IDOK;
    }

    return nResponse;
}

// ==========================================================================
// Displays messagebox w/content created via FormatMessage
// ==========================================================================
int CError::ShowFormattedMessage(LPCTSTR szMsgFmt,
                                 LPCTSTR szCaption,
                                 UINT nIcon,
                                 LPCTSTR szArg )
{
    LPVOID pMessage = NULL;

    int nResponse = IDCANCEL;

    LPCTSTR args[1] = {szArg};

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        szMsgFmt,
        0,
        0,
        (LPTSTR)&pMessage,
        0,
        (va_list*)args
    );

    if ( !g_settings.GetQuietMode() )
        nResponse = ::MessageBox( NULL, (LPCTSTR)pMessage, szCaption, nIcon ) ;
    else
        _ftprintf ( stderr, (LPCTSTR)pMessage ) ;

    if (g_settings.GetQuietMode())
        nResponse = IDOK;

    LocalFree(pMessage);

    return nResponse;
}

// ==========================================================================
// Displays messagebox w/formatted content, including HRESULT text
// ==========================================================================
int CError::ShowHResultMessage(UINT nMsg,
                               UINT nCaption,
                               UINT nIcon,
                               HRESULT hr,
                               LPCTSTR pszArg)
{
    TCHAR szCapFmt[MAX_PATH]  = EMPTY_BUFFER;
    TCHAR szMsgFmt[MAX_MSG]   = EMPTY_BUFFER;
    TCHAR szCaption[MAX_PATH] = EMPTY_BUFFER;
    TCHAR szMessage[MAX_MSG]  = EMPTY_BUFFER;
    TCHAR szHResult[1024]     = EMPTY_BUFFER;

    LPVOID pMessage = NULL;

    UINT nResponse = IDCANCEL;

    // if a caption resource id has been specified, load it
    if (nCaption > 0)
    {
        ::LoadString( g_settings.GetHInstance(),
                      nCaption,
                      szCapFmt,
                      LENGTH(szCapFmt) );

        sprintf_s( szCaption, MAX_PATH, szCapFmt, g_settings.GetProductName() );
    }
    // otherwise use the global caption string
    else
    {
        sprintf_s( szCaption, MAX_PATH, g_settings.GetErrorCaptionText(), g_settings.GetProductName() );
    }

    ::LoadString( g_settings.GetHInstance(),
                  nMsg,
                  szMsgFmt,
                  LENGTH(szMsgFmt));

   FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      hr,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      szHResult,
      LENGTH(szHResult),
      NULL
      );

    if (NULL == pszArg)
    {
        LPCTSTR args[1] = {szHResult};

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            szMsgFmt,
            0,
            0,
            (LPTSTR)&pMessage,
            0,
            (va_list*)args
        );
    }
    else
    {
        LPCTSTR args[2] = {pszArg, szHResult};

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            szMsgFmt,
            0,
            0,
            (LPTSTR)&pMessage,
            0,
            (va_list*)args
        );
    }

    if ( !g_settings.GetQuietMode() )
    {
        nResponse = MessageBox( NULL, (
                                LPCTSTR)pMessage,
                                szCaption,
                                nIcon ) ;
    }
    // else print message to stderr
    else
        _ftprintf ( stderr, (LPCTSTR)szMessage ) ;

    // for quiet mode, we assume 'yes' as response
    if (g_settings.GetQuietMode())
        nResponse = IDOK;

    LocalFree(pMessage);

    return nResponse;
}
