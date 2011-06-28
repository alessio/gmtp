/* 
 *
 *   File: callbacks.c
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

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#if GMTP_USE_GTK2
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#else
#include <gio/gio.h>
#endif
#include <gtk/gtk.h>
#include <libmtp.h>
#include <id3tag.h>

#include "main.h"
#include "callbacks.h"
#include "interface.h"
#include "mtp.h"
#include "prefs.h"
#include "dnd.h"

static gboolean formatThreadWorking = TRUE;

/**
 * on_quit1_activate - Call back for Quit toolbar and menu option.
 * @param menuitem
 * @param user_data
 */
void on_quit1_activate(GtkMenuItem *menuitem, gpointer user_data) {
    savePreferences();
#if GMTP_USE_GTK2
    gtk_exit(EXIT_SUCCESS);
#else
    exit(EXIT_SUCCESS);
#endif
} // end on_quit1_activate()

// ************************************************************************************************

/**
 * on_about1_activate - Call back for displaying the About Dialog Box
 * @param menuitem
 * @param user_data
 */
void on_about1_activate(GtkMenuItem *menuitem, gpointer user_data) {
    displayAbout();
} // end on_about1_activate()

// ************************************************************************************************

/**
 * on_deviceProperties_activate - Callback for displaying the device Properties Dialog box.
 * @param menuitem
 * @param user_data
 */
void on_deviceProperties_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gchar *tmp_string;

    // We confirm our device properties, this should setup the device structure information we use below.
    deviceProperties();

    // Update the status bar with our information.
    if (DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE) {
        tmp_string = g_strdup_printf(_("Connected to %s - %d MB free"), DeviceMgr.devicename->str,
            (int) (DeviceMgr.devicestorage->FreeSpaceInBytes / MEGABYTE));
    } else {
        if (DeviceMgr.devicestorage->StorageDescription != NULL) {
            tmp_string = g_strdup_printf(_("Connected to %s (%s) - %d MB free"),
                DeviceMgr.devicename->str,
                DeviceMgr.devicestorage->StorageDescription,
                (int) (DeviceMgr.devicestorage->FreeSpaceInBytes / MEGABYTE));
        } else {
            tmp_string = g_strdup_printf(_("Connected to %s - %d MB free"), DeviceMgr.devicename->str,
                (int) (DeviceMgr.devicestorage->FreeSpaceInBytes / MEGABYTE));
        }
    }
    statusBarSet(tmp_string);
    g_free(tmp_string);

    // No idea how this could come about, but we should take it into account so we don't have a
    // memleak due to recreating the window multiple times.
    if (windowPropDialog != NULL) {
        gtk_widget_hide(windowPropDialog);
        gtk_widget_destroy(windowPropDialog);
    }

    // Create and show the dialog box.
    windowPropDialog = create_windowProperties();
    gtk_widget_show(GTK_WIDGET(windowPropDialog));
} // end on_deviceProperties_activate()

// ************************************************************************************************

/**
 * on_quitProp_activate - Callback used to close the Properties Dialog.
 * @param menuitem
 * @param user_data
 */
void on_quitProp_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gtk_widget_hide(windowPropDialog);
    gtk_widget_destroy(windowPropDialog);
    windowPropDialog = NULL;
} // end on_quitProp_activate()

// ************************************************************************************************

/**
 * on_deviceRescan_activate - Callback to rescan the device properties and update the main
 * application window.
 * @param menuitem
 * @param user_data
 */
void on_deviceRescan_activate(GtkMenuItem *menuitem, gpointer user_data) {
    deviceRescan();
} // end on_deviceRescan_activate()

// ************************************************************************************************

/**
 * on_filesAdd_activate - Callback to initiate an Add Files operation.
 * @param menuitem
 * @param user_data
 */
void on_filesAdd_activate(GtkMenuItem *menuitem, gpointer user_data) {
    GSList* files;

    files = getFileGetList2Add();
    if (files != NULL)
        g_slist_foreach(files, (GFunc) __filesAdd, NULL);

    // Now clear the GList;
    g_slist_foreach(files, (GFunc) g_free, NULL);
    g_slist_free(files);

    // Now do a device rescan to see the new files.
    deviceRescan();
    deviceoverwriteop = MTP_ASK;
} // end on_filesAdd_activate()

// ************************************************************************************************

/**
 * Callback to handle the Rename Device menu option.
 * @param menuitem
 * @param user_data
 */
