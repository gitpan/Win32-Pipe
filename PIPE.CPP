
#define WIN32_LEAN_AND_MEAN
#define HINSTANCE   HINSTANCE       //  Pathetic but necessary for ActiveState build 307

#include <c:/dev/vc98/include/io.h> //  For the _open_osfhandle() function
//#include <C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE/io.h> //  For the _open_osfhandle() function
#include <FCNTL.H>

#ifdef __BORLANDC__
typedef wchar_t wctype_t; /* in tchar.h, but unavailable unless _UNICODE */
#endif

#include <windows.h>
#include <stdio.h>		//	Gurusamy's right, Borland is brain damaged!
#include <math.h>		//	Gurusamy's right, MS is brain damaged!


	//	Win32 Perl Stuff
	//	used for the core distribution
#include <preWin32Perl.h>
#include <Win32Perl.h>

/*
#if defined( __cplusplus )  && !defined( PERL_OBJECT )
extern "C" {
#endif	//	C++ and not PERL_OBJECT
#include "EXTERN.h"
#include "perl.h"
#include "XSub.h"
#if defined( __cplusplus )  && !defined( PERL_OBJECT )
}	
#endif	//	C++ and not PERL_OBJECT
*/

#include "constant.h"
#include "cpipe.hpp"
#include "pipe.h"

#ifdef _open_osfhandle
#undef _open_osfhandle
#endif

#ifdef _get_osfhandle
#undef _get_osfhandle
#endif


char gszError[ERROR_TEXT_SIZE];
int	giError = 0;

///////////////////////////////////////////////////
//  PIPE.CPP specific prototypes

HV	*GetSelf( PERL_OBJECT_PROTO SV *svSelf );
CPipe *GetObjectPointer( PERL_OBJECT_PROTO HV *pHv );
BOOL DelObjectPointer( PERL_OBJECT_PROTO HV *pHv );
SV *GetObjectPointerSV( PERL_OBJECT_PROTO HV *pHv );

/*----------------------- M I S C   F U N C T I O N S -------------------*/
int	Error( int iErrorNum, char *szErrorText )
{
	strncpy( ( char * ) gszError, szErrorText, ERROR_TEXT_SIZE );
	gszError[ ERROR_TEXT_SIZE ] = '\0';
	giError = iErrorNum;
	return( giError );
}		

//	Correct slashes to be backslashes.
void FixPath( char *pszPath )
{
	while( 0 != *pszPath )
	{
		if( '/' == *pszPath )
		{
			*pszPath = '\\';
		}
		pszPath++;
	}
	return;
}

HV *GetSelf( PERL_OBJECT_PROTO SV *svSelf )
{
	HV	*hvSelf = NULL;

	if( ( NULL != svSelf ) && ( SvROK( svSelf ) ) )
	{
		svSelf = SvRV( svSelf );
		if( SVt_PVHV == SvTYPE( svSelf ) )
		{
			hvSelf = (HV*) svSelf;
		}
	}

	return( hvSelf );
}

CPipe *GetObjectPointer( PERL_OBJECT_PROTO HV *pHv )
{
	CPipe	*pPointer = NULL;
    SV		*pSv = GetObjectPointerSV( PERL_OBJECT_ARGS pHv );

    if( NULL != pSv )
    {
        pPointer = (CPipe*) SvIV( pSv );
    }
	
	return( pPointer );
}

BOOL DelObjectPointer( PERL_OBJECT_PROTO HV *pHv )
{
    BOOL bResult = FALSE;
    SV   *pSv = GetObjectPointerSV( PERL_OBJECT_ARGS pHv );

    if( NULL != pSv )
    {
        sv_setiv( pSv, (IV) NULL );
        bResult = TRUE;
    }

    return( bResult );
}

SV *GetObjectPointerSV( PERL_OBJECT_PROTO HV *pHv )
{
	SV		*pSv = NULL;
	CPipe	*pPointer = NULL;

	if( ( NULL != pHv ) && ( SVt_PVHV == SvTYPE( (SV*) pHv ) ) )
	{
		if( hv_exists( ( HV* ) pHv, CLASS_POINTER_MEMBER_NAME, strlen( CLASS_POINTER_MEMBER_NAME ) ) )
		{
            //  Remember hv_fetch() returns a SV**
			pSv = *( hv_fetch( ( HV* ) pHv, CLASS_POINTER_MEMBER_NAME, strlen( CLASS_POINTER_MEMBER_NAME ), 0 ) );
		}
	}

	return( pSv );
}

/*------------------- P E R L   P I P E   F U N C T I O N S ---------------*/

