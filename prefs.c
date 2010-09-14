#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <libmtp.h>
#include <id3tag.h>

#include "main.h"
#include "interface.h"
#include "callbacks.h"
#include "mtp.h"
#include "prefs.h"

Preferences_Struct Preferences;
GConfClient *gconfconnect = NULL;
guint gconf_callback_id;

void setupPreferences(){
	// We setup default Preferences.
	Preferences.ask_download_path = TRUE;
	Preferences.attemptDeviceConnectOnStart = TRUE;
	#ifdef WIN32
	Preferences.fileSystemDownloadPath = g_string_new(g_getenv("HOMEPATH"));
	Preferences.fileSystemUploadPath = g_string_new(g_getenv("HOMEPATH"));
	#else
	Preferences.fileSystemDownloadPath = g_string_new(g_getenv("HOME"));
	Preferences.fileSystemUploadPath = g_string_new(g_getenv("HOME"));
	#endif
    // Now setup our gconf callbacks;
    if(gconfconnect == NULL)
        gconfconnect = gconf_client_get_default();
    if(gconf_client_dir_exists(gconfconnect, "/apps/gMTP", NULL) == TRUE) {
        gconf_client_add_dir(gconfconnect, "/apps/gMTP", GCONF_CLIENT_PRELOAD_ONELEVEL , NULL );
        gconf_callback_id = gconf_client_notify_add(gconfconnect, "/apps/gMTP", (GConfClientNotifyFunc) gconf_callback_func, NULL, NULL, NULL) ;
    }
	// Now attempt to read the config file from the user config folder.
	loadPreferences();
	// Now print our preferences
	//g_printf("Upload Path = %s\nDownload Path = %s\nDownloadPrompt = %x\nAutoConnect = %x\n",
	//		 Preferences.fileSystemUploadPath->str, Preferences.fileSystemDownloadPath->str,
	//		 Preferences.ask_download_path, Preferences.attemptDeviceConnectOnStart);
}

