/* 
 *
 *   File: prefs.h
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

#ifndef _PREFS_H
#define _PREFS_H

#ifdef  __cplusplus
extern "C" {
#endif

    typedef struct {
        GString *fileSystemDownloadPath;
        GString *fileSystemUploadPath;
        gboolean attemptDeviceConnectOnStart;
        gboolean ask_download_path;
        gboolean prompt_overwrite_file_op;
        gboolean confirm_file_delete_op;
        gboolean view_size;
        gboolean view_type;
        gboolean view_track_number;
        gboolean view_title;
        gboolean view_artist;
        gboolean view_album;
        gboolean view_year;
        gboolean view_genre;
        gboolean view_duration;
    } Preferences_Struct;

    Preferences_Struct Preferences;


    void setupPreferences();
    gboolean loadPreferences();
    gboolean savePreferences();

#if GMTP_USE_GTK2
    GConfClient *gconfconnect;
    void gconf_callback_func(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data);
#else
    GSettings *gsettings_connect;
    void gsettings_callback_func(GSettings *settings, gchar *key, gpointer user_data);
#endif

#ifdef  __cplusplus
}
#endif

#endif  /* _PREFS_H */

