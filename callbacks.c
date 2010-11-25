
#include "config.h"

#include <glib.h>
#include <glib/gprintf.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <libmtp.h>
#include <id3tag.h>

#include "main.h"
#include "callbacks.h"
#include "interface.h"
#include "mtp.h"
#include "prefs.h"
#include "dnd.h"

void
on_quit1_activate                      (GtkMenuItem     *menuitem,
										gpointer user_data)
{
	savePreferences();

	gtk_exit(0);
}

void
on_about1_activate                     (GtkMenuItem     *menuitem,
										gpointer user_data)
{
    displayAbout();
}

void on_deviceProperties_activate       (GtkMenuItem     *menuitem,
										 gpointer user_data)
{
	gchar tmp_string[1024];
	//g_print("You selected Properties\n");
	deviceProperties(); // We confirm our device properties, this should setup the device structure information we use below.
	//g_sprintf(tmp_string, "Connected to %s - %d MB free", DeviceMgr.devicename->str, (int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ));
	if(DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE ){
        g_sprintf(tmp_string, "Connected to %s - %d MB free", DeviceMgr.devicename->str, (int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ));
    } else {
        if(DeviceMgr.devicestorage->StorageDescription != NULL){
            g_sprintf(tmp_string, "Connected to %s (%s) - %d MB free", DeviceMgr.devicename->str, DeviceMgr.devicestorage->StorageDescription ,(int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ));
        } else {
            g_sprintf(tmp_string, "Connected to %s - %d MB free", DeviceMgr.devicename->str, (int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ));
        }
    }
    statusBarSet((gchar *)&tmp_string);
	// No idea how this could come about, but we should take it into account so we don't have a memleak due to recreating the window multiple times.
	if(windowPropDialog != NULL) {
		gtk_widget_hide (windowPropDialog);
		gtk_widget_destroy (windowPropDialog);
	}
	windowPropDialog = create_windowProperties();
	gtk_widget_show (GTK_WIDGET (windowPropDialog));

}

void on_quitProp_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_hide (windowPropDialog);
	gtk_widget_destroy (windowPropDialog);
	windowPropDialog = NULL;
}


void on_deviceRescan_activate (GtkMenuItem     *menuitem,
							   gpointer user_data)
{
	deviceRescan();
}

void on_filesAdd_activate (GtkMenuItem     *menuitem,
						   gpointer user_data)
{
	GSList* files;
	files = getFileGetList2Add();
	if(files != NULL)
		g_slist_foreach(files, (GFunc)__filesAdd, NULL);
	// Now clear the GList;
	g_slist_foreach(files, (GFunc)g_free, NULL);
	g_slist_free(files);
	// Now do a device rescan to see the new files.
	deviceRescan();
    deviceoverwriteop = MTP_ASK;
}

void on_filesDelete_activate (GtkMenuItem     *menuitem,
							  gpointer user_data)
{
	//g_print("You selected Delete Files\n");
	GtkWidget *dialog;
    // Let's check to see if we have anything selected in our treeview?
    if(fileListGetSelection() == NULL){
        displayInformation("No files/folders selected?");
        return;
    }
	// Now we prompt to confirm delete?
	if(Preferences.confirm_file_delete_op == FALSE) {
		// Now download the actual file from the MTP device.
		fileListRemove(fileListGetSelection());
	} else {
		dialog = gtk_message_dialog_new (GTK_WINDOW(windowMain),
										 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										 GTK_MESSAGE_WARNING,
										 GTK_BUTTONS_YES_NO,
										 "Are you sure you want to delete these files?");
		gtk_window_set_title(GTK_WINDOW (dialog), "Confirm Delete");
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));
		//g_printf("Delete Result = %d\n", result);
		if(result == GTK_RESPONSE_YES)
			fileListRemove(fileListGetSelection());
		gtk_widget_destroy (dialog);
	}
}

void on_filesDownload_activate (GtkMenuItem     *menuitem,
								gpointer user_data)
{
	//filesDownload((gchar*)NULL);
    // Let's check to see if we have anything selected in our treeview?
    if(fileListGetSelection() == NULL){
        displayInformation("No files/folders selected?");
        return;
    }
	gboolean result = fileListDownload(fileListGetSelection());
	//g_printf("Download result was %d\n", result);
}

