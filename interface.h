/*
 * File:   interface.h
 * Author: darran
 *
 * Created on October 29, 2009, 5:01 PM
 */

#ifndef _INTERFACE_H
#define _INTERFACE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

typedef struct {
	uint32_t itemid;
	gboolean isFolder;
	gchar    *filename;
	uint64_t filesize;
} FileListStruc;

typedef struct {
    uint32_t album_id;
    gchar* filename;
} Album_Struct;

enum MTP_OVERWRITEOP {
    MTP_ASK,
	MTP_SKIP,
	MTP_SKIP_ALL,
	MTP_OVERWRITE,
	MTP_OVERWRITE_ALL
};

GtkListStore *fileList;

GtkWidget* create_windowMain (void);
GtkWidget* create_windowPreferences (void);
GtkWidget* create_windowProperties(void);
GtkWidget* create_windowMainContextMenu(void);

void SetToolbarButtonState (gboolean);
void statusBarSet(gchar *text);
void statusBarClear();

gboolean  fileListClear();
GSList* getFileGetList2Add();
gboolean fileListAdd();
gboolean fileListRemove(GList *List);
gboolean fileListDownload(GList *List);
GList* fileListGetSelection();
gboolean fileListClearSelection();

gboolean folderListRemove(GList *List);

// Flag to allow overwrite of files on device.
gint deviceoverwriteop;
// Aggreegate function for adding a file to the device.
void __filesAdd(gchar* filename);

// Progress
GtkWidget* create_windowProgressDialog (gchar* msg);
void displayProgressBar (gchar* msg);
void destroyProgressBar (void);
void setProgressFilename(gchar* filename_stripped);
int fileprogress (const uint64_t sent, const uint64_t total, void const * const data);

// About Dialog box.
void displayAbout(void);

// Error dialog.
void displayError(gchar* msg);
void displayInformation(gchar* msg);

// New Folder Dialog;
gchar* displayFolderNewDialog(void);

// Overwrite this file dialog?
gint displayFileOverwriteDialog(gchar *filename);

// Multidevice/Multistorage dialog;
gint displayMultiDeviceDialog(void);
gint displayDeviceStorageDialog(void);
gchar* displayChangeDeviceNameDialog(gchar* devicename);

// Set Album Art dialog;
Album_Struct* displayAddAlbumArtDialog(void);

// Widgets for menu items;
GtkWidget *fileConnect;
GtkWidget *fileAdd;
GtkWidget *fileDownload;
GtkWidget *fileRemove;
GtkWidget *fileNewFolder;
GtkWidget *fileRemoveFolder;
GtkWidget *fileRescan;
GtkWidget *editDeviceName;
GtkWidget *editAddAlbumArt;
GtkWidget *contextMenu;
GtkTooltips *tooltipsToolbar;

// Widgets for preferences buttons;
GtkWidget *checkbuttonDeviceConnect;
GtkWidget *entryDownloadPath;
GtkWidget *entryUploadPath;
GtkWidget *checkbuttonDownloadPath;
GtkWidget *checkbuttonConfirmFileOp;
GtkWidget *checkbuttonConfirmOverWriteFileOp;

// AlbumArt Dialog global pointers
GtkWidget *AlbumArtDialog;
GtkWidget *AlbumArtFilename;


#ifdef  __cplusplus
}
#endif

#endif  /* _INTERFACE_H */


