
#define WIN32_LEAN_AND_MEAN
#include <FCNTL.H>

#ifdef __BORLANDC__
typedef wchar_t wctype_t; /* in tchar.h, but unavailable unless _UNICODE */
#endif

#include <windows.h>

#include <stdio.h>		//	Gurusamy's right, Borland is brain damaged!
#include <math.h>		//	Gurusamy's right, MS is brain damaged!

#include "CPipe.hpp"

#ifdef _open_osfhandle
#undef _open_osfhandle
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


////////////////////////////////////////////////////////////////////////
CPipe::CPipe()
{
			              	
	ZeroMemory( m_szName, sizeof( m_szName ) );
	ZeroMemory( m_szError, sizeof( m_szError ) );
	ZeroMemory( (void *) &m_sPipeInfo, sizeof( sPipeInfo ) );

	m_pBuffer = NULL;
	m_dwBufferSize = 0;
	m_dwError = 0;
	
	m_hPipe = INVALID_HANDLE_VALUE;
  	m_dwOpenMode = 0;
    m_dwPipeMode = 0;
    m_dwMaxInstances = 0;
    m_dwOutBufferSize = 0;
    m_dwInBufferSize = 0;
    m_dwDefaultTimeOut = 0;
	m_fdFile = 0;

	m_iPipeType = SERVER;
	m_bServerIsClient = 0;
	
	m_dwBytes = 0;
    m_pLocalSD = NULL;

    m_bImpersonated = FALSE;

}

