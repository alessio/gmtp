/*
        This file contains all the Drag and Drop Functionality for gMTP
 */

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <sys/types.h>
#include <libgen.h>
#include <string.h>
#include <id3tag.h>

#include "main.h"
#include "interface.h"
#include "callbacks.h"
#include "mtp.h"
#include "prefs.h"
#include "dnd.h"

const GtkTargetEntry _gmtp_drop_types[] =
{
	{"text/plain",			0,	GMTP_DROP_PLAINTEXT},
	{"text/uri-list",		0,	GMTP_DROP_URLENCODED},
	{"STRING",              0,	GMTP_DROP_STRING}
};

void gmtp_drag_data_received(GtkWidget * widget,
				       GdkDragContext * context,
				       gint x,
				       gint y,
				       GtkSelectionData * selection_data,
				       guint info,
				       guint time,
				       gpointer user_data)
{
	if (selection_data->data)
	{
        
        GSList* files;
        //g_printf("Selection->data = %s\n", selection_data->data);

        files = getFilesListURI((gchar *)selection_data->data);
        if(files != NULL){
            //g_print("Do each file\n");
            g_slist_foreach(files, (GFunc)__filesAdd, NULL);
        }
        // Now clear the GList;
        g_slist_foreach(files, (GFunc)g_free, NULL);
        g_slist_free(files);
        // Now do a device rescan to see the new files.
        deviceRescan();
        deviceoverwriteop = MTP_ASK;
	}
}

GSList* getFilesListURI(gchar* rawdata){
    // The data is just the data in string form
    // Files are in the URI form of file:///filename\n so just look for those,
    // and if found see if a folder or not?
    // Then create a slist of those and use filesAdd() to add them in .
    GSList* filelist;
    gchar* tmpstring;
    gchar *fullpath;
    gchar *filepath;
    gchar *token;
    //struct stat statbuf;
    const char delimit[]="\n\r";

    filelist = NULL;

    fullpath = g_strdup(rawdata);

    token = strtok (fullpath, delimit);
    while((token != NULL)){
        // Now test to see if we have it here...
        filepath = g_strdup(token);
        // See if we have a local file URI, otherwise discard.
        if(!g_ascii_strncasecmp(filepath, "file://", 7)){
            tmpstring = g_filename_from_uri(filepath, NULL, NULL);
            if (g_file_test(tmpstring, G_FILE_TEST_IS_REGULAR ) == TRUE){
                filelist = g_slist_append(filelist, g_strdup(tmpstring));
            } else {
                //g_printf("Parsing folder \"%s\" via DnD interface\n", tmpstring);
                addFilesinFolder(tmpstring);
            }
            g_free(tmpstring);
        }
        token = strtok(NULL, delimit);
        g_free(filepath);
    }
    g_free(fullpath);
    return filelist;
}

void addFilesinFolder(gchar* foldername){
    // foldername is the name of the folder as the absolute path with leading /
    // We save the currentFolderID, create a new folder on the device,
    // and set currentFolderID to the new folders ID.
    // Then scan the folder (on the filesystem) adding in files as needed.
    // Found folders are always added first, so files are copied from
    // the deepest level of the folder hierarchy first as well, and we
    // work our way back down towards to the initial folder that was
    // dragged in.
    // Lastly we restore the currentFolderID back to what it was.
    GDir *fileImageDir;
    GSList* filelist;
    const gchar *filename;
    gchar* relative_foldername;
    gchar *tmpstring;
    uint32_t oldFolderID;

    filelist = NULL;
    // Save our current working folder.
    oldFolderID = currentFolderID;
    // Get just the folder name, as we are given a full absolute path.
    relative_foldername = basename(foldername);
    if(relative_foldername != NULL){
        //g_printf("New MTP Folder = %s\n", relative_foldername);
        // Add our folder to the mtp device and set our new current working folder ID.
        currentFolderID = folderAdd(relative_foldername);
    }

    // Start scanning the folder on the filesystem for our new files/folders.
    fileImageDir = g_dir_open(foldername, 0, NULL);
    // Now parse that directory looking for JPEG/PNG files (based on settings).
    // If we find one, we create a new GString and add it to the list.
    if(fileImageDir != NULL){
        filename = g_dir_read_name(fileImageDir);
        while(filename != NULL){
            // See if a file or a folder?
            tmpstring = g_strconcat(foldername, "/", filename, NULL);
            if (g_file_test(tmpstring, G_FILE_TEST_IS_REGULAR ) == TRUE){
                // We have a regular file. So add it to the list.
                //g_printf("File = %s\n", tmpstring);
                filelist = g_slist_append(filelist, g_strdup(tmpstring));
            } else {
                if (g_file_test(tmpstring, G_FILE_TEST_IS_DIR ) == TRUE){
                    // We have another folder so recursively call ourselves...
                    //g_printf("Folder = %s\n", tmpstring);
                    addFilesinFolder( tmpstring);
                }
            }
            filename = g_dir_read_name(fileImageDir);
            g_free(tmpstring);
        }
    }
    // Upload our given files in the current selected folder.
    if(filelist != NULL){
        //g_print("Do each file\n");
        g_slist_foreach(filelist, (GFunc)__filesAdd, NULL);
    }
    // Now clear the GList;
    g_slist_foreach(filelist, (GFunc)g_free, NULL);
    g_slist_free(filelist);

    if(fileImageDir != NULL)
        g_dir_close(fileImageDir);
    // Restore our current working folder.
    currentFolderID = oldFolderID;
}