XS( XS_WIN32__Pipe_Constant )
{
	dXSARGS;
    EXTENSION_VARS;
	char *pszName = NULL;
    LPVOID pBuffer = NULL;
    DWORD dwValue;
    eConstantType eResult;

	if( 2 != items )
	{
		croak( "Usage: " EXTENSION "::Constant( $Name, $Arg )\n" );
    }
	
	pszName = (char*) SvPV( ST( 0 ), na );

	eResult = Constant( pszName, &pBuffer );
    switch( eResult )
    {
        case String:
            sv_setpv( ST( 1 ), (char*) pBuffer );
            break;

        case Numeric:
            sv_setiv( ST( 1 ), (IV) pBuffer );
            break;
    }

        //  Return the result type.
    PUSH_IV( eResult );

	EXTENSION_RETURN;
}


XS( XS_WIN32__Pipe_Create )
{
	dXSARGS;
    EXTENSION_VARS;
	
	UCHAR	*szName = 0;
	DWORD	dwWait = DEFAULT_WAIT_TIME;
	CPipe	*pPipe = 0;
	DWORD	dwType = PIPE_TYPE_BYTE;
    DWORD   dwError = 0;
    SECURITY_DESCRIPTOR *pSD = NULL;
	

	if( 1 > items  || 5 < items )
    {
		CROAK( "usage: " EXTENSION "::Create( $Name [, $TimeToWait [, $Type [, $SecurityDescriptor ] ] ] );\n" );
	}

	szName = ( UCHAR * ) SvPV( ST( 0 ), na );
	FixPath( ( char * ) szName );
	
	if( 1 < items )
	{
		dwWait = ( DWORD ) SvIV( ST( 1 ) );
	}

	if( 2 < items )
	{
		dwType = ( DWORD ) SvIV( ST( 2 ) );
	}

    if( 3 < items )
    {
        SV *pSv = ST( 3 );

        if( sv_isobject( (SV*) pSv ) )
        {
            // Is pSD an object (blessed object)?
            LPTSTR pszObjectType = NULL;
            SV *pSvTemp = NULL;

            //  Yep, it's a reference to a blessed object...
            //  This means that pSv is actually a blessed HV*
            pSvTemp = SvRV( (SV*) pSv );
            pszObjectType = HvNAME( SvSTASH( pSvTemp ) );

            if( 0 == stricmp( pszObjectType, PERL_WIN32_PERMS_EXTENSION  ) )
            {
                dSP;
                int iCount;

                //  We have a Win32::Perms object. So let's call into its GetSD() method
                //  to get a pointer to the absolute Security Descriptor...
            
                // Save our current position on the stack
                PUSHMARK( SP );
            
                // Push the Win32::Perms object onto the stack
                XPUSHs( (SV*) pSv );

                //  Go back to the previously stored stack position
                PUTBACK;
             
                //  Remember the position on the stack so when we free temp vars only up to this point
                //  will be freed
                ENTER;
                SAVETMPS;
            
                //  Call the method...
                iCount = perl_call_method( (char*) "GetSD", G_SCALAR );

                if( 0 < iCount )
                {
                    //  Yahooo! The method returned a value; assume it is a Security Descriptor pointer
                    //  so pop it off as a long
                    pSD = (SECURITY_DESCRIPTOR*) POPl;
                }

                //  Free all scalars created since the ENTER/SAVETMPS combo
                FREETMPS;
                LEAVE;
            }
        }
        else
        {
            // Okay, first check if we have an absolute security descriptor...
            pSD = (SECURITY_DESCRIPTOR*) SvIV( pSv );
            if( ! IsValidSecurityDescriptor( pSD ) )
            {
                // Hmmm. Okay, let's try a relative security descriptor...
                pSD = (SECURITY_DESCRIPTOR*) SvPV( pSv, na );
                if( ! IsValidSecurityDescriptor( pSD ) )
                {
                    // Sigh. No, we don't seem to have any security descriptor
                    pSD = NULL;
                }
            }
        }
    }

//	PUSHMARK( sp );

	if( 255 < strlen( ( const char * ) szName ) )
	{
		Error( ERROR_NAME_TOO_LONG );
	}
    else
    {
		pPipe = CPipe::Create( ( char * ) szName, dwWait, dwType, pSD, &dwError );
	}
	
    if( NULL != pPipe )
	{
        PUSH_IV( pPipe );
        PUSH_IV( pPipe->GetHandle() );
	}
    else
    {
		PUSH_IV( 0 );
        PUSH_IV( dwError );
	}

    EXTENSION_RETURN;
} 

XS( XS_WIN32__Pipe_Close )
{
	dXSARGS;
	CPipe	*pPipe = NULL;
    BOOL bNoDisconnect = FALSE;

	if( ( 1 > items ) || ( 2 < items ) )
	{
		CROAK( "usage: " EXTENSION "::Close( [$NoDisconnect] );\n" );
	}

	pPipe = GET_PIPE( );

	if( 2 == items )
	{
		bNoDisconnect = ( BOOL ) SvIV( ST( 1 ) );
	}
	
	if( NULL != pPipe )
    {
/*
		if( FALSE != bNoDisconnect )
		{
			pPipe->Close( TRUE );
		}
*/
		delete pPipe;
        pPipe = NULL;
        DEL_PIPE();
	}

	XSRETURN_NO;
} 