////////////////////////////////////////////////////////////////////////
BOOL CPipe::Initialize( char *szName, DWORD dwWait, DWORD dwType, SECURITY_DESCRIPTOR *pSD, DWORD *pdwError )
{
	BOOL bResult = TRUE;
	char *pszTemp = NULL;
	int	iTemp;
	int	iFlag = 1;
		
	m_dwBufferSize = BUFFER_SIZE;

	//	We use the dwType passed in to decide what kind of pipe
	//	this is (byte or message based)
	m_dwPipeMode = 	dwType;	

    //
    //  The pipe should be opened as full duplex. Whether we grant ourselves 
    //  the permissions to modify DACL/SACL/owner will be determined later
    //
	m_dwOpenMode = PIPE_ACCESS_DUPLEX;

    // Limit the number of instances by only available resources:
	m_dwMaxInstances =	PIPE_UNLIMITED_INSTANCES;
	
	AllocateBuffer( m_dwBufferSize );
	if( NULL == m_pBuffer )
    {
		m_dwError = 998;
		strcpy((char *)m_szError, "Could not allocate a buffer for the pipe connection");
		bResult = FALSE;
	}
    else
    {
        SECURITY_DESCRIPTOR SecurityDesc;
        int iSACLPrivilege = 0;

        m_dwOutBufferSize = m_dwBufferSize;			
	    m_dwInBufferSize  = m_dwBufferSize;
	    m_dwDefaultTimeOut = PIPE_TIMEOUT;			

	    m_bServerIsClient = FALSE;

	    m_sPipeInfo.dwState = 0;
	    m_sPipeInfo.dwInstances = 0;
	    ZeroMemory( m_sPipeInfo.szUserName, 0 );
	    m_sPipeInfo.cbUserName = sizeof( m_sPipeInfo.szUserName );
	    m_sPipeInfo.dwMaxCollectionCount = 0;
	    m_sPipeInfo.dwCollectDataTimeout = 0; 

	    strcpy( (char *) m_szError, "" );
	    if( strncmp( (char *) szName, "\\\\", 2 ) == 0)
	    {
		    char	szBuffer[ MAX_COMPUTERNAME_LENGTH + 1 ];
		    DWORD	cbBuffer = sizeof( szBuffer );

		    m_iPipeType = CLIENT;
		    iTemp = 0;

		    m_bServerIsClient = TRUE;
			    
		    if( GetComputerName( 
			    szBuffer,
			    &cbBuffer ) )
		    {
			    char szCopyOfName[256];
			    char *szTemp;
		    
			    strcpy( szCopyOfName, szName );
			    
			    szTemp = &szCopyOfName[ strspn( szCopyOfName, "\\/" ) ];
			    
			    szTemp[ strcspn( szName, "\\/" ) ] = '\0';
			    
			    if( 0 != stricmp( szBuffer, szTemp ) )
			    {
				    m_bServerIsClient = FALSE;
			    }
		    }
	    }
        else
        {
		    m_iPipeType = SERVER;
		    strcpy( (char *) m_szName, PIPE_NAME_PREFIX);
		    iTemp = strlen( PIPE_NAME_PREFIX );
	    }
	    strncat((char *)m_szName, szName, PIPE_NAME_SIZE - iTemp);

        //  Look into the security stuff
	    m_saSecAttrib.nLength = sizeof( m_saSecAttrib );
	    m_saSecAttrib.bInheritHandle = 1;

        // If we passed in a SECURITY DESCRIPTOR (probably from Win32::Perms) then use it...
	    if( NULL == pSD )
        {
            BOOL bResult = FALSE;

            // ...otherwise allocate an NULL DACL and use that -- grants Everyone Full access.
            // Remember, an Empty DACL grants access to NOBODY!
    /*
            m_pLocalSD = (SECURITY_DESCRIPTOR*) LocalAlloc( LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH );
            if( NULL != m_pLocalSD )
            {
    */
            if( InitializeSecurityDescriptor( &SecurityDesc, SECURITY_DESCRIPTOR_REVISION ) ) 
            {
                // Set the NULL DACL
			    bResult = SetSecurityDescriptorDacl( &SecurityDesc, TRUE, NULL, FALSE );
            }
            bResult = IsValidSecurityDescriptor( &SecurityDesc );
    /*
		    }
    */
            pSD = &SecurityDesc;
	    }
        
        //
        //  Add the DACL and owner access attributes here.
        //  If we successfully set the security privilege then we can
        //  set the SACL access...
        //
        m_dwOpenMode |= WRITE_DAC | WRITE_OWNER;

        iSACLPrivilege = GrantPrivilege( "SeSecurityPrivilege" );
        if( 0 != iSACLPrivilege )
        {
            //
            // If the iSACLPrivilege flag is either 1 or -1 then we have 
            //  been granted the privilege so allow us the attribute to
            //  access the SACL info...
            //
            m_dwOpenMode |= ACCESS_SYSTEM_SECURITY;
        }
        m_saSecAttrib.lpSecurityDescriptor = pSD;	

        //
        //  Set the timeout value (passed in in seconds--convert to ms)
        //
        if( 100 > dwWait )
        {
            //
            //  If we passed in a timeout less than 100 let's just assume
            //  that the time was passed in as seconds. After all, who
            //  would really want a timeout value of less than 100ms???
            //
            dwWait *= 1000;
        }
        m_dwDefaultTimeOut = dwWait;

	    if( SERVER == m_iPipeType )
	    {
		    m_hPipe = CreateNamedPipe( (const char *) m_szName,
								    m_dwOpenMode, 
								    m_dwPipeMode, 
								    m_dwMaxInstances, 
								    m_dwOutBufferSize, 
								    m_dwInBufferSize, 
								    m_dwDefaultTimeOut,
								    &m_saSecAttrib );
	    }
	    else
	    {
		    while( iFlag )
		    {
			    m_hPipe = CreateFile((LPCTSTR) m_szName,
								    GENERIC_READ | GENERIC_WRITE, 
								    FILE_SHARE_READ	| FILE_SHARE_WRITE,
								    NULL,
								    OPEN_EXISTING,
								    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
								    NULL);
                m_dwError = GetLastError();
			    if( ERROR_PIPE_BUSY == m_dwError )
			    {
				    iFlag = WaitNamedPipe( (LPCTSTR) m_szName, m_dwDefaultTimeOut );
			    }
			    else
			    {
				    iFlag = 0;
			    }
		    }
			//	Set the type (eg. PIPE_TYPE_MESSAGE ) because CreateFile will
			//	not do it.
		    State( dwType );
	    }

	    if( INVALID_HANDLE_VALUE == m_hPipe )
	    {
		    m_dwError = GetLastError();
		    bResult = FALSE;
	    }
        else
        {
            //  We are not really using this so let's get rid of it. IF WE DO use it
            //  make sure to close the file in the destructor!
	        //  m_fdFile = _open_osfhandle( (long) m_hPipe, _O_TEXT );
        }

        if( 1 == iSACLPrivilege )
        {
            RevokePrivilege( "SE_SECURITY_NAME" );
        }
    }

    if( NULL != pdwError )
    {
        *pdwError = m_dwError;
    }

	return( bResult );
}	

