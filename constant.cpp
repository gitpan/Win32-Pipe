#define WIN32_LEAN_AND_MEAN

#ifdef __BORLANDC__
typedef wchar_t wctype_t; /* in tchar.h, but unavailable unless _UNICODE */
#endif

#include <windows.h>
#include <tchar.h>
#include <wtypes.h>
#include <stdio.h>		//	Gurusamy's right, Borland is brain damaged!
#include <math.h>		//	Gurusamy's right, MS is brain damaged!

#include "constant.h"
#include "CPipe.hpp"

static DWORD gdwConstTotal = 0;
static ConstantStruct gsConst[] =
{
//  Beginning Of Constants
    {   "DEFAULT_WAIT_TIME",        (LPVOID) ( DEFAULT_WAIT_TIME ),             Numeric },  
    {   "ERROR_MORE_DATA",          (LPVOID) ( ERROR_MORE_DATA ),               Numeric },  
    {   "NMPWAIT_NOWAIT",           (LPVOID) ( NMPWAIT_NOWAIT ),                Numeric },  
    {   "NMPWAIT_WAIT_FOREVER",     (LPVOID) ( NMPWAIT_WAIT_FOREVER ),          Numeric },  
    {   "NMPWAIT_USE_DEFAULT_WAIT", (LPVOID) ( NMPWAIT_USE_DEFAULT_WAIT ),      Numeric },  
    {   "PIPE_TYPE_BYTE",           (LPVOID) ( PIPE_TYPE_BYTE ),                Numeric },  
    {   "PIPE_TYPE_MESSAGE",        (LPVOID) ( PIPE_TYPE_MESSAGE ),             Numeric },  
    {   "PIPE_READMODE_BYTE",       (LPVOID) ( PIPE_READMODE_BYTE ),            Numeric },  
    {   "PIPE_READMODE_MESSAGE",    (LPVOID) ( PIPE_READMODE_MESSAGE ),         Numeric },  
    {   "STD_INPUT_HANDLE",         (LPVOID) ( STD_INPUT_HANDLE ),              Numeric },  
    {   "STD_ERROR_HANDLE",         (LPVOID) ( STD_ERROR_HANDLE ),              Numeric },  
    {   "STD_OUTPUT_HANDLE",        (LPVOID) ( STD_OUTPUT_HANDLE ),             Numeric },  
    {   "WRITE_DAC",                (LPVOID) ( WRITE_DAC ),                     Numeric },  
    {   "WRITE_OWNER",              (LPVOID) ( WRITE_OWNER ),                   Numeric },  
    {   "ACCESS_SYSTEM_SECURITY",   (LPVOID) ( ACCESS_SYSTEM_SECURITY ),        Numeric },  
    
    //  Connect() error types if in nonblocking mode
    {   "ERROR_PIPE_CONNECTED",     (LPVOID) ( ERROR_PIPE_CONNECTED ),          Numeric },
    {   "ERROR_NO_DATA",            (LPVOID) ( ERROR_NO_DATA ),                 Numeric },
    {   "ERROR_PIPE_LISTENING",     (LPVOID) ( ERROR_PIPE_LISTENING ),          Numeric },
    
    //  Blocking/nonblocking types
    {   "PIPE_NOWAIT",              (LPVOID) ( PIPE_NOWAIT ),                   Numeric },
    {   "PIPE_WAIT",                (LPVOID) ( PIPE_WAIT),                      Numeric },


    {   NULL,                        (LPVOID) ( 0 ),                           Numeric },
//  End Of Constants    
};

eConstantType Constant( LPTSTR pszConstant, LPVOID *ppBuffer )
{
    eConstantType eResult = NotDefined;
    DWORD dwIndex = 0;

    while( NULL != gsConst[ dwIndex ].m_Name )
    {
        if( NULL == gsConst[ dwIndex ].m_Name )
        {
            break;
        }

        if( *pszConstant == *gsConst[ dwIndex ].m_Name )
        {
            int iResult = _tcsicmp( gsConst[ dwIndex ].m_Name, pszConstant );
            if( 0 == iResult )
            {
                *ppBuffer = gsConst[ dwIndex ].m_pBuffer;
                eResult = gsConst[ dwIndex ].m_eType;
                break;
            }
            /*
                //  This code segment is commented out so that we don't run into 
                //  the problem of a constant being out of alpha order hence not
                //  resolving.
            else if( 0 < iResult )
            {
                    //  We have passed the spot where this constant
                    //  *should* have been if it were in alpha order
                break;
            }
            */
        }
        dwIndex++;
    }
    return( eResult );
}

LPTSTR GetConstantName( DWORD dwIndex )
{
    LPTSTR pszBuffer = NULL;
    if( gdwConstTotal > dwIndex )
    {
        pszBuffer = gsConst[ dwIndex ].m_Name;
    }
    return( pszBuffer );
}

DWORD GetTotalConstants()
{
    return( gdwConstTotal );
}

void CountConstants()
{
   gdwConstTotal = 0;
   while( NULL != gsConst[ gdwConstTotal++ ].m_Name ){};  
}
