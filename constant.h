//  Constant.h
//  Constant support for Win32 Perl extensions
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  2001.02.08

#ifndef _CONSTANT_H_
#define _CONSTANT_H_

    enum eConstantType { NotDefined = 0, NotPresent, String, Numeric };

    eConstantType Constant( char *pszConstant, LPVOID *ppBuffer );
    char *GetConstantName( DWORD dwIndex );
    DWORD GetTotalConstants();
    void CountConstants();

    typedef struct tagConstStruct
    {
        char *m_Name;
        LPVOID m_pBuffer;
        eConstantType m_eType;
    } ConstantStruct;

#endif // _CONSTANT_H_