void on_fileRenameFile_activate(GtkMenuItem *menuitem, gpointer user_data) {

    GtkTreePath *path;
    GtkTreeIter iter;
    gchar *newfilename = NULL;
    gchar *filename = NULL;
    gboolean isFolder;
    uint32_t ObjectID = 0;

    // Let's check to see if we have anything selected in our treeview?
    if (fileListGetSelection() == NULL) {
        displayInformation(_("No files/folders selected?"));
        return;
    }
    GList *List = fileListGetSelection();

    // We only care about the first entry.
    // convert the referenece to a path and retrieve the iterator;
    path = gtk_tree_row_reference_get_path(List->data);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
    // We have our Iter now.
    // Before we download, is it a folder ?
    gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_FILENAME_ACTUAL, &filename, COL_ISFOLDER, &isFolder,
        COL_FILEID, &ObjectID, -1);

    // Make sure we are not attempting to edit the parent link folder.
    if (g_ascii_strcasecmp(filename, "..") == 0) {
        g_fprintf(stderr, _("Unable to rename parent folder\n"));
        displayInformation(_("Unable to rename this folder"));
        return;
    }
    // Get our new device name.
    newfilename = displayRenameFileDialog(filename);

    // If the user supplied something, then update the name of the device.
    if (newfilename != NULL) {
        filesRename(newfilename, ObjectID);
        g_free(newfilename);
        deviceRescan();
    }
} // end on_editDeviceName_activate()

// ************************************************************************************************

/**
 * on_filesDelete_activate - Callback to initiate a Delete Files operation.
 * @param menuitem
 * @param user_data
 */
void on_filesDelete_activate(GtkMenuItem *menuitem, gpointer user_data) {
    GtkWidget *dialog;

    // Let's check to see if we have anything selected in our treeview?
    if (fileListGetSelection() == NULL) {
        displayInformation(_("No files/folders selected?"));
        return;
    }

    // Now we prompt to confirm delete?
    if (Preferences.confirm_file_delete_op == FALSE) {
        // Now download the actual file from the MTP device.
        fileListRemove(fileListGetSelection());
    } else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(windowMain),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_YES_NO,
            _("Are you sure you want to delete these files?"));
        gtk_window_set_title(GTK_WINDOW(dialog), _("Confirm Delete"));

        // Run the Dialog and get our result.
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        if (result == GTK_RESPONSE_YES)
            fileListRemove(fileListGetSelection());

        // Destroy the dialog box.
        gtk_widget_destroy(dialog);
    }
} // on_filesDelete_activate()

// ************************************************************************************************

/**
 * on_filesDownload_activate - Callback to initiate a download files operation.
 * @param menuitem
 * @param user_data
 */
void on_filesDownload_activate(GtkMenuItem *menuitem, gpointer user_data) {

    // Let's check to see if we have anything selected in our treeview?
    if (fileListGetSelection() == NULL) {
        displayInformation(_("No files/folders selected?"));
        return;
    }

    // Download the selected files.
    fileListDownload(fileListGetSelection());
} // end on_filesDownload_activate()

// ************************************************************************************************

/**
 * on_deviceConnect_activate - Callback used to connect a device to the application.
 * @param menuitem
 * @param user_data
 */
void on_deviceConnect_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gchar *tmp_string;
    GtkWidget *menuText;

    deviceConnect();
    //g_printf("Device connect/disconnect code = %d\n", result);
    // Update our label to indicate current condition.
    if (DeviceMgr.deviceConnected == TRUE) {
        // Set up our properties.
        deviceProperties();
        deviceRescan();

        // Update the toolbar to show a disconnect string.
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolbuttonConnect), _("Disconnect"));

        // Now update the status bar;
        if (DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE) {
            tmp_string = g_strdup_printf(_("Connected to %s - %d MB free"), DeviceMgr.devicename->str,
                (int) (DeviceMgr.devicestorage->FreeSpaceInBytes / MEGABYTE));
        } else {
            if (DeviceMgr.devicestorage->StorageDescription != NULL) {
                tmp_string = g_strdup_printf(_("Connected to %s (%s) - %d MB free"),
                    DeviceMgr.devicename->str,
                    DeviceMgr.devicestorage->StorageDescription,
                    (int) (DeviceMgr.devicestorage->FreeSpaceInBytes / MEGABYTE));
            } else {
                tmp_string = g_strdup_printf(_("Connected to %s - %d MB free"),
                    DeviceMgr.devicename->str,
                    (int) (DeviceMgr.devicestorage->FreeSpaceInBytes / MEGABYTE));
            }
        }
        statusBarSet(tmp_string);
        g_free(tmp_string);

        // Now update the filemenu;
        menuText = gtk_bin_get_child(GTK_BIN(fileConnect));
        gtk_label_set_text(GTK_LABEL(menuText), _("Disconnect Device"));

        // Enable the Drag'n'Drop interface for the main window.
        gmtp_drag_dest_set(windowMain);

    } else {

        // Update the toolbar to show the Connect String.
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolbuttonConnect), _("Connect"));

        // Now update the status bar;
        statusBarSet(_("No device attached"));

        // Now update the filemenu;
        menuText = gtk_bin_get_child(GTK_BIN(fileConnect));
        gtk_label_set_text(GTK_LABEL(menuText), _("Connect Device"));

        // Now update the file list area and disable Drag'n'Drop.
        fileListClear();
        gtk_drag_dest_unset(windowMain);
    }

    // Update the Toolbar and Menus enabling/disabling the menu items.
    SetToolbarButtonState(DeviceMgr.deviceConnected);
} // on_deviceConnect_activate()

