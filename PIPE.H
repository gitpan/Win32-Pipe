#include "Win32Perl.h"
 
#ifndef _PIPE_H
#   define _PIPE_H 

#   ifndef WIN32
#       ifdef _WIN32
#           define WIN32   
#       endif // _WIN32  
#   endif // WIN32  
  
///////////////////////////////////////////////////////////////////////////////////////////
//	Begin resource compiler macro block

    //  Include the version information...
    #include "version.h"

    #define	EXTENSION_NAME			"Pipe"

    #define	EXTENSION_PARENT_NAMESPACE		"Win32"
    #define EXTENSION				EXTENSION_PARENT_NAMESPACE "::" EXTENSION_NAME
    
    #define	EXTENSION_FILE_NAME		EXTENSION_NAME

    #define	EXTENSION_VERSION		VERSION
    #define	EXTENSION_AUTHOR		"Dave Roth <rothd@roth.net>"

    #define	COPYRIGHT_YEAR			"1996-1999"
    #define	COPYRIGHT_NOTICE		"Copyright (c) " COPYRIGHT_YEAR

    #define COMPANY_NAME			"Roth Consulting\r\nhttp://www.roth.net/consult"

    #define	VERSION_TYPE			"Release"
//	End resource compiler macro block
///////////////////////////////////////////////////////////////////////////////////////////

    #define EXTENSION_DESCRIPTION       "Pipe extension for Win32 Perl"
    #define PERL_WIN32_PERMS_EXTENSION  "Win32::Perms"


    #define	SYNTAX_ERROR	999
    #define CROAK(xxxx)													\
				    PUSHMARK(sp);										\
				    XPUSHs(sv_2mortal(newSVnv((double)SYNTAX_ERROR)));	\
				    XPUSHs(sv_2mortal(newSVpv(xxxx, strlen(xxxx))));	\
				    PUTBACK;											\
				    return;												\

    #define	CLASS_POINTER_MEMBER_NAME		"m_pPipe"
    #define PERL_WIN32_PERMS_EXTENSION      "Win32::Perms"

    #define	GET_PIPE()			GetObjectPointer( PERL_OBJECT_ARGS GetSelf( PERL_OBJECT_ARGS (SV *) ST( 0 ) ) )
    #define DEL_PIPE()          DelObjectPointer( PERL_OBJECT_ARGS GetSelf( PERL_OBJECT_ARGS (SV *) ST( 0 ) ) )

    int	Error(int iErrorNum, char *szErrorText);
    void FixPath( char *pszPath );

#endif // _PIPE_H_