XS( XS_WIN32__Pipe_Write )
{
	dXSARGS;
    EXTENSION_VARS;
	CPipe	*pPipe = 0;
	void	*pData = 0;
	int		iResult = 0;
	STRLEN	dwDataLen;

	if( 2 != items )
	{
		CROAK( "usage: " EXTENSION "::Write( $Data );\n" );
	}
	
	pPipe = GET_PIPE( );
	
	pData = ( void * ) SvPV( ST( 1 ), na );
	dwDataLen = SvCUR( ST( 1 ) );

	if( NULL != pPipe )
	{
		iResult = pPipe->Write( ( void * ) pData, ( DWORD )dwDataLen );
	}

    PUSH_IV( iResult );

    EXTENSION_RETURN;
} 

XS( XS_WIN32__Pipe_Read )
{
	dXSARGS;
    EXTENSION_VARS;
	CPipe	*pPipe = 0;
	BOOL    bFlag = TRUE;
	DWORD	dwLen = 0;
	PVOID   pBuffer = NULL;
    SV      *pSv = &sv_undef;

	if( 1 != items )
	{
		CROAK( "usage: " EXTENSION "::Read( );\n" );
	}
	 
	pPipe = GET_PIPE( );
	
	if( NULL != pPipe )
	{ 
        pSv = newSVpv( "", 0 );

		while( TRUE == bFlag )
		{
			pBuffer = pPipe->Read( &dwLen );
			bFlag = FALSE;
				//	If we have more data to read then for God's sake, do it!

				//	I don't know if this will work ... it would return an
				//	array. This may not be good. Hmmmm.
			if( ( NULL != pBuffer ) && ( ERROR_MORE_DATA == GetLastError( ) ) )
            {
				bFlag = TRUE;
			}				

			if( 0 != dwLen )
            {
                //  Append any received data to the end of the SV that we will return to Perl
                sv_catpvn( pSv, (LPSTR) pBuffer, dwLen );
			}
		}
	}

    PUSH_NOREF( pSv );
    
    EXTENSION_RETURN;
} 

XS( XS_WIN32__Pipe_Connect )
{
	dXSARGS;
    EXTENSION_VARS;
	CPipe	*pPipe;
	int		iResult = 0;

	if( items != 1 )
    {
		CROAK( "usage: " EXTENSION "::Connect( );\n" );
	}
	
	pPipe = GET_PIPE( );
	
	if( NULL != pPipe )
	{
		iResult = pPipe->Connect( );
	}

    PUSH_IV( iResult );

    EXTENSION_RETURN;
} 

XS( XS_WIN32__Pipe_Disconnect )
{
	dXSARGS;
    EXTENSION_VARS;
	CPipe	*pPipe;
	int		iResult = 0;
	int		iPurge = 0;

	if( ( 1 > items ) || ( 2 < items ) )
	{
		CROAK( "usage: " EXTENSION "::Disconnect( [$iPurge] );\n" );
	}

	pPipe = GET_PIPE( );

	if( 2 == items )
	{
		iPurge = ( int ) SvIV( ST( 1 ) );
	}
	
	if( NULL != pPipe )
	{
		iResult = pPipe->Disconnect( iPurge );
	}

    PUSH_IV( iResult );

    EXTENSION_RETURN;
} 

XS( XS_WIN32__Pipe_ResizeBuffer )
{
	dXSARGS;
	CPipe	*pPipe = 0;
	DWORD	dwResult = 0;
	DWORD	dwSize;
	
	if( 2 != items )
	{
		CROAK( "usage: " EXTENSION "::ResizeBuffer( $Size );\n" );
	}
	
	pPipe = GET_PIPE( );

	dwSize = ( DWORD ) SvIV( ST( 1 ) );

	if( NULL != pPipe )
	{
		dwResult = pPipe->ResizeBuffer( dwSize );
	}


    XSRETURN_IV( (IV) dwResult );
} 

XS( XS_WIN32__Pipe_BufferSize )
{
	dXSARGS;
	CPipe	*pPipe = NULL;
	DWORD	dwResult = 0;
	
	if( 1 != items )
	{
		CROAK( "usage: " EXTENSION "::BufferSize( );\n" );
	}
	
	pPipe = GET_PIPE( );

	if( NULL != pPipe )
	{
		dwResult = pPipe->BufferSize( );
	}

    XSRETURN_IV( (IV) dwResult );
} 