// Here is the preferences callbacks and dialog box creation.
// ************************************************************************************************

/**
 * Callback to show the Preferences Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_preferences1_activate(GtkMenuItem *menuitem, gpointer user_data) {
    // No idea how this could come about, but we should take it into account so we don't have a memleak
    // due to recreating the window multiple times.
    if (windowPrefsDialog != NULL) {
        gtk_widget_hide(windowPrefsDialog);
        gtk_widget_destroy(windowPrefsDialog);
    }

    // Create and display the dialog
    windowPrefsDialog = create_windowPreferences();
    gtk_widget_show(GTK_WIDGET(windowPrefsDialog));

} // end on_preferences1_activate()

// ************************************************************************************************

/**
 * Callback to close the Preferences Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_quitPrefs_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gtk_widget_hide(windowPrefsDialog);
    gtk_widget_destroy(windowPrefsDialog);
    windowPrefsDialog = NULL;
} // end on_quitPrefs_activate()

// ************************************************************************************************

/**
 * Callback for Auto Connect Device toggle in Preferences Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_PrefsDevice_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbuttonDeviceConnect));

#if GMTP_USE_GTK2
    if (gconfconnect != NULL)
        gconf_client_set_bool(gconfconnect, "/apps/gMTP/autoconnectdevice", state, NULL);
#else
    if (gsettings_connect != NULL)
        g_settings_set_boolean(gsettings_connect, "autoconnectdevice", state);
    g_settings_sync();
#endif
} // end on_PrefsDevice_activate()

// ************************************************************************************************

/**
 * Callback for Confirm Delete Operations toggle in Preferences Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_PrefsConfirmDelete_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmFileOp));

#if GMTP_USE_GTK2
    if (gconfconnect != NULL)
        gconf_client_set_bool(gconfconnect, "/apps/gMTP/confirmFileDelete", state, NULL);
#else
    if (gsettings_connect != NULL)
        g_settings_set_boolean(gsettings_connect, "confirmfiledelete", state);
    g_settings_sync();
#endif
} // end on_PrefsConfirmDelete_activate()

// ************************************************************************************************

/**
 * Callback for Ask Download Path Operations toggle in Preferences Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_PrefsAskDownload_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbuttonDownloadPath));

#if GMTP_USE_GTK2
    if (gconfconnect != NULL)
        gconf_client_set_bool(gconfconnect, "/apps/gMTP/promptDownloadPath", state, NULL);
#else
    if (gsettings_connect != NULL)
        g_settings_set_boolean(gsettings_connect, "promptdownloadpath", state);
    g_settings_sync();
#endif
} // end on_PrefsAskDownload_activate()

// ************************************************************************************************

/**
 * Callback for Confirm Overwrite of File Operations toggle in Preferences Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_PrefsConfirmOverWriteFileOp_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmOverWriteFileOp));

#if GMTP_USE_GTK2
    if (gconfconnect != NULL)
        gconf_client_set_bool(gconfconnect, "/apps/gMTP/promptOverwriteFile", state, NULL);
#else
    if (gsettings_connect != NULL)
        g_settings_set_boolean(gsettings_connect, "promptoverwritefile", state);
    g_settings_sync();
#endif
} // end on_PrefsConfirmOverWriteFileOp_activate()

// ************************************************************************************************

/**
 * Callback for setting download path in Preferences Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_PrefsDownloadPath_activate(GtkMenuItem *menuitem, gpointer user_data) {
    // What we do here is display a find folder dialog, and save the resulting folder into the text wigdet and preferences item.
    gchar *savepath = NULL;
    GtkWidget *FileDialog;
    // First of all, lets set the download path.
    FileDialog = gtk_file_chooser_dialog_new(_("Select Path to Download to"),
        GTK_WINDOW(windowPrefsDialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
        NULL);
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemDownloadPath->str);
    if (gtk_dialog_run(GTK_DIALOG(FileDialog)) == GTK_RESPONSE_ACCEPT) {
        savepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileDialog));

        // Save our download path.
#if GMTP_USE_GTK2
        if (gconfconnect != NULL)
            gconf_client_set_string(gconfconnect, "/apps/gMTP/DownloadPath", savepath, NULL);
#else
        if (gsettings_connect != NULL)
            g_settings_set_string(gsettings_connect, "downloadpath", savepath);
        g_settings_sync();
#endif
        g_free(savepath);
    }
    gtk_widget_destroy(FileDialog);
} // on_PrefsDownloadPath_activate()

// ************************************************************************************************

/**
 * Callback for setting upload path in Preferences Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_PrefsUploadPath_activate(GtkMenuItem *menuitem, gpointer user_data) {
    // What we do here is display a find folder dialog, and save the resulting folder into the text wigdet and preferences item.
    gchar *savepath = NULL;
    GtkWidget *FileDialog;
    // First of all, lets set the upload path.
    FileDialog = gtk_file_chooser_dialog_new(_("Select Path to Upload From"),
        GTK_WINDOW(windowPrefsDialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
        NULL);
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemUploadPath->str);
    if (gtk_dialog_run(GTK_DIALOG(FileDialog)) == GTK_RESPONSE_ACCEPT) {
        savepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileDialog));

        // Save our download path.
#if GMTP_USE_GTK2
        if (gconfconnect != NULL)
            gconf_client_set_string(gconfconnect, "/apps/gMTP/UploadPath", savepath, NULL);
#else
        if (gsettings_connect != NULL)
            g_settings_set_string(gsettings_connect, "uploadpath", savepath);
        g_settings_sync();
#endif
        g_free(savepath);
    }
    gtk_widget_destroy(FileDialog);
} //on_PrefsUploadPath_activate()

// ************************************************************************************************

/**
 * Callback to handle double click on item in main window. If it's a folder, then change to it,
 * other attempt to download the file(s).
 * @param treeview
 * @param path
 * @param column
 * @param data
 */
void fileListRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data) {
    GtkTreeModel *model;
    GtkTreeIter iter;

    gchar *filename = NULL;
    gboolean isFolder;
    uint32_t objectID;

    // Obtain the iter, and the related objectID.
    model = gtk_tree_view_get_model(treeview);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_ISFOLDER, &isFolder, COL_FILENAME, &filename, COL_FILEID, &objectID, -1);
        if (isFolder == FALSE) {
            // Now download the actual file from the MTP device.
            filesDownload(filename, objectID);
        } else {
            // We have a folder so change to it?
            currentFolderID = objectID;
            fileListClear();
            fileListAdd();
        }
    }
    g_free(filename);
} // end fileListRowActivated()

// ************************************************************************************************

/**
 * Callback to handle selecting NewFolder from menu or toolbar.
 * @param menuitem
 * @param user_data
 */
void on_fileNewFolder_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gchar *foldername = NULL;

    // Get the folder name by displaying a dialog.
    foldername = displayFolderNewDialog();
    if (foldername != NULL) {
        // Add in folder to MTP device.
        folderAdd(foldername);
        g_free(foldername);
        deviceRescan();
    }
} // end on_fileNewFolder_activate()

// ************************************************************************************************

/**
 * Callback handle to handle deleting a folder menu option.
 * @param menuitem
 * @param user_data
 */
void on_fileRemoveFolder_activate(GtkMenuItem *menuitem, gpointer user_data) {
    GtkWidget *dialog;

    // Let's check to see if we have anything selected in our treeview?
    if (fileListGetSelection() == NULL) {
        displayInformation(_("No files/folders selected?"));
        return;
    }

    // Now we prompt to confirm delete?
    if (Preferences.confirm_file_delete_op == FALSE) {
        // Now download the actual file from the MTP device.
        folderListRemove(fileListGetSelection());
    } else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(windowMain),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_YES_NO,
            _("Are you sure you want to delete this folder (and all contents)?"));
        gtk_window_set_title(GTK_WINDOW(dialog), _("Confirm Delete"));
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        if (result == GTK_RESPONSE_YES)
            folderListRemove(fileListGetSelection());
        gtk_widget_destroy(dialog);
    }
} // end on_fileRemoveFolder_activate()

// ************************************************************************************************

/**
 * Callback to handle the Rename Device menu option.
 * @param menuitem
 * @param user_data
 */
void on_editDeviceName_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gchar *devicename = NULL;
    gchar *tmp_string = NULL;

    // Get our new device name.
    devicename = displayChangeDeviceNameDialog(DeviceMgr.devicename->str);

    // If the user supplied something, then update the name of the device.
    if (devicename != NULL) {
        // add change to MTP device.
        setDeviceName(devicename);
        g_free(devicename);
        // Attempt to read it back as confirmation that something may of happened.
        tmp_string = LIBMTP_Get_Friendlyname(DeviceMgr.device);
        if (tmp_string == NULL) {
            DeviceMgr.devicename = g_string_new(_("N/A"));
        } else {
            DeviceMgr.devicename = g_string_new(tmp_string);
            g_free(tmp_string);
        }
        // Perform a device Rescan operation to reset all device parameters.
        deviceRescan();
    }
} // end on_editDeviceName_activate()

// ************************************************************************************************

/**
 * Callback to format the current storage device.
 * @param menuitem
 * @param user_data
 */
