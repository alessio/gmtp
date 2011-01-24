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
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
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

GtkWidget *windowMain;
GtkWidget *windowPrefsDialog;
GtkWidget *windowPropDialog;
GtkWidget *windowPlaylistDialog;
GtkWidget *windowStatusBar;
GtkWidget *toolbuttonConnect;
GtkWidget *treeviewFiles;

Device_Struct DeviceMgr;

LIBMTP_file_t   *deviceFiles;
LIBMTP_folder_t *deviceFolders;
LIBMTP_track_t *deviceTracks;
LIBMTP_playlist_t *devicePlayLists;
uint32_t currentFolderID;

gchar *applicationpath = NULL;
GString *file_icon_png;
GString *file_icon16_png;
GString *file_about_png;

int
main (int argc, char *argv[])
{
    setFilePaths(argc, argv);

    gtk_set_locale ();
	gtk_init (&argc, &argv);

    #ifdef ENABLE_NLS
        //setlocale(LC_ALL, "");  // This is done in gtk_set_locale();
        bindtextdomain (PACKAGE, g_strconcat(applicationpath, "/../share/locale", NULL));
        //bindtextdomain (PACKAGE, "/usr/local/share/locale");
        bind_textdomain_codeset (PACKAGE, "UTF-8");
        textdomain (PACKAGE);
    #endif

	LIBMTP_Init();

   	windowMain = create_windowMain ();
	gtk_widget_show (windowMain);

	//Set default state for application
	DeviceMgr.deviceConnected = FALSE;
	statusBarSet(_("No device attached"));
	SetToolbarButtonState(DeviceMgr.deviceConnected);

    setupPreferences();

	// If preference is to auto-connect then attempt to do so.
	if(Preferences.attemptDeviceConnectOnStart == TRUE)
		on_deviceConnect_activate(NULL, NULL);

	// If we do have a connected device, then do a rescan operation to fill in the filelist.
	if(DeviceMgr.deviceConnected == TRUE)
		deviceRescan();

	gtk_main ();
	return 0;
}

void setFilePaths(int argc, char *argv[]){
    applicationpath = getRuntimePath(argc, argv);
    file_icon_png = g_string_new(applicationpath);
    file_icon_png = g_string_append(file_icon_png, "/../share/gmtp/icon.png");
    file_icon16_png = g_string_new(applicationpath);
    file_icon16_png = g_string_append(file_icon16_png, "/../share/gmtp/icon-16.png");
    file_about_png = g_string_new(applicationpath);
    file_about_png = g_string_append(file_about_png, "/../share/gmtp/stock-about-16.png");
}

gchar *getRuntimePath(int argc, char *argv[]){
    //gint i;
    gchar *fullpath;
    gchar *filepath;
    gchar *foundpath = NULL;
    const char delimit[]=";:";
    gchar *token;
    
    // Prints arguments
    /*g_printf("Arguments:\n");
    for (i = 0; i < argc; i++) {
        g_printf("%i: %s\n", i, argv[i]);
    }*/
    if(g_ascii_strcasecmp(PACKAGE, argv[0]) == 0){
        //g_printf("Executed from %%PATH\n");
        //g_printf("%%PATH = %s\n", getenv("PATH"));
        // list each directory individually.
        fullpath = g_strdup(getenv("PATH"));
        token = strtok (fullpath, delimit);
        while((token != NULL)&&(foundpath == NULL)){
            //g_printf("Path = %s\n", token);
            // Now test to see if we have it here...
            filepath = g_strdup(token);
            filepath = g_strconcat(filepath, "/", PACKAGE, NULL);
            //g_printf("Testing %s\n", filepath);
            if(access(filepath, F_OK) != -1){
               // g_printf("Found the file: %s\n", filepath);
                foundpath = g_strdup(token);
            }
            token = strtok(NULL, delimit);
            g_free(filepath);
        }
    } else {
        foundpath = g_strdup(dirname(argv[0]));
    }
    if(argc == 3){
        // We have some other options, lets check for --datapath
        if(g_ascii_strcasecmp("--datapath", argv[1]) == 0){
            // our first argument is --datapath, so set the path to argv[2];
            foundpath = g_strdup(argv[2]);
        }
    }
    //if(foundpath != NULL)
        //g_printf("Executed from %s\n", foundpath );
    return foundpath;
}
