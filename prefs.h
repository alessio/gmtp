/*
 * File:   prefs.h
 * Author: darran
 *
 * Created on November 27, 2009, 11:34 AM
 */

#ifndef _PREFS_H
#define _PREFS_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct  {
	GString *fileSystemDownloadPath;
	GString *fileSystemUploadPath;
	gboolean attemptDeviceConnectOnStart;
	gboolean ask_download_path;
    gboolean prompt_overwrite_file_op;
	gboolean confirm_file_delete_op;

} Preferences_Struct;

Preferences_Struct Preferences;
GConfClient *gconfconnect;

void setupPreferences();
gboolean loadPreferences();
gboolean savePreferences();

void gconf_callback_func(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data);

#ifdef  __cplusplus
}
#endif

#endif  /* _PREFS_H */