void on_editFormatDevice_activate(GtkMenuItem *menuitem, gpointer user_data) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(GTK_WINDOW(windowMain),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO,
        _("Are you sure you want to format this device?"));
    gtk_window_set_title(GTK_WINDOW(dialog), _("Format Device"));
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_hide(GTK_WIDGET(dialog));
    gtk_widget_destroy(dialog);

    if (result == GTK_RESPONSE_YES) {
        dialog = create_windowFormat();
        // Show progress dialog.
        gtk_widget_show_all(dialog);
        // Ensure GTK redraws the window.

        formatThreadWorking = TRUE;

        g_thread_create((GThreadFunc) on_editFormatDevice_thread, NULL, FALSE, NULL);

        while (formatThreadWorking) {
            while (gtk_events_pending())
                gtk_main_iteration();

            if (formatDialog_progressBar1 != NULL) {
                gtk_progress_bar_pulse(GTK_PROGRESS_BAR(formatDialog_progressBar1));
                g_usleep(G_USEC_PER_SEC * 0.1);
            }

        }
        // The worker thread has finished so let's continue.

        // Disconnect and reconnect the device.
        on_deviceConnect_activate(NULL, NULL);
        // Sleep for 2 secs to allow the device to settle itself
        g_usleep(G_USEC_PER_SEC * 2);
        on_deviceConnect_activate(NULL, NULL);
        // Close progress dialog.
        gtk_widget_hide(dialog);
        gtk_widget_destroy(dialog);
        formatDialog_progressBar1 = NULL;
    }
    //
} // end on_editFormatDevice_activate()

// ************************************************************************************************

/**
 * Worker thread for on_editFormatDevice_activate();
 */
void on_editFormatDevice_thread(void) {
    formatStorageDevice();
    // Add a 5 sec wait so the device has time to settle itself.
    g_usleep(G_USEC_PER_SEC * 5);
    formatThreadWorking = FALSE;
    g_thread_exit(NULL);
}

// ************************************************************************************************

/**
 * Callback to handle the displaying of the context menu.
 * @param widget
 * @param event
 * @return
 */
gboolean on_windowMainContextMenu_activate(GtkWidget *widget, GdkEvent *event) {
    GtkMenu *menu;
    GdkEventButton *event_button;
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_MENU(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    /* The "widget" is the menu that was supplied when
     * g_signal_connect_swapped() was called.
     */
    menu = GTK_MENU(widget);
    if (event->type == GDK_BUTTON_PRESS) {
        event_button = (GdkEventButton *) event;
        if (event_button->button == 3) {
            gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
                event_button->button, event_button->time);
            return TRUE;
        }
    }
    return FALSE;
} // end on_windowMainContextMenu_activate()

// ************************************************************************************************

/**
 * Callback to handle the Add Album Art menu option.
 * @param menuitem
 * @param user_data
 */
void on_editAddAlbumArt_activate(GtkMenuItem *menuitem, gpointer user_data) {
    // Get a filename of the album art.
    displayAddAlbumArtDialog();

    // If the user supplied a filename, then attempt to upload the image.
    /*if (albumart != NULL) {
        albumAddArt(albumart->album_id, albumart->filename);

        // Free our memory allocations
        g_free(albumart->filename);
        g_free(albumart);
    }*/
} // end on_editAddAlbumArt_activate()

// ************************************************************************************************

/**
 * Callback to hanlde the Playlist menu/toolbar operations.
 * @param menuitem
 * @param user_data
 */
void on_editPlaylist_activate(GtkMenuItem *menuitem, gpointer user_data) {
    displayPlaylistDialog();
} // end on_editPlaylist_activate()

// ************************************************************************************************

/**
 * Callback to handle the select file button in the Add Album Art Dialog box.
 * @param button
 * @param user_data
 */
void on_buttonAlbumArtAdd_activate(GtkWidget *button, gpointer user_data) {
    // What we do here is display a find folder dialog, and save the resulting folder into the text wigdet and preferences item.
    //gchar *filename;
    gchar *filename = NULL;
    GtkWidget *FileDialog;
    FileDialog = gtk_file_chooser_dialog_new(_("Select Album Art File"),
        GTK_WINDOW(AlbumArtDialog), GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(FileDialog), TRUE);

    // Set the default path to be the normal upload folder.
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemUploadPath->str);

    if (gtk_dialog_run(GTK_DIALOG(FileDialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileDialog));

        if (filename != NULL) {
            // Upload the file to the selected album.
            gint selected = gtk_combo_box_get_active(GTK_COMBO_BOX(textboxAlbumArt));
            gint count = 0;
            LIBMTP_album_t *albumlist = LIBMTP_Get_Album_List_For_Storage(DeviceMgr.device, DeviceMgr.devicestorage->id);
            LIBMTP_album_t *albuminfo = albumlist;

            while (albuminfo != NULL) {
                if (count == selected) {
                    // Found our album, so update the image on the device, then update the display.
                    albumAddArt(albuminfo->album_id, filename);
                    AlbumArtUpdateImage(albuminfo);
                    clearAlbumStruc(albumlist);
                    g_free(filename);
                    gtk_widget_destroy(FileDialog);
                    return;
                }
                // Next album_entry
                albuminfo = albuminfo->next;
                count++;
            }
            // Set a default image as we didn't find our album.
            AlbumArtUpdateImage(NULL);
            clearAlbumStruc(albumlist);

            g_free(filename);
        }
    }
    gtk_widget_destroy(FileDialog);
} // end on_buttonAlbumArtAdd_activate()

