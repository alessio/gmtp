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
#define PACKAGE_VERSION "1.3.1"

#define GMTP_GSETTINGS_SCHEMA "org.gnome.gmtp"

#define ENABLE_NLS
// #define GMTP_USE_GTK3

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

        /* Define that all files are to be C99 compliant with POSIX. */
#define _STDC_C99
#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 600

#ifdef	__cplusplus
}
#endif

#endif	/* _CONFIG_H */

