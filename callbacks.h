/*
 * File:   callbacks.h
 * Author: darran
 *
 * Created on October 29, 2009, 5:01 PM
 */

#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

enum fileListID {
	COL_FILENAME = 0,
	COL_FILESIZE,
	COL_FILEID,
	COL_ISFOLDER,
    COL_FILESIZE_HID,
	NUM_COLUMNS

};

void on_quit1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_preferences1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_about1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_deviceConnect_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_deviceProperties_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_deviceRescan_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_filesAdd_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_filesDelete_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_filesDownload_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_fileNewFolder_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_fileRemoveFolder_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_editDeviceName_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_editAddAlbumArt_activate (GtkMenuItem *menuitem, gpointer user_data);

// Treeview handling.
void fileListRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data);
gboolean on_windowMainContextMenu_activate (GtkWidget *widget, GdkEvent *event);

// Preferences Dialog
void on_quitPrefs_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_PrefsDevice_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_PrefsAskDownload_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_PrefsDownloadPath_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_PrefsUploadPath_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_PrefsConfirmDelete_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_PrefsConfirmOverWriteFileOp_activate(GtkMenuItem *menuitem, gpointer user_data);

// Properties Dialog
void on_quitProp_activate (GtkMenuItem *menuitem, gpointer user_data);

// Add Album Art Dialog
void on_buttonFilePath_activate (GtkMenuItem *menuitem, gpointer user_data);

#ifdef  __cplusplus
}
#endif

#endif  /* _CALLBACKS_H */
