#define PIPE_NAME_PREFIX	"\\\\.\\pipe\\"
#define	PIPE_NAME_SIZE		256
	
#define	BUFFER_SIZE			512
#define	ERROR_TEXT_SIZE		128

#define ERROR_NAME_TOO_LONG 100, "Pipe Name is too long"

#define	DEFAULT_WAIT_TIME	10000
#define	WAIT_FOR_PIPE		1
#define	DONT_WAIT_FOR_PIPE	0

#define	PIPE_TIMEOUT	    DEFAULT_WAIT_TIME
#define	MAX_BUFFER_SIZE		10240

#define	CLIENT				1
#define SERVER				2

#define	PIPEINFO_NAME_SIZE	256

#define	PIPE_HEADER_STRING	"CPipe Object"

typedef struct PIPE_INFO {
	DWORD	dwState;
	DWORD	dwInstances;
	char	szUserName[PIPEINFO_NAME_SIZE];
	DWORD	cbUserName;
	DWORD	dwMaxCollectionCount;
	DWORD	dwCollectDataTimeout; 
} sPipeInfo, *psPipeInfo;

class CPipe {
	public:
		static CPipe *Create( char *pszName, 
                              DWORD dwWait, 
                              DWORD dwType = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 
                              SECURITY_DESCRIPTOR *pSD = NULL, 
                              DWORD *pdwError = NULL );
		
		BOOL Initialize( char *pszPipeName, 
                         DWORD dwWait, 
                         DWORD dwType = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 
                         SECURITY_DESCRIPTOR *pSD = NULL,
                         DWORD *pdwError = NULL );
		
		CPipe();
		~CPipe();

		int	Write(void *vBuffer, DWORD dSize);
		char *Read(DWORD *dLen);
		int	Error(int iErrorNum, char *szErrorText);
		int	EndOfFile();
		int Connect();
		int	Disconnect(int iPurge);
		DWORD BufferSize();
		DWORD ResizeBuffer(DWORD dNewSize);
		HANDLE GetHandle();
		HANDLE SetHandle( HANDLE hHandle );
		BOOL Transact(void **pBuffer, DWORD *dwSize);
		sPipeInfo *GetInfo();
		DWORD State();
		DWORD State( DWORD dwState );
		PVOID Peek( DWORD *pdwBytesRead );
		DWORD GetError();
		const char *GetErrorText();
		const char *GetBuffer();
		DWORD GetBufferSize();
		HANDLE Duplicate();
        BOOL ImpersonateClient();
        BOOL RevertToSelf();

	private:
		DWORD AllocateBuffer( DWORD dwSize );
        int GrantPrivilege( LPCTSTR szPrivilege );
        BOOL RevokePrivilege( LPCTSTR szPrivilege ); 

		CHAR	*m_pBuffer;
		DWORD	m_dwBufferSize;
		DWORD	m_dwError;
		LPSTR	m_szError[ ERROR_TEXT_SIZE ];

		HANDLE	m_hPipe;
  		LPSTR	m_szName[ 257 ];					// address of pipe name 
    	DWORD	m_dwOpenMode;
    	DWORD	m_dwPipeMode;
    	DWORD	m_dwMaxInstances;					// maximum number of instances  
    	DWORD	m_dwOutBufferSize;
    	DWORD	m_dwInBufferSize;
    	DWORD	m_dwDefaultTimeOut;					// time-out time, in milliseconds 
    	SECURITY_ATTRIBUTES  m_saSecAttrib;
		int		m_fdFile;
	
		int		m_iPipeType;						//	Type of connection.
		BOOL	m_bServerIsClient;					//	TRUE if this is a client AND the server is on the same machine
		DWORD	m_dwBytes;
		sPipeInfo	m_sPipeInfo;
        SECURITY_DESCRIPTOR *m_pLocalSD;            //  This is used ONLY if we must create a default SD
        BOOL    m_bImpersonated;                    //  TRUE if we are impersonating the client

};
		

		