// ************************************************************************************************

/**
 * Callback to handle removal of associated album art.
 * @param button
 * @param user_data
 */
void on_buttonAlbumArtDelete_activate(GtkWidget *button, gpointer user_data) {

    // Send a blank representation.
    gint selected = gtk_combo_box_get_active(GTK_COMBO_BOX(textboxAlbumArt));
    gint count = 0;
    LIBMTP_album_t *albumlist = LIBMTP_Get_Album_List_For_Storage(DeviceMgr.device, DeviceMgr.devicestorage->id);
    LIBMTP_album_t *albuminfo = albumlist;

    while (albuminfo != NULL) {
        if (count == selected) {
            // Found our album, so update the image on the device, then update the display.
            albumDeleteArt(albuminfo->album_id);
            AlbumArtUpdateImage(NULL);
            clearAlbumStruc(albumlist);
            return;
        }
        // Next album_entry
        albuminfo = albuminfo->next;
        count++;
    }
    // Set a default image as we didn't find our album.
    AlbumArtUpdateImage(NULL);
    clearAlbumStruc(albumlist);

} // end on_buttonAlbumArtDelete_activate()

// ************************************************************************************************

/**
 * Retrieve the album art and attempt to save the file.
 * @param button
 * @param user_data
 */
void on_buttonAlbumArtDownload_activate(GtkWidget *button, gpointer user_data) {
    FILE* fd;
    gint selected = gtk_combo_box_get_active(GTK_COMBO_BOX(textboxAlbumArt));
    gint count = 0;
    GtkWidget *FileDialog;
    gchar *filename = NULL;
    LIBMTP_filesampledata_t *imagedata = NULL;
    LIBMTP_album_t *albumlist = LIBMTP_Get_Album_List_For_Storage(DeviceMgr.device, DeviceMgr.devicestorage->id);
    LIBMTP_album_t *albuminfo = albumlist;

    // Scan our albums, looking for the correct one.
    while (albuminfo != NULL) {
        if (count == selected) {
            // Found our album, let's get our data..
            imagedata = albumGetArt(albuminfo);
            if (imagedata != NULL) {

                FileDialog = gtk_file_chooser_dialog_new(_("Save Album Art File"),
                    GTK_WINDOW(AlbumArtDialog), GTK_FILE_CHOOSER_ACTION_SAVE,
                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                    NULL);

                gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(FileDialog), TRUE);

                // Set the default path to be the normal download folder.
                gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(FileDialog),
                    Preferences.fileSystemDownloadPath->str);

                // Set a default name to be the album.JPG
                gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(FileDialog),
                    g_strdup_printf("%s.jpg", albuminfo->name));

                if (gtk_dialog_run(GTK_DIALOG(FileDialog)) == GTK_RESPONSE_ACCEPT) {
                    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileDialog));
                    if (filename != NULL) {
                        // The user has selected a file to save as, so do the deed.
                        fd = fopen(filename, "w");
                        if (fd == NULL) {
                            g_fprintf(stderr, _("Couldn't save image file %s\n"), filename);
                            displayError(_("Couldn't save image file\n"));
                        } else {
                            fwrite(imagedata->data, imagedata->size, 1, fd);
                            fclose(fd);
                        }
                        g_free(filename);
                    }
                }
                // Clean up our image data and dialog.
                LIBMTP_destroy_filesampledata_t(imagedata);
                gtk_widget_destroy(FileDialog);
            }
            clearAlbumStruc(albumlist);
            return;
        }
        // Next album_entry
        albuminfo = albuminfo->next;
        count++;
    }
    // Set a default image as we didn't find our album.
    clearAlbumStruc(albumlist);
    gtk_widget_destroy(FileDialog);
} // end on_buttonAlbumArtDownload_activate()

// ************************************************************************************************

/**
 * Update the Album Image in the Add Album Art Dialog Box.
 * @param menuitem
 * @param user_data
 */
