/* 
 * File:   dirent.h
 * Author: fbsagr
 *
 * Created on September 15, 2014, 2:51 PM
 */

#ifndef EASTWOOD_DIRENT_H
#define	EASTWOOD_DIRENT_H


#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MSC_VER
    #include "win32/dirent_win32.h"
#else
    #include <dirent.h>
#endif

#endif	/* EASTWOOD_DIRENT_H */

