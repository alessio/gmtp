/* 
*
*   File: config.h
*   
*   Copyright (C) 2009-2011 Darran Kartaschew
*
*   This file is part of the gMTP package.
*
*   gMTP is free software; you can redistribute it and/or modify
*   it under the terms of the BSD License as included within the
*   file 'COPYING' located in the root directory
*
*/

#ifndef _CONFIG_H
#define	_CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#define PACKAGE "gmtp"
#define PACKAGE_TITLE "gMTP"
#define PACKAGE_VERSION "0.9"

#define GMTP_GSETTINGS_SCHEMA "org.gnome.gmtp"

#define ENABLE_NLS
//  #define GMTP_USE_GTK3

#ifndef GMTP_USE_GTK3
    #ifdef GMTP_USE_GTK2
        #undef GMTP_USE_GTK2
    #endif
    #define GMTP_USE_GTK2 1
#else
    #ifdef GMTP_USE_GTK2
        #undef GMTP_USE_GTK2
    #endif
    #define GMTP_USE_GTK2 0
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* _CONFIG_H */