XS( XS_WIN32__Pipe_Error )
{
	dXSARGS;
	CPipe	*pPipe = NULL;
	int		iResult = 0;
	char	*szError = 0;
	int		iError = 0;
	
	if( items > 1 )
    {
		CROAK( "usage: " EXTENSION "::Error( [$PipeHandle] );\n" );
	}
	if( 1 == items )
    {
		pPipe = GET_PIPE( );
	}

	PUSHMARK( sp );
	
	if( NULL != pPipe )
	{
		iError = pPipe->GetError( );
		szError = ( char * ) pPipe->GetErrorText( );
	}
    else
    {
		iError = giError;
		szError = gszError;
	}

	XPUSHs( sv_2mortal( newSViv( ( long ) iError ) ) );
	XPUSHs( sv_2mortal( newSVpv( ( char * ) szError, strlen( szError ) ) ) ); 

	PUTBACK;
} 


XS( XS_WIN32__Pipe_Anonymous )
{
	dXSARGS;
    EXTENSION_VARS;
	int		iResult = 0;
	HANDLE	hRead, hWrite;
	int		fhRead, fhWrite;
	SECURITY_ATTRIBUTES	saTemp;
		
	if( 0 != items )
	{
		CROAK( "usage: ( $Error, $Read, $Write ) = " EXTENSION "::Anonymous( );\n" );
	}

	fhRead = fhWrite = 0;
	hRead = hWrite = 0;
	saTemp.nLength = sizeof( SECURITY_ATTRIBUTES );
	saTemp.bInheritHandle = 1;
	saTemp.lpSecurityDescriptor = 0;

	if( CreatePipe( &hRead, &hWrite, &saTemp, 512 ) )
	{
		if( 0 <= ( fhRead = _open_osfhandle( ( long ) hRead, _O_TEXT | _O_RDONLY ) ) )
		{
			if( 0 <=( fhWrite = _open_osfhandle( ( long ) hWrite, _O_TEXT | _O_WRONLY ) ) )
			{
				iResult = 1;
			}
		}
	}
	
	if( 0 != iResult )
	{
        PUSH_IV( fhRead );
        PUSH_IV( fhWrite );
        PUSH_IV( hRead );
        PUSH_IV( hWrite );
	}
	
	EXTENSION_RETURN;
} 

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_ReOpen )
{
	dXSARGS;
    EXTENSION_VARS;
	int		iResult = 0;
	HANDLE	hRead, hWrite;
	int		fhRead, fhWrite;
		
	if( 2 != items )
	{
		CROAK( "usage: ( $Read, $Write ) = " EXTENSION "::ReOpen( $ReadOSHandle, $WriteOSHandle );\n" );
	}

	fhRead = fhWrite = 0;
	hRead = hWrite = 0;

	hRead  = ( HANDLE ) SvIV( ST( 0 ) );
	hWrite = ( HANDLE ) SvIV( ST( 1 ) );

	if( 0 != hRead )
	{
		if( 0 <= ( fhRead = _open_osfhandle( ( long ) hRead, _O_TEXT | _O_RDONLY ) ) )
		{
			iResult = 1;
		}
	}
	
	if( 0 != hWrite )
	{
		if( 0 <= ( fhWrite = _open_osfhandle( ( long ) hWrite, _O_TEXT | _O_WRONLY ) ) )
		{
			iResult = 1;
		}
	}
	
	if( 0 != iResult )
	{
        PUSH_IV( fhRead );
        PUSH_IV( fhWrite );
	}
	
	EXTENSION_RETURN;
} 

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_ClosePipe )
{
	dXSARGS;
    EXTENSION_VARS;
	HANDLE	hPipe;
	int		fhPipe;
	int		iResult = 0;
		
	if( 1 != items )
	{
		CROAK( "usage: ( $Error ) = " EXTENSION "::ClosePipe( $PipeFD );\n" );
	}	
	
	fhPipe = SvIV( ST( 0 ) );
	hPipe = 0;

	if( 0 <= ( hPipe = ( HANDLE ) _get_osfhandle( fhPipe ) ) )
	{
		iResult = CloseHandle( hPipe );
	}
	
    PUSH_IV( iResult );
	
	EXTENSION_RETURN;
} 

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_StdHandle )
{
	dXSARGS;
    EXTENSION_VARS;
	HANDLE	hFile = 0;
	DWORD	dwType;
	int		fhFile = -1;
	BOOL	iResult = FALSE;
		
	if( 1 > items || 2 < items )
	{
		CROAK( "usage: ( $Error, $Read, $Write ) = " EXTENSION "::StdHandle( $Type [, $OSHandle ] );\n" );
	}	
	
	dwType = ( DWORD ) SvIV( ST( 0 ) );	

	if( 2 == items )
	{
		hFile = ( HANDLE ) SvIV( ST( 0 ) );
	}

	if( 0 < hFile )
	{	
		iResult = SetStdHandle( dwType, hFile );
	}else{
		hFile = GetStdHandle( dwType );
		if( INVALID_HANDLE_VALUE != hFile )
		{
			iResult = TRUE;
		}
	}
		
    PUSH_IV( iResult );
	if( 1 == items )
	{
        PUSH_IV( hFile );
	}			
	
	EXTENSION_RETURN;
} 

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_GetHandle )
{
	dXSARGS;
	CPipe *pPipe = NULL;
	HANDLE	hFile;
	int		fhFile = -1;
		
	if( 1 != items )
	{
		CROAK( "usage: ( $OSHandle ) = " EXTENSION "::GetHandle( $FileDescriptor );\n" );
	}	
	
	pPipe = GET_PIPE( );

	if( NULL == pPipe )
	{
		fhFile = SvIV( ST( 0 ) );	
	}

	PUSHMARK( sp );

	if( NULL != pPipe )
	{
		hFile = ( void * ) pPipe->GetHandle( );
	}
	else
	{
		hFile =( void * ) _get_osfhandle( fhFile );
	}
	
	XPUSHs( sv_2mortal( newSViv( ( long ) hFile ) ) );
	
	PUTBACK;
} 

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_State )
{
	dXSARGS;
	CPipe	*pPipe = NULL;
	DWORD	dwState = 0;
		
	if( 1 > items || 2 < items ){
		CROAK( "usage: $State = " EXTENSION "::SetState( $State );\n" );
	}	
		
	PUSHMARK( sp );

	pPipe = GET_PIPE( );
	
	if( NULL != pPipe )
	{
		if( 2 == items )
		{
			dwState = pPipe->State( ( DWORD ) SvIV( ST( 1 ) ) );
		}
		else
		{
			dwState = pPipe->State( );
		}
	}

	XPUSHs( sv_2mortal( newSViv( ( long ) dwState ) ) );
	
	PUTBACK;
}

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_CloseHandle )
{
	dXSARGS;
	HANDLE	hFile;
	int		iResult = 0;
		
	if( 1 != items )
	{
		CROAK( "usage: = " EXTENSION "::CloseHandle( $OSHandle );\n" );
	}	
		
	PUSHMARK( sp );
	
	hFile = ( void * ) SvIV( ST( 0 ) );	

	iResult = CloseHandle( hFile );
			
	XPUSHs( sv_2mortal( newSViv( ( long ) iResult ) ) );
	
	PUTBACK;
} 

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_PeekPipe )
{
	dXSARGS;
    EXTENSION_VARS;
	HANDLE	hPipe;
	DWORD	dwSize = 0;
	DWORD	dwAvail = 0;
	LPTSTR	pszBuffer = NULL;
	BOOL	bResult = TRUE;
		
	if( 2 != items )
	{
		CROAK( "usage: ( $Data, $Size, $Available ) = " EXTENSION "::PeekPipe( $PipeHandle, $Size );\n" );
	}	
		
	hPipe = ( HANDLE ) SvIV( ST( 0 ) );	
	
	if( FALSE != bResult )
	{
		dwSize = SvIV( ST( 1 ) );
		if( 0 != dwSize )
		{
			if( NULL != ( pszBuffer = new TCHAR [ dwSize + 1 ] ) )
			{
				bResult = TRUE;
			}
		}
		if( FALSE != bResult )
		{
			bResult = PeekNamedPipe( hPipe, pszBuffer, dwSize, &dwSize, &dwAvail, 0 );
	    }
	}		
	if( FALSE != bResult )
	{
        PUSH_PNV( pszBuffer, dwSize );
        PUSH_IV( dwSize );
        PUSH_IV( dwAvail );
	}

	delete [] pszBuffer;
	
    EXTENSION_RETURN;
}

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_Peek )
{
	dXSARGS;
    EXTENSION_VARS;
	DWORD	dwLen = 0;
	CPipe	*pPipe = NULL;
	PVOID   pBuffer = NULL;
	
	if( 1 != items )
	{
		CROAK( "usage: ( $Data ) = " EXTENSION "::Peek( );\n" );
	}	
		
	pPipe = GET_PIPE( );

	if( NULL != pPipe )
	{
		pBuffer = pPipe->Peek( &dwLen );
			    
	}	

	if( NULL == pBuffer )
	{
		pBuffer = TEXT( "" );
		dwLen = 0;
	}
	
    PUSH_PNV( (LPCTSTR) pBuffer, dwLen );

    EXTENSION_RETURN;
} 

