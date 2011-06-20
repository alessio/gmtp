/* 
 *
 *   File: main.c
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

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>
#include <locale.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#if GMTP_USE_GTK2
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#else
#include <gio/gio.h>
#endif
#include <libmtp.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <id3tag.h>

#include "main.h"
#include "interface.h"
#include "callbacks.h"
#include "mtp.h"
#include "prefs.h"
#include "dnd.h"

// Global Widgets needed by various functions.
GtkWidget *windowMain;
GtkWidget *windowPrefsDialog;
GtkWidget *windowPropDialog;
GtkWidget *windowPlaylistDialog;
GtkWidget *windowStatusBar;
GtkWidget *toolbuttonConnect;
GtkWidget *treeviewFiles;

// The device which we are connected with.
Device_Struct DeviceMgr;

// Device structures for files, folders, etc.
LIBMTP_file_t *deviceFiles = NULL;
LIBMTP_folder_t *deviceFolders = NULL;
LIBMTP_track_t *deviceTracks = NULL;
LIBMTP_playlist_t *devicePlayLists = NULL;
uint32_t currentFolderID = 0;

// Paths to the application, and images used within the application.
gchar *applicationpath = NULL;
GString *file_icon_png = NULL;
GString *file_icon16_png = NULL;
GString *file_about_png = NULL;
// File view Icons
GString *file_audio_png = NULL;
GString *file_video_png = NULL;
GString *file_playlist_png = NULL;
GString *file_album_png = NULL;
GString *file_textfile_png = NULL;
GString *file_generic_png = NULL;
GString *file_folder_png = NULL;
GString *file_image_png = NULL;


// ************************************************************************************************

/**
 * Main Function
 * @param argc - Number of arguments to the function
 * @param argv - Argument list
 * @return - exit code
 */
int main(int argc, char *argv[]) {
    setFilePaths(argc, argv);

#if GMTP_USE_GTK2
    gtk_set_locale();
#endif
    gtk_init(&argc, &argv);

#ifdef ENABLE_NLS
    bindtextdomain(PACKAGE, g_strconcat(applicationpath, "/../share/locale", NULL));
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);
#endif

    // Initialise libmtp library
    LIBMTP_Init();
    //LIBMTP_Set_Debug(LIBMTP_DEBUG_ALL);

    // Create our main window for the application.
    windowMain = create_windowMain();
    gtk_widget_show(windowMain);

    //Set default state for application
    DeviceMgr.deviceConnected = FALSE;
    statusBarSet(_("No device attached"));
    SetToolbarButtonState(DeviceMgr.deviceConnected);

    setupPreferences();

    // If preference is to auto-connect then attempt to do so.
    if (Preferences.attemptDeviceConnectOnStart == TRUE)
        on_deviceConnect_activate(NULL, NULL);

    // If we do have a connected device, then do a rescan operation to fill in the filelist.
    if (DeviceMgr.deviceConnected == TRUE)
        deviceRescan();

    gtk_main();
    
    return EXIT_SUCCESS;
} // end main()

// ************************************************************************************************

/**
 * setFilePaths - set paths for image used within gMTP
 * @param argc
 * @param argv
 */
void setFilePaths(int argc, char *argv[]) {
    // Get our executable location.
    applicationpath = getRuntimePath(argc, argv);

    // Set our image locations.
    file_icon_png = g_string_new(applicationpath);
    file_icon_png = g_string_append(file_icon_png, "/../share/gmtp/icon.png");
    file_icon16_png = g_string_new(applicationpath);
    file_icon16_png = g_string_append(file_icon16_png, "/../share/gmtp/icon-16.png");
    file_about_png = g_string_new(applicationpath);
    file_about_png = g_string_append(file_about_png, "/../share/gmtp/stock-about-16.png");

    file_audio_png = g_string_new(applicationpath);
    file_audio_png = g_string_append(file_audio_png, "/../share/gmtp/audio-x-mpeg.png");;
    file_video_png = g_string_new(applicationpath);
    file_video_png = g_string_append(file_video_png, "/../share/gmtp/video-x-generic.png");;
    file_playlist_png = g_string_new(applicationpath);
    file_playlist_png = g_string_append(file_playlist_png, "/../share/gmtp/audio-x-mp3-playlist.png");;
    file_album_png = g_string_new(applicationpath);
    file_album_png = g_string_append(file_album_png, "/../share/gmtp/media-cdrom-audio.png");;
    file_textfile_png = g_string_new(applicationpath);
    file_textfile_png = g_string_append(file_textfile_png, "/../share/gmtp/text-plain.png");;
    file_generic_png = g_string_new(applicationpath);
    file_generic_png = g_string_append(file_generic_png, "/../share/gmtp/empty.png");;
    file_folder_png = g_string_new(applicationpath);
    file_folder_png = g_string_append(file_folder_png, "/../share/gmtp/folder.png");;
    file_image_png = g_string_new(applicationpath);
    file_image_png = g_string_append(file_image_png, "/../share/gmtp/image-x-generic.png");;
} // end setFilePaths()

// ************************************************************************************************

/**
 * getRuntimePath - Returns the path which the application was run from
 * @param argc
 * @param argv
 * @return pointer to string with location of the binary.
 */
gchar *getRuntimePath(int argc, char *argv[]) {

    gchar *fullpath;
    gchar *filepath;
    gchar *foundpath = NULL;
    const char delimit[] = ";:";
    gchar *token;

    if (g_ascii_strcasecmp(PACKAGE, argv[0]) == 0) {
        // list each directory individually.
        fullpath = g_strdup(getenv("PATH"));
        token = strtok(fullpath, delimit);
        while ((token != NULL) && (foundpath == NULL)) {
            // Now test to see if we have it here...
            filepath = g_strdup(token);
            filepath = g_strconcat(filepath, "/", PACKAGE, NULL);
            if (access(filepath, F_OK) != -1) {
                foundpath = g_strdup(token);
            }
            token = strtok(NULL, delimit);
            g_free(filepath);
        }
    } else {
        // We were started with full file path.
        foundpath = g_strdup(dirname(argv[0]));
    }
    if (argc == 3) {
        // We have some other options, lets check for --datapath
        if (g_ascii_strcasecmp("--datapath", argv[1]) == 0) {
            // our first argument is --datapath, so set the path to argv[2];
            foundpath = g_strdup(argv[2]);
        }
    }
    return foundpath;
} // end getRuntimePath
