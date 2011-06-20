/* 
 *
 *   File: interface.h
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

#ifndef _INTERFACE_H
#define _INTERFACE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

    // Main Window List

    enum fileListID {
        COL_FILENAME = 0,
        COL_FILENAME_HIDDEN,
        COL_FILENAME_ACTUAL,
        COL_FILESIZE,
        COL_FILEID,
        COL_ISFOLDER,
        COL_FILESIZE_HID,
        COL_TYPE,
        COL_TRACK_NUMBER,
        COL_TRACK_NUMBER_HIDDEN,
        COL_TITLE,
        COL_FL_ARTIST,
        COL_FL_ALBUM,
        COL_YEAR,
        COL_GENRE,
        COL_DURATION,
        COL_DURATION_HIDDEN,
        COL_ICON,
        NUM_COLUMNS
    };

    // Playlist windows lists.

    enum fileTrackID {
        COL_ARTIST = 0,
        COL_ALBUM,
        COL_TRACKID,
        COL_TRACKNAME,
        COL_TRACKDURATION,
        NUM_TCOLUMNS
    };

    enum filePlaylistID {
        COL_PL_ORDER_NUM = 0,
        COL_PL_ARTIST,
        COL_PL_ALBUM,
        COL_PL_TRACKID,
        COL_PL_TRACKNAME,
        COL_PL_TRACKDURATION,
        NUM_PL_COLUMNS
    };

    typedef struct {
        uint32_t itemid;
        gboolean isFolder;
        gchar *filename;
        uint64_t filesize;
    } FileListStruc;

    typedef struct {
        uint32_t album_id;
        gchar* filename;
    } Album_Struct;

    // File operation enums

    enum MTP_OVERWRITEOP {
        MTP_ASK,
        MTP_SKIP,
        MTP_SKIP_ALL,
        MTP_OVERWRITE,
        MTP_OVERWRITE_ALL
    };

    // Main Window widgets
    GtkListStore *fileList;

    GtkWidget* create_windowMain(void);
    GtkWidget* create_windowPreferences(void);
    GtkWidget* create_windowProperties(void);
    GtkWidget* create_windowMainContextMenu(void);
    GtkWidget* create_windowPlaylist(void);

    void SetToolbarButtonState(gboolean);
    void statusBarSet(gchar *text);
    void statusBarClear();

    gboolean fileListClear();
    GSList* getFileGetList2Add();
    gboolean fileListAdd();
    gchar* displayRenameFileDialog(gchar* currentfilename);
    gboolean fileListRemove(GList *List);
    gboolean fileListDownload(GList *List);
    GList* fileListGetSelection();
    gboolean fileListClearSelection();

    gboolean folderListRemove(GList *List);

    // Flag to allow overwrite of files on device.
    gint deviceoverwriteop;
    // Aggreegate function for adding a file to the device.
    void __filesAdd(gchar* filename);

    // Progress Dialog
    GtkWidget* create_windowProgressDialog(gchar* msg);
    void displayProgressBar(gchar* msg);
    void destroyProgressBar(void);
    void setProgressFilename(gchar* filename_stripped);
    int fileprogress(const uint64_t sent, const uint64_t total, void const * const data);
    GtkWidget *progressDialog;
    gboolean progressDialog_killed;

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
#define ALBUM_SIZE 96

    void displayAddAlbumArtDialog(void);
    void AlbumArtUpdateImage(LIBMTP_album_t* selectedAlbum);
    void AlbumArtSetDefault(void);

    // Playlists

    gint playlist_number;
    gint comboboxentry_playlist_entries;

    void displayPlaylistDialog(void);
    void setupTrackList(GtkTreeView *treeviewFiles);
    void setup_PL_List(GtkTreeView *treeviewFiles);
    void SetPlaylistButtonState(gboolean state);
    void setPlayListComboBox(void);
    void setPlaylistField(gint PlayListID);
    gchar* displayPlaylistNewDialog(void);

    gboolean playlist_PL_ListClearSelection();
    GList* playlist_PL_ListGetSelection();
    gboolean playlist_PL_ListRemove(GList *List);
    void __playlist_PL_Remove(GtkTreeRowReference *Row);

    GList* playlist_TrackList_GetSelection();
    gboolean playlist_TrackList_Add(GList *List);
    void __playlist_TrackList_Add(GtkTreeRowReference *Row);

    gboolean playlist_move_files(gint direction);
    void __playlist_move_files_up(GtkTreeRowReference *Row);
    void __playlist_move_files_down(GtkTreeRowReference *Row);

    void playlist_SavePlaylist(gint PlayListID);

    // Widgets for menu items;
    GtkWidget *fileConnect;
    GtkWidget *fileAdd;
    GtkWidget *fileDownload;
    GtkWidget *fileRemove;
    GtkWidget *fileRename;
    GtkWidget *fileNewFolder;
    GtkWidget *fileRemoveFolder;
    GtkWidget *fileRescan;
    GtkWidget *editDeviceName;
    GtkWidget *editAddAlbumArt;
    GtkWidget *contextMenu;
#if GMTP_USE_GTK2
    GtkTooltips *tooltipsToolbar;
#endif

    // Columns in main file view;
    GtkTreeViewColumn *column_Size;
    GtkTreeViewColumn *column_Type;
    GtkTreeViewColumn *column_Track_Number;
    GtkTreeViewColumn *column_Title;
    GtkTreeViewColumn *column_Artist;
    GtkTreeViewColumn *column_Album;
    GtkTreeViewColumn *column_Year;
    GtkTreeViewColumn *column_Genre;
    GtkTreeViewColumn *column_Duration;

    // Main menu widgets
    GtkWidget *menu_view_filesize;
    GtkWidget *menu_view_filetype;
    GtkWidget *menu_view_track_number;
    GtkWidget *menu_view_title;
    GtkWidget *menu_view_artist;
    GtkWidget *menu_view_album;
    GtkWidget *menu_view_year;
    GtkWidget *menu_view_genre;
    GtkWidget *menu_view_duration;

    // Widgets for preferences buttons;
    GtkWidget *checkbuttonDeviceConnect;
    GtkWidget *entryDownloadPath;
    GtkWidget *entryUploadPath;
    GtkWidget *checkbuttonDownloadPath;
    GtkWidget *checkbuttonConfirmFileOp;
    GtkWidget *checkbuttonConfirmOverWriteFileOp;

    // AlbumArt Dialog global pointers
    GtkWidget *AlbumArtDialog;
    //GtkWidget *AlbumArtFilename;
    GtkWidget *AlbumArtImage;
    GtkWidget *buttonAlbumAdd;
    GtkWidget *buttonAlbumDownload;
    GtkWidget *buttonAlbumDelete;
    GtkWidget *textboxAlbumArt;

    // Playlist
    GtkWidget *treeview_Avail_Files;
    GtkWidget *treeview_Playlist_Files;
    GtkWidget *comboboxentry_playlist;
    GtkListStore *playlist_TrackList;
    GtkListStore *playlist_PL_List;

    // Buttons for playlist
    GtkWidget *button_Del_Playlist;
    GtkWidget *button_File_Move_Up;
    GtkWidget *button_File_Move_Down;
    GtkWidget *button_Del_File;
    GtkWidget *button_Add_Files;

#ifdef  __cplusplus
}
#endif

#endif  /* _INTERFACE_H */