gboolean loadPreferences(){

	if(gconf_client_dir_exists(gconfconnect, "/apps/gMTP", NULL) == TRUE) {
		gconf_client_preload(gconfconnect, "/apps/gMTP", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
		Preferences.ask_download_path = gconf_client_get_bool (gconfconnect, "/apps/gMTP/promptDownloadPath", NULL);
		Preferences.confirm_file_delete_op = gconf_client_get_bool (gconfconnect, "/apps/gMTP/confirmFileDelete", NULL);
        Preferences.prompt_overwrite_file_op = gconf_client_get_bool (gconfconnect, "/apps/gMTP/promptOverwriteFile", NULL);
		Preferences.attemptDeviceConnectOnStart = gconf_client_get_bool (gconfconnect, "/apps/gMTP/autoconnectdevice", NULL);
		Preferences.fileSystemDownloadPath = g_string_new(gconf_client_get_string(gconfconnect, "/apps/gMTP/DownloadPath", NULL));
		Preferences.fileSystemUploadPath = g_string_new(gconf_client_get_string(gconfconnect, "/apps/gMTP/UploadPath", NULL));
		//g_print("Settings loaded\n");
	} else {
		g_print("Error: gconf schema invalid, reverting to defaults. Please ensure schema is loaded in gconf database.\n");
	}
	gconf_client_clear_cache(gconfconnect);
	return TRUE;
}

gboolean savePreferences(){
	//GConfClient *gconfconnect;
	//gconfconnect = gconf_client_get_default();
	if(gconf_client_dir_exists(gconfconnect, "/apps/gMTP", NULL) == TRUE) {
		gconf_client_preload(gconfconnect, "/apps/gMTP", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
		gconf_client_set_bool (gconfconnect, "/apps/gMTP/promptDownloadPath", Preferences.ask_download_path, NULL);
		gconf_client_set_bool (gconfconnect, "/apps/gMTP/autoconnectdevice", Preferences.attemptDeviceConnectOnStart, NULL);
        gconf_client_set_bool (gconfconnect, "/apps/gMTP/promptOverwriteFile", Preferences.prompt_overwrite_file_op, NULL);
		gconf_client_set_bool (gconfconnect, "/apps/gMTP/confirmFileDelete", Preferences.confirm_file_delete_op, NULL);
		gconf_client_set_string(gconfconnect, "/apps/gMTP/DownloadPath",Preferences.fileSystemDownloadPath->str, NULL);
		gconf_client_set_string(gconfconnect, "/apps/gMTP/UploadPath", Preferences.fileSystemUploadPath->str, NULL);
		//g_print("Settings saved\n");
	} else {
		g_print("Error: gconf schema invalid, unable to save! Please ensure schema is loaded in gconf database.\n");
	}
	gconf_client_suggest_sync(gconfconnect, NULL);
	gconf_client_clear_cache(gconfconnect);
	return TRUE;
}

void gconf_callback_func(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data){
    g_printf("Gconf callback - %s\n", entry->key);
    if(g_ascii_strcasecmp(entry->key, "/apps/gMTP/promptDownloadPath") == 0){
        //set our promptDownloadPath in Preferences
        Preferences.ask_download_path = (gboolean) gconf_value_get_bool((const GConfValue*)gconf_entry_get_value((const GConfEntry*)entry));
        //g_printf("/apps/gMTP/promptDownloadPath = %d\n", Preferences.ask_download_path );
        if(windowPrefsDialog != NULL) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonDownloadPath), Preferences.ask_download_path);
        return;
    }
    if(g_ascii_strcasecmp(entry->key, "/apps/gMTP/autoconnectdevice") == 0){
        //set our promptDownloadPath in Preferences
        Preferences.attemptDeviceConnectOnStart = (gboolean) gconf_value_get_bool((const GConfValue*)gconf_entry_get_value((const GConfEntry*)entry));
        //g_printf("/apps/gMTP/autoconnectdevice = %d\n", Preferences.attemptDeviceConnectOnStart );
        if(windowPrefsDialog != NULL) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonDeviceConnect), Preferences.attemptDeviceConnectOnStart);
        return;
    }
    if(g_ascii_strcasecmp(entry->key, "/apps/gMTP/promptOverwriteFile") == 0){
        //set our promptDownloadPath in Preferences
        Preferences.prompt_overwrite_file_op = (gboolean) gconf_value_get_bool((const GConfValue*)gconf_entry_get_value((const GConfEntry*)entry));
        //g_printf("/apps/gMTP/promptOverwriteFile = %d\n", Preferences.prompt_overwrite_file_op );
        if(windowPrefsDialog != NULL) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmOverWriteFileOp), Preferences.prompt_overwrite_file_op);
        return;
    }
    if(g_ascii_strcasecmp(entry->key, "/apps/gMTP/confirmFileDelete") == 0){
        //set our promptDownloadPath in Preferences
        Preferences.confirm_file_delete_op = (gboolean) gconf_value_get_bool((const GConfValue*)gconf_entry_get_value((const GConfEntry*)entry));
        //g_printf("/apps/gMTP/confirmFileDelete = %d\n", Preferences.confirm_file_delete_op );
        if(windowPrefsDialog != NULL) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmFileOp), Preferences.confirm_file_delete_op);
        return;
    }
    if(g_ascii_strcasecmp(entry->key, "/apps/gMTP/DownloadPath") == 0){
        //set our promptDownloadPath in Preferences
        Preferences.fileSystemDownloadPath = g_string_assign(Preferences.fileSystemDownloadPath, gconf_value_to_string((const GConfValue*)gconf_entry_get_value((const GConfEntry*)entry)));
        //g_printf("/apps/gMTP/DownloadPath = %s\n", Preferences.fileSystemDownloadPath->str );
        if(windowPrefsDialog != NULL) gtk_entry_set_text(GTK_ENTRY(entryDownloadPath),  Preferences.fileSystemDownloadPath->str);
        return;
    }
    if(g_ascii_strcasecmp(entry->key, "/apps/gMTP/UploadPath") == 0){
        //set our promptDownloadPath in Preferences
        Preferences.fileSystemUploadPath = g_string_assign(Preferences.fileSystemUploadPath, gconf_value_to_string((const GConfValue*)gconf_entry_get_value((const GConfEntry*)entry)));
        //g_printf("/apps/gMTP/UploadPath = %s\n", Preferences.fileSystemUploadPath->str );
        if(windowPrefsDialog != NULL) gtk_entry_set_text(GTK_ENTRY(entryUploadPath), Preferences.fileSystemUploadPath->str);
        return;
    }
    g_printf("gconf_callback failed - we got a callback for a key thats not ours?\n");
}