void on_albumtextbox_activate(GtkComboBox *combobox, gpointer user_data) {
    gint selected = gtk_combo_box_get_active(combobox);
    gint count = 0;
    LIBMTP_album_t *albumlist = LIBMTP_Get_Album_List_For_Storage(DeviceMgr.device, DeviceMgr.devicestorage->id);
    LIBMTP_album_t *albuminfo = albumlist;

    while (albuminfo != NULL) {
        if (count == selected) {
            AlbumArtUpdateImage(albuminfo);
            clearAlbumStruc(albumlist);
            return;
        }
        // Text the album_entry
        albuminfo = albuminfo->next;
        count++;
    }
    // Set a default image
    AlbumArtUpdateImage(NULL);
    clearAlbumStruc(albumlist);
}

// Playlist Callbacks.
// ************************************************************************************************

/**
 * Callback to handle closing the Playlist editor dialog.
 * @param menuitem
 * @param user_data
 */
void on_quitPlaylist_activate(GtkMenuItem *menuitem, gpointer user_data) {
    // Save our current selected playlist!
    if (devicePlayLists != NULL)
        playlist_SavePlaylist(playlist_number);
    // Kill our window
    gtk_widget_hide(windowPlaylistDialog);
    gtk_widget_destroy(windowPlaylistDialog);
    windowPlaylistDialog = NULL;
    // Do a device rescan to show the new playlists in the file window
    deviceRescan();
} // end on_quitPlaylist_activate()

// ************************************************************************************************

/**
 * Callback to handle the new Playlist button in the Playlist editor dialog.
 * @param menuitem
 * @param user_data
 */
void on_Playlist_NewPlaylistButton_activate(GtkMenuItem *menuitem, gpointer user_data) {
    //g_printf("Clicked on new playlist button\n");
    gchar *playlistname = NULL;

    // Save our current selected playlist!
    if (devicePlayLists != NULL)
        playlist_SavePlaylist(playlist_number);

    playlistname = displayPlaylistNewDialog();
    if (playlistname != NULL) {
        // Add in playlist to MTP device.
        playlistAdd(playlistname);
        // Refresh our playlist information.
        devicePlayLists = getPlaylists();
        gtk_list_store_clear(GTK_LIST_STORE(playlist_PL_List));
        // Add it to our combobox

#if GMTP_USE_GTK2
        gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry_playlist), g_strdup(playlistname));
#else
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboboxentry_playlist), g_strdup(playlistname));
#endif
        g_free(playlistname);

        // Set the active combobox item.
        comboboxentry_playlist_entries++;
        playlist_number = comboboxentry_playlist_entries - 1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxentry_playlist), comboboxentry_playlist_entries - 1);
        SetPlaylistButtonState(TRUE);
        setPlaylistField(playlist_number);
    }
} // end on_Playlist_NewPlaylistButton_activate()

// ************************************************************************************************

/**
 * Callback to handle the Delete Playlist button in the Playlist editor dialog.
 * @param menuitem
 * @param user_data
 */
void on_Playlist_DelPlaylistButton_activate(GtkMenuItem *menuitem, gpointer user_data) {

    gint PlayListID = gtk_combo_box_get_active(GTK_COMBO_BOX(comboboxentry_playlist));

    if (PlayListID != -1) {
        // We have something selected so lets do the dance.
        LIBMTP_playlist_t* tmpplaylist = devicePlayLists;
        if (PlayListID > 0) {
            while (PlayListID--)
                if (tmpplaylist->next != NULL)
                    tmpplaylist = tmpplaylist->next;
        }
        // We should be in the correct playlist LIBMTP structure.
        playlistDelete(tmpplaylist);
        // Clear the PL list view box
        gtk_list_store_clear(GTK_LIST_STORE(playlist_PL_List));
        // Rebuild the playlist structure and combobox.
        devicePlayLists = getPlaylists();
        setPlayListComboBox();
    }
} // end on_Playlist_DelPlaylistButton_activate()

// ************************************************************************************************

/**
 * Callback to handle the Delete Track button in the Playlist editor dialog.
 * @param menuitem
 * @param user_data
 */
void on_Playlist_DelFileButton_activate(GtkMenuItem *menuitem, gpointer user_data) {
    if (playlist_PL_ListGetSelection() == NULL)
        return;
    playlist_PL_ListRemove(playlist_PL_ListGetSelection());
} // end on_Playlist_DelFileButton_activate()

// ************************************************************************************************

/**
 * Callback to handle the Add Track button in the Playlist editor dialog.
 * @param menuitem
 * @param user_data
 */
void on_Playlist_AddFileButton_activate(GtkMenuItem *menuitem, gpointer user_data) {
    //g_printf("Clicked on add file in playlist button\n");
    if (playlist_TrackList_GetSelection() == NULL)
        return;
    playlist_TrackList_Add(playlist_TrackList_GetSelection());
} // end on_Playlist_AddFileButton_activate()

// ************************************************************************************************

/**
 * Callback to handle the Move Track Up button in the Playlist editor dialog.
 * @param menuitem
 * @param user_data
 */
void on_Playlist_FileUpButton_activate(GtkMenuItem *menuitem, gpointer user_data) {
    playlist_move_files(-1);
} // end on_Playlist_FileUpButton_activate()

