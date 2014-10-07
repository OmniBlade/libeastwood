/* 
 * File:   strings.h
 * Author: fbsagr
 *
 * Created on September 15, 2014, 11:30 AM
 */

#ifndef EASTWOOD_STRINGS_H
#define	EASTWOOD_STRINGS_H

#include <stdio.h>

#ifdef _MSC_VER
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
    #include <string.h>
    #define DIR_SEP '\\'
#else
    #include <string.h>
    #include <strings.h>
    #define DIR_SEP '/'
#endif

#include <string>

#endif	/* EASTWOOD_STRINGS_H */