void on_deviceConnect_activate (GtkMenuItem     *menuitem,
								gpointer user_data)
{
	gchar tmp_string[1024];
	GtkWidget *menuText;
	guint result = deviceConnect();
	//g_printf("Device connect/disconnect code = %d\n", result);
	// Update our label to indicate current condition.
	if(DeviceMgr.deviceConnected == TRUE) {
		// Set up our properties.
		deviceProperties();
		deviceRescan();
		gtk_tool_button_set_label(GTK_TOOL_BUTTON (toolbuttonConnect), "Disconnect" );
		// Now update the status bar;
		//g_sprintf(tmp_string, "Connected to %s - %d MB free", DeviceMgr.devicename->str, (int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ));
		if(DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE ){
            g_sprintf(tmp_string, "Connected to %s - %d MB free", DeviceMgr.devicename->str, (int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ));
        } else {
            if(DeviceMgr.devicestorage->StorageDescription != NULL){
                g_sprintf(tmp_string, "Connected to %s (%s) - %d MB free", DeviceMgr.devicename->str, DeviceMgr.devicestorage->StorageDescription ,(int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ));
            } else {
                g_sprintf(tmp_string, "Connected to %s - %d MB free", DeviceMgr.devicename->str, (int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ));
            }
        }
        statusBarSet((gchar *)&tmp_string);
		// Now update the filemenu;
		menuText = gtk_bin_get_child(GTK_BIN(fileConnect));
		gtk_label_set_text(GTK_LABEL(menuText), "Disconnect Device");
        gmtp_drag_dest_set(windowMain);
	} else {
		
        gtk_tool_button_set_label(GTK_TOOL_BUTTON  (toolbuttonConnect), "Connect" );
		// Now update the status bar;
		statusBarSet("No device attached");
		// Now update the filemenu;
		menuText = gtk_bin_get_child(GTK_BIN(fileConnect));
		gtk_label_set_text(GTK_LABEL(menuText), "Connect Device");
		// Now update the file list area.
		fileListClear();
        gtk_drag_dest_unset(windowMain);
	}
	SetToolbarButtonState(DeviceMgr.deviceConnected);
}

// Here is the preferences callbacks and dialog box creation.

void
on_preferences1_activate               (GtkMenuItem     *menuitem,
										gpointer user_data)
{
	// No idea how this could come about, but we should take it into account so we don't have a memleak due to recreating the window multiple times.
	if(windowPrefsDialog != NULL) {
		gtk_widget_hide (windowPrefsDialog);
		gtk_widget_destroy (windowPrefsDialog);
	}
	//g_print("You selected Preferences\n");
	windowPrefsDialog = create_windowPreferences();
	gtk_widget_show (GTK_WIDGET (windowPrefsDialog));

}
void
on_quitPrefs_activate                  (GtkMenuItem     *menuitem,
										gpointer user_data)
{
	gtk_widget_hide (windowPrefsDialog);
	gtk_widget_destroy (windowPrefsDialog);
	windowPrefsDialog = NULL;
}

void on_PrefsDevice_activate (GtkMenuItem *menuitem, gpointer user_data){
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbuttonDeviceConnect));
	//Preferences.attemptDeviceConnectOnStart = state;
    if(gconfconnect != NULL)
        gconf_client_set_bool (gconfconnect, "/apps/gMTP/autoconnectdevice", state, NULL);
}

void on_PrefsConfirmDelete_activate (GtkMenuItem *menuitem, gpointer user_data){
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmFileOp));
	//Preferences.confirm_file_delete_op = state;
    if(gconfconnect != NULL)
        gconf_client_set_bool (gconfconnect, "/apps/gMTP/confirmFileDelete", state, NULL);
}

void on_PrefsAskDownload_activate (GtkMenuItem *menuitem, gpointer user_data){
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbuttonDownloadPath));
	//Preferences.ask_download_path = state;
    if(gconfconnect != NULL)
        gconf_client_set_bool (gconfconnect, "/apps/gMTP/promptDownloadPath", state, NULL);
}

void on_PrefsConfirmOverWriteFileOp_activate(GtkMenuItem *menuitem, gpointer user_data){
	gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmOverWriteFileOp));
	//Preferences.prompt_overwrite_file_op = state;
    if(gconfconnect != NULL)
        gconf_client_set_bool (gconfconnect, "/apps/gMTP/promptOverwriteFile", state, NULL);
}

void on_PrefsDownloadPath_activate (GtkMenuItem *menuitem, gpointer user_data){
	// What we do here is display a find folder dialog, and save the resulting folder into the text wigdet and preferences item.
	//gchar *filename;
	gchar *savepath;
	GtkWidget *FileDialog;
	//filename = g_strndup("", 8192);
	// First of all, lets set the download path.
	FileDialog = gtk_file_chooser_dialog_new("Select Path to Download to",
											 GTK_WINDOW(windowPrefsDialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
											 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											 NULL);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemDownloadPath->str);
	if (gtk_dialog_run (GTK_DIALOG (FileDialog)) == GTK_RESPONSE_ACCEPT) {
		savepath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (FileDialog));
		// Save our download path.
		//Preferences.fileSystemDownloadPath = g_string_assign(Preferences.fileSystemDownloadPath, savepath);
        if(gconfconnect != NULL)
            gconf_client_set_string(gconfconnect, "/apps/gMTP/DownloadPath", savepath, NULL);
		//gtk_entry_set_text(GTK_ENTRY(entryDownloadPath), savepath);
		//g_free (filename);
	}
	gtk_widget_destroy (FileDialog);
	//g_printf("Download path = %s\n", Preferences.fileSystemDownloadPath->str);
}

