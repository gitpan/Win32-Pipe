#define	Pipe	100

#pragma message( "*** Using Perl " PERLVER ".\n" )
#pragma message( "*** Creating the " VERSION_TYPE " " EXTENSION_NAMESPACE " extension v" VERSION_STRING " ( " EXTENSION_FILE_NAME "." FILE_EXTENSION " )...\n" )
#pragma message( "    ================================================================================\n\n" )


#ifdef  NT_BUILD_NUMBER
    #define     BUILD_HI    0
    #define     BUILD_LO    NT_BUILD_NUMBER
#else
    #define     BUILD_HI    0
    #define     BUILD_LO    0
#endif    //    NT_BUILD_NUMBER

#if _DEBUG
    #define VER_DEBUG       VS_FF_DEBUG 
    #define DEBUG_STRING    "Debug"
#else
    #define VER_DEBUG       0
    #define DEBUG_STRING    ""
#endif


#define	VER_FILEVERSION	HIWORD(VERSION_HI),LOWORD(VERSION_HI), HIWORD(VERSION_LO), LOWORD(VERSION_LO)
#define	VER_PRODVERSION	HIWORD(VERSION_HI),LOWORD(VERSION_HI), HIWORD(VERSION_LO), LOWORD(VERSION_LO)

#define INTERNALNAME	EXTENSION

#define	VERNAME			EXTENSION " extension for Win32 Perl"
#define VERSION_NUM		VERSION_HI
#define	VERSION_TEXT	VERSION_STRING " " VERSION_TYPE
#define VERDATE			__DATE__
#define VERTIME			__TIME__
#define VERAUTH     	EXTENSION_AUTHOR 
#define VERCRED			COPYRIGHT_NOTICE " " VERAUTH "."	
#define VERCOMMENT		"This version requires " PERLVER "."

#define RC_COMMENTS		VERNAME "\r\n----\r\n" VERCOMMENT "\0"
#define	RC_COMPANY		COMPANY_NAME "\0"
#define	RC_FILEDESC		INTERNALNAME " for " PERLVER "\0"
#define RC_FILEVER		VERSION_TEXT "\0"
#define	RC_INTNAME		INTERNALNAME " " DEBUG_STRING "\0"
#define	RC_COPYRIGHT	"\251 " COPYRIGHT_YEAR " by " VERAUTH "\0"
#define	RC_FILENAME		EXTENSION_FILE_NAME "." FILE_EXTENSION "\0"
#define	RC_PBUILD		VERSION_STRING " " VERSION_TYPE " " DEBUG_STRING "\0"
#define	RC_PRODNAME		INTERNALNAME "\0"
#define	RC_PRODVER		VERSION_STRING "\0"			  


/*
    We should delete this old block!


	//	We MUST include perl.h so that we can later determine which verion of Perl
	//	we are using 5.003, 5.004, 5.005, CPerl object, or whatever...
#include "perl.h"
#include "pipe.h"

#define	Pipe    100

#define	VERSION_HI		EXTENSION_VERSION

#ifdef	PERL_OBJECT
#	ifndef		NT_BUILD_NUMBER
#		ifndef PERLVER
#			define		PERLVER	"Win32 Perl"
#		endif // PERLVER
#	else // NT_BUILD_NUMBER
#		ifndef PERLVER
#			define		PERLVER "Win32 Perl build " NT_BUILD_NUMBER
#		endif // PERLVER
#	endif
#endif	//	PERL_OBJECT

#ifdef	NT_BUILD_NUMBER
#define		VERSION_LO	NT_BUILD_NUMBER
#else
#define		VERSION_LO		"XXX"
#endif	//	NT_BUILD_NUMBER

#ifndef VERSION_TYPE
	#ifdef	VERSION_RELEASE
		#define		VERSION_TYPE	"Release"
	#else
		#define		VERSION_TYPE	"Beta"
	#endif	//	VERSION_RELEASE

	#ifdef	VERSION_BETA
		#define		VERSION_TYPE	"Beta"
	#endif
#endif // VERSION_TYPE

#if _DEBUG
#define		DEBUG_STRING	"Debug"
#else
#define		DEBUG_STRING	""
#endif

#define	VER_FILEVERSION	HIWORD(VERSION_HI),LOWORD(VERSION_HI), HIWORD(VERSION_LO), LOWORD(VERSION_LO)
#define	VER_PRODVERSION	HIWORD(VERSION_HI),LOWORD(VERSION_HI), HIWORD(VERSION_LO), LOWORD(VERSION_LO)

#define INTERNALNAME	EXTENSION_NAMESPACE "::" EXTENSION_NAME

#define	VERNAME			EXTENSION_NAMESPACE "::" EXTENSION_NAME " extension for Win32 Perl"
#define VERSION_NUM		VERSION_HI
#define	VERSION_TEXT	VERSION_NUM " " VERSION_TYPE
#define VERDATE			__DATE__
#define VERTIME			__TIME__
#define VERAUTH     	EXTENSION_AUTHOR 
#define VERCRED			COPYRIGHT_NOTICE " " VERAUTH "."	
#define VERCOMMENT		"This version requires " PERLVER "."

#define RC_COMMENTS		VERNAME "\r\n----\r\n" VERCOMMENT "\0"
#define	RC_COMPANY		COMPANY_NAME "\0"
#define	RC_FILEDESC		INTERNALNAME " for " PERLVER "\0"
#define RC_FILEVER		VERSION_STRING "\0"
#define	RC_INTNAME		INTERNALNAME " " DEBUG_STRING "\0"
#define	RC_COPYRIGHT	"\251 " COPYRIGHT_YEAR " by " VERAUTH "\0"
#define	RC_FILENAME		EXTENSION_FILE_NAME "." FILE_EXTENSION "\0"
#define	RC_PBUILD		VERSION_STRING " " VERSION_TYPE " " DEBUG_STRING "\0"
#define	RC_PRODNAME		INTERNALNAME "\0"
#define	RC_PRODVER		VERSION_STRING "\0"			  
*/

