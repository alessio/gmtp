/* 
 *
 *   File: main.h
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

#ifndef _MAIN_H
#define _MAIN_H

#ifdef  __cplusplus
extern "C" {
#endif

    // Main Device information struct.

    typedef struct {
        gboolean deviceConnected;
        gint numrawdevices;
        gint rawdeviceID;
        gint storagedeviceID;

        LIBMTP_raw_device_t * rawdevices;
        LIBMTP_mtpdevice_t *device;
        LIBMTP_devicestorage_t *devicestorage;
        LIBMTP_error_number_t err;

        GString *devicename;
        GString *manufacturername;
        GString *modelname;
        GString *serialnumber;
        GString *deviceversion;
        GString *syncpartner;
        GString *sectime;
        GString *devcert;

        // Raw device
        GString *Vendor;
        GString *Product;
        uint32_t VendorID;
        uint32_t ProductID;
        uint32_t DeviceID;
        uint32_t BusLoc;

        uint16_t *filetypes;
        uint16_t filetypes_len;
        uint8_t maxbattlevel;
        uint8_t currbattlevel;

    } Device_Struct;

    // Main Window Widgets.
    GtkWidget *windowMain;
    GtkWidget *windowPrefsDialog;
    GtkWidget *windowPropDialog;
    GtkWidget *windowPlaylistDialog;
    GtkWidget *windowStatusBar;
    GtkWidget *toolbuttonConnect;
    GtkWidget *treeviewFiles;

    // Device information struct
    Device_Struct DeviceMgr;

    // File/Folder/Track/Playlist pointers
    LIBMTP_file_t *deviceFiles;
    LIBMTP_folder_t *deviceFolders;
    LIBMTP_track_t *deviceTracks;
    LIBMTP_playlist_t *devicePlayLists;
    uint32_t currentFolderID; // This is the ID of the current folder....

    // Icon file locations.
    GString *file_icon_png;
    GString *file_icon16_png;
    GString *file_about_png;
    // File view Icons
    GString *file_audio_png;
    GString *file_video_png;
    GString *file_playlist_png;
    GString *file_album_png;
    GString *file_textfile_png;
    GString *file_generic_png;
    GString *file_folder_png;
    GString *file_image_png;

    // Misc Utility function;
    void setFilePaths(int argc, char *argv[]);
    gchar *getRuntimePath(int argc, char *argv[]);

    // Common magic numbers.

#define MEGABYTE 1048576

#ifdef  __cplusplus
}
#endif

#endif  /* _MAIN_H */