////////////////////////////////////////////////////////////////////////
XS( XS_WIN32__Pipe_GetPipeInfo )
{
	dXSARGS;
    EXTENSION_VARS;
	CPipe	*pCPipe;
	int		iResult = TRUE;
	sPipeInfo	*psInfo = NULL;
	
	if( items != 1 ){
		CROAK( "usage: ( $State, $Instances, $User, $MaxCollectionCount, $CollectTimeout ) = " EXTENSION "::GetPipeInfo( $Handle );\n" );
	}	
		
	pCPipe = GET_PIPE( );

	if( NULL != pCPipe )
	{
		psInfo = pCPipe->GetInfo( );
		if( NULL != psInfo )
		{
            PUSH_IV( psInfo->dwState );
            PUSH_IV( psInfo->dwInstances );
            PUSH_PV( psInfo->szUserName );
            PUSH_IV( psInfo->dwMaxCollectionCount );
            PUSH_IV( psInfo->dwCollectDataTimeout );
		}	
	}

    EXTENSION_RETURN;
} 

		//	Test string; p Win32::Pipe::CallNamedPipe( "//dev/pipe/$Pipe", "run:dir", $Buff );
XS( XS_WIN32__Pipe_CallNamedPipe )
{
	dXSARGS;
	char	*pszPipeName = NULL;
	char	*pSendData = NULL;
	char	pReceiveData[ MAX_BUFFER_SIZE ];
	DWORD	dwSendBufferSize = 0;
	DWORD	dwReceiveBufferSize = sizeof( pReceiveData );
	DWORD	dwcbReceived = 0;
	DWORD	dwTimeOut = NMPWAIT_USE_DEFAULT_WAIT;
	SV		*svReceiveBuffer = NULL;
	BOOL	bResult = FALSE;

		
	if( items < 3 || items > 5 ){
		CROAK( "usage: $bResult = " EXTENSION "::CallNamedPipe( $PipeName, $SendData, $RecieveData [, $TimeOut ] );\n" );
	}	
	
	//	Get the name of the named pipe	
	pszPipeName = ( char * ) SvPV( ST( 0 ), na );
	FixPath( pszPipeName );

	//	Get the Send Buffer and discover it's size
	pSendData = ( char * ) SvPV( ST( 1 ), na );
	dwSendBufferSize = SvCUR( ST( 1 ) ) + 1;

	svReceiveBuffer = ( SV * ) ST( 2 );

	if( items > 3 )
	{
		dwTimeOut = ( DWORD ) SvIV( ST( 3 ) );
	}

    //	Clean out the recieve buffer
	ZeroMemory( pReceiveData, sizeof( pReceiveData ) );

	//	If bResult is FALSE *AND* GetLastError is ERROR_MORE_DATA then the $Receive buffer
	//	will have the partial message retrieved.
    bResult = CallNamedPipe( 
			                pszPipeName,
			                ( void * ) pSendData,
			                dwSendBufferSize,
			                ( void * ) pReceiveData,
			                dwReceiveBufferSize,
			                &dwcbReceived,
			                dwTimeOut );
	
	sv_setpvn( ST( 2 ), ( char * ) pReceiveData, dwcbReceived );


	if( FALSE != bResult )
	{
		XST_mYES( 0 );
	}
	else
	{
		XST_mNO( 0 );
	}

	XSRETURN( 1 );
}