////////////////////////////////////////////////////////////////////////
CPipe *CPipe::Create( char *szName, DWORD dwWait, DWORD dwType, SECURITY_DESCRIPTOR *pSD, DWORD *pdwError )
{
	CPipe * pPipe = NULL;

	pPipe = new CPipe();
	if( NULL != pPipe )
	{
		if( FALSE == pPipe->Initialize( szName, dwWait, dwType, pSD, pdwError ) )
		{
			delete pPipe;
			pPipe = NULL;
		}
	}

    if( NULL != pdwError )
    {
        *pdwError = ::GetLastError();
    }

	return( pPipe );
}

////////////////////////////////////////////////////////////////////////
CPipe::~CPipe()
{
    if( TRUE == m_bImpersonated )
    {
        RevertToSelf();
    }

	Disconnect( TRUE );

	if( ( INVALID_HANDLE_VALUE != m_hPipe ) )
	{
		if( CLIENT == m_iPipeType )
		{
			CloseHandle( m_hPipe );
		}
		m_hPipe = INVALID_HANDLE_VALUE;
	}

	if( NULL != m_pBuffer )
	{
		delete [] m_pBuffer;
	}
	if( NULL != m_pLocalSD )
	{
		LocalFree( m_pLocalSD );
	}
}

////////////////////////////////////////////////////////////////////////
HANDLE CPipe::GetHandle(){
	return m_hPipe;
}

////////////////////////////////////////////////////////////////////////
HANDLE CPipe::SetHandle( HANDLE hHandle )
{
	return( m_hPipe = hHandle );
}


////////////////////////////////////////////////////////////////////////
HANDLE CPipe::Duplicate()
{
	HANDLE hDuplicate = 0;

	if( FALSE == DuplicateHandle( GetCurrentProcess(),
								  m_hPipe,
								  GetCurrentProcess(),
								  &hDuplicate,
								  0,
								  TRUE,
								  DUPLICATE_SAME_ACCESS) )
	{
		hDuplicate = 0;
	}

	return( hDuplicate );
}

////////////////////////////////////////////////////////////////////////
sPipeInfo *CPipe::GetInfo()
{
	BOOL bResult;

	m_sPipeInfo.dwState = PIPE_NOWAIT; 
	m_sPipeInfo.dwInstances = 0;
	m_sPipeInfo.dwMaxCollectionCount = 0;
	m_sPipeInfo.dwCollectDataTimeout = 0;
	strcpy( m_sPipeInfo.szUserName, "" );

	bResult = GetNamedPipeHandleState( 
		m_hPipe,
		&m_sPipeInfo.dwState,
		&m_sPipeInfo.dwInstances,
		( ( SERVER == m_iPipeType ) && m_bServerIsClient )? &m_sPipeInfo.dwMaxCollectionCount:NULL,
		( ( SERVER == m_iPipeType ) && m_bServerIsClient )? &m_sPipeInfo.dwCollectDataTimeout:NULL, 
		m_sPipeInfo.szUserName, 
 		m_sPipeInfo.cbUserName );

	return( ( 0 != bResult )? &m_sPipeInfo:NULL );
}

////////////////////////////////////////////////////////////////////////
DWORD CPipe::BufferSize()
{
	return m_dwBufferSize;
}

////////////////////////////////////////////////////////////////////////
DWORD CPipe::ResizeBuffer( DWORD dwNewSize )
{
	CHAR	*pBuffer;

	if( 0 < dwNewSize )
    {
		pBuffer = new char [ dwNewSize ];
        if( NULL != pBuffer )
        {
            ZeroMemory( (PVOID) pBuffer, dwNewSize );
			delete [] m_pBuffer;
			m_dwBufferSize = dwNewSize;
			m_pBuffer = pBuffer;
		}
	}
	return( m_dwBufferSize );
}

