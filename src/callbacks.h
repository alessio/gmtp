/* 
 *
 *   File: callbacks.h
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

#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

    // Main window functions.
    void on_quit1_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_preferences1_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_about1_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_deviceConnect_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_deviceProperties_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_deviceRescan_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_filesAdd_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_filesDelete_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_filesDownload_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_fileNewFolder_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_fileRemoveFolder_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_fileRenameFile_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_editDeviceName_activate(GtkMenuItem *menuitem, gpointer user_data);
    void  on_editFormatDevice_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_editAddAlbumArt_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_editPlaylist_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_view_activate(GtkMenuItem *menuitem, gpointer user_data);

    // Treeview handling.
    void fileListRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data);
    gboolean on_windowMainContextMenu_activate(GtkWidget *widget, GdkEvent *event);

    // Preferences Dialog
    void on_quitPrefs_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_PrefsDevice_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_PrefsAskDownload_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_PrefsDownloadPath_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_PrefsUploadPath_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_PrefsConfirmDelete_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_PrefsConfirmOverWriteFileOp_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_PrefsAutoAddTrackPlaylist_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_PrefsIgnorePathInPlaylist_activate(GtkMenuItem *menuitem, gpointer user_data);

    // Properties Dialog
    void on_quitProp_activate(GtkMenuItem *menuitem, gpointer user_data);

    // Add Album Art Dialog
    void on_buttonAlbumArtAdd_activate(GtkWidget *button, gpointer user_data);
    void on_buttonAlbumArtDelete_activate(GtkWidget *button, gpointer user_data);
    void on_buttonAlbumArtDownload_activate(GtkWidget *button, gpointer user_data);
    void on_albumtextbox_activate (GtkComboBox *combobox, gpointer user_data);

    // Playlist Dialog
    void on_quitPlaylist_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_NewPlaylistButton_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_ImportPlaylistButton_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_ExportPlaylistButton_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_DelPlaylistButton_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_DelFileButton_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_AddFileButton_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_FileUpButton_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_FileDownButton_activate(GtkMenuItem *menuitem, gpointer user_data);
    void on_Playlist_Combobox_activate(GtkComboBox *combobox, gpointer user_data);

    // Progress Dialog
    void on_progressDialog_Close(GtkWidget *window, gpointer user_data);
    void on_progressDialog_Cancel(GtkWidget *window, gpointer user_data);

    // Format Device Progress Bar.
    void on_editFormatDevice_thread(void);

    // Add Track to Playlist option.
    void on_TrackPlaylist_NewPlaylistButton_activate(GtkWidget *button, gpointer user_data);

#ifdef  __cplusplus
}
#endif

#endif  /* _CALLBACKS_H */
