//
//   Copyright (c) Microsoft Corporation.  All rights reserved.
//
/// ==========================================================================
// Name    :    CError.h
// Purpose :    simple wrapper class for displaying error msgs


#ifndef ERROR_H
#define ERROR_H

#include <windows.h>
#include <tchar.h>

//defines
//
#define EMPTY_BUFFER { _T( '\0' ) }
#define END_OF_STRING  _T( '\0' )
#define MAX_MSG    4096
#define LENGTH(A) (sizeof(A)/sizeof(A[0]))

// ==========================================================================
// class CError
//
// Purpose:
//  This class displays (error) messages and holds a return-code
// ==========================================================================
class CError
{
public:
    // Constructors
    CError();

    CError( UINT nMsg,
            UINT nCap = 0,
            UINT nIcon = MB_OK,
            int nRetCode = ERROR_SUCCESS,
            LPCTSTR pszArg = NULL);

    // Operations
    int ShowMessage();

    int ShowMessage(LPCTSTR pszMsg,
                    LPCTSTR pszCaption,
                    UINT nIcon,
                    LPCTSTR pszArg );

    int ShowHResultMessage(UINT nMsg,
                           UINT nCaption,
                           UINT nIcon,
                           HRESULT hr,
                           LPCTSTR pszArg);

    int ShowFormattedMessage(LPCTSTR pszMsg,
                             LPCTSTR pszCaption,
                             UINT nIcon,
                             LPCTSTR pszArg );

    // Data
    int m_nRetCode;

private:
    static TCHAR s_szProductName[MAX_PATH+1];

    LPCTSTR m_pszArg;
    UINT    m_nCaption;
    UINT    m_nMessage;
    UINT    m_nIconType;
    bool    m_bUseSprintf;
    va_list *m_pArgs;
};

#endif