////////////////////////////////////////////////////////////////////////
char *CPipe::Read( DWORD *pdwLen )
{                   
	BOOL bResult;
	DWORD	cbBytes = 0, cbReply = 0;

	bResult = ReadFile( m_hPipe, m_pBuffer, m_dwBufferSize, pdwLen, NULL ); 
	m_dwBytes = *pdwLen;
	return( ( bResult )? m_pBuffer : NULL );
}

////////////////////////////////////////////////////////////////////////
int	CPipe::Write(void *pBuffer, DWORD dwSize )
{
	BOOL bResult;
	DWORD	cbBytes = 0, cbReply = 0;

	bResult = WriteFile( m_hPipe, pBuffer, dwSize, &cbBytes, NULL );
	return( bResult );
}

////////////////////////////////////////////////////////////////////////
BOOL CPipe::Transact(void **pBuffer, DWORD *dwSize)
{
	BOOL bResult;
	DWORD	cbBytes = 0, cbReply = 0;

	ZeroMemory( m_pBuffer, m_dwBufferSize );

	bResult = TransactNamedPipe(
			m_hPipe,
			*pBuffer,
			*dwSize,
			m_pBuffer,
			m_dwBufferSize,
			&cbBytes,
			NULL );
	if( FALSE != bResult )
	{
		*dwSize = cbBytes;
		*pBuffer = m_pBuffer;
	}
	else
	{
		*dwSize = 0;
		*pBuffer = NULL;
	}

	return( bResult );
}


////////////////////////////////////////////////////////////////////////
int CPipe::Connect()
{
	BOOL bResult;

	bResult = ConnectNamedPipe( m_hPipe, NULL );

	//	Just in case the pipe is already connected return TRUE even though
	//	ConnectNamedPipe() returned FALSE!
    m_dwError = GetLastError();
    if( PIPE_NOWAIT & m_dwPipeMode )
    {
        // We are in nonblocking mode so the following logic
        // is required:
        if( bResult )
        {
            // TRUE means that the pipe has disconnected and is
            // ready for a client to connect.
            bResult = FALSE;    
        }
        else
        {
            switch( m_dwError )
            {
            case ERROR_PIPE_LISTENING:
                bResult = FALSE;
                break;

            case ERROR_PIPE_CONNECTED:
                bResult = TRUE;
                break;

            case ERROR_NO_DATA:
                // Client has closed the pipe but server has yet
                // to disconnect.
                bResult = FALSE;
                break;
            }
        }
    }
    else
    {
        if( ( FALSE == bResult ) && ( ERROR_PIPE_CONNECTED == m_dwError ) )
        {
		    bResult = TRUE;
	    }
    }
    
	return( bResult );
}

////////////////////////////////////////////////////////////////////////
int CPipe::Disconnect( BOOL bPurge )
{
	BOOL	bResult = FALSE;

	if( FALSE != bPurge )
	{
		FlushFileBuffers( m_hPipe );
	}

	if( SERVER == m_iPipeType )
	{
		bResult = DisconnectNamedPipe( m_hPipe );
	}
    else if( CLIENT == m_iPipeType )
	{
		bResult = CloseHandle( m_hPipe );
		m_hPipe = INVALID_HANDLE_VALUE;
	}
	
    m_dwError = GetLastError();
	return( bResult );
}	


////////////////////////////////////////////////////////////////////////
int	CPipe::Error( int dwErrorNum, char *pszErrorText )
{
	strncpy( (char *) m_szError, pszErrorText, sizeof( m_szError ) - 1 );
	m_dwError = dwErrorNum;
	return( m_dwError );
}

////////////////////////////////////////////////////////////////////////
int	CPipe::EndOfFile()
{
	return( ( m_dwBytes )? 1 : 0 );
}

////////////////////////////////////////////////////////////////////////
DWORD CPipe::State( DWORD dwState )
{
	BOOL bResult = SetNamedPipeHandleState(
			m_hPipe,
			&dwState,
			NULL,
			NULL );
	return( State() );
}

////////////////////////////////////////////////////////////////////////
DWORD CPipe::State()
{
	DWORD dwState = 0;
	BOOL bResult = GetNamedPipeHandleState(
		m_hPipe,
		&dwState,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL );

	return( dwState );
}