///////////////////////////////////////////////////
//	THIS assumes that the first parameter passed in is the $self
///////////////////////////////////////////////////
XS( XS_WIN32__Pipe_Transact )
{
	dXSARGS;
	CPipe	*pPipe = NULL;
	void	*pData = NULL;
	DWORD	dwDataLen = 0;
	int		iResult = TRUE;
	sPipeInfo	*psInfo = NULL;
	BOOL	bResult = FALSE;
	
		//	There are three inputs even though we croak that there are only two.
		//	The first one is auto sent when referenced as a method.
		//	This prevents classless calling! 
	if( items != 3 ){
		CROAK( "usage: " EXTENSION "::Transact( $SendData, $ReadBuffer );\n" );
	}	
		
	pData = ( void * ) SvPV( ST( 1 ), na );
	dwDataLen = SvCUR( ST( 1 ) );

	pPipe = GET_PIPE( );

	if( NULL != pPipe )
	{
		bResult = pPipe->Transact( &pData, &dwDataLen );
	}

	if( FALSE != bResult )
	{
		XST_mYES( 0 );
	}
	else
	{
		XST_mNO( 0 );
	}

	sv_setpvn( ST( 2 ), ( char * ) ( ( NULL == pData )? "":pData ), ( NULL == pData )? 0:dwDataLen );

	XSRETURN( 1 );
} 


XS( XS_WIN32__Pipe_TransactNamedPipe )
{
	dXSARGS;
	HANDLE hHandle = 0;
	char	*pReceiveBuffer[ MAX_BUFFER_SIZE ];
	DWORD	dwRecieveBufferSize = sizeof( pReceiveBuffer );
	DWORD	dwcbReceived = 0;
	void	*pData = NULL;
	DWORD	dwDataLen = 0;
	int		iResult = TRUE;
	sPipeInfo	*psInfo = NULL;
	BOOL	bResult = FALSE;
	
	if( items != 3 ){
		CROAK( "usage: " EXTENSION "::TransactNamedPipe( $Handle, $SendBuffer, $ReceiveBuffer );\n" );
	}	
		
	pData = ( void * ) SvPV( ST( 1 ), na );
	dwDataLen = SvCUR( ST( 1 ) );

	hHandle= ( HANDLE ) SvIV( ST( 0 ) );	

	ZeroMemory( pReceiveBuffer, dwRecieveBufferSize );

	if( 0 != hHandle )
	{
		bResult = TransactNamedPipe( 
				hHandle,
				pData,
				dwDataLen,
				( void * ) pReceiveBuffer,
				dwRecieveBufferSize,
				&dwcbReceived,
				NULL );
	}

	if( FALSE != bResult )
	{
		XST_mYES( 0 );
	}
	else
	{
		XST_mNO( 0 );
	}

	sv_setpvn( ST( 2 ), ( char * ) pReceiveBuffer, dwRecieveBufferSize );

	XSRETURN( 1 );
} 