void on_PrefsUploadPath_activate (GtkMenuItem *menuitem, gpointer user_data){
	// What we do here is display a find folder dialog, and save the resulting folder into the text wigdet and preferences item.
	//gchar *filename;
	gchar *savepath;
	GtkWidget *FileDialog;
	//filename = g_strndup("", 8192);
	// First of all, lets set the download path.
	FileDialog = gtk_file_chooser_dialog_new("Select Path to Upload From",
											 GTK_WINDOW(windowPrefsDialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
											 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											 NULL);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemUploadPath->str);
	if (gtk_dialog_run (GTK_DIALOG (FileDialog)) == GTK_RESPONSE_ACCEPT) {
		savepath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (FileDialog));
		// Save our download path.
		//Preferences.fileSystemUploadPath = g_string_assign(Preferences.fileSystemUploadPath, savepath);
        if(gconfconnect != NULL)
            gconf_client_set_string(gconfconnect, "/apps/gMTP/UploadPath", savepath, NULL);
		//gtk_entry_set_text(GTK_ENTRY(entryUploadPath), savepath);
		//g_free (filename);
	}
	gtk_widget_destroy (FileDialog);
	//g_printf("Upload path = %s\n", Preferences.fileSystemUploadPath->str);
}

void fileListRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data){
	GtkTreeModel *model;
	GtkTreeIter iter;

	gchar *filename;
	gboolean isFolder;
	uint32_t objectID;

	filename = g_strndup("", 8192);
	model = gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model, &iter, path)) {
		gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_ISFOLDER, &isFolder, COL_FILENAME, &filename, COL_FILEID, &objectID, -1);
		if(isFolder == FALSE) {
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
}

void on_fileNewFolder_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gchar *foldername;
	//g_print("You selected New Folder\n");

	foldername = displayFolderNewDialog();
	if(foldername != NULL) {
		// Add in folder to MTP device.
		//g_printf("New Folder = %s\n", foldername);
		folderAdd(foldername);
		g_free(foldername);
		deviceRescan();
	}
}

void on_fileRemoveFolder_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	//g_print("You selected Delete Folder\n");
	GtkWidget *dialog;
    // Let's check to see if we have anything selected in our treeview?
    if(fileListGetSelection() == NULL){
        displayInformation("No files/folders selected?");
        return;
    }

	// Now we prompt to confirm delete?
	if(Preferences.confirm_file_delete_op == FALSE) {
		// Now download the actual file from the MTP device.
		folderListRemove(fileListGetSelection());
	} else {
		dialog = gtk_message_dialog_new (GTK_WINDOW(windowMain),
										 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										 GTK_MESSAGE_WARNING,
										 GTK_BUTTONS_YES_NO,
										 "Are you sure you want to delete this folder (and all contents)?");
		gtk_window_set_title(GTK_WINDOW (dialog), "Confirm Delete");
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));
		//g_printf("Delete Result = %d\n", result);
		if(result == GTK_RESPONSE_YES)
			folderListRemove(fileListGetSelection());
		gtk_widget_destroy (dialog);
	}


}

void on_editDeviceName_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gchar *devicename;
    gchar *tmp_string;

	devicename = displayChangeDeviceNameDialog(DeviceMgr.devicename->str);
	if(devicename != NULL) {
		// add change to MTP device.
		//g_printf("New device name = %s\n", devicename);
        setDeviceName(devicename);
		g_free(devicename);
        tmp_string = LIBMTP_Get_Friendlyname(DeviceMgr.device);
		if(tmp_string == NULL) {
			DeviceMgr.devicename = g_string_new("N/A");
		} else {
			DeviceMgr.devicename = g_string_new(tmp_string);
			g_free(tmp_string);
		}
		deviceRescan();
	}
}

gboolean on_windowMainContextMenu_activate (GtkWidget *widget, GdkEvent *event){
    GtkMenu *menu;
    GdkEventButton *event_button;
    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_MENU (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

      /* The "widget" is the menu that was supplied when
       * g_signal_connect_swapped() was called.
       */
    menu = GTK_MENU (widget);
    if (event->type == GDK_BUTTON_PRESS){
        event_button = (GdkEventButton *) event;
        if (event_button->button == 3){
            gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
                        event_button->button, event_button->time);
            return TRUE;
        }
    }
    return FALSE;
}

void on_editAddAlbumArt_activate (GtkMenuItem *menuitem, gpointer user_data){

    Album_Struct* albumart;

    albumart = displayAddAlbumArtDialog();
    if(albumart != NULL){
        albumAddArt(albumart->album_id, albumart->filename);
        g_free(albumart->filename);
    }
}

void on_buttonFilePath_activate (GtkMenuItem *menuitem, gpointer user_data){
	// What we do here is display a find folder dialog, and save the resulting folder into the text wigdet and preferences item.
	//gchar *filename;
	gchar *savepath;
	GtkWidget *FileDialog;
	FileDialog = gtk_file_chooser_dialog_new("Select Album Art File",
											 GTK_WINDOW(AlbumArtDialog), GTK_FILE_CHOOSER_ACTION_OPEN,
											 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											 NULL);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemUploadPath->str);
	if (gtk_dialog_run (GTK_DIALOG (FileDialog)) == GTK_RESPONSE_ACCEPT) {
		savepath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (FileDialog));
		// Save our download path.
        gtk_entry_set_text(GTK_ENTRY(AlbumArtFilename), g_strdup(savepath));
	}
	gtk_widget_destroy (FileDialog);
}