////////////////////////////////////////////////////////////////////////
PVOID CPipe::Peek( DWORD *pdwBytesRead )
{
    PVOID   pBuffer = NULL;
	DWORD	dwBytesTotal = 0;
	DWORD	dwBytesRemain = 0;
    
    *pdwBytesRead = 0;
    ZeroMemory( m_pBuffer, m_dwBufferSize );

	if( FALSE == PeekNamedPipe( m_hPipe, m_pBuffer, m_dwBufferSize, pdwBytesRead, &dwBytesTotal, &dwBytesRemain ) )
	{
		pBuffer = NULL;
		*pdwBytesRead = 0;
	}
	else
	{
		pBuffer = m_pBuffer;
	}

	return( pBuffer );	
}

////////////////////////////////////////////////////////////////////////
DWORD CPipe::GetError()
{
	return( m_dwError );
}

////////////////////////////////////////////////////////////////////////
const char *CPipe::GetErrorText()
{
	return( (char *) m_szError );
}

////////////////////////////////////////////////////////////////////////
DWORD CPipe::GetBufferSize()
{
	return( m_dwBufferSize );
}

////////////////////////////////////////////////////////////////////////
const char *CPipe::GetBuffer()
{
	return( m_pBuffer );
}

////////////////////////////////////////////////////////////////////////
DWORD CPipe::AllocateBuffer( DWORD dwSize )
{
	if( NULL != m_pBuffer )
	{
		delete [] m_pBuffer;
		m_pBuffer = NULL;
	}

	m_dwBufferSize = dwSize;
	
	m_pBuffer = new char [ m_dwBufferSize ];
	if( NULL == m_pBuffer )
	{
		m_dwBufferSize = 0;
	}
	else
	{
		ZeroMemory( m_pBuffer, m_dwBufferSize );
	}

	return( m_dwBufferSize );
}

////////////////////////////////////////////////////////////////////////
BOOL CPipe::ImpersonateClient()
{
    if( ( INVALID_HANDLE_VALUE != m_hPipe ) && ( SERVER == m_iPipeType ) )
    {
        m_bImpersonated = ImpersonateNamedPipeClient( m_hPipe );
    }
    return( m_bImpersonated );
}

////////////////////////////////////////////////////////////////////////
BOOL CPipe::RevertToSelf()
{
    BOOL bResult = FALSE;
    if( ( INVALID_HANDLE_VALUE != m_hPipe ) && ( SERVER == m_iPipeType ) && ( TRUE == m_bImpersonated ) )
    {
        m_bImpersonated = ::RevertToSelf();
    }
    return( m_bImpersonated );
}


/*
////////////////////////////////////////////////////////////////////////
BOOL CPipe::SetSecurity()
{
	BOOL bResult = FALSE;	
	PSECURITY_DESCRIPTOR pSD = NULL;

		// Allocate memory for the security descriptor.
	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if( NULL == pSD )
	{
		
			// Initialize the new security descriptor.
		InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
			
			// Add a NULL descriptor ACL to the security descriptor.
		SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE);

		m_saSecAttrib.nLength = sizeof(m_saSecAttrib);
		m_saSecAttrib.lpSecurityDescriptor = pSD;
		m_saSecAttrib.bInheritHandle = TRUE;
		
		bResult = TRUE;
	}

	return( bResult );
}

	//	
	//	Set the NULL security discriptor
	//

	{
		PSECURITY_DESCRIPTOR	pSD;
		PACL	pACLNew;
		DWORD	cbACL;
		PSID	pSID;
		DWORD	cbSID;
		LPSTR	lpszAccount;
		LPSTR	lpszDomain;
		DWORD	cchDomainName;
		DWORD	cbType;
		PSID_NAME_USE	psnuType;

			//	Just creating a shortcut! :) 
		pSD = &m_saSecAttrib.lpSecurityDescriptor;
		pACLNew = 0;                               
		cbACL = DEFAULT_ACL_SIZE;                  
		pSID = 0;                                  
		cbSID = DEFAULT_ACL_SIZE;                  
		lpszAccount = 0;                           
		lpszDomain  = 0;                           
		cchDomainName = 0;       
		cbType = 1024;                             
		psnuType = 0;
		pSID = (PSID) LocalAlloc(LPTR, cbSID);
			//	No error checking ... we are [stupidly] assuming all is well.
		LookupAccountName( 0, "Everyone", pSID, &cbSID, lpszDomain, &cchDomainName, psnuType);
		if (IsValidSid(pSID)){
			PACL	pACL;
			if(InitializeAcl(pACL, cbACL, ACL_REVISION2)){
				if(AddAccessAllowedAce(pAcl, ACL_REVISION2, GENERIC_ALL, pSID)){
					if(SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)){
						iResult = 1;
					}
				}
			}
		}
		if (!iResult){
			pSD = 0;
		}
			
	}
*/