// ************************************************************************************************

/**
 * Callback to handle the Move Track Down button in the Playlist editor dialog.
 * @param menuitem
 * @param user_data
 */
void on_Playlist_FileDownButton_activate(GtkMenuItem *menuitem, gpointer user_data) {
    playlist_move_files(1);
} // end on_Playlist_FileDownButton_activate()

// ************************************************************************************************

/**
 * Callback to handle the change of Playlist selection in the Playlist editor dialog.
 * @param menuitem
 * @param user_data
 */
void on_Playlist_Combobox_activate(GtkComboBox *combobox, gpointer user_data) {
    // Save our current selected playlist
    playlist_SavePlaylist(playlist_number);
    // Get our new playlist ID, and display the contents of it.
    playlist_number = gtk_combo_box_get_active(GTK_COMBO_BOX(comboboxentry_playlist));
    setPlaylistField(playlist_number);
} // end on_Playlist_Combobox_activate()

// ************************************************************************************************

/**
 * Callback to handle the change of columns viewable in the main window.
 * @param menuitem
 * @param user_data
 */
void on_view_activate(GtkMenuItem *menuitem, gpointer user_data) {
#if GMTP_USE_GTK2
    gchar *gconf_path = NULL;
    gboolean state = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
    if ((void *) menuitem == (void *) menu_view_filesize) gconf_path = g_strdup("/apps/gMTP/viewFileSize");
    if ((void *) menuitem == (void *) menu_view_filetype) gconf_path = g_strdup("/apps/gMTP/viewFileType");
    if ((void *) menuitem == (void *) menu_view_track_number) gconf_path = g_strdup("/apps/gMTP/viewTrackNumber");
    if ((void *) menuitem == (void *) menu_view_title) gconf_path = g_strdup("/apps/gMTP/viewTitle");
    if ((void *) menuitem == (void *) menu_view_artist) gconf_path = g_strdup("/apps/gMTP/viewArtist");
    if ((void *) menuitem == (void *) menu_view_album) gconf_path = g_strdup("/apps/gMTP/viewAlbum");
    if ((void *) menuitem == (void *) menu_view_year) gconf_path = g_strdup("/apps/gMTP/viewYear");
    if ((void *) menuitem == (void *) menu_view_genre) gconf_path = g_strdup("/apps/gMTP/viewGenre");
    if ((void *) menuitem == (void *) menu_view_duration) gconf_path = g_strdup("/apps/gMTP/viewDuration");

    if ((gconfconnect != NULL) && (gconf_path != NULL)) {
        gconf_client_set_bool(gconfconnect, gconf_path, state, NULL);
        g_free(gconf_path);
    }
#else
    gchar *gsetting_path = NULL;
    gboolean state = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
    if ((void *) menuitem == (void *) menu_view_filesize) gsetting_path = g_strdup("viewfilesize");
    if ((void *) menuitem == (void *) menu_view_filetype) gsetting_path = g_strdup("viewfiletype");
    if ((void *) menuitem == (void *) menu_view_track_number) gsetting_path = g_strdup("viewtracknumber");
    if ((void *) menuitem == (void *) menu_view_title) gsetting_path = g_strdup("viewtitle");
    if ((void *) menuitem == (void *) menu_view_artist) gsetting_path = g_strdup("viewartist");
    if ((void *) menuitem == (void *) menu_view_album) gsetting_path = g_strdup("viewalbum");
    if ((void *) menuitem == (void *) menu_view_year) gsetting_path = g_strdup("viewyear");
    if ((void *) menuitem == (void *) menu_view_genre) gsetting_path = g_strdup("viewgenre");
    if ((void *) menuitem == (void *) menu_view_duration) gsetting_path = g_strdup("viewduration");

    if ((gsettings_connect != NULL) && (gsetting_path != NULL)) {
        g_settings_set_boolean(gsettings_connect, gsetting_path, state);
        g_settings_sync();
        g_free(gsetting_path);
    }
#endif
} // end on_view_activate()

// ************************************************************************************************

/**
 * Callback to handle when a user closes the Progress Dialog box, via the X button.
 * @param window
 * @param user_data
 */
void on_progressDialog_Close(GtkWidget *window, gpointer user_data) {
    // Set the global flag that the user has done it.
    progressDialog_killed = TRUE;
} // end on_progressDialog_Close()

// ************************************************************************************************

/**
 * Callback to handle when a user presses the Cancel button in the Progress Dialog box
 * @param window
 * @param user_data
 */
void on_progressDialog_Cancel(GtkWidget *button, gpointer user_data) {
    // Set the global flag that the user has done it.
    progressDialog_killed = TRUE;
    // Destroy the dialog box.
    gtk_widget_hide(progressDialog);
    gtk_widget_destroy(progressDialog);
} // end on_progressDialog_Cancel()