//////////////////////////////////////////////////////////////////
//	VERY TEST! Don't use yet
//////////////////////////////////////////////////////////////////
/*
XS( XS_WIN32__Pipe_OpenAsPerlFile )
{
	dXSARGS;
	int		iResult = TRUE;
	HANDLE	hFile;
	
	if( items != 2 ){
		CROAK( "usage: " EXTENSION "::OpenAsPerlFile( $FileName, $Handle );\n" );
	}	
		
	PUSHMARK( sp );
	
	hFile = SvIV( ST( 1 ) );	


	if ( NULL != ( fNewFile = _open_osfhandle ( hFile , 0 ) )
	{
		dSP; dTARGET;
		GV *gv;
		SV *svFileName;
		char *tmps;
		STRLEN len;

		svFileName = ST( 0 );

 
//		else if ( SvTYPE( TOPs ) == SVt_PVGV )
//			sv = GvSV( TOPs );
//		else
//			DIE( no_usym, "filehandle" );

		gv = ( GV* )POPs;
		if ( IoFLAGS( GvIOn( gv ) ) & IOf_UNTAINT )   // This GV has UNTAINT previously set
			IoFLAGS( GvIOp( gv ) ) &= ~IOf_UNTAINT; // Clear it. We don't carry that over
		
		tmps = SvPV( sv, len );
		
		if ( do_open( gv, tmps, len, FALSE, 0, 0, Nullfp ) ) 
		{
			IoLINES( GvIOp( gv ) ) = 0;
			PUSHi( ( I32 )forkprocess );
		}
		else if ( forkprocess == 0 )		// we are a new child
		{
			PUSHi( 0 );
		}
		else
		{
			RETPUSHUNDEF;
		}
	}
	PUTBACK;
} 
*/

XS( XS_WIN32__Pipe_Info ) 
{
	dXSARGS;

	if( 1 < items )
    {
		CROAK( "usage: ( $ExtName, $Version, $Date, $Author, $CompileDate, $Credits ) = " EXTENSION "::Info( )\n" );
	}
	
	PUSHMARK( sp );
	
	XPUSHs( sv_2mortal( newSVpv( EXTENSION_DESCRIPTION, strlen( EXTENSION_DESCRIPTION ) ) ) );
	XPUSHs( sv_2mortal( newSVpv( VERSION_STRING, strlen( VERSION_STRING ) ) ) );
	XPUSHs( sv_2mortal( newSVpv( __DATE__, strlen( __DATE__ ) ) ) );
	XPUSHs( sv_2mortal( newSVpv( EXTENSION_AUTHOR, strlen( EXTENSION_AUTHOR ) ) ) );
	XPUSHs( sv_2mortal( newSVpv( __DATE__, strlen( __DATE__ ) ) ) );
	XPUSHs( sv_2mortal( newSVpv( __TIME__, strlen( __TIME__ ) ) ) );
	XPUSHs( sv_2mortal( newSVpv( COPYRIGHT_NOTICE, strlen( COPYRIGHT_NOTICE ) ) ) );

	PUTBACK;
}

XS( XS_WIN32__Pipe_GetVersion )
{
	dXSARGS;

	PUSHMARK( sp );
	XPUSHs( sv_2mortal( newSVpv( VERSION_STRING, strlen( VERSION_STRING ) ) ) );
	PUTBACK;
}

XS( XS_WIN32__Pipe_CheckVersion )
{
	dXSARGS;
	DWORD dwVersionRequested;
	DWORD dwVersion = atol( VERSION_STRING );
	
	if( items )
	{
		dwVersionRequested = SvIV( ST( 0 ) );
	}

    if( dwVersion < dwVersionRequested )
    {
            //  If we are requesting a version that is higher
            //  than our current version we will return 0
            //  ( a FALSE ).
        dwVersion = 0;
    }

    XSRETURN_IV( (IV) dwVersion );
}


XS( XS_WIN32__Pipe_Duplicate )
{
	dXSARGS;
	CPipe	*pCPipe;
	BOOL	bResult = TRUE;
	HANDLE hDuplicate = 0;
	
	if( 1 != items ) 
	{
		CROAK( "usage: $Win32Handle = " EXTENSION "::Duplicate( );\n" );
	}	
		
	PUSHMARK( sp );

	pCPipe = GET_PIPE( );

	if( NULL != pCPipe )
	{
		hDuplicate = pCPipe->Duplicate( );
	}

	XPUSHs( sv_2mortal( newSViv( ( long ) hDuplicate ) ) );

	PUTBACK;
}