////////////////////////////////////////////////////////////////////////
int CPipe::GrantPrivilege( LPCTSTR szPrivilege ) 
{ 
    HANDLE	hToken; 
    LUID	uidPrivilegeValue; 
    TOKEN_PRIVILEGES	tkp; 
    int iResult = 0;            //  Return value 0==Failed, 1==Success, -1==Alredy Set

    if( OpenProcessToken( GetCurrentProcess(), 
                            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
                            &hToken ) ) 
    { 

        if( LookupPrivilegeValue( (LPSTR) NULL, 
                                          szPrivilege, 
                                          &uidPrivilegeValue ) ) 
        { 
            BOOL bResult = FALSE;
            BOOL bPreviouslySet = FALSE;
            PRIVILEGE_SET privCheck;
            DWORD dwPrevSize = 0;
                //  We need to assign the ptkpPrevious with a non NULL value so that the AdjustTokenPrivilegs
                //  function does not give up on attempting to return the required buffersize for the previous
                //  privileges.
            PTOKEN_PRIVILEGES ptkpPrevious = &tkp; 

            tkp.PrivilegeCount = 1; 
            tkp.Privileges[0].Luid = uidPrivilegeValue; 
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

            privCheck.PrivilegeCount = 1;
            privCheck.Control = PRIVILEGE_SET_ALL_NECESSARY;
            privCheck.Privilege[0].Luid = tkp.Privileges[0].Luid;
            privCheck.Privilege[0].Attributes = tkp.Privileges[0].Attributes;

            if( PrivilegeCheck( hToken,
                                &privCheck,
                                &bPreviouslySet ) )
            {
                if( NULL != ptkpPrevious )
                {
                    if( AdjustTokenPrivileges(  hToken, 
                                                FALSE, 
                                                &tkp, 
                                                0, 
                                                (PTOKEN_PRIVILEGES) NULL,
                                                (PDWORD) NULL ) )
                    {
                            //  iResult == 1 or (if bPreviouslySet is TRUE) then iResult == -1
                        iResult = 1 - ( 2 * bPreviouslySet );
                    }
                }
            }
        }
        CloseHandle( hToken );
    }	
    return( iResult );
}

////////////////////////////////////////////////////////////////////////
BOOL CPipe::RevokePrivilege( LPCTSTR szPrivilege ) 
{ 
    HANDLE	hToken; 
    LUID	uidPrivilegeValue; 
    TOKEN_PRIVILEGES	tkp; 
    BOOL	bResult = FALSE;

    if( OpenProcessToken( GetCurrentProcess(), 
                            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
                            &hToken ) )
    { 
        if( LookupPrivilegeValue( (LPSTR) NULL, 
                                        SE_TAKE_OWNERSHIP_NAME, 
                                        &uidPrivilegeValue ) ) 
        { 
            tkp.PrivilegeCount = 1; 
            tkp.Privileges[0].Luid = uidPrivilegeValue; 
            tkp.Privileges[0].Attributes = 0; 

            AdjustTokenPrivileges(  hToken, 
                                    FALSE,
                                    (PTOKEN_PRIVILEGES) &tkp,
                                    sizeof(TOKEN_PRIVILEGES), 
                                    (PTOKEN_PRIVILEGES) NULL, 
                                    (PDWORD) NULL ); 

            if( ERROR_SUCCESS == GetLastError() ) 
            {
                bResult = TRUE;
            } 
        }
        CloseHandle( hToken );
    }
    return( bResult );
}