XS( XS_WIN32__Pipe_ImpersonateClient )
{
	dXSARGS;
    EXTENSION_VARS;
	CPipe	*pCPipe;
	BOOL	bResult = FALSE;
	
	if( 1 != items ) 
	{
		CROAK( "usage: " EXTENSION "::Impersonate();\n" );
	}	
		
	pCPipe = GET_PIPE( );

	if( NULL != pCPipe )
	{
		bResult = pCPipe->ImpersonateClient();
	}

    PUSH_IV( bResult );

    EXTENSION_RETURN;
}

XS( XS_WIN32__Pipe_StopImpersonation )
{
	dXSARGS;
    EXTENSION_VARS;
	CPipe	*pCPipe;
	BOOL	bResult = FALSE;
	
	if( 1 != items ) 
	{
		CROAK( "usage: " EXTENSION "::StopImpersonation();\n" );
	}	
		
	pCPipe = GET_PIPE( );

	if( NULL != pCPipe )
	{
		bResult = pCPipe->RevertToSelf();
	}

    PUSH_IV( bResult );

    EXTENSION_RETURN;
}

XS( boot_Win32__Pipe )
{
	dXSARGS;
	char* file = __FILE__;
	int i;

    // Initialize our constants...
    CountConstants();

	giError = 0;
	memset( ( void * )gszError, 0, ERROR_TEXT_SIZE );

	newXS( EXTENSION "::Constant",			XS_WIN32__Pipe_Constant, file );
	newXS( EXTENSION "::Create",			XS_WIN32__Pipe_Create,  file );
	newXS( EXTENSION "::Close",			XS_WIN32__Pipe_Close,  file );
	newXS( EXTENSION "::Write",			XS_WIN32__Pipe_Write,  file );
	newXS( EXTENSION "::Read",			XS_WIN32__Pipe_Read,  file );
	newXS( EXTENSION "::Connect",			XS_WIN32__Pipe_Connect,  file );
	newXS( EXTENSION "::Disconnect",		XS_WIN32__Pipe_Disconnect,  file );
	newXS( EXTENSION "::PipeError",			XS_WIN32__Pipe_Error,  file );
	newXS( EXTENSION "::ResizeBuffer",	XS_WIN32__Pipe_ResizeBuffer,  file );
	newXS( EXTENSION "::BufferSize",		XS_WIN32__Pipe_BufferSize,  file );
	newXS( EXTENSION "::Info",				XS_WIN32__Pipe_Info, file );
	newXS( EXTENSION "::Anonymous",			XS_WIN32__Pipe_Anonymous, file );
	newXS( EXTENSION "::ClosePipe",			XS_WIN32__Pipe_ClosePipe, file );
	newXS( EXTENSION "::ReOpen",				XS_WIN32__Pipe_ReOpen, file );
	newXS( EXTENSION "::SetStdHandle",		XS_WIN32__Pipe_StdHandle, file );
	newXS( EXTENSION "::GetStdHandle",		XS_WIN32__Pipe_StdHandle, file );
	newXS( EXTENSION "::StdHandle",			XS_WIN32__Pipe_StdHandle, file );
	newXS( EXTENSION "::GetHandle",			XS_WIN32__Pipe_GetHandle, file );
	newXS( EXTENSION "::CloseHandle",			XS_WIN32__Pipe_CloseHandle, file );
	newXS( EXTENSION "::PeekPipe",			XS_WIN32__Pipe_PeekPipe, file );
	newXS( EXTENSION "::GetInfo",			XS_WIN32__Pipe_GetPipeInfo, file );

	newXS( EXTENSION "::CallNamedPipe",		XS_WIN32__Pipe_CallNamedPipe, file );
	newXS( EXTENSION "::Transact",			XS_WIN32__Pipe_Transact, file );
	newXS( EXTENSION "::TransactNamedPipe",	XS_WIN32__Pipe_TransactNamedPipe, file );
	newXS( EXTENSION "::State",				XS_WIN32__Pipe_State, file );
	newXS( EXTENSION "::Peek",				XS_WIN32__Pipe_Peek, file );
	newXS( EXTENSION "::GetVersion",			XS_WIN32__Pipe_GetVersion, file );
	newXS( EXTENSION "::Duplicate",			XS_WIN32__Pipe_Duplicate, file );

	newXS( EXTENSION "::ImpersonateStart",			XS_WIN32__Pipe_ImpersonateClient, file );
    newXS( EXTENSION "::ImpersonateStop",			XS_WIN32__Pipe_StopImpersonation, file );

	newXS( EXTENSION "::VERSION",			XS_WIN32__Pipe_CheckVersion, file );

	ST( 0 ) = &sv_yes;
	XSRETURN( 1 );

}



