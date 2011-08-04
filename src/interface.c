/* 
 *
 *   File: interface.c
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


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#if GMTP_USE_GTK2
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#else
#include <gio/gio.h>
#endif
#include <libgen.h>
#include <libmtp.h>
#include <id3tag.h>

#include "main.h"
#include "callbacks.h"
#include "interface.h"
#include "mtp.h"
#include "prefs.h"
#include "dnd.h"

void setupFileList();
void __fileRemove(GtkTreeRowReference *Row);
void __fileDownload(GtkTreeRowReference *Row);
void __folderRemove(GtkTreeRowReference *Row);

GtkWidget *toolbuttonAddFile;
GtkWidget *toolbuttonRetrieve;
GtkWidget *toolbuttonRemoveFile;
GtkWidget *toolbuttonRescan;
GtkWidget *toolbuttonAlbumArt;
GtkWidget *toolbuttonPlaylist;
GtkWidget *toolbuttonProperties;
GtkWidget *properties1;
GtkWidget *fileConnect;
GtkWidget *fileAdd;
GtkWidget *fileDownload;
GtkWidget *fileRemove;
GtkWidget *fileRename;
GtkWidget *fileNewFolder;
GtkWidget *fileRemoveFolder;
GtkWidget *fileRescan;
GtkWidget *editDeviceName;
GtkWidget *editFormatDevice;
GtkWidget *editAddAlbumArt;
GtkWidget *editPlaylist;

GtkWidget *contextMenu;

#if GMTP_USE_GTK2
GtkTooltips *tooltipsToolbar;
#endif

// Menu widget for Properties
GtkListStore *fileList;
GtkTreeSelection *fileSelection;
GList *fileSelection_RowReferences = NULL;

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
GtkWidget *checkbuttonAutoAddTrackPlaylist;
GtkWidget *checkbuttonIgnorePathInPlaylist;

// Widget for Progress Bar Dialog box.
GtkWidget *progressDialog;
GtkWidget *progressDialog_Text;
GtkWidget *progressDialog_Bar;
gchar *progressDialog_filename;
gboolean progressDialog_killed = FALSE;

// Widget for formatDevice progress bar.
GtkWidget *formatDialog_progressBar1;

// Flags for overwriting files of host PC and device.
gint fileoverwriteop = MTP_ASK;
// Flag to allow overwrite of files on device.
gint deviceoverwriteop = MTP_ASK;

// AlbumArt Dialog global pointers
GtkWidget *AlbumArtDialog;
GtkWidget *AlbumArtFilename;
GtkWidget *AlbumArtImage;
GtkWidget *buttonAlbumAdd;
GtkWidget *buttonAlbumDownload;
GtkWidget *buttonAlbumDelete;
GtkWidget *textboxAlbumArt;

// Playlist

GtkWidget *comboboxentry_playlist;
gint playlist_number = 0;
gint comboboxentry_playlist_entries = 0;
gint playlist_track_count = 0;

GtkWidget *treeview_Avail_Files;
GtkWidget *treeview_Playlist_Files;

GtkListStore *playlist_TrackList;
GtkTreeSelection *playlist_TrackSelection;
GList *playlist_Selection_TrackRowReferences = NULL;

GtkListStore *playlist_PL_List;
GtkTreeSelection *playlist_PL_Selection;
GList *playlist_Selection_PL_RowReferences = NULL;

// Buttons for playlist
GtkWidget *button_Del_Playlist;
GtkWidget *button_Export_Playlist;
GtkWidget *button_File_Move_Up;
GtkWidget *button_File_Move_Down;
GtkWidget *button_Del_File;
GtkWidget *button_Add_Files;

// Combobox used in AddTrackPlaylist feature.
GtkWidget *combobox_AddTrackPlaylist;

// ************************************************************************************************

/**
 * Create the main window for the application
 * @return Ptr to the main window widget
 */
GtkWidget* create_windowMain(void) {
    GtkWidget *windowMain;
    GtkWidget *vbox1;
    GtkWidget *menubarMain;
    GtkWidget *menuitem1;
    GtkWidget *menuitem1_menu;
    GtkWidget *menuseparator1;
    GtkWidget *menuseparator2;
    GtkWidget *menuseparator3;
    GtkWidget *menuseparator4;
    GtkWidget *menuseparator5;
    GtkWidget *menuseparator6;
    GtkWidget *quit1;
    GtkWidget *menuitem2;
    GtkWidget *menuitem2_menu;
    GtkWidget *preferences1;
    GtkWidget *menuView;
    GtkWidget *menuView_menu;

    GtkWidget *menuitem4;
    GtkWidget *menuitem4_menu;
    GtkWidget *about1;
    GtkWidget *handlebox1;
    GtkWidget *toolbarMain;
    gint tmp_toolbar_icon_size;
    GtkWidget *tmp_image;
    GtkWidget *toolbuttonPreferences;
    GtkWidget *scrolledwindowMain;
    GtkWidget *toolbarSeparator;
    GtkWidget *toolbarSeparator2;
    GtkWidget *toolbuttonQuit;
    GtkAccelGroup *accel_group;

    GtkWidget *menuText;

    accel_group = gtk_accel_group_new();

    windowMain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gchar * winTitle;
    winTitle = g_strconcat(PACKAGE_TITLE, " v", PACKAGE_VERSION, NULL);
    gtk_window_set_title(GTK_WINDOW(windowMain), (winTitle));
    gtk_window_set_default_size(GTK_WINDOW(windowMain), 820, 400);
    gtk_window_set_icon_from_file(GTK_WINDOW(windowMain), file_icon48_png, NULL);
    g_free(winTitle);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(windowMain), vbox1);

    menubarMain = gtk_menu_bar_new();
    gtk_widget_show(menubarMain);
    gtk_box_pack_start(GTK_BOX(vbox1), menubarMain, FALSE, FALSE, 0);

    menuitem1 = gtk_menu_item_new_with_mnemonic(_("_File"));
    gtk_widget_show(menuitem1);
    gtk_container_add(GTK_CONTAINER(menubarMain), menuitem1);

    menuitem1_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem1), menuitem1_menu);

    fileConnect = gtk_image_menu_item_new_from_stock(GTK_STOCK_NETWORK, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(fileConnect));
    gtk_label_set_text(GTK_LABEL(menuText), _("Connect Device"));
    gtk_widget_show(fileConnect);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), fileConnect);

    menuseparator4 = gtk_separator_menu_item_new();
    gtk_widget_show(menuseparator4);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), menuseparator4);

    fileAdd = gtk_image_menu_item_new_from_stock(GTK_STOCK_ADD, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(fileAdd));
    gtk_label_set_text(GTK_LABEL(menuText), _("Add Files"));
    gtk_widget_show(fileAdd);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), fileAdd);

    fileRemove = gtk_image_menu_item_new_from_stock(GTK_STOCK_REMOVE, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(fileRemove));
    gtk_label_set_text(GTK_LABEL(menuText), _("Delete Files"));
    gtk_widget_show(fileRemove);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), fileRemove);

    fileRename = gtk_image_menu_item_new_from_stock(GTK_STOCK_STRIKETHROUGH, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(fileRename));
    gtk_label_set_text(GTK_LABEL(menuText), _("Rename File"));
    gtk_widget_show(fileRename);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), fileRename);

    fileDownload = gtk_image_menu_item_new_from_stock(GTK_STOCK_GOTO_BOTTOM, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(fileDownload));
    gtk_label_set_text(GTK_LABEL(menuText), _("Download Files"));
    gtk_widget_show(fileDownload);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), fileDownload);

    menuseparator1 = gtk_separator_menu_item_new();
    gtk_widget_show(menuseparator1);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), menuseparator1);

    fileNewFolder = gtk_image_menu_item_new_with_label(_("Create Folder"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(fileNewFolder), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU));
    gtk_widget_show(fileNewFolder);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), fileNewFolder);

    fileRemoveFolder = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(fileRemoveFolder));
    gtk_label_set_text(GTK_LABEL(menuText), _("Delete Folder"));
    gtk_widget_show(fileRemoveFolder);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), fileRemoveFolder);

    menuseparator2 = gtk_separator_menu_item_new();
    gtk_widget_show(menuseparator2);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), menuseparator2);

    fileRescan = gtk_image_menu_item_new_from_stock(GTK_STOCK_REFRESH, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(fileRescan));
    gtk_label_set_text(GTK_LABEL(menuText), _("Refresh Device"));
    gtk_widget_show(fileRescan);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), fileRescan);

    properties1 = gtk_image_menu_item_new_from_stock(GTK_STOCK_PROPERTIES, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(properties1));
    gtk_label_set_text(GTK_LABEL(menuText), _("Device Properties"));
    gtk_widget_show(properties1);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), properties1);

    menuseparator3 = gtk_separator_menu_item_new();
    gtk_widget_show(menuseparator3);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), menuseparator3);

    quit1 = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
    gtk_widget_show(quit1);
    gtk_container_add(GTK_CONTAINER(menuitem1_menu), quit1);

    menuitem2 = gtk_menu_item_new_with_mnemonic(_("_Edit"));
    gtk_widget_show(menuitem2);
    gtk_container_add(GTK_CONTAINER(menubarMain), menuitem2);

    menuitem2_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem2), menuitem2_menu);

    editDeviceName = gtk_image_menu_item_new_with_label(_("Change Device Name"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(editDeviceName), gtk_image_new_from_file(file_icon16_png));

    gtk_widget_show(editDeviceName);
    gtk_container_add(GTK_CONTAINER(menuitem2_menu), editDeviceName);

    editFormatDevice = gtk_image_menu_item_new_with_label(_("Format Device"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(editFormatDevice), gtk_image_new_from_file(file_format_png));

    gtk_widget_show(editFormatDevice);
    gtk_container_add(GTK_CONTAINER(menuitem2_menu), editFormatDevice);

    menuseparator6 = gtk_separator_menu_item_new();
    gtk_widget_show(menuseparator6);
    gtk_container_add(GTK_CONTAINER(menuitem2_menu), menuseparator6);

    editAddAlbumArt = gtk_image_menu_item_new_from_stock(GTK_STOCK_CDROM, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(editAddAlbumArt));
    gtk_label_set_text(GTK_LABEL(menuText), _("Album Art"));
    gtk_widget_show(editAddAlbumArt);
    gtk_container_add(GTK_CONTAINER(menuitem2_menu), editAddAlbumArt);

    editPlaylist = gtk_image_menu_item_new_from_stock(GTK_STOCK_DND_MULTIPLE, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(editPlaylist));
    gtk_label_set_text(GTK_LABEL(menuText), _("Edit Playlist(s)"));
    gtk_widget_show(editPlaylist);
    gtk_container_add(GTK_CONTAINER(menuitem2_menu), editPlaylist);

    menuseparator5 = gtk_separator_menu_item_new();
    gtk_widget_show(menuseparator5);
    gtk_container_add(GTK_CONTAINER(menuitem2_menu), menuseparator5);

    preferences1 = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, accel_group);
    gtk_widget_show(preferences1);
    gtk_container_add(GTK_CONTAINER(menuitem2_menu), preferences1);

    menuView = gtk_menu_item_new_with_mnemonic(_("_View"));
    gtk_widget_show(menuView);
    gtk_container_add(GTK_CONTAINER(menubarMain), menuView);

    menuView_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuView), menuView_menu);

    menu_view_filesize = gtk_check_menu_item_new_with_label(_("File Size"));
    gtk_widget_show(menu_view_filesize);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_filesize);

    menu_view_filetype = gtk_check_menu_item_new_with_label(_("File Type"));
    gtk_widget_show(menu_view_filetype);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_filetype);

    menu_view_track_number = gtk_check_menu_item_new_with_label(_("Track Number"));
    gtk_widget_show(menu_view_track_number);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_track_number);

    menu_view_title = gtk_check_menu_item_new_with_label(_("Track Name"));
    gtk_widget_show(menu_view_title);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_title);

    menu_view_artist = gtk_check_menu_item_new_with_label(_("Artist"));
    gtk_widget_show(menu_view_artist);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_artist);

    menu_view_album = gtk_check_menu_item_new_with_label(_("Album"));
    gtk_widget_show(menu_view_album);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_album);

    menu_view_year = gtk_check_menu_item_new_with_label(_("Year"));
    gtk_widget_show(menu_view_year);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_year);

    menu_view_genre = gtk_check_menu_item_new_with_label(_("Genre"));
    gtk_widget_show(menu_view_genre);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_genre);

    menu_view_duration = gtk_check_menu_item_new_with_label(_("Duration"));
    gtk_widget_show(menu_view_duration);
    gtk_container_add(GTK_CONTAINER(menuView_menu), menu_view_duration);

    menuitem4 = gtk_menu_item_new_with_mnemonic(_("_Help"));
    gtk_widget_show(menuitem4);
    gtk_container_add(GTK_CONTAINER(menubarMain), menuitem4);

    menuitem4_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem4), menuitem4_menu);

    about1 = gtk_image_menu_item_new_with_mnemonic(_("_About"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(about1), gtk_image_new_from_file(file_about_png));
    gtk_widget_show(about1);
    gtk_container_add(GTK_CONTAINER(menuitem4_menu), about1);

    handlebox1 = gtk_handle_box_new();
    gtk_widget_show(handlebox1);
    gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox1), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start(GTK_BOX(vbox1), handlebox1, FALSE, FALSE, 0);

#if GMTP_USE_GTK2
    tooltipsToolbar = gtk_tooltips_new();
#endif

    toolbarMain = gtk_toolbar_new();
    gtk_widget_show(toolbarMain);
    gtk_container_add(GTK_CONTAINER(handlebox1), toolbarMain);
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbarMain), GTK_TOOLBAR_BOTH);
    tmp_toolbar_icon_size = gtk_toolbar_get_icon_size(GTK_TOOLBAR(toolbarMain));

#if GMTP_USE_GTK2
    gtk_toolbar_set_tooltips(GTK_TOOLBAR(toolbarMain), TRUE);
#else
    g_object_set(gtk_settings_get_default(), "gtk-enable-tooltips", TRUE, NULL);
#endif

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_NETWORK, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonConnect = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Connect"));
    gtk_widget_show(toolbuttonConnect);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonConnect);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonConnect), GTK_TOOLTIPS(tooltipsToolbar), _("Connect/Disconnect to your device."), _("Connect/Disconnect to your device."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonConnect), _("Connect/Disconnect to your device."));
#endif

    toolbarSeparator = (GtkWidget*) gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbarSeparator), TRUE);
    gtk_widget_show(toolbarSeparator);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbarSeparator);

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_ADD, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonAddFile = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Add"));
    gtk_widget_show(toolbuttonAddFile);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonAddFile);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonAddFile), GTK_TOOLTIPS(tooltipsToolbar), _("Add Files to your device."), _("Add a varity of Files to your device in the current folder."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonAddFile), _("Add Files to your device."));
#endif

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_REMOVE, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonRemoveFile = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Delete"));
    gtk_widget_show(toolbuttonRemoveFile);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonRemoveFile);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonRemoveFile), GTK_TOOLTIPS(tooltipsToolbar), _("Delete Files/Folders from your device."), _("Permanently remove files/folders from your device. Note: Albums are stored as *.alb files."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonRemoveFile), _("Delete Files/Folders from your device."));
#endif

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_GOTO_BOTTOM, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonRetrieve = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Download"));
    gtk_widget_show(toolbuttonRetrieve);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonRetrieve);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonRetrieve), GTK_TOOLTIPS(tooltipsToolbar), _("Download Files from your device to your Host PC."), _("Download files from your device to your PC. Default Download path is set in the prefernces dialog."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonRetrieve), _("Download Files from your device to your Host PC."));
#endif

    toolbarSeparator2 = (GtkWidget*) gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbarSeparator2), TRUE);
    gtk_widget_show(toolbarSeparator2);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbarSeparator2);

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_CDROM, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonAlbumArt = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Album Art"));
    gtk_widget_show(toolbuttonAlbumArt);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonAlbumArt);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonAlbumArt), GTK_TOOLTIPS(tooltipsToolbar), _("Upload an image file as Album Art."), _("Upload a JPG file and assign it as Album Art."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonAlbumArt), _("Upload an image file as Album Art."));
#endif

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_DND_MULTIPLE, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonPlaylist = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Playlists"));
    gtk_widget_show(toolbuttonPlaylist);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonPlaylist);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonPlaylist), GTK_TOOLTIPS(tooltipsToolbar), _("Add and Modify Playlists."), _("Add and Modify Playlists."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonPlaylist), _("Add and Modify Playlists."));
#endif

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_REFRESH, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonRescan = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Refresh"));
    gtk_widget_show(toolbuttonRescan);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonRescan);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonRescan), GTK_TOOLTIPS(tooltipsToolbar), _("Refresh File/Folder listing."), _("Refresh File/Folder listing."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonRescan), _("Refresh File/Folder listing."));
#endif

    toolbarSeparator = (GtkWidget*) gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbarSeparator), TRUE);
    gtk_widget_show(toolbarSeparator);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbarSeparator);

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_PROPERTIES, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonProperties = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Properties"));
    gtk_widget_show(toolbuttonProperties);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonProperties);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonProperties), GTK_TOOLTIPS(tooltipsToolbar), _("View Device Properties."), _("View Device Properties."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonProperties), _("View Device Properties."));
#endif

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonPreferences = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Preferences"));
    gtk_widget_show(toolbuttonPreferences);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonPreferences);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonPreferences), GTK_TOOLTIPS(tooltipsToolbar), _("View/Change gMTP Preferences."), _("View/Change gMTP Preferences."));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonPreferences), _("View/Change gMTP Preferences."));
#endif

    toolbarSeparator = (GtkWidget*) gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbarSeparator), TRUE);
    gtk_widget_show(toolbarSeparator);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbarSeparator);

    tmp_image = gtk_image_new_from_stock(GTK_STOCK_QUIT, tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    toolbuttonQuit = (GtkWidget*) gtk_tool_button_new(tmp_image, _("Quit"));
    gtk_widget_show(toolbuttonQuit);
    gtk_container_add(GTK_CONTAINER(toolbarMain), toolbuttonQuit);
#if GMTP_USE_GTK2
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonQuit), GTK_TOOLTIPS(tooltipsToolbar), _("Quit gMTP."), _("Quit"));
#else
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(toolbuttonQuit), _("Quit gMTP."));
#endif

#if GMTP_USE_GTK2
    gtk_tooltips_enable(tooltipsToolbar);
#endif

    scrolledwindowMain = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(scrolledwindowMain);
    gtk_box_pack_start(GTK_BOX(vbox1), scrolledwindowMain, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindowMain), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    treeviewFiles = gtk_tree_view_new();
    gtk_widget_show(treeviewFiles);
    gtk_container_add(GTK_CONTAINER(scrolledwindowMain), treeviewFiles);
    gtk_container_set_border_width(GTK_CONTAINER(treeviewFiles), 5);
    fileSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeviewFiles));
    gtk_tree_selection_set_mode(fileSelection, GTK_SELECTION_MULTIPLE);

    fileList = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
        G_TYPE_UINT, G_TYPE_BOOLEAN, G_TYPE_UINT64, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, GDK_TYPE_PIXBUF);
    setupFileList(treeviewFiles);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeviewFiles), GTK_TREE_MODEL(fileList));
    g_object_unref(fileList);

    windowStatusBar = gtk_statusbar_new();
    gtk_widget_show(windowStatusBar);
    gtk_box_pack_start(GTK_BOX(vbox1), windowStatusBar, FALSE, FALSE, 0);

    // Build our right-click context menu;
    contextMenu = create_windowMainContextMenu();

    // DnD functions
    //gmtp_drag_dest_set(windowMain); // This gets done in the connect callback and
    // only activates if we actually are connected to a device.

    g_signal_connect((gpointer) windowMain, "drag-data-received",
        G_CALLBACK(gmtp_drag_data_received), NULL);
    // End Dnd functions

    g_signal_connect((gpointer) windowMain, "destroy",
        G_CALLBACK(on_quit1_activate),
        NULL);

    g_signal_connect((gpointer) properties1, "activate",
        G_CALLBACK(on_deviceProperties_activate),
        NULL);
    g_signal_connect((gpointer) toolbuttonProperties, "clicked",
        G_CALLBACK(on_deviceProperties_activate),
        NULL);

    g_signal_connect((gpointer) quit1, "activate",
        G_CALLBACK(on_quit1_activate),
        NULL);

    g_signal_connect((gpointer) preferences1, "activate",
        G_CALLBACK(on_preferences1_activate),
        NULL);

    g_signal_connect((gpointer) editDeviceName, "activate",
        G_CALLBACK(on_editDeviceName_activate),
        NULL);

    g_signal_connect((gpointer) editFormatDevice, "activate",
        G_CALLBACK(on_editFormatDevice_activate),
        NULL);

    g_signal_connect((gpointer) editAddAlbumArt, "activate",
        G_CALLBACK(on_editAddAlbumArt_activate),
        NULL);

    g_signal_connect((gpointer) editPlaylist, "activate",
        G_CALLBACK(on_editPlaylist_activate),
        NULL);

    g_signal_connect((gpointer) fileAdd, "activate",
        G_CALLBACK(on_filesAdd_activate),
        NULL);

    g_signal_connect((gpointer) fileDownload, "activate",
        G_CALLBACK(on_filesDownload_activate),
        NULL);

    g_signal_connect((gpointer) fileRemove, "activate",
        G_CALLBACK(on_filesDelete_activate),
        NULL);

    g_signal_connect((gpointer) fileRename, "activate",
        G_CALLBACK(on_fileRenameFile_activate),
        NULL);

    g_signal_connect((gpointer) fileConnect, "activate",
        G_CALLBACK(on_deviceConnect_activate),
        NULL);

    g_signal_connect((gpointer) fileNewFolder, "activate",
        G_CALLBACK(on_fileNewFolder_activate),
        NULL);

    g_signal_connect((gpointer) fileRemoveFolder, "activate",
        G_CALLBACK(on_fileRemoveFolder_activate),
        NULL);

    g_signal_connect((gpointer) fileRescan, "activate",
        G_CALLBACK(on_deviceRescan_activate),
        NULL);

    g_signal_connect((gpointer) about1, "activate",
        G_CALLBACK(on_about1_activate),
        NULL);

    g_signal_connect((gpointer) toolbuttonQuit, "clicked",
        G_CALLBACK(on_quit1_activate),
        NULL);

    g_signal_connect((gpointer) toolbuttonRescan, "clicked",
        G_CALLBACK(on_deviceRescan_activate),
        NULL);

    g_signal_connect((gpointer) toolbuttonAddFile, "clicked",
        G_CALLBACK(on_filesAdd_activate),
        NULL);

    g_signal_connect((gpointer) toolbuttonRemoveFile, "clicked",
        G_CALLBACK(on_filesDelete_activate),
        NULL);

    g_signal_connect((gpointer) toolbuttonRetrieve, "clicked",
        G_CALLBACK(on_filesDownload_activate),
        NULL);

    g_signal_connect((gpointer) toolbuttonAlbumArt, "clicked",
        G_CALLBACK(on_editAddAlbumArt_activate),
        NULL);

    g_signal_connect((gpointer) toolbuttonPlaylist, "clicked",
        G_CALLBACK(on_editPlaylist_activate),
        NULL);

    g_signal_connect((gpointer) toolbuttonConnect, "clicked",
        G_CALLBACK(on_deviceConnect_activate),
        NULL);
    g_signal_connect((gpointer) toolbuttonPreferences, "clicked",
        G_CALLBACK(on_preferences1_activate),
        NULL);

    g_signal_connect((gpointer) treeviewFiles, "row-activated",
        G_CALLBACK(fileListRowActivated),
        NULL);

    g_signal_connect_swapped(treeviewFiles, "button_press_event",
        G_CALLBACK(on_windowMainContextMenu_activate), contextMenu);

    g_signal_connect((gpointer) menu_view_filesize, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    g_signal_connect((gpointer) menu_view_filetype, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    g_signal_connect((gpointer) menu_view_track_number, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    g_signal_connect((gpointer) menu_view_title, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    g_signal_connect((gpointer) menu_view_artist, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    g_signal_connect((gpointer) menu_view_album, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    g_signal_connect((gpointer) menu_view_year, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    g_signal_connect((gpointer) menu_view_genre, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    g_signal_connect((gpointer) menu_view_duration, "toggled",
        G_CALLBACK(on_view_activate),
        NULL);

    gtk_window_add_accel_group(GTK_WINDOW(windowMain), accel_group);
    return windowMain;
}

// ************************************************************************************************

/**
 * Set the text on the status bar within the main window
 * @param text
 */
void statusBarSet(gchar *text) {
    statusBarClear();
    guint c_id1 = gtk_statusbar_get_context_id(GTK_STATUSBAR(windowStatusBar), "");
    gtk_statusbar_push(GTK_STATUSBAR(windowStatusBar), c_id1, text);
}

// ************************************************************************************************

/**
 * Clear the text within the status bar window.
 */
void statusBarClear() {
    guint c_id1 = gtk_statusbar_get_context_id(GTK_STATUSBAR(windowStatusBar), "");
    gtk_statusbar_pop(GTK_STATUSBAR(windowStatusBar), c_id1);
}

// ************************************************************************************************

/**
 * Toggle the active state of the buttons on the toolbar.
 * @param state
 */
void SetToolbarButtonState(gboolean state) {
    gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonAddFile), state);
    gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonRemoveFile), state);
    gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonRetrieve), state);
    gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonRescan), state);
    gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonAlbumArt), state);
    gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonPlaylist), state);
    gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonProperties), state);
    gtk_widget_set_sensitive(GTK_WIDGET(properties1), state);
    gtk_widget_set_sensitive(GTK_WIDGET(fileAdd), state);
    gtk_widget_set_sensitive(GTK_WIDGET(fileDownload), state);
    gtk_widget_set_sensitive(GTK_WIDGET(fileRemove), state);
    gtk_widget_set_sensitive(GTK_WIDGET(fileRename), state);
    gtk_widget_set_sensitive(GTK_WIDGET(fileNewFolder), state);
    gtk_widget_set_sensitive(GTK_WIDGET(fileRemoveFolder), state);
    gtk_widget_set_sensitive(GTK_WIDGET(fileRescan), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editDeviceName), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editFormatDevice), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editAddAlbumArt), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editPlaylist), state);
    gtk_widget_set_sensitive(GTK_WIDGET(treeviewFiles), state);
}

// ************************************************************************************************

/**
 * Construct the main file view within the main application window.
 * @param treeviewFiles
 */
void setupFileList(GtkTreeView *treeviewFiles) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // Filename column
    //renderer = gtk_cell_renderer_text_new();
    //column = gtk_tree_view_column_new_with_attributes(_("Filename"), renderer,
    //    "text", COL_FILENAME,
    //    NULL);

    column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title (column, _("Filename"));
    renderer = gtk_cell_renderer_pixbuf_new ();

    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "pixbuf", COL_ICON, NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", COL_FILENAME_ACTUAL, NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_sort_column_id(column, COL_FILENAME_HIDDEN);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_spacing(column, 5);

    // Filename column for sorting.
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Filename Hidden", renderer,
        "text", COL_FILENAME_HIDDEN,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);

    // File name actual - used for renaming operations.
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Filename Actual", renderer,
        "text", COL_FILENAME_ACTUAL,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);

    // File Size column
    renderer = gtk_cell_renderer_text_new();
    column_Size = gtk_tree_view_column_new_with_attributes(_("Size"), renderer,
        "text", COL_FILESIZE,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Size);
    gtk_tree_view_column_set_sort_column_id(column_Size, COL_FILESIZE_HID);
    gtk_tree_view_column_set_resizable(column_Size, TRUE);
    gtk_tree_view_column_set_spacing(column_Size, 5);
    gtk_tree_view_column_set_visible(column_Size, Preferences.view_size);

    // Folder/FileID column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Object ID", renderer,
        "text", COL_FILEID,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);

    // isFolder column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("isFolder", renderer,
        "text", COL_ISFOLDER,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);

    // File size column - hidden used for sorting the visible file size column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("FileSize Hidden", renderer,
        "text", COL_FILESIZE_HID,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);

    // File Type column
    renderer = gtk_cell_renderer_text_new();
    column_Type = gtk_tree_view_column_new_with_attributes(_("File Type"), renderer,
        "text", COL_TYPE,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Type);
    gtk_tree_view_column_set_sort_column_id(column_Type, COL_TYPE);
    gtk_tree_view_column_set_resizable(column_Type, TRUE);
    gtk_tree_view_column_set_spacing(column_Type, 5);
    gtk_tree_view_column_set_visible(column_Type, Preferences.view_type);

    // Track Number column
    renderer = gtk_cell_renderer_text_new();
    column_Track_Number = gtk_tree_view_column_new_with_attributes(_("Track"), renderer,
        "text", COL_TRACK_NUMBER,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Track_Number);
    gtk_tree_view_column_set_sort_column_id(column_Track_Number, COL_TRACK_NUMBER_HIDDEN);
    gtk_tree_view_column_set_resizable(column_Track_Number, TRUE);
    gtk_tree_view_column_set_spacing(column_Track_Number, 5);
    gtk_tree_view_column_set_visible(column_Track_Number, Preferences.view_track_number);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Track Num Hidden", renderer,
        "text", COL_TRACK_NUMBER_HIDDEN,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);

    // Track Title column
    renderer = gtk_cell_renderer_text_new();
    column_Title = gtk_tree_view_column_new_with_attributes(_("Track Name"), renderer,
        "text", COL_TITLE,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Title);
    gtk_tree_view_column_set_sort_column_id(column_Title, COL_TITLE);
    gtk_tree_view_column_set_resizable(column_Title, TRUE);
    gtk_tree_view_column_set_spacing(column_Title, 5);
    gtk_tree_view_column_set_visible(column_Title, Preferences.view_track_number);

    // Artist column
    renderer = gtk_cell_renderer_text_new();
    column_Artist = gtk_tree_view_column_new_with_attributes(_("Artist"), renderer,
        "text", COL_FL_ARTIST,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Artist);
    gtk_tree_view_column_set_sort_column_id(column_Artist, COL_FL_ARTIST);
    gtk_tree_view_column_set_resizable(column_Artist, TRUE);
    gtk_tree_view_column_set_spacing(column_Artist, 5);
    gtk_tree_view_column_set_visible(column_Artist, Preferences.view_artist);

    // Album column
    renderer = gtk_cell_renderer_text_new();
    column_Album = gtk_tree_view_column_new_with_attributes(_("Album"), renderer,
        "text", COL_FL_ALBUM,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Album);
    gtk_tree_view_column_set_sort_column_id(column_Album, COL_FL_ALBUM);
    gtk_tree_view_column_set_resizable(column_Album, TRUE);
    gtk_tree_view_column_set_spacing(column_Album, 5);
    gtk_tree_view_column_set_visible(column_Album, Preferences.view_album);

    // Year column
    renderer = gtk_cell_renderer_text_new();
    column_Year = gtk_tree_view_column_new_with_attributes(_("Year"), renderer,
        "text", COL_YEAR,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Year);
    gtk_tree_view_column_set_sort_column_id(column_Year, COL_YEAR);
    gtk_tree_view_column_set_resizable(column_Year, TRUE);
    gtk_tree_view_column_set_spacing(column_Year, 5);
    gtk_tree_view_column_set_visible(column_Year, Preferences.view_year);

    // Genre column
    renderer = gtk_cell_renderer_text_new();
    column_Genre = gtk_tree_view_column_new_with_attributes(_("Genre"), renderer,
        "text", COL_GENRE,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Genre);
    gtk_tree_view_column_set_sort_column_id(column_Genre, COL_GENRE);
    gtk_tree_view_column_set_resizable(column_Genre, TRUE);
    gtk_tree_view_column_set_spacing(column_Genre, 5);
    gtk_tree_view_column_set_visible(column_Genre, Preferences.view_genre);

    // Duration Visible column
    renderer = gtk_cell_renderer_text_new();
    column_Duration = gtk_tree_view_column_new_with_attributes(_("Duration"), renderer,
        "text", COL_DURATION,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column_Duration);
    gtk_tree_view_column_set_sort_column_id(column_Duration, COL_DURATION_HIDDEN);
    gtk_tree_view_column_set_resizable(column_Duration, TRUE);
    gtk_tree_view_column_set_spacing(column_Duration, 5);
    gtk_tree_view_column_set_visible(column_Duration, Preferences.view_duration);

    // Duration Hidden column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Duration Hidden", renderer,
        "text", COL_DURATION_HIDDEN,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);
}

// ************************************************************************************************

/**
 * Clear all entries within the main file window.
 * @return
 */
gboolean fileListClear() {
    gtk_list_store_clear(GTK_LIST_STORE(fileList));
    return TRUE;
}

// ************************************************************************************************

/**
 * Display the Add Files dialog box and add the files as selected.
 * @return List of files to add to the device.
 */
GSList* getFileGetList2Add() {
    GSList* files = NULL;
    GtkWidget *FileDialog;
    gchar *savepath = NULL;

    savepath = g_malloc0(8192);
    FileDialog = gtk_file_chooser_dialog_new(_("Select Files to Add"),
        GTK_WINDOW(windowMain), GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(FileDialog), TRUE);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemUploadPath->str);
    if (gtk_dialog_run(GTK_DIALOG(FileDialog)) == GTK_RESPONSE_ACCEPT) {
        savepath = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(FileDialog));
        // Save our upload path.
        Preferences.fileSystemUploadPath = g_string_assign(Preferences.fileSystemUploadPath, savepath);
        files = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(FileDialog));
    }

    gtk_widget_hide(FileDialog);
    gtk_widget_destroy(FileDialog);
    g_free(savepath);
    return files;
}

// ************************************************************************************************

/**
 * Add the applicable files to the file view in the main window.
 * @return
 */
gboolean fileListAdd() {
    GtkTreeIter rowIter;
    //gchar *filename = NULL;
    gchar *filename_hid = NULL;
    gchar *filesize = NULL;
    gchar *filetype = NULL;
    gchar *trackduration = NULL;
    gchar *tracknumber = NULL;
    gchar *fileext = NULL;
    LIBMTP_folder_t *tmpfolder;
    LIBMTP_file_t *tmpfile;
    guint parentID;
    GdkPixbuf *image = NULL;

    // We start with the folder list...
    if (currentFolderID != 0) {
        // If we are not folderID = 0; then...
        image = gdk_pixbuf_new_from_file(file_folder_png, NULL);
        // Scan the folder list for the current folderID, and set the parent ID,
        tmpfolder = deviceFolders;
        parentID = getParentFolderID(tmpfolder, currentFolderID);
        // Now add in the row information.
        gtk_list_store_append(GTK_LIST_STORE(fileList), &rowIter);
        gtk_list_store_set(GTK_LIST_STORE(fileList), &rowIter,
            //COL_FILENAME, "< .. >",
            COL_FILENAME_HIDDEN, "     < .. >",
            COL_FILENAME_ACTUAL, "..",
            COL_FILESIZE, "",
            COL_FILEID, parentID,
            COL_ISFOLDER, TRUE,
            COL_FILESIZE_HID, (guint64) 0,
            COL_ICON, image,
            -1);
        
        // Indicate we are done with this image.
        g_object_unref(image);

    }
    // What we scan for is the folder's details where 'parent_id' == currentFolderID and display those.
    tmpfolder = getParentFolderPtr(deviceFolders, currentFolderID);
    while (tmpfolder != NULL) {
        if ((tmpfolder->parent_id == currentFolderID) && (tmpfolder->storage_id == DeviceMgr.devicestorage->id)) {
            image = gdk_pixbuf_new_from_file(file_folder_png, NULL);
            gtk_list_store_append(GTK_LIST_STORE(fileList), &rowIter);
            //filename = g_strdup_printf("< %s >", tmpfolder->name);
            filename_hid = g_strdup_printf("     < %s >", tmpfolder->name);
            gtk_list_store_set(GTK_LIST_STORE(fileList), &rowIter,
                //COL_FILENAME, filename,
                COL_FILENAME_HIDDEN, filename_hid,
                COL_FILENAME_ACTUAL, tmpfolder->name,
                COL_FILESIZE, "",
                COL_FILEID, tmpfolder->folder_id,
                COL_ISFOLDER, TRUE,
                COL_FILESIZE_HID, (guint64) 0,
                COL_ICON, image,
                -1);
            //g_free(filename);
            g_free(filename_hid);
            // Indicate we are done with this image.
            g_object_unref(image);
        }
        tmpfolder = tmpfolder->sibling;
    }
    // We don't destroy the structure, only on a rescan operation.

    // We scan for files in the file details we 'parent_id' == currentFolderID and display those.
    tmpfile = deviceFiles;
    while (tmpfile != NULL) {
        if ((tmpfile->parent_id == currentFolderID) && (tmpfile->storage_id == DeviceMgr.devicestorage->id)) {
            gtk_list_store_append(GTK_LIST_STORE(fileList), &rowIter);

            if (tmpfile->filesize < 1000) {
                filesize = g_strdup_printf("%lu B", tmpfile->filesize);
            } else {
                if (tmpfile->filesize < (1000000)) {
                    filesize = g_strdup_printf("%.3f KB", (tmpfile->filesize / 1024.00));
                } else {
                    filesize = g_strdup_printf("%.3f MB", (tmpfile->filesize / (1024.00 * 1024.00)));
                }
            }

            fileext = rindex(tmpfile->filename, '.');
            // This accounts for the case with a filename without any "." (period).
            if (!fileext) {
                filetype = g_strconcat(g_ascii_strup(tmpfile->filename, -1), " File", NULL);
            } else {
                filetype = g_strconcat(g_ascii_strup(++fileext, -1), " File", NULL);
            }
            // Now if it's a track type file, eg OGG, WMA, MP3 or FLAC, get it's metadata.
            if ((tmpfile->filetype == LIBMTP_FILETYPE_MP3) ||
                (tmpfile->filetype == LIBMTP_FILETYPE_OGG) ||
                (tmpfile->filetype == LIBMTP_FILETYPE_FLAC) ||
                (tmpfile->filetype == LIBMTP_FILETYPE_WMA)) {
                LIBMTP_track_t *trackinfo;
                trackinfo = LIBMTP_Get_Trackmetadata(DeviceMgr.device, tmpfile->item_id);
                if (trackinfo != NULL) {
                    trackduration = g_strdup_printf("%d:%.2d", (int) ((trackinfo->duration / 1000) / 60),
                        (int) ((trackinfo->duration / 1000) % 60));
                    if (trackinfo->tracknumber != 0) {
                        tracknumber = g_strdup_printf("%d", trackinfo->tracknumber);
                    } else {
                        tracknumber = g_strdup(" ");
                    }
                    // Some basic sanitisation.
                    if (trackinfo->title == NULL) trackinfo->title = g_strdup("");
                    if (trackinfo->artist == NULL) trackinfo->artist = g_strdup("");
                    if (trackinfo->album == NULL) trackinfo->album = g_strdup("");
                    if (trackinfo->date == NULL) trackinfo->date = g_strdup("");
                    if (trackinfo->genre == NULL) trackinfo->genre = g_strdup("");

                    // Icon
                    image = gdk_pixbuf_new_from_file(file_audio_png, NULL);

                    gtk_list_store_set(GTK_LIST_STORE(fileList), &rowIter,
                        //COL_FILENAME, tmpfile->filename,
                        COL_FILENAME_HIDDEN, tmpfile->filename,
                        COL_FILENAME_ACTUAL, tmpfile->filename,
                        COL_FILESIZE, filesize,
                        COL_FILEID, tmpfile->item_id,
                        COL_ISFOLDER, FALSE,
                        COL_FILESIZE_HID, tmpfile->filesize,
                        COL_TYPE, filetype,
                        COL_TRACK_NUMBER, tracknumber,
                        COL_TRACK_NUMBER_HIDDEN, trackinfo->tracknumber,
                        COL_TITLE, trackinfo->title,
                        COL_FL_ARTIST, trackinfo->artist,
                        COL_FL_ALBUM, trackinfo->album,
                        COL_YEAR, trackinfo->date,
                        COL_GENRE, trackinfo->genre,
                        COL_DURATION, trackduration,
                        COL_DURATION_HIDDEN, trackinfo->duration,
                        COL_ICON, image,
                        -1);
                    g_free(trackduration);
                    g_free(tracknumber);
                    trackduration = NULL;
                    tracknumber = NULL;
                    // Indicate we are done with this image.
                    g_object_unref(image);
                    LIBMTP_destroy_track_t(trackinfo);
                } else {
                    LIBMTP_Dump_Errorstack(DeviceMgr.device);
                    LIBMTP_Clear_Errorstack(DeviceMgr.device);
                }
            } else {
                // Determine the file type.
                if(LIBMTP_FILETYPE_IS_AUDIO(tmpfile->filetype)){
                    image = gdk_pixbuf_new_from_file(file_audio_png, NULL);
                } else if(LIBMTP_FILETYPE_IS_AUDIOVIDEO(tmpfile->filetype)){
                    image = gdk_pixbuf_new_from_file(file_video_png, NULL);
                } else if(LIBMTP_FILETYPE_IS_VIDEO(tmpfile->filetype)){
                    image = gdk_pixbuf_new_from_file(file_video_png, NULL);
                } else if(LIBMTP_FILETYPE_IS_IMAGE(tmpfile->filetype)){
                    image = gdk_pixbuf_new_from_file(file_image_png, NULL);
                } else if(tmpfile->filetype == LIBMTP_FILETYPE_ALBUM) {
                    image = gdk_pixbuf_new_from_file(file_album_png, NULL);
                } else if(tmpfile->filetype == LIBMTP_FILETYPE_PLAYLIST) {
                    image = gdk_pixbuf_new_from_file(file_playlist_png, NULL);
                } else if(tmpfile->filetype == LIBMTP_FILETYPE_TEXT) {
                    image = gdk_pixbuf_new_from_file(file_textfile_png, NULL);
                } else {
                    image = gdk_pixbuf_new_from_file(file_generic_png, NULL);
                }

                // Otherwise just show the file information
                gtk_list_store_set(GTK_LIST_STORE(fileList), &rowIter,
                    //COL_FILENAME, tmpfile->filename,
                    COL_FILENAME_HIDDEN, tmpfile->filename,
                    COL_FILENAME_ACTUAL, tmpfile->filename,
                    COL_FILESIZE, filesize,
                    COL_FILEID, tmpfile->item_id,
                    COL_ISFOLDER, FALSE,
                    COL_FILESIZE_HID, tmpfile->filesize,
                    COL_TYPE, filetype,
                    COL_ICON, image,
                    -1);
                // Indicate we are done with this image.
                g_object_unref(image);
            }

            if (filetype != NULL)
                g_free(filetype);
            filetype = NULL;

            if (filesize != NULL)
                g_free(filesize);
            filesize = NULL;
        }
        tmpfile = tmpfile->next;
    }
    return TRUE;
}

// ************************************************************************************************

/**
 * Download the selected files.
 * @param List The files to download.
 * @return
 */
gboolean fileListDownload(GList *List) {
    GtkWidget *FileDialog;
    gchar *savepath = NULL;
    savepath = g_malloc0(8192);

    // Let's confirm our download path.

    if (Preferences.ask_download_path == TRUE) {
        FileDialog = gtk_file_chooser_dialog_new(_("Select Path to Download"),
            GTK_WINDOW(windowMain), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
            NULL);

        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemDownloadPath->str);
        if (gtk_dialog_run(GTK_DIALOG(FileDialog)) == GTK_RESPONSE_ACCEPT) {
            savepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileDialog));
            // Save our download path.
            Preferences.fileSystemDownloadPath = g_string_assign(Preferences.fileSystemDownloadPath, savepath);
        }
        gtk_widget_destroy(FileDialog);
    }
    g_free(savepath);

    // We do the deed.
    g_list_foreach(List, (GFunc) __fileDownload, NULL);
    fileoverwriteop = MTP_ASK;
    return TRUE;
}

// ************************************************************************************************

/**
 * Perform each file individually.
 * @param Row
 */
void __fileDownload(GtkTreeRowReference *Row) {
    GtkTreePath *path;
    GtkTreeIter iter;
    gchar *filename = NULL;
    gchar* fullfilename = NULL;

    gboolean isFolder;
    uint32_t objectID;


    fullfilename = g_malloc0(8192);
    // First of all, lets set the download path.

    // convert the referenece to a path and retrieve the iterator;
    path = gtk_tree_row_reference_get_path(Row);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
    // We have our Iter now.
    // Before we download, is it a folder ?
    gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_ISFOLDER, &isFolder, COL_FILENAME_ACTUAL, &filename, COL_FILEID, &objectID, -1);
    if (isFolder == FALSE) {
        // Our strings are not equal, so we get to download the file.
        g_sprintf(fullfilename, "%s/%s", Preferences.fileSystemDownloadPath->str, filename);
        // Now download the actual file from the MTP device.
        // Check if file exists?
        if (access(fullfilename, F_OK) != -1) {
            // We have that file already?
            if ((Preferences.prompt_overwrite_file_op == TRUE)) {
                if (fileoverwriteop == MTP_ASK) {
                    fileoverwriteop = displayFileOverwriteDialog(filename);
                }
                switch (fileoverwriteop) {
                    case MTP_ASK:
                        break;
                    case MTP_SKIP:
                        fileoverwriteop = MTP_ASK;
                        break;
                    case MTP_SKIP_ALL:
                        break;
                    case MTP_OVERWRITE:
                        filesDownload(filename, objectID);
                        fileoverwriteop = MTP_ASK;
                        break;
                    case MTP_OVERWRITE_ALL:
                        filesDownload(filename, objectID);
                        break;
                }
            } else {
                filesDownload(filename, objectID);
            }
        } else {
            filesDownload(filename, objectID);
        }
    } else {
        // Overwrite critera performed within this call...
        folderDownload(filename, objectID, TRUE);
    }
    g_free(filename);
    g_free(fullfilename);
}

// ************************************************************************************************

/**
 * Remove selected files from the device.
 * @param List
 * @return
 */
gboolean fileListRemove(GList *List) {
    // Clear any selection that is present.
    fileListClearSelection();
    // List is a list of Iter's to be removed
    g_list_foreach(List, (GFunc) __fileRemove, NULL);
    // We have 2 options, manually scan the file structure for that file and manually fix up...
    // or do a rescan...
    // I'll be cheap, and do a full rescan of the device.
    deviceRescan();
    return TRUE;
}

// ************************************************************************************************

/**
 * Remove each selected file from the device.
 * @param Row
 */
void __fileRemove(GtkTreeRowReference *Row) {
    GtkTreePath *path;
    GtkTreeIter iter;
    gchar* filename = NULL;
    uint32_t objectID;
    gboolean isFolder;
    // convert the referenece to a path and retrieve the iterator;
    path = gtk_tree_row_reference_get_path(Row);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
    // We have our Iter now.
    gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_ISFOLDER, &isFolder, COL_FILENAME_ACTUAL, &filename,
        COL_FILEID, &objectID, -1);
    if (isFolder == FALSE) {
        gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
        gtk_list_store_remove(GTK_LIST_STORE(fileList), &iter);
        // Now get rid of the actual file from the MTP device.
        filesDelete(filename, objectID);
    } else {
        // Our file is really a folder, so perform a folder remove operation.
        __folderRemove(Row);
    }
    g_free(filename);
}

// ************************************************************************************************

/**
 * Remove the selected folders from the device.
 * @param List
 * @return
 */
gboolean folderListRemove(GList *List) {
    // Clear any selection that is present.
    fileListClearSelection();
    // List is a list of Iter's to be removed
    g_list_foreach(List, (GFunc) __folderRemove, NULL);
    // We have 2 options, manually scan the file structure for that file and manually fix up...
    // or do a rescan...
    // I'll be cheap, and do a full rescan of the device.
    deviceRescan();
    return TRUE;
}

// ************************************************************************************************

/**
 * Remove the indivual folder from the device.
 * @param Row
 */
void __folderRemove(GtkTreeRowReference *Row) {
    GtkTreePath *path;
    GtkTreeIter iter;
    gchar* filename = NULL;
    uint32_t objectID;
    gboolean isFolder;
    // convert the referenece to a path and retrieve the iterator;
    path = gtk_tree_row_reference_get_path(Row);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
    // We have our Iter now.
    gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_ISFOLDER, &isFolder, COL_FILENAME_ACTUAL, &filename,
        COL_FILEID, &objectID, -1);
    if (isFolder == TRUE) {
        if (g_ascii_strcasecmp(filename, "< .. >") != 0) {
            gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
            gtk_list_store_remove(GTK_LIST_STORE(fileList), &iter);
            // Now get rid of the actual file from the MTP device.
            folderDelete(getCurrentFolderPtr(deviceFolders, objectID), 0);
        } else {
            g_fprintf(stderr, _("I don't know how to delete a parent folder reference?\n"));
        }
    } else {
        // Our folder is really a file, so delete the file instead.
        __fileRemove(Row);
    }
    g_free(filename);
}

// ************************************************************************************************

/**
 * Add an individual file to the device.
 * @param filename
 */
void __filesAdd(gchar* filename) {
    gchar* filename_stripped = NULL;

    filename_stripped = basename(filename);
    if (Preferences.prompt_overwrite_file_op == FALSE) {
        filesAdd(filename);
        return;
    }
    // I guess we want to know if we should replace the file, but first
    if (deviceoverwriteop == MTP_ASK) {
        if (fileExists(filename_stripped) == TRUE) {
            deviceoverwriteop = displayFileOverwriteDialog(filename_stripped);
            switch (deviceoverwriteop) {
                case MTP_ASK:
                    break;
                case MTP_SKIP:
                    deviceoverwriteop = MTP_ASK;
                    break;
                case MTP_SKIP_ALL:
                    break;
                case MTP_OVERWRITE:
                    filesAdd(filename);
                    deviceoverwriteop = MTP_ASK;
                    break;
                case MTP_OVERWRITE_ALL:
                    filesAdd(filename);
                    break;
            }
        } else {
            filesAdd(filename);
        }
    } else {
        if (deviceoverwriteop == MTP_OVERWRITE_ALL)
            filesAdd(filename);
    }
}

// ************************************************************************************************

/**
 * Get a GList of the TREE ROW REFERENCES that are selected.
 * @return
 */
GList* fileListGetSelection() {
    GList *selectedFiles, *ptr;
    GtkTreeRowReference *ref;
    GtkTreeModel *model;
    // Lets clear up the old list.
    g_list_free(fileSelection_RowReferences);
    fileSelection_RowReferences = NULL;

    if (gtk_tree_selection_count_selected_rows(fileSelection) == 0) {
        // We have no rows.
        return NULL;
    }
    // So now we must convert each selection to a row reference and store it in a new GList variable
    // which we will return below.
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeviewFiles));
    selectedFiles = gtk_tree_selection_get_selected_rows(fileSelection, &model);
    ptr = selectedFiles;
    while (ptr != NULL) {
        ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(fileList), (GtkTreePath*) ptr->data);
        fileSelection_RowReferences = g_list_prepend(fileSelection_RowReferences, gtk_tree_row_reference_copy(ref));
        gtk_tree_row_reference_free(ref);
        ptr = ptr->next;
    }
    g_list_foreach(selectedFiles, (GFunc) gtk_tree_path_free, NULL);
    g_list_free(selectedFiles);
    return fileSelection_RowReferences;
}

// ************************************************************************************************

/**
 * Clear all selected rows from the main file list.
 * @return
 */
gboolean fileListClearSelection() {
    if (fileSelection != NULL)
        gtk_tree_selection_unselect_all(fileSelection);
    return TRUE;
}

// ************************************************************************************************

/**
 * Create the Preferences Dialog Box.
 * @return
 */
GtkWidget* create_windowPreferences(void) {
    GtkWidget *windowDialog;
    GtkWidget *vbox1;
    GtkWidget *frame1;
    GtkWidget *alignment1;

    GtkWidget *frame3;
    GtkWidget *alignment3;
    GtkWidget *alignment4;
    GtkWidget *labelPlaylist;
    GtkWidget *frame4;
    GtkWidget *alignment5;
    GtkWidget *alignment6;
    GtkWidget *vbox4;
    GtkWidget *vbox2;

    GtkWidget *labelDevice;
    GtkWidget *frame2;
    GtkWidget *alignment2;
    GtkWidget *vbox3;

    GtkWidget *table1;
    GtkWidget *labelDownloadPath;
    GtkWidget *labelUploadPath;

    GtkWidget *buttonDownloadPath;
    GtkWidget *buttonUploadPath;
    GtkWidget *labelFilePath;
    GtkWidget *hbox1;
    GtkWidget *buttonClose;

    windowDialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gchar * winTitle;
    winTitle = g_strconcat(PACKAGE_TITLE, " v", PACKAGE_VERSION, _(" Preferences"), NULL);
    gtk_window_set_title(GTK_WINDOW(windowDialog), winTitle);
    gtk_window_set_modal(GTK_WINDOW(windowDialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(windowDialog), GTK_WINDOW(windowMain));
    gtk_window_set_position(GTK_WINDOW(windowDialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable(GTK_WINDOW(windowDialog), FALSE);
    gtk_window_set_type_hint(GTK_WINDOW(windowDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    g_free(winTitle);

    vbox1 = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(windowDialog), vbox1);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 5);

    frame1 = gtk_frame_new(NULL);
    gtk_widget_show(frame1);
    gtk_box_pack_start(GTK_BOX(vbox1), frame1, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_NONE);
    alignment1 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment1);
    gtk_container_add(GTK_CONTAINER(frame1), alignment1);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment1), 0, 0, 12, 0);

    checkbuttonDeviceConnect = gtk_check_button_new_with_mnemonic(_("Attempt to connect to Device on startup"));
    gtk_widget_show(checkbuttonDeviceConnect);
    gtk_container_add(GTK_CONTAINER(alignment1), checkbuttonDeviceConnect);

    labelDevice = gtk_label_new(_("<b>Device</b>"));
    gtk_widget_show(labelDevice);
    gtk_frame_set_label_widget(GTK_FRAME(frame1), labelDevice);
    gtk_label_set_use_markup(GTK_LABEL(labelDevice), TRUE);

    frame3 = gtk_frame_new(NULL);
    gtk_widget_show(frame3);
    gtk_box_pack_start(GTK_BOX(vbox1), frame3, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame3), GTK_SHADOW_NONE);

    vbox2 = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox2);
    gtk_container_add(GTK_CONTAINER(frame3), vbox2);

    alignment3 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment3);
    gtk_container_add(GTK_CONTAINER(vbox2), alignment3);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment3), 0, 0, 12, 0);

    alignment4 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment4);
    gtk_container_add(GTK_CONTAINER(vbox2), alignment4);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment4), 0, 0, 12, 0);

    checkbuttonConfirmFileOp = gtk_check_button_new_with_mnemonic(_("Confirm File/Folder Delete"));
    gtk_widget_show(checkbuttonConfirmFileOp);
    gtk_container_add(GTK_CONTAINER(alignment3), checkbuttonConfirmFileOp);

    checkbuttonConfirmOverWriteFileOp = gtk_check_button_new_with_mnemonic(_("Prompt if to Overwrite file if already exists"));
    gtk_widget_show(checkbuttonConfirmOverWriteFileOp);
    gtk_container_add(GTK_CONTAINER(alignment4), checkbuttonConfirmOverWriteFileOp);

    // Playlist Frame.

    frame4 = gtk_frame_new(NULL);
    gtk_widget_show(frame4);
    gtk_box_pack_start(GTK_BOX(vbox1), frame4, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame4), GTK_SHADOW_NONE);

    labelPlaylist = gtk_label_new(_("<b>Playlist</b>"));
    gtk_widget_show(labelPlaylist);
    gtk_frame_set_label_widget(GTK_FRAME(frame4), labelPlaylist);
    gtk_label_set_use_markup(GTK_LABEL(labelPlaylist), TRUE);

    vbox4 = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox4);
    gtk_container_add(GTK_CONTAINER(frame4), vbox4);
    
    alignment5 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment5);
    gtk_container_add(GTK_CONTAINER(vbox4), alignment5);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment5), 0, 0, 12, 0);

    alignment6 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment6);
    gtk_container_add(GTK_CONTAINER(vbox4), alignment6);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment6), 0, 0, 12, 0);

    checkbuttonAutoAddTrackPlaylist = gtk_check_button_new_with_mnemonic(_("Prompt to add New Music track to existing playlist"));
    gtk_widget_show(checkbuttonAutoAddTrackPlaylist);
    gtk_container_add(GTK_CONTAINER(alignment5), checkbuttonAutoAddTrackPlaylist);

    checkbuttonIgnorePathInPlaylist = gtk_check_button_new_with_mnemonic(_("Ignore path information when importing playlist"));
    gtk_widget_show(checkbuttonIgnorePathInPlaylist);
    gtk_container_add(GTK_CONTAINER(alignment6), checkbuttonIgnorePathInPlaylist);

    labelDevice = gtk_label_new(_("<b>File Operations</b>"));
    gtk_widget_show(labelDevice);
    gtk_frame_set_label_widget(GTK_FRAME(frame3), labelDevice);
    gtk_label_set_use_markup(GTK_LABEL(labelDevice), TRUE);

    frame2 = gtk_frame_new(NULL);
    gtk_widget_show(frame2);
    gtk_box_pack_start(GTK_BOX(vbox1), frame2, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_NONE);

    alignment2 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment2);
    gtk_container_add(GTK_CONTAINER(frame2), alignment2);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment2), 0, 0, 12, 0);

    vbox3 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox3);
    gtk_container_add(GTK_CONTAINER(alignment2), vbox3);

    checkbuttonDownloadPath = gtk_check_button_new_with_mnemonic(_("Always show Download Path?"));
    gtk_widget_show(checkbuttonDownloadPath);
    gtk_box_pack_start(GTK_BOX(vbox3), checkbuttonDownloadPath, FALSE, FALSE, 0);

    table1 = gtk_table_new(2, 3, FALSE);
    gtk_widget_show(table1);
    gtk_box_pack_start(GTK_BOX(vbox3), table1, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(table1), 5);
    gtk_table_set_row_spacings(GTK_TABLE(table1), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table1), 5);

    labelDownloadPath = gtk_label_new(_("Download Path:"));
    gtk_widget_show(labelDownloadPath);
    gtk_table_attach(GTK_TABLE(table1), labelDownloadPath, 0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelDownloadPath), 0, 0.5);

    labelUploadPath = gtk_label_new(_("Upload Path:"));
    gtk_widget_show(labelUploadPath);
    gtk_table_attach(GTK_TABLE(table1), labelUploadPath, 0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelUploadPath), 0, 0.5);

    entryDownloadPath = gtk_entry_new();
    gtk_widget_show(entryDownloadPath);
    gtk_editable_set_editable(GTK_EDITABLE(entryDownloadPath), FALSE);
    gtk_table_attach(GTK_TABLE(table1), entryDownloadPath, 1, 2, 0, 1,
        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);

    entryUploadPath = gtk_entry_new();
    gtk_widget_show(entryUploadPath);
    gtk_editable_set_editable(GTK_EDITABLE(entryUploadPath), FALSE);
    gtk_table_attach(GTK_TABLE(table1), entryUploadPath, 1, 2, 1, 2,
        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);

    buttonDownloadPath = gtk_button_new_with_mnemonic(("..."));
    gtk_widget_show(buttonDownloadPath);
    gtk_table_attach(GTK_TABLE(table1), buttonDownloadPath, 2, 3, 0, 1,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);

    buttonUploadPath = gtk_button_new_with_mnemonic(("..."));
    gtk_widget_show(buttonUploadPath);
    gtk_table_attach(GTK_TABLE(table1), buttonUploadPath, 2, 3, 1, 2,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);

    labelFilePath = gtk_label_new(_("<b>Filepaths on PC</b>"));
    gtk_widget_show(labelFilePath);
    gtk_frame_set_label_widget(GTK_FRAME(frame2), labelFilePath);
    gtk_label_set_use_markup(GTK_LABEL(labelFilePath), TRUE);

    // Now do the ask confirm delete...

    hbox1 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox1);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);

    buttonClose = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_widget_show(buttonClose);
    gtk_box_pack_end(GTK_BOX(hbox1), buttonClose, FALSE, FALSE, 0);

    g_signal_connect((gpointer) windowDialog, "destroy",
        G_CALLBACK(on_quitPrefs_activate),
        NULL);

    g_signal_connect((gpointer) buttonClose, "clicked",
        G_CALLBACK(on_quitPrefs_activate),
        NULL);

    g_signal_connect((gpointer) checkbuttonDeviceConnect, "toggled",
        G_CALLBACK(on_PrefsDevice_activate),
        NULL);

    g_signal_connect((gpointer) checkbuttonConfirmFileOp, "toggled",
        G_CALLBACK(on_PrefsConfirmDelete_activate),
        NULL);

    g_signal_connect((gpointer) checkbuttonConfirmOverWriteFileOp, "toggled",
        G_CALLBACK(on_PrefsConfirmOverWriteFileOp_activate),
        NULL);

    g_signal_connect((gpointer) checkbuttonAutoAddTrackPlaylist, "toggled",
        G_CALLBACK(on_PrefsAutoAddTrackPlaylist_activate),
        NULL);

    g_signal_connect((gpointer) checkbuttonIgnorePathInPlaylist, "toggled",
        G_CALLBACK(on_PrefsIgnorePathInPlaylist_activate),
        NULL);

    g_signal_connect((gpointer) checkbuttonDownloadPath, "toggled",
        G_CALLBACK(on_PrefsAskDownload_activate),
        NULL);

    g_signal_connect((gpointer) buttonDownloadPath, "clicked",
        G_CALLBACK(on_PrefsDownloadPath_activate),
        NULL);

    g_signal_connect((gpointer) buttonUploadPath, "clicked",
        G_CALLBACK(on_PrefsUploadPath_activate),
        NULL);

    // And now set the fields.

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonDeviceConnect), Preferences.attemptDeviceConnectOnStart);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonDownloadPath), Preferences.ask_download_path);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmFileOp), Preferences.confirm_file_delete_op);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmOverWriteFileOp), Preferences.prompt_overwrite_file_op);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonAutoAddTrackPlaylist), Preferences.auto_add_track_to_playlist);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonIgnorePathInPlaylist), Preferences.ignore_path_in_playlist_import);
    gtk_entry_set_text(GTK_ENTRY(entryDownloadPath), Preferences.fileSystemDownloadPath->str);
    gtk_entry_set_text(GTK_ENTRY(entryUploadPath), Preferences.fileSystemUploadPath->str);

    // To save the fields, we use callbacks on the widgets via gconf.

    return windowDialog;
}

// ************************************************************************************************

/**
 * Create the Properties Dialog Box.
 * @return
 */
GtkWidget* create_windowProperties() {
    GtkWidget *windowDialog;
    GtkWidget *windowNotebook;
    GtkWidget *vbox1;
    GtkWidget *vbox2;
    GtkWidget *alignment2;
    GtkWidget *table2;
    GtkWidget *label15;
    GtkWidget *labelName;
    GtkWidget *labelModel;
    GtkWidget *labelSerial;
    GtkWidget *labelBattery;
    GtkWidget *labelManufacturer;
    GtkWidget *labelDeviceVer;
    GtkWidget *label26;
    GtkWidget *label29;
    GtkWidget *label28;
    GtkWidget *label27;
    GtkWidget *label17;
    GtkWidget *label25;
    GtkWidget *label18;
    GtkWidget *label19;
    GtkWidget *label16;
    GtkWidget *labelStorage;
    GtkWidget *labelSupportedFormat;
    GtkWidget *labelSecTime;
    GtkWidget *labelSyncPartner;
    GtkWidget *label2;
    GtkWidget *alignment1;
    GtkWidget *table1;
    GtkWidget *label3;
    GtkWidget *label4;
    GtkWidget *label5;
    GtkWidget *label6;
    GtkWidget *label7;
    GtkWidget *label8;
    GtkWidget *labelDeviceVendor;
    GtkWidget *labelDeviceProduct;
    GtkWidget *labelVenID;
    GtkWidget *labelProdID;
    GtkWidget *labelBusLoc;
    GtkWidget *labelDevNum;
    GtkWidget *label1;
    GtkWidget *hbox2;
    GtkWidget *buttonClose;

    GtkWidget *label50;

    GString *tmp_string2;
    gchar *tmp_string = NULL;

    windowDialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gchar * winTitle;
    winTitle = g_strconcat(DeviceMgr.devicename->str, _(" Properties"), NULL);
    gtk_window_set_title(GTK_WINDOW(windowDialog), winTitle);
    gtk_window_set_modal(GTK_WINDOW(windowDialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(windowDialog), GTK_WINDOW(windowMain));
    gtk_window_set_position(GTK_WINDOW(windowDialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable(GTK_WINDOW(windowDialog), FALSE);
    gtk_window_set_type_hint(GTK_WINDOW(windowDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    g_free(winTitle);

    // Main Window
    vbox1 = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 5);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(windowDialog), vbox1);

    // Device Properties Pane
    label2 = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label2), _("<b>MTP Device Properties</b>"));
    gtk_widget_show(label2);

    alignment2 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment2);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment2), 0, 0, 12, 0);

    // Raw Device Information Pane
    label1 = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label1), _("<b>Raw Device Information</b>"));
    gtk_widget_show(label1);

    alignment1 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment1);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment1), 0, 0, 12, 0);

    // Build the Notebook.
    windowNotebook = gtk_notebook_new();
    gtk_widget_show(windowNotebook);
    gtk_notebook_append_page(GTK_NOTEBOOK(windowNotebook), alignment2, label2 );
    gtk_notebook_append_page(GTK_NOTEBOOK(windowNotebook), alignment1, label1 );
    gtk_container_add(GTK_CONTAINER(vbox1), windowNotebook);

    // Start the Device Properties Pane.
    table2 = gtk_table_new(10, 2, FALSE);
    gtk_widget_show(table2);
    gtk_container_add(GTK_CONTAINER(alignment2), table2);
    gtk_container_set_border_width(GTK_CONTAINER(table2), 5);
    gtk_table_set_row_spacings(GTK_TABLE(table2), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table2), 5);

    label15 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label15), _("<b>Name:</b>"));
    gtk_widget_show(label15);
    gtk_table_attach(GTK_TABLE(table2), label15, 0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label15), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label15), 0, 1);

    labelName = gtk_label_new(("label20"));
    gtk_widget_show(labelName);
    gtk_table_attach(GTK_TABLE(table2), labelName, 1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelName), 0, 0.5);

    labelModel = gtk_label_new(("label21"));
    gtk_widget_show(labelModel);
    gtk_table_attach(GTK_TABLE(table2), labelModel, 1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelModel), 0, 0.5);

    labelSerial = gtk_label_new(("label22"));
    gtk_widget_show(labelSerial);
    gtk_table_attach(GTK_TABLE(table2), labelSerial, 1, 2, 2, 3,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelSerial), 0, 0.5);

    labelBattery = gtk_label_new(("label24"));
    gtk_widget_show(labelBattery);
    gtk_table_attach(GTK_TABLE(table2), labelBattery, 1, 2, 5, 6,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelBattery), 0, 0.5);

    labelManufacturer = gtk_label_new(("label23"));
    gtk_widget_show(labelManufacturer);
    gtk_table_attach(GTK_TABLE(table2), labelManufacturer, 1, 2, 4, 5,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelManufacturer), 0, 0.5);

    labelDeviceVer = gtk_label_new(("label26"));
    gtk_widget_show(labelDeviceVer);
    gtk_table_attach(GTK_TABLE(table2), labelDeviceVer, 1, 2, 3, 4,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelDeviceVer), 0, 0.5);

    label26 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label26), _("<b>Model Number:</b>"));
    gtk_widget_show(label26);
    gtk_table_attach(GTK_TABLE(table2), label26, 0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label26), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label26), 0, 1);

    label29 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label29), _("<b>Serial Number:</b>"));
    gtk_widget_show(label29);
    gtk_table_attach(GTK_TABLE(table2), label29, 0, 1, 2, 3,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label29), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label29), 0, 1);

    label28 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label28), _("<b>Device Version:</b>"));
    gtk_widget_show(label28);
    gtk_table_attach(GTK_TABLE(table2), label28, 0, 1, 3, 4,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label28), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label28), 0, 1);

    label27 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label27), _("<b>Manufacturer:</b>"));
    gtk_widget_show(label27);
    gtk_table_attach(GTK_TABLE(table2), label27, 0, 1, 4, 5,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label27), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label27), 0, 1);

    label17 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label17), _("<b>Battery Level:</b>"));
    gtk_widget_show(label17);
    gtk_table_attach(GTK_TABLE(table2), label17, 0, 1, 5, 6,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label17), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label17), 0, 1);

    label25 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label25), _("<b>Storage:</b>"));
    gtk_widget_show(label25);
    gtk_table_attach(GTK_TABLE(table2), label25, 0, 1, 6, 7,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label25), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label25), 0, 1);

    vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox2);
    //gtk_container_add (GTK_CONTAINER (windowDialog), vbox1);
    gtk_table_attach(GTK_TABLE(table2), vbox2, 0, 1, 7, 8,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    label18 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label18), _("<b>Supported Formats:</b>"));
    gtk_box_pack_start(GTK_BOX(vbox2), label18, FALSE, TRUE, 0);
    gtk_widget_show(label18);
    gtk_label_set_justify(GTK_LABEL(label18), GTK_JUSTIFY_RIGHT);
    //gtk_misc_set_alignment (GTK_MISC (label18), 1, 0);
    gtk_label_set_line_wrap(GTK_LABEL(label18), TRUE);
    gtk_misc_set_alignment(GTK_MISC(label18), 0, 1);


    label50 = gtk_label_new((""));
    gtk_box_pack_start(GTK_BOX(vbox2), label50, FALSE, TRUE, 0);
    gtk_widget_show(label50);


    label19 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label19), _("<b>Secure Time:</b>"));
    gtk_widget_show(label19);
    gtk_table_attach(GTK_TABLE(table2), label19, 0, 1, 8, 9,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label19), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label19), 0, 1);

    label16 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label16), _("<b>Sync Partner:</b>"));
    gtk_widget_show(label16);
    gtk_table_attach(GTK_TABLE(table2), label16, 0, 1, 9, 10,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label16), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label16), 0, 1);

    labelStorage = gtk_label_new(("label30"));
    gtk_widget_show(labelStorage);
    gtk_table_attach(GTK_TABLE(table2), labelStorage, 1, 2, 6, 7,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelStorage), 0, 0.5);

    labelSupportedFormat = gtk_label_new(("label31"));
    gtk_widget_show(labelSupportedFormat);
    gtk_table_attach(GTK_TABLE(table2), labelSupportedFormat, 1, 2, 7, 8,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_line_wrap(GTK_LABEL(labelSupportedFormat), TRUE);
    gtk_misc_set_alignment(GTK_MISC(labelSupportedFormat), 0, 0.5);

    labelSecTime = gtk_label_new(("label32"));
    gtk_widget_show(labelSecTime);
    gtk_table_attach(GTK_TABLE(table2), labelSecTime, 1, 2, 8, 9,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_line_wrap(GTK_LABEL(labelSecTime), TRUE);
    gtk_misc_set_alignment(GTK_MISC(labelSecTime), 0, 0.5);

    labelSyncPartner = gtk_label_new(("label33"));
    gtk_widget_show(labelSyncPartner);
    gtk_table_attach(GTK_TABLE(table2), labelSyncPartner, 1, 2, 9, 10,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelSyncPartner), 0, 0.5);

     // Start the Raw Device Pane.
    table1 = gtk_table_new(6, 2, FALSE);
    gtk_widget_show(table1);
    gtk_container_add(GTK_CONTAINER(alignment1), table1);
    gtk_container_set_border_width(GTK_CONTAINER(table1), 5);
    gtk_table_set_row_spacings(GTK_TABLE(table1), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table1), 5);

    label3 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label3), _("<b>Vendor:</b>"));
    gtk_widget_show(label3);
    gtk_table_attach(GTK_TABLE(table1), label3, 0, 1, 0, 1,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label3), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label3), 0, 1);

    label4 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label4), _("<b>Product:</b>"));
    gtk_widget_show(label4);
    gtk_table_attach(GTK_TABLE(table1), label4, 0, 1, 1, 2,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label4), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label4), 0, 1);

    label5 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label5), _("<b>Vendor ID:</b>"));
    gtk_widget_show(label5);
    gtk_table_attach(GTK_TABLE(table1), label5, 0, 1, 2, 3,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label5), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label5), 0, 1);

    label6 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label6), _("<b>Product ID:</b>"));
    gtk_widget_show(label6);
    gtk_table_attach(GTK_TABLE(table1), label6, 0, 1, 3, 4,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label6), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label6), 0, 1);

    label7 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label7), _("<b>Device Number:</b>"));
    gtk_widget_show(label7);
    gtk_table_attach(GTK_TABLE(table1), label7, 0, 1, 5, 6,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label7), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label7), 0, 1);

    label8 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label8), _("<b>Bus Location:</b>"));
    gtk_widget_show(label8);
    gtk_table_attach(GTK_TABLE(table1), label8, 0, 1, 4, 5,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify(GTK_LABEL(label8), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment(GTK_MISC(label8), 0, 1);

    labelDeviceVendor = gtk_label_new(("label9"));
    gtk_widget_show(labelDeviceVendor);
    gtk_table_attach(GTK_TABLE(table1), labelDeviceVendor, 1, 2, 0, 1,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelDeviceVendor), 0, 0.5);

    labelDeviceProduct = gtk_label_new(("label10"));
    gtk_widget_show(labelDeviceProduct);
    gtk_table_attach(GTK_TABLE(table1), labelDeviceProduct, 1, 2, 1, 2,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelDeviceProduct), 0, 0.5);

    labelVenID = gtk_label_new(("label11"));
    gtk_widget_show(labelVenID);
    gtk_table_attach(GTK_TABLE(table1), labelVenID, 1, 2, 2, 3,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelVenID), 0, 0.5);

    labelProdID = gtk_label_new(("label12"));
    gtk_widget_show(labelProdID);
    gtk_table_attach(GTK_TABLE(table1), labelProdID, 1, 2, 3, 4,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelProdID), 0, 0.5);

    labelBusLoc = gtk_label_new(("label13"));
    gtk_widget_show(labelBusLoc);
    gtk_table_attach(GTK_TABLE(table1), labelBusLoc, 1, 2, 4, 5,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelBusLoc), 0, 0.5);

    labelDevNum = gtk_label_new(("label14"));
    gtk_widget_show(labelDevNum);
    gtk_table_attach(GTK_TABLE(table1), labelDevNum, 1, 2, 5, 6,
        (GtkAttachOptions) (GTK_FILL),
        (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment(GTK_MISC(labelDevNum), 0, 0.5);

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox2);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox2, TRUE, TRUE, 0);

    buttonClose = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_widget_show(buttonClose);
    gtk_box_pack_end(GTK_BOX(hbox2), buttonClose, FALSE, FALSE, 0);

    g_signal_connect((gpointer) windowDialog, "destroy",
        G_CALLBACK(on_quitProp_activate),
        NULL);

    g_signal_connect((gpointer) buttonClose, "clicked",
        G_CALLBACK(on_quitProp_activate),
        NULL);

    // Now we need to update our strings for the information on the unit.
    gtk_label_set_text(GTK_LABEL(labelName), DeviceMgr.devicename->str);
    gtk_label_set_text(GTK_LABEL(labelModel), DeviceMgr.modelname->str);
    gtk_label_set_text(GTK_LABEL(labelSerial), DeviceMgr.serialnumber->str);
    if (DeviceMgr.maxbattlevel != 0) {
        tmp_string = g_strdup_printf("%d / %d (%d%%)", DeviceMgr.currbattlevel, DeviceMgr.maxbattlevel,
            (int) (((float) DeviceMgr.currbattlevel / (float) DeviceMgr.maxbattlevel) * 100.0));
    } else {
        tmp_string = g_strdup_printf("%d / %d", DeviceMgr.currbattlevel, DeviceMgr.maxbattlevel);
    }
    gtk_label_set_text(GTK_LABEL(labelBattery), tmp_string);
    gtk_label_set_text(GTK_LABEL(labelManufacturer), DeviceMgr.manufacturername->str);
    gtk_label_set_text(GTK_LABEL(labelDeviceVer), DeviceMgr.deviceversion->str);


    if (DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE) {
        tmp_string = g_strdup_printf(_("%d MB (free) / %d MB (total)"),
            (int) (DeviceMgr.devicestorage->FreeSpaceInBytes / MEGABYTE),
            (int) (DeviceMgr.devicestorage->MaxCapacity / MEGABYTE));
        gtk_label_set_text(GTK_LABEL(labelStorage), tmp_string);
    } else {
        tmp_string2 = g_string_new("");
        // Cycle through each storage device and list the name and capacity.
        LIBMTP_devicestorage_t* deviceStorage = DeviceMgr.device->storage;
        while (deviceStorage != NULL) {
            if (tmp_string2->len > 0)
                tmp_string2 = g_string_append(tmp_string2, "\n");
            if (deviceStorage->StorageDescription != NULL) {
                tmp_string2 = g_string_append(tmp_string2, deviceStorage->StorageDescription);
            } else {
                tmp_string2 = g_string_append(tmp_string2, deviceStorage->VolumeIdentifier);
            }
            tmp_string = g_strdup_printf(" : %d MB (free) / %d MB (total)",
                (int) (deviceStorage->FreeSpaceInBytes / MEGABYTE),
                (int) (deviceStorage->MaxCapacity / MEGABYTE));
            tmp_string2 = g_string_append(tmp_string2, tmp_string);
            deviceStorage = deviceStorage->next;
        }
        gtk_label_set_text(GTK_LABEL(labelStorage), tmp_string2->str);
        g_string_free(tmp_string2, TRUE);
    }

    tmp_string2 = g_string_new("");
    // Build a string for us to use.
    gint i = 0;
    for (i = 0; i < DeviceMgr.filetypes_len; i++) {
        if (tmp_string2->len > 0)
            tmp_string2 = g_string_append(tmp_string2, "\n");
        tmp_string2 = g_string_append(tmp_string2, LIBMTP_Get_Filetype_Description(DeviceMgr.filetypes[i]));
    }

    gtk_label_set_text(GTK_LABEL(labelSupportedFormat), tmp_string2->str);
    g_string_free(tmp_string2, TRUE);

    gtk_label_set_text(GTK_LABEL(labelSecTime), DeviceMgr.sectime->str);
    gtk_label_set_text(GTK_LABEL(labelSyncPartner), DeviceMgr.syncpartner->str);

    // This is our raw information.
    gtk_label_set_text(GTK_LABEL(labelDeviceVendor), DeviceMgr.Vendor->str);
    gtk_label_set_text(GTK_LABEL(labelDeviceProduct), DeviceMgr.Product->str);
    gtk_label_set_text(GTK_LABEL(labelVenID),  g_strdup_printf("0x%x", DeviceMgr.VendorID));
    gtk_label_set_text(GTK_LABEL(labelProdID), g_strdup_printf("0x%x", DeviceMgr.ProductID));
    gtk_label_set_text(GTK_LABEL(labelBusLoc), g_strdup_printf("0x%x", DeviceMgr.BusLoc));
    gtk_label_set_text(GTK_LABEL(labelDevNum), g_strdup_printf("0x%x", DeviceMgr.DeviceID));

    return windowDialog;
}

// ************************************************************************************************

/**
 * Create a Upload/Download Progress Window.
 * @param msg Default message to be displayed.
 * @return
 */
GtkWidget* create_windowProgressDialog(gchar* msg) {
    GtkWidget *window1;
    GtkWidget *vbox1;
    GtkWidget *hbox1;
    GtkWidget *label_FileProgress;
    GtkWidget *label1;
    GtkWidget *cancelButton;
    GtkWidget *progressbar_Main;

    window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gchar * winTitle;
    winTitle = g_strconcat(PACKAGE_TITLE, " v", PACKAGE_VERSION, NULL);
    gtk_window_set_title(GTK_WINDOW(window1), winTitle);
    gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal(GTK_WINDOW(window1), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(window1), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(window1), GTK_WINDOW(windowMain));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window1), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(window1), GDK_WINDOW_TYPE_HINT_DIALOG);
    g_free(winTitle);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(window1), vbox1);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 10);
    gtk_box_set_spacing(GTK_BOX(vbox1), 5);

    label1 = gtk_label_new(NULL);
    winTitle = g_strconcat("<b><big>", msg, "</big></b>", NULL);
    gtk_label_set_markup(GTK_LABEL(label1), winTitle);
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(vbox1), label1, TRUE, TRUE, 0);
    gtk_misc_set_padding(GTK_MISC(label1), 0, 5);
    gtk_misc_set_alignment(GTK_MISC(label1), 0, 0);
    g_free(winTitle);

    label_FileProgress = gtk_label_new(_("file = ( x / x ) x %"));
    gtk_widget_set_size_request(label_FileProgress, 320, -1);
    gtk_widget_show(label_FileProgress);
    gtk_box_pack_start(GTK_BOX(vbox1), label_FileProgress, TRUE, TRUE, 0);
    gtk_misc_set_padding(GTK_MISC(label_FileProgress), 0, 5);

    progressbar_Main = gtk_progress_bar_new();
    gtk_widget_show(progressbar_Main);
    gtk_box_pack_start(GTK_BOX(vbox1), progressbar_Main, TRUE, TRUE, 0);

    // Insert a cancel button.
    hbox1 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox1);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);
    cancelButton = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_widget_show(cancelButton);
    gtk_box_pack_end(GTK_BOX(hbox1), cancelButton, FALSE, FALSE, 0);

    progressDialog = window1;
    progressDialog_Text = label_FileProgress;
    progressDialog_Bar = progressbar_Main;

    g_signal_connect((gpointer) cancelButton, "clicked",
        G_CALLBACK(on_progressDialog_Cancel),
        NULL);

    return window1;
}

// ************************************************************************************************

/**
 * Display the File Progress Window.
 * @param msg Message to be displayed.
 */
void displayProgressBar(gchar *msg) {
    // No idea how this could come about, but we should take it into account so we don't have a memleak
    // due to recreating the window multiple times.
    if (progressDialog != NULL) {
        destroyProgressBar();
    }
    // create our progress window.
    progressDialog = create_windowProgressDialog(msg);
    progressDialog_killed = FALSE;
    // Attach a callback to get notification that it has closed.
    g_signal_connect((gpointer) progressDialog, "destroy",
        G_CALLBACK(on_progressDialog_Close),
        NULL);

    // Show the progress window.
    gtk_widget_show_all(progressDialog);
}

// ************************************************************************************************

/**
 * Destroy the Progress Window.
 */
void destroyProgressBar(void) {
    if (progressDialog_killed == FALSE) {
        gtk_widget_hide(progressDialog);
        gtk_widget_destroy(progressDialog);
    }
    g_free(progressDialog_filename);
    progressDialog = NULL;
    progressDialog_Text = NULL;
    progressDialog_Bar = NULL;
    progressDialog_killed = FALSE;
}

// ************************************************************************************************

/**
 * Update the filename displayed in the Progress Window.
 * @param filename
 */
void setProgressFilename(gchar* filename) {
    progressDialog_filename = g_strdup(filename);
}

// ************************************************************************************************

/**
 * Callback to handle updating the Progress Window.
 * @param sent
 * @param total
 * @param data
 * @return
 */
int fileprogress(const uint64_t sent, const uint64_t total, void const * const data) {
    gchar* tmp_string;
    gint percent = (sent * 100) / total;

    // See if our dialog box was killed, and if so, just return which also kill our download/upload...
    if (progressDialog_killed == TRUE)
        return TRUE;

    // Now update the progress dialog.
    if (progressDialog != NULL) {
        if (progressDialog_filename != NULL) {
            tmp_string = g_strdup_printf(_("%s - %lluKB of %lluKB (%d%%)"), progressDialog_filename,
                (sent / 1024), (total / 1024), percent);
        } else {
            tmp_string = g_strdup_printf(_("%lluKB of %lluKB (%d%%)"),
                (sent / 1024), (total / 1024), percent);
        }
        gtk_label_set_text(GTK_LABEL(progressDialog_Text), tmp_string);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressDialog_Bar), (double) percent / 100.00);
        while (gtk_events_pending())
            gtk_main_iteration();
        g_free(tmp_string);
    }
    return 0;
}

// ************************************************************************************************

/**
 * Display the About Dialog Box.
 */
void displayAbout(void) {
    GtkWidget *dialog, *vbox, *label, *label2, *label3, *label4, *label5, *image;
    gchar *version_string;
    gchar *gtk_version_string;

    dialog = gtk_dialog_new_with_buttons(_("About gMTP"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
        NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

#if GMTP_USE_GTK2
    gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
#endif
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox);
#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);
#endif


    // Add in our icon.
    image = gtk_image_new_from_file(file_logo_png);
    gtk_widget_show(image);
    gtk_container_add(GTK_CONTAINER(vbox), image);

    version_string = g_strconcat("<span size=\"xx-large\"><b>", PACKAGE_TITLE, " v", PACKAGE_VERSION, "</b></span>", NULL);

    label = gtk_label_new(version_string);
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(vbox), label);

    label2 = gtk_label_new(_("A simple MP3 Player Client for Solaris 10\nand other UNIX / UNIX-like systems\n"));
    gtk_label_set_use_markup(GTK_LABEL(label2), TRUE);
    gtk_label_set_justify(GTK_LABEL(label2), GTK_JUSTIFY_CENTER);
    gtk_misc_set_padding(GTK_MISC(label2), 5, 0);
    gtk_widget_show(label2);
    gtk_container_add(GTK_CONTAINER(vbox), label2);

    label5 = gtk_label_new("http://gmtp.sourceforge.net\n");
    gtk_label_set_use_markup(GTK_LABEL(label5), TRUE);
    gtk_widget_show(label5);
    gtk_container_add(GTK_CONTAINER(vbox), label5);

    label3 = gtk_label_new(_("<small>Copyright 2009-2011, Darran Kartaschew\nReleased under the BSD Licence</small>"));
    gtk_label_set_use_markup(GTK_LABEL(label3), TRUE);
    gtk_widget_show(label3);
    gtk_container_add(GTK_CONTAINER(vbox), label3);

    gtk_version_string = g_strdup_printf("<small>Built with GTK v%d.%d.%d</small>\n", GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
    label4 = gtk_label_new(gtk_version_string);
    gtk_label_set_use_markup(GTK_LABEL(label4), TRUE);
    gtk_widget_show(label4);
    gtk_container_add(GTK_CONTAINER(vbox), label4);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(version_string);
    g_free(gtk_version_string);
}

// ************************************************************************************************

/**
 * Display an Error Dialog Message Box.
 * @param msg
 */
void displayError(gchar* msg) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(windowMain),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        msg);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// ************************************************************************************************

/**
 * Display an Information Dialog Message Box.
 * @param msg
 */
void displayInformation(gchar* msg) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(GTK_WINDOW(windowMain),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        msg);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Information"));
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// ************************************************************************************************

/**
 * Display the Add New Folder Dialog Box.
 * @return The name of the folder to be created.
 */
gchar* displayFolderNewDialog(void) {
    GtkWidget *dialog, *hbox, *label, *textbox;
    gchar* textfield;

    dialog = gtk_dialog_new_with_buttons(_("New Folder"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_OK, GTK_RESPONSE_OK,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);

#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
#endif

    label = gtk_label_new(_("Folder Name:"));
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(hbox), label);

    textbox = gtk_entry_new();
    gtk_widget_show(textbox);
    gtk_entry_set_max_length(GTK_ENTRY(textbox), 64);
    gtk_entry_set_has_frame(GTK_ENTRY(textbox), TRUE);
    gtk_entry_set_activates_default(GTK_ENTRY(textbox), TRUE);
    gtk_container_add(GTK_CONTAINER(hbox), textbox);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        textfield = g_strdup(gtk_entry_get_text(GTK_ENTRY(textbox)));
        if (strlen(textfield) == 0) {
            // We have an emtpy string.
            gtk_widget_destroy(dialog);
            return NULL;
        } else {
            gtk_widget_destroy(dialog);
            return textfield;
        }
    } else {
        gtk_widget_destroy(dialog);
        return NULL;
    }
}

// ************************************************************************************************

/**
 * Display the Change Device Name dialog box.
 * @param devicename The new name of the device.
 * @return
 */
gchar* displayChangeDeviceNameDialog(gchar* devicename) {
    GtkWidget *dialog, *hbox, *label, *textbox;
    gchar* textfield;

    dialog = gtk_dialog_new_with_buttons(_("Change Device Name"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_OK, GTK_RESPONSE_OK,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);

#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
#endif

    label = gtk_label_new(_("Device Name:"));
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(hbox), label);

    textbox = gtk_entry_new();
    gtk_widget_show(textbox);
    if (devicename != NULL) {
        gtk_entry_set_text(GTK_ENTRY(textbox), devicename);
    }
    gtk_entry_set_max_length(GTK_ENTRY(textbox), 64);
    gtk_entry_set_has_frame(GTK_ENTRY(textbox), TRUE);
    gtk_entry_set_activates_default(GTK_ENTRY(textbox), TRUE);
    gtk_container_add(GTK_CONTAINER(hbox), textbox);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        textfield = g_strdup(gtk_entry_get_text(GTK_ENTRY(textbox)));
        if (strlen(textfield) == 0) {
            // We have an emtpy string.
            gtk_widget_destroy(dialog);
            return NULL;
        } else {
            gtk_widget_destroy(dialog);
            return textfield;
        }
    } else {
        gtk_widget_destroy(dialog);
        return NULL;
    }
}

// ************************************************************************************************

/**
 * Display the rename Filename dialog box.
 * @param currentfilename The new name of the file.
 * @return
 */
gchar* displayRenameFileDialog(gchar* currentfilename) {
    GtkWidget *dialog, *hbox, *label, *textbox;
    gchar* textfield;

    dialog = gtk_dialog_new_with_buttons(_("Rename File/Folder"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_OK, GTK_RESPONSE_OK,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);

#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
#endif

    label = gtk_label_new(_("File Name:"));
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(hbox), label);

    textbox = gtk_entry_new();
    gtk_widget_show(textbox);
    if (currentfilename != NULL) {
        gtk_entry_set_text(GTK_ENTRY(textbox), currentfilename);
    }
    gtk_entry_set_max_length(GTK_ENTRY(textbox), 64);
    gtk_entry_set_has_frame(GTK_ENTRY(textbox), TRUE);
    gtk_entry_set_activates_default(GTK_ENTRY(textbox), TRUE);
    gtk_container_add(GTK_CONTAINER(hbox), textbox);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        textfield = g_strdup(gtk_entry_get_text(GTK_ENTRY(textbox)));
        if (strlen(textfield) == 0) {
            // We have an emtpy string.
            gtk_widget_destroy(dialog);
            return NULL;
        } else {
            gtk_widget_destroy(dialog);
            return textfield;
        }
    } else {
        gtk_widget_destroy(dialog);
        return NULL;
    }
}

// ************************************************************************************************

/**
 * Display the Which Device dialog box.
 * @return
 */
gint displayMultiDeviceDialog(void) {
    GtkWidget *dialog, *hbox, *label, *textbox;
    gchar *tmp_string;
    gint dialog_selection = 0;

    dialog = gtk_dialog_new_with_buttons(_("Connect to which device?"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_OK, GTK_RESPONSE_OK,
        NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
#endif

    label = gtk_label_new(_("Device:"));
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(hbox), label);

    // Now create the combo box.
#if GMTP_USE_GTK2
    textbox = gtk_combo_box_new_text();
#else
    textbox = gtk_combo_box_text_new();
#endif
    gtk_widget_show(textbox);
    gtk_container_add(GTK_CONTAINER(hbox), textbox);
    // Now add in our selection strings.
    // We should just take straight strings here, but this is quicker/easier.
    gint i = 0;
    for (i = 0; i < DeviceMgr.numrawdevices; i++) {
        if (DeviceMgr.rawdevices[i].device_entry.vendor != NULL ||
            DeviceMgr.rawdevices[i].device_entry.product != NULL) {
            tmp_string = g_strdup_printf("   %s %s : (%04x:%04x) @ bus %d, dev %d",
                DeviceMgr.rawdevices[i].device_entry.vendor,
                DeviceMgr.rawdevices[i].device_entry.product,
                DeviceMgr.rawdevices[i].device_entry.vendor_id,
                DeviceMgr.rawdevices[i].device_entry.product_id,
                DeviceMgr.rawdevices[i].bus_location,
                DeviceMgr.rawdevices[i].devnum);
        } else {
            tmp_string = g_strdup_printf( _("Unknown : %04x:%04x @ bus %d, dev %d"),
                DeviceMgr.rawdevices[i].device_entry.vendor_id,
                DeviceMgr.rawdevices[i].device_entry.product_id,
                DeviceMgr.rawdevices[i].bus_location,
                DeviceMgr.rawdevices[i].devnum);
        }
#if GMTP_USE_GTK2
        gtk_combo_box_append_text(GTK_COMBO_BOX(textbox), tmp_string);
#else
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(textbox), tmp_string);
#endif
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(textbox), 0);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        dialog_selection = gtk_combo_box_get_active(GTK_COMBO_BOX(textbox));
    }
    gtk_widget_destroy(dialog);
    g_free(tmp_string);
    return dialog_selection;
}

// ************************************************************************************************

/**
 * Display the Which Storage Device dialog box.
 * @return
 */
gint displayDeviceStorageDialog(void) {
    GtkWidget *dialog, *hbox, *label, *textbox;
    LIBMTP_devicestorage_t *devicestorage;
    gchar *tmp_string;
    gint dialog_selection = 0;

    devicestorage = DeviceMgr.device->storage;

    dialog = gtk_dialog_new_with_buttons(_("Connect to which storage device?"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_OK, GTK_RESPONSE_OK,
        NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    //gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
#endif

    label = gtk_label_new(_("Storage Device:"));
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(hbox), label);

    // Now create the combo box.
#if GMTP_USE_GTK2
    textbox = gtk_combo_box_new_text();
#else
    textbox = gtk_combo_box_text_new();
#endif
    gtk_widget_show(textbox);
    gtk_container_add(GTK_CONTAINER(hbox), textbox);
    // Now add in our selection strings.
    while (devicestorage != NULL) {
        if (devicestorage->StorageDescription != NULL) {
#if GMTP_USE_GTK2
            gtk_combo_box_append_text(GTK_COMBO_BOX(textbox), devicestorage->StorageDescription);
#else
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(textbox), devicestorage->StorageDescription);
#endif
        } else {
            tmp_string = g_strdup_printf(_("Unknown id: %d, %lu MB"), devicestorage->id, (devicestorage->MaxCapacity / MEGABYTE));
#if GMTP_USE_GTK2
            gtk_combo_box_append_text(GTK_COMBO_BOX(textbox), tmp_string);
#else
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(textbox), tmp_string);
#endif
        }
        devicestorage = devicestorage->next;
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(textbox), 0);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        dialog_selection = gtk_combo_box_get_active(GTK_COMBO_BOX(textbox));
    }
    gtk_widget_destroy(dialog);
    g_free(tmp_string);
    return dialog_selection;

}

// ************************************************************************************************

/**
 * Display the Overwrite File dialog box.
 * @param filename
 * @return
 */
gint displayFileOverwriteDialog(gchar *filename) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        _("File <b>%s</b> already exists in target folder.\nDo you want to:"), filename);
    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
        _("Skip"), MTP_SKIP,
        _("Skip All"), MTP_SKIP_ALL,
        _("Overwrite"), MTP_OVERWRITE,
        _("Overwrite All"), MTP_OVERWRITE_ALL,
        NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Question: Confirm Overwrite of Existing File?"));
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), MTP_OVERWRITE);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return result;
}

// ************************************************************************************************

/**
 * Display the Add Album Art dialog box.
 * @return
 */
void displayAddAlbumArtDialog(void) {
    //Album_Struct* albumdetails;
    GtkWidget *hbox, *label;
    GtkWidget *buttonBox;
    LIBMTP_album_t *albuminfo = NULL;
    LIBMTP_album_t *album_orig = NULL;

    AlbumArtDialog = gtk_dialog_new_with_buttons(_("Album Art"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
        NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(AlbumArtDialog), GTK_RESPONSE_CLOSE);
    gtk_window_set_resizable(GTK_WINDOW(AlbumArtDialog), FALSE);
#if GMTP_USE_GTK2
    gtk_dialog_set_has_separator(GTK_DIALOG(AlbumArtDialog), TRUE);
#endif

    // Set some nice 5px spacing.
#if GMTP_USE_GTK2
    gtk_box_set_spacing(GTK_BOX(GTK_DIALOG(AlbumArtDialog)->vbox), 10);
#else
    gtk_box_set_spacing(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(AlbumArtDialog))), 10);
#endif

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    
#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(AlbumArtDialog)->vbox), hbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(AlbumArtDialog))), hbox);
#endif

    label = gtk_label_new(_("Album:"));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

    // Now create the combo box.
#if GMTP_USE_GTK2
    textboxAlbumArt = gtk_combo_box_new_text();
#else
    textboxAlbumArt = gtk_combo_box_text_new();
#endif
    gtk_widget_show(textboxAlbumArt);
    gtk_box_pack_start(GTK_BOX(hbox), textboxAlbumArt, TRUE, TRUE, 0);
    // Now add in our selection strings.
    albuminfo = LIBMTP_Get_Album_List_For_Storage(DeviceMgr.device, DeviceMgr.devicestorage->id);
    // Better check to see if we actually have anything?
    if (albuminfo == NULL) {
        // we have no albums.
        displayInformation(_("No Albums available to set Album Art with. Either:\n1. You have no music files uploaded?\n2. Your device does not support Albums?\n3. Previous applications used to upload files do not autocreate albums for you or support the metadata for those files in order to create the albums for you?\n"));
        gtk_widget_destroy(AlbumArtDialog);
        AlbumArtImage = NULL;
        AlbumArtDialog = NULL;
        textboxAlbumArt = NULL;
        return;
    }

    album_orig = albuminfo;
    while (albuminfo != NULL) {
        if (albuminfo->name != NULL)
#if GMTP_USE_GTK2
            gtk_combo_box_append_text(GTK_COMBO_BOX(textboxAlbumArt), albuminfo->name);
#else
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(textboxAlbumArt), albuminfo->name);
#endif
        albuminfo = albuminfo->next;
    }
    // End add selection.
    gtk_combo_box_set_active(GTK_COMBO_BOX(textboxAlbumArt), 0);

    // Add in a image view of the current uploaded album art.
    AlbumArtImage = gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_DIALOG);

#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(AlbumArtDialog)->vbox), AlbumArtImage);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(AlbumArtDialog))), AlbumArtImage);
#endif
    gtk_widget_show(AlbumArtImage);


    // Add in the album art operations area.
    buttonBox = gtk_hbutton_box_new();
    gtk_box_set_spacing(GTK_BOX(buttonBox), 5);
    gtk_widget_show(buttonBox);

#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(AlbumArtDialog)->vbox), buttonBox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(AlbumArtDialog))), buttonBox);
#endif

    buttonAlbumAdd = gtk_button_new_with_mnemonic(_("Upload"));
    gtk_widget_show(buttonAlbumAdd);
    gtk_box_pack_start(GTK_BOX(buttonBox), buttonAlbumAdd, TRUE, TRUE, 0);

    buttonAlbumDelete = gtk_button_new_with_mnemonic(_("Remove"));
    gtk_widget_show(buttonAlbumDelete);
    gtk_box_pack_start(GTK_BOX(buttonBox), buttonAlbumDelete, TRUE, TRUE, 0);
    gtk_widget_set_sensitive(GTK_WIDGET(buttonAlbumDelete), FALSE);

    buttonAlbumDownload = gtk_button_new_with_mnemonic(_("Download"));
    gtk_widget_show(buttonAlbumDownload);
    gtk_box_pack_start(GTK_BOX(buttonBox), buttonAlbumDownload, TRUE, TRUE, 0);
    gtk_widget_set_sensitive(GTK_WIDGET(buttonAlbumDownload), FALSE);

    // Now, update the stock image with one from the selected album.
    AlbumArtUpdateImage(album_orig);

    g_signal_connect((gpointer) textboxAlbumArt, "changed",
        G_CALLBACK(on_albumtextbox_activate),
        NULL);

    g_signal_connect((gpointer) buttonAlbumAdd, "clicked",
        G_CALLBACK(on_buttonAlbumArtAdd_activate),
        NULL);

    g_signal_connect((gpointer) buttonAlbumDelete, "clicked",
        G_CALLBACK(on_buttonAlbumArtDelete_activate),
        NULL);

    g_signal_connect((gpointer) buttonAlbumDownload, "clicked",
        G_CALLBACK(on_buttonAlbumArtDownload_activate),
        NULL);

    gtk_dialog_run(GTK_DIALOG(AlbumArtDialog));

    gtk_widget_destroy(AlbumArtDialog);
    clearAlbumStruc(album_orig);
    
    //Clean up global pointers.
    AlbumArtImage = NULL;
    AlbumArtDialog = NULL;
    textboxAlbumArt = NULL;
}

// ************************************************************************************************

/**
 * Set the image in the AddAlbumDialog to be that supplied with the album information.
 * @return 
 */
void AlbumArtUpdateImage(LIBMTP_album_t* selectedAlbum){
    LIBMTP_filesampledata_t *imagedata = NULL;
    GdkPixbufLoader *BufferLoader = NULL;
    GdkPixbuf *gdk_image = NULL;
    GdkPixbuf *gdk_image_scale = NULL;

    // Ensure our widget exists.
    if(AlbumArtImage == NULL)
        return;
    // Ensure we have a selected album.
    if(selectedAlbum == NULL){
        AlbumArtSetDefault();
        return;
    }
    imagedata = albumGetArt(selectedAlbum);
    if(imagedata != NULL){
        // We have our image data.
        // Create a GdkPixbuf
        BufferLoader = gdk_pixbuf_loader_new();
        if(gdk_pixbuf_loader_write(BufferLoader, (const guchar*)imagedata->data, imagedata->size, NULL) == TRUE){
            // Set the GtkImage to use that GdkPixbuf.
            gdk_image = gdk_pixbuf_loader_get_pixbuf (BufferLoader);
            gdk_pixbuf_loader_close(BufferLoader, NULL);
            gdk_image_scale = gdk_pixbuf_scale_simple(gdk_image, ALBUM_SIZE, ALBUM_SIZE, GDK_INTERP_BILINEAR);
            gtk_image_set_from_pixbuf(GTK_IMAGE(AlbumArtImage), gdk_image_scale);
            g_object_unref(gdk_image);
            g_object_unref(gdk_image_scale);

            // Set button states, so we can do stuff on the image.
            gtk_widget_set_sensitive(GTK_WIDGET(buttonAlbumDownload), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(buttonAlbumDelete), TRUE);
        } else {
            AlbumArtSetDefault();
        }
        // Clean up the image buffer.
        LIBMTP_destroy_filesampledata_t(imagedata);
    } else {
        AlbumArtSetDefault();
    }
}

// ************************************************************************************************

/**
 * Set the album art to be the default image.
 */
void AlbumArtSetDefault(void){
    //gtk_image_set_from_stock(GTK_IMAGE(AlbumArtImage), GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_DIALOG );
    GdkPixbuf *gdk_image = NULL;
    GdkPixbuf *gdk_image_scale = NULL;
    gdk_image = gtk_widget_render_icon(GTK_WIDGET(AlbumArtImage), GTK_STOCK_MISSING_IMAGE,
                                                GTK_ICON_SIZE_DIALOG, "gtk-missing-image");
    gdk_image_scale = gdk_pixbuf_scale_simple(gdk_image, ALBUM_SIZE, ALBUM_SIZE, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(AlbumArtImage), gdk_image_scale);
    g_object_unref(gdk_image);
    g_object_unref(gdk_image_scale);

    // Disable the buttons, since we have a default image.
    gtk_widget_set_sensitive(GTK_WIDGET(buttonAlbumDownload), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(buttonAlbumDelete), FALSE);
}

// ************************************************************************************************

/**
 * Create the Context Menu widget.
 * @return
 */
GtkWidget* create_windowMainContextMenu(void) {
    GtkWidget* menu;
    GtkWidget* cfileAdd;
    GtkWidget* cfileRename;
    GtkWidget* cfileRemove;
    GtkWidget* cfileDownload;
    GtkWidget* cfileNewFolder;
    GtkWidget* cfileRemoveFolder;
    GtkWidget* cfileRescan;
    GtkWidget* menuseparator1;
    GtkWidget* menuseparator2;

    menu = gtk_menu_new();

    cfileAdd = gtk_menu_item_new_with_label(_("Add Files"));
    gtk_widget_show(cfileAdd);
    gtk_container_add(GTK_CONTAINER(menu), cfileAdd);

    cfileRemove = gtk_menu_item_new_with_label(_("Delete Files"));
    gtk_widget_show(cfileRemove);
    gtk_container_add(GTK_CONTAINER(menu), cfileRemove);

    cfileDownload = gtk_menu_item_new_with_label(_("Download Files"));
    gtk_widget_show(cfileDownload);
    gtk_container_add(GTK_CONTAINER(menu), cfileDownload);

    cfileRename = gtk_menu_item_new_with_label(_("Rename File"));
    gtk_widget_show(cfileRename);
    gtk_container_add(GTK_CONTAINER(menu), cfileRename);

    menuseparator1 = gtk_separator_menu_item_new();
    gtk_widget_show(menuseparator1);
    gtk_container_add(GTK_CONTAINER(menu), menuseparator1);

    cfileNewFolder = gtk_menu_item_new_with_label(_("Create Folder"));
    gtk_widget_show(cfileNewFolder);
    gtk_container_add(GTK_CONTAINER(menu), cfileNewFolder);

    cfileRemoveFolder = gtk_menu_item_new_with_label(_("Delete Folder"));
    gtk_widget_show(cfileRemoveFolder);
    gtk_container_add(GTK_CONTAINER(menu), cfileRemoveFolder);

    menuseparator2 = gtk_separator_menu_item_new();
    gtk_widget_show(menuseparator2);
    gtk_container_add(GTK_CONTAINER(menu), menuseparator2);

    cfileRescan = gtk_menu_item_new_with_label(_("Refresh Device"));
    gtk_widget_show(cfileRescan);
    gtk_container_add(GTK_CONTAINER(menu), cfileRescan);

    // Now our call backs.
    g_signal_connect((gpointer) cfileAdd, "activate",
        G_CALLBACK(on_filesAdd_activate),
        NULL);

    g_signal_connect((gpointer) cfileDownload, "activate",
        G_CALLBACK(on_filesDownload_activate),
        NULL);

    g_signal_connect((gpointer) cfileRename, "activate",
        G_CALLBACK(on_fileRenameFile_activate),
        NULL);

    g_signal_connect((gpointer) cfileRemove, "activate",
        G_CALLBACK(on_filesDelete_activate),
        NULL);

    g_signal_connect((gpointer) cfileNewFolder, "activate",
        G_CALLBACK(on_fileNewFolder_activate),
        NULL);

    g_signal_connect((gpointer) cfileRemoveFolder, "activate",
        G_CALLBACK(on_fileRemoveFolder_activate),
        NULL);

    g_signal_connect((gpointer) cfileRescan, "activate",
        G_CALLBACK(on_deviceRescan_activate),
        NULL);

    return menu;

}

// ************************************************************************************************

// Playlist support

/**
 * Create the Playlist Editor Window.
 * @return
 */
GtkWidget* create_windowPlaylist(void) {
    GtkWidget *window_playlist;
    GtkWidget *vbox1;
    GtkWidget *hbox1;
    GtkWidget *label_Playlist;

    GtkWidget *button_Add_Playlist;
    GtkWidget *button_Import_Playlist;
    GtkWidget *alignment2;
    GtkWidget *hbox3;
    GtkWidget *image2;
    GtkWidget *label3;

    GtkWidget *alignment1;
    GtkWidget *hbox2;
    GtkWidget *image1;
    GtkWidget *label2;
    GtkWidget *hbox4;
    GtkWidget *scrolledwindow2;

    GtkWidget *vbuttonbox1;

    GtkWidget *alignment6;
    GtkWidget *hbox8;
    GtkWidget *image6;
    GtkWidget *label10;

    GtkWidget *alignment7;
    GtkWidget *hbox9;
    GtkWidget *image7;
    GtkWidget *label11;
    GtkWidget *scrolledwindow3;

    GtkWidget *vbuttonbox2;

    GtkWidget *hbuttonbox1;
    GtkWidget *button_Close;


#if GMTP_USE_GTK2
    GtkTooltips *tooltips;
    tooltips = gtk_tooltips_new();
#endif

    window_playlist = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gchar * winTitle;
    winTitle = g_strconcat(PACKAGE_TITLE, " v", PACKAGE_VERSION, _(" Playlists"), NULL);
    gtk_window_set_title(GTK_WINDOW(window_playlist), winTitle);
    gtk_window_set_modal(GTK_WINDOW(window_playlist), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window_playlist), 760, 400);
    gtk_window_set_transient_for(GTK_WINDOW(window_playlist), GTK_WINDOW(windowMain));
    gtk_window_set_position(GTK_WINDOW(window_playlist), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window_playlist), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(window_playlist), GDK_WINDOW_TYPE_HINT_DIALOG);
    g_free(winTitle);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(window_playlist), vbox1);

    hbox1 = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox1);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, TRUE, 5);

    label_Playlist = gtk_label_new(_("Current Playlist: "));
    gtk_widget_show(label_Playlist);
    gtk_box_pack_start(GTK_BOX(hbox1), label_Playlist, FALSE, FALSE, 5);
    gtk_misc_set_padding(GTK_MISC(label_Playlist), 5, 0);

#if GMTP_USE_GTK2
    comboboxentry_playlist = gtk_combo_box_new_text();
#else
    comboboxentry_playlist = gtk_combo_box_text_new();
#endif
    gtk_widget_show(comboboxentry_playlist);
    gtk_box_pack_start(GTK_BOX(hbox1), comboboxentry_playlist, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(comboboxentry_playlist), 5);

    button_Add_Playlist = gtk_button_new();
    gtk_widget_show(button_Add_Playlist);
    gtk_box_pack_start(GTK_BOX(hbox1), button_Add_Playlist, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(button_Add_Playlist), 5);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, button_Add_Playlist, _("Add New Playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(button_Add_Playlist, _("Add New Playlist"));
#endif

    alignment2 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_widget_show(alignment2);
    gtk_container_add(GTK_CONTAINER(button_Add_Playlist), alignment2);

    hbox3 = gtk_hbox_new(FALSE, 2);
    gtk_widget_show(hbox3);
    gtk_container_add(GTK_CONTAINER(alignment2), hbox3);

    image2 = gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON);
    gtk_widget_show(image2);
    gtk_box_pack_start(GTK_BOX(hbox3), image2, FALSE, FALSE, 0);

    label3 = gtk_label_new_with_mnemonic(_("Add"));
    gtk_widget_show(label3);
    gtk_box_pack_start(GTK_BOX(hbox3), label3, FALSE, FALSE, 0);

    button_Del_Playlist = gtk_button_new();
    gtk_widget_show(button_Del_Playlist);
    gtk_box_pack_start(GTK_BOX(hbox1), button_Del_Playlist, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(button_Del_Playlist), 5);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, button_Del_Playlist, _("Remove Current Selected Playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(button_Del_Playlist, _("Remove Current Selected Playlist"));
#endif

    alignment1 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_widget_show(alignment1);
    gtk_container_add(GTK_CONTAINER(button_Del_Playlist), alignment1);

    hbox2 = gtk_hbox_new(FALSE, 2);
    gtk_widget_show(hbox2);
    gtk_container_add(GTK_CONTAINER(alignment1), hbox2);

    image1 = gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_BUTTON);
    gtk_widget_show(image1);
    gtk_box_pack_start(GTK_BOX(hbox2), image1, FALSE, FALSE, 0);

    label2 = gtk_label_new_with_mnemonic(_("Del"));
    gtk_widget_show(label2);
    gtk_box_pack_start(GTK_BOX(hbox2), label2, FALSE, FALSE, 0);

    // Import Button

    button_Import_Playlist = gtk_button_new_from_stock(GTK_STOCK_OPEN);
    gtk_widget_show(button_Import_Playlist);
    gtk_box_pack_start(GTK_BOX(hbox1), button_Import_Playlist, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(button_Import_Playlist), 5);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, button_Import_Playlist, _("Import Playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(button_Import_Playlist, _("Import Playlist"));
#endif

    // Export Button

    button_Export_Playlist = gtk_button_new_from_stock(GTK_STOCK_SAVE_AS);
    gtk_widget_show(button_Export_Playlist);
    gtk_box_pack_start(GTK_BOX(hbox1), button_Export_Playlist, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(button_Export_Playlist), 5);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, button_Export_Playlist, _("Export Playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(button_Export_Playlist, _("Export Playlist"));
#endif

    // Scrolled Window.

    hbox4 = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox4);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox4, TRUE, TRUE, 0);

    scrolledwindow2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(scrolledwindow2);
    gtk_box_pack_start(GTK_BOX(hbox4), scrolledwindow2, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(scrolledwindow2), 5);

    treeview_Avail_Files = gtk_tree_view_new();
    gtk_widget_show(treeview_Avail_Files);
    gtk_container_add(GTK_CONTAINER(scrolledwindow2), treeview_Avail_Files);
    gtk_container_set_border_width(GTK_CONTAINER(treeview_Avail_Files), 5);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, treeview_Avail_Files, _("Device Audio Tracks"), NULL);
#else
    gtk_widget_set_tooltip_text(treeview_Avail_Files, _("Device Audio Tracks"));
#endif

    playlist_TrackSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_Avail_Files));
    gtk_tree_selection_set_mode(playlist_TrackSelection, GTK_SELECTION_MULTIPLE);

    playlist_TrackList = gtk_list_store_new(NUM_TCOLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
    setupTrackList(GTK_TREE_VIEW(treeview_Avail_Files));
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_Avail_Files), GTK_TREE_MODEL(playlist_TrackList));
    g_object_unref(playlist_TrackList);

    vbuttonbox1 = gtk_vbutton_box_new();
    gtk_widget_show(vbuttonbox1);
    gtk_box_pack_start(GTK_BOX(hbox4), vbuttonbox1, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(vbuttonbox1), GTK_BUTTONBOX_SPREAD);

    button_Add_Files = gtk_button_new();
    gtk_widget_show(button_Add_Files);
    gtk_container_add(GTK_CONTAINER(vbuttonbox1), button_Add_Files);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, button_Add_Files, _("Add file to playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(button_Add_Files, _("Add file to playlist"));
#endif

    alignment6 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_widget_show(alignment6);
    gtk_container_add(GTK_CONTAINER(button_Add_Files), alignment6);

    hbox8 = gtk_hbox_new(FALSE, 2);
    gtk_widget_show(hbox8);
    gtk_container_add(GTK_CONTAINER(alignment6), hbox8);

    image6 = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON);
    gtk_widget_show(image6);
    gtk_box_pack_start(GTK_BOX(hbox8), image6, FALSE, FALSE, 0);

    label10 = gtk_label_new_with_mnemonic(_("Add File"));
    gtk_widget_show(label10);
    gtk_box_pack_start(GTK_BOX(hbox8), label10, FALSE, FALSE, 0);

    button_Del_File = gtk_button_new();
    gtk_widget_show(button_Del_File);
    gtk_container_add(GTK_CONTAINER(vbuttonbox1), button_Del_File);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, button_Del_File, _("Remove file from playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(button_Del_File, _("Remove file from playlist"));
#endif

    alignment7 = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_widget_show(alignment7);
    gtk_container_add(GTK_CONTAINER(button_Del_File), alignment7);

    hbox9 = gtk_hbox_new(FALSE, 2);
    gtk_widget_show(hbox9);
    gtk_container_add(GTK_CONTAINER(alignment7), hbox9);

    image7 = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_BUTTON);
    gtk_widget_show(image7);
    gtk_box_pack_start(GTK_BOX(hbox9), image7, FALSE, FALSE, 0);

    label11 = gtk_label_new_with_mnemonic(_("Del File"));
    gtk_widget_show(label11);
    gtk_box_pack_start(GTK_BOX(hbox9), label11, FALSE, FALSE, 0);

    scrolledwindow3 = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(scrolledwindow3);
    gtk_box_pack_start(GTK_BOX(hbox4), scrolledwindow3, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(scrolledwindow3), 5);

    treeview_Playlist_Files = gtk_tree_view_new();
    gtk_widget_show(treeview_Playlist_Files);
    gtk_container_add(GTK_CONTAINER(scrolledwindow3), treeview_Playlist_Files);
    gtk_container_set_border_width(GTK_CONTAINER(treeview_Playlist_Files), 5);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, treeview_Playlist_Files, _("Playlist Audio Tracks"), NULL);
#else
    gtk_widget_set_tooltip_text(treeview_Playlist_Files, _("Playlist Audio Tracks"));
#endif

    playlist_PL_Selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_Playlist_Files));
    gtk_tree_selection_set_mode(playlist_PL_Selection, GTK_SELECTION_MULTIPLE);

    playlist_PL_List = gtk_list_store_new(NUM_PL_COLUMNS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
    setup_PL_List(GTK_TREE_VIEW(treeview_Playlist_Files));
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_Playlist_Files), GTK_TREE_MODEL(playlist_PL_List));
    g_object_unref(playlist_PL_List);

    vbuttonbox2 = gtk_vbutton_box_new();
    gtk_widget_show(vbuttonbox2);
    gtk_box_pack_start(GTK_BOX(hbox4), vbuttonbox2, FALSE, FALSE, 5);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(vbuttonbox2), GTK_BUTTONBOX_SPREAD);

    button_File_Move_Up = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
    gtk_widget_show(button_File_Move_Up);
    gtk_container_add(GTK_CONTAINER(vbuttonbox2), button_File_Move_Up);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, button_File_Move_Up, _("Move selected file up in the playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(button_File_Move_Up, _("Move selected file up in the playlist"));
#endif

    button_File_Move_Down = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
    gtk_widget_show(button_File_Move_Down);
    gtk_container_add(GTK_CONTAINER(vbuttonbox2), button_File_Move_Down);
#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, button_File_Move_Down, _("Move selected file down in the playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(button_File_Move_Down, _("Move selected file down in the playlist"));
#endif

    hbuttonbox1 = gtk_hbutton_box_new();
    gtk_widget_show(hbuttonbox1);
    gtk_box_pack_start(GTK_BOX(vbox1), hbuttonbox1, FALSE, FALSE, 5);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox1), GTK_BUTTONBOX_END);

    button_Close = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_widget_show(button_Close);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), button_Close);
    gtk_container_set_border_width(GTK_CONTAINER(button_Close), 5);

    g_signal_connect((gpointer) window_playlist, "destroy",
        G_CALLBACK(on_quitPlaylist_activate),
        NULL);

    g_signal_connect((gpointer) button_Close, "clicked",
        G_CALLBACK(on_quitPlaylist_activate),
        NULL);

    g_signal_connect((gpointer) button_Add_Playlist, "clicked",
        G_CALLBACK(on_Playlist_NewPlaylistButton_activate),
        NULL);

    g_signal_connect((gpointer) button_Import_Playlist, "clicked",
        G_CALLBACK(on_Playlist_ImportPlaylistButton_activate),
        NULL);

    g_signal_connect((gpointer) button_Export_Playlist, "clicked",
        G_CALLBACK(on_Playlist_ExportPlaylistButton_activate),
        NULL);

    g_signal_connect((gpointer) button_Del_Playlist, "clicked",
        G_CALLBACK(on_Playlist_DelPlaylistButton_activate),
        NULL);

    g_signal_connect((gpointer) button_Del_File, "clicked",
        G_CALLBACK(on_Playlist_DelFileButton_activate),
        NULL);

    g_signal_connect((gpointer) button_Add_Files, "clicked",
        G_CALLBACK(on_Playlist_AddFileButton_activate),
        NULL);

    g_signal_connect((gpointer) button_File_Move_Up, "clicked",
        G_CALLBACK(on_Playlist_FileUpButton_activate),
        NULL);

    g_signal_connect((gpointer) button_File_Move_Down, "clicked",
        G_CALLBACK(on_Playlist_FileDownButton_activate),
        NULL);

    g_signal_connect((gpointer) comboboxentry_playlist, "changed",
        G_CALLBACK(on_Playlist_Combobox_activate),
        NULL);

    return window_playlist;
}

// ************************************************************************************************

/**
 * Display the Playlist Editor.
 */
void displayPlaylistDialog(void) {
    //LIBMTP_playlist_t* tmpplaylist;
    LIBMTP_track_t* tmptrack;
    GtkTreeIter rowIter;
    gchar * tmp_string;

    if (windowPlaylistDialog != NULL) {
        gtk_widget_hide(windowPlaylistDialog);
        gtk_widget_destroy(windowPlaylistDialog);
    }
    windowPlaylistDialog = create_windowPlaylist();
    playlist_number = 0;
    // Clear the track and playlist lists;
    gtk_list_store_clear(GTK_LIST_STORE(playlist_PL_List));
    gtk_list_store_clear(GTK_LIST_STORE(playlist_TrackList));
    // Populate the playlist changebox.
    devicePlayLists = getPlaylists();
    deviceTracks = getTracks();
    setPlayListComboBox();

    // Populate the available track list.
    if (deviceTracks != NULL) {
        // Populate the track list;
        tmptrack = deviceTracks;
        while (tmptrack != NULL) {
            if ((tmptrack->storage_id == DeviceMgr.devicestorage->id) && (LIBMTP_FILETYPE_IS_AUDIO(tmptrack->filetype))) {
                gtk_list_store_append(GTK_LIST_STORE(playlist_TrackList), &rowIter);
                tmp_string = g_strdup_printf("%d:%.2d", (int) ((tmptrack->duration / 1000) / 60), (int) ((tmptrack->duration / 1000) % 60));
                gtk_list_store_set(GTK_LIST_STORE(playlist_TrackList), &rowIter, COL_ARTIST, tmptrack->artist, COL_ALBUM, tmptrack->album,
                    COL_TRACKID, tmptrack->item_id, COL_TRACKNAME, tmptrack->title, COL_TRACKDURATION, tmp_string, -1);
                g_free(tmp_string);
                tmp_string = NULL;
            }
            tmptrack = tmptrack->next;
        }
    }
    gtk_widget_show(GTK_WIDGET(windowPlaylistDialog));
    // Save the current selected playlist if needed.
}

// ************************************************************************************************

/**
 * Setup the display for the Playlist.
 * @param treeviewFiles
 */
void setupTrackList(GtkTreeView *treeviewFiles) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // Artist
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Artist"), renderer,
        "text", COL_ARTIST,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_sort_column_id(column, COL_ARTIST);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_spacing(column, 5);

    // Album column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Album"), renderer,
        "text", COL_ALBUM,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_sort_column_id(column, COL_ALBUM);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_spacing(column, 5);

    // Folder/FileID column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Object ID", renderer,
        "text", COL_TRACKID,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);

    // Track column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Track"), renderer,
        "text", COL_TRACKNAME,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_sort_column_id(column, COL_TRACKNAME);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_visible(column, TRUE);

    // Track Duration
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Duration"), renderer,
        "text", COL_TRACKDURATION,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_visible(column, TRUE);


}

// ************************************************************************************************

/**
 * Setup the list of tracks in the current playlist.
 * @param treeviewFiles
 */
void setup_PL_List(GtkTreeView *treeviewFiles) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // Order Num
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Num"), renderer,
        "text", COL_PL_ORDER_NUM,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    //gtk_tree_view_column_set_sort_column_id(column, COL_PL_ORDER_NUM);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_spacing(column, 5);

    // Artist
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Artist"), renderer,
        "text", COL_PL_ARTIST,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    //gtk_tree_view_column_set_sort_column_id(column, COL_PL_ARTIST);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_spacing(column, 5);

    // Album column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Album"), renderer,
        "text", COL_PL_ALBUM,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    //gtk_tree_view_column_set_sort_column_id(column, COL_PL_ALBUM);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_spacing(column, 5);

    // Folder/FileID column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Object ID", renderer,
        "text", COL_PL_TRACKID,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_visible(column, FALSE);

    // Track column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Track"), renderer,
        "text", COL_PL_TRACKNAME,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    //gtk_tree_view_column_set_sort_column_id(column, COL_TRACKNAME);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_visible(column, TRUE);

    // Track Duration
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Duration"), renderer,
        "text", COL_PL_TRACKDURATION,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_visible(column, TRUE);
}

// ************************************************************************************************

/**
 * Set the state of the buttons within the playlist editor.
 * @param state
 */
void SetPlaylistButtonState(gboolean state) {
    gtk_widget_set_sensitive(GTK_WIDGET(button_Del_Playlist), state);
    gtk_widget_set_sensitive(GTK_WIDGET(button_Export_Playlist), state);
    gtk_widget_set_sensitive(GTK_WIDGET(button_File_Move_Up), state);
    gtk_widget_set_sensitive(GTK_WIDGET(button_File_Move_Down), state);
    gtk_widget_set_sensitive(GTK_WIDGET(button_Del_File), state);
    gtk_widget_set_sensitive(GTK_WIDGET(button_Add_Files), state);
    gtk_widget_set_sensitive(GTK_WIDGET(treeview_Avail_Files), state);
    gtk_widget_set_sensitive(GTK_WIDGET(treeview_Playlist_Files), state);
}

// ************************************************************************************************

/**
 * Setup the Playlist selection Combo Box in the playlist editor.
 */
void setPlayListComboBox(void) {
    LIBMTP_playlist_t* tmpplaylist = NULL;
    comboboxentry_playlist_entries = 0;
    // We need to remove all entries in the combo box before starting.
    // This is a little bit of a hack - but does work.
    gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(comboboxentry_playlist))));

    if (devicePlayLists != NULL) {
        // Populate the playlist dropdown box;
        //comboboxentry_playlist;
        tmpplaylist = devicePlayLists;
        while (tmpplaylist != NULL) {
            if (tmpplaylist->storage_id == DeviceMgr.devicestorage->id) {
#if GMTP_USE_GTK2
                gtk_combo_box_append_text(GTK_COMBO_BOX(comboboxentry_playlist), g_strdup(tmpplaylist->name));
#else
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboboxentry_playlist), g_strdup(tmpplaylist->name));
#endif
                comboboxentry_playlist_entries++;
            }
            tmpplaylist = tmpplaylist->next;
        }
    }
    if (devicePlayLists != NULL) {
        // Set our playlist to the first one.
        gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxentry_playlist), 0);
        playlist_number = 0;
        // Now populate the playlist screen with it's details.
        setPlaylistField(0);
    } else {
        playlist_number = -1;
    }
    // If no playlists set parts of dialog to disabled.
    if (devicePlayLists == NULL) {
        SetPlaylistButtonState(FALSE);
    } else {
        SetPlaylistButtonState(TRUE);
    }

}

// ************************************************************************************************

/**
 * Setup the list of tracks in the current selected playlist.
 * @param PlayListID
 */
void setPlaylistField(gint PlayListID) {
    // This function will populate the playlist_PL_List widget with the
    // details of the selected playlist.
    LIBMTP_playlist_t* tmpplaylist = devicePlayLists;
    gint tmpplaylistID = PlayListID;
    gint trackID = 0;
    GtkTreeIter rowIter;
    gchar * tmp_string = NULL;

    playlist_track_count = 0;

    gtk_list_store_clear(GTK_LIST_STORE(playlist_PL_List));

    if (PlayListID > 0) {
        while (tmpplaylistID--)
            if (tmpplaylist->next != NULL)
                tmpplaylist = tmpplaylist->next;
    }
    // tmpplaylist points to our playlist;
    for (trackID = 0; trackID < tmpplaylist->no_tracks; trackID++) {
        LIBMTP_track_t *trackinfo;
        trackinfo = LIBMTP_Get_Trackmetadata(DeviceMgr.device, tmpplaylist->tracks[trackID]);
        if (trackinfo != NULL) {
            playlist_track_count++;
            gtk_list_store_append(GTK_LIST_STORE(playlist_PL_List), &rowIter);
            tmp_string = g_strdup_printf("%d:%.2d", (int) ((trackinfo->duration / 1000) / 60), (int) ((trackinfo->duration / 1000) % 60));
            gtk_list_store_set(GTK_LIST_STORE(playlist_PL_List), &rowIter, COL_PL_ORDER_NUM, playlist_track_count,
                COL_PL_ARTIST, trackinfo->artist,
                COL_PL_ALBUM, trackinfo->album, COL_PL_TRACKID, trackinfo->item_id,
                COL_PL_TRACKNAME, trackinfo->title, COL_PL_TRACKDURATION, tmp_string, -1);
            g_free(tmp_string);
            tmp_string = NULL;

            LIBMTP_destroy_track_t(trackinfo);
        } else {
            LIBMTP_Dump_Errorstack(DeviceMgr.device);
            LIBMTP_Clear_Errorstack(DeviceMgr.device);
        }
    }
}

// ************************************************************************************************

/**
 * Display the New Playlist Dialog box.
 * @return The name of the new playlist.
 */
gchar* displayPlaylistNewDialog(void) {
    GtkWidget *dialog, *hbox, *label, *textbox;
    gchar* textfield;

    dialog = gtk_dialog_new_with_buttons(_("New Playlist"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_OK, GTK_RESPONSE_OK,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    
#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
#endif

    label = gtk_label_new(_("Playlist Name:"));
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(hbox), label);

    textbox = gtk_entry_new();
    gtk_widget_show(textbox);
    gtk_entry_set_max_length(GTK_ENTRY(textbox), 64);
    gtk_entry_set_has_frame(GTK_ENTRY(textbox), TRUE);
    gtk_entry_set_activates_default(GTK_ENTRY(textbox), TRUE);
    gtk_container_add(GTK_CONTAINER(hbox), textbox);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        textfield = g_strdup(gtk_entry_get_text(GTK_ENTRY(textbox)));
        if (strlen(textfield) == 0) {
            // We have an emtpy string.
            gtk_widget_destroy(dialog);
            return NULL;
        } else {
            gtk_widget_destroy(dialog);
            return textfield;
        }
    } else {
        gtk_widget_destroy(dialog);
        return NULL;
    }
}

// ************************************************************************************************

/**
 * Get the list of selected tracks in the playlist editor.
 * @return
 */
GList* playlist_PL_ListGetSelection() {
    GList *selectedFiles, *ptr;
    GtkTreeRowReference *ref;
    GtkTreeModel *model;
    // Lets clear up the old list.
    g_list_free(playlist_Selection_PL_RowReferences);
    playlist_Selection_PL_RowReferences = NULL;

    if (gtk_tree_selection_count_selected_rows(playlist_PL_Selection) == 0) {
        // We have no rows.
        return NULL;
    }
    // So now we must convert each selection to a row reference and store it in a new GList variable
    // which we will return below.
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview_Playlist_Files));
    selectedFiles = gtk_tree_selection_get_selected_rows(playlist_PL_Selection, &model);
    ptr = selectedFiles;
    while (ptr != NULL) {
        ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(playlist_PL_List), (GtkTreePath*) ptr->data);
        playlist_Selection_PL_RowReferences = g_list_append(playlist_Selection_PL_RowReferences, gtk_tree_row_reference_copy(ref));
        gtk_tree_row_reference_free(ref);
        ptr = ptr->next;
    }
    g_list_foreach(selectedFiles, (GFunc) gtk_tree_path_free, NULL);
    g_list_free(selectedFiles);
    return playlist_Selection_PL_RowReferences;
}

// ************************************************************************************************

/**
 * Clear the selection of tracks in the playlist.
 * @return
 */
gboolean playlist_PL_ListClearSelection() {
    if (playlist_PL_Selection != NULL)
        gtk_tree_selection_unselect_all(playlist_PL_Selection);
    return TRUE;
}

// ************************************************************************************************

/**
 * Remove the selected tracks from the playlist.
 * @param List
 * @return
 */
gboolean playlist_PL_ListRemove(GList *List) {
    GtkTreeIter iter;
    gint tracknumber = 1;

    playlist_PL_ListClearSelection();
    g_list_foreach(List, (GFunc) __playlist_PL_Remove, NULL);

    // Now reorder all tracks in this playlist.
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playlist_PL_List), &iter)) {
        gtk_list_store_set(GTK_LIST_STORE(playlist_PL_List), &iter, COL_PL_ORDER_NUM, tracknumber, -1);
        tracknumber++;
        while (gtk_tree_model_iter_next(GTK_TREE_MODEL(playlist_PL_List), &iter)) {
            gtk_list_store_set(GTK_LIST_STORE(playlist_PL_List), &iter, COL_PL_ORDER_NUM, tracknumber, -1);
            tracknumber++;
        }
    }
    return TRUE;
}

// ************************************************************************************************

/**
 * Remove the track from the current playlist.
 * @param Row
 */
void __playlist_PL_Remove(GtkTreeRowReference *Row) {
    GtkTreePath *path;
    GtkTreeIter iter;
    // convert the referenece to a path and retrieve the iterator;
    path = gtk_tree_row_reference_get_path(Row);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_PL_List), &iter, path);
    // We have our Iter now.
    gtk_list_store_remove(GTK_LIST_STORE(playlist_PL_List), &iter);
    playlist_track_count--;
}

// ************************************************************************************************

/**
 * Get the selection of tracks in the current playlist.
 * @return
 */
GList* playlist_TrackList_GetSelection() {
    GList *selectedFiles, *ptr;
    GtkTreeRowReference *ref;
    GtkTreeModel *model;
    // Lets clear up the old list.
    g_list_free(playlist_Selection_TrackRowReferences);
    playlist_Selection_TrackRowReferences = NULL;

    if (gtk_tree_selection_count_selected_rows(playlist_TrackSelection) == 0) {
        // We have no rows.
        return NULL;
    }
    // So now we must convert each selection to a row reference and store it in a new GList variable
    // which we will return below.
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview_Avail_Files));
    selectedFiles = gtk_tree_selection_get_selected_rows(playlist_TrackSelection, &model);
    ptr = selectedFiles;
    while (ptr != NULL) {
        ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(playlist_TrackList), (GtkTreePath*) ptr->data);
        playlist_Selection_TrackRowReferences = g_list_append(playlist_Selection_TrackRowReferences, gtk_tree_row_reference_copy(ref));
        gtk_tree_row_reference_free(ref);
        ptr = ptr->next;
    }
    g_list_foreach(selectedFiles, (GFunc) gtk_tree_path_free, NULL);
    g_list_free(selectedFiles);
    return playlist_Selection_TrackRowReferences;
}

// ************************************************************************************************

/**
 * Add the list of tracks to the selected playlist.
 * @param List
 * @return
 */
gboolean playlist_TrackList_Add(GList *List) {
    g_list_foreach(List, (GFunc) __playlist_TrackList_Add, NULL);
    return TRUE;
}

// ************************************************************************************************

/**
 * Add the individual track to the playlist.
 * @param Row
 */
void __playlist_TrackList_Add(GtkTreeRowReference *Row) {
    GtkTreePath *path = NULL;
    GtkTreeIter iter;
    GtkTreeIter PL_rowIter;
    gchar* artist = NULL;
    gchar* album = NULL;
    gchar* title = NULL;
    gint item_id = 0;
    gchar * duration = NULL;

    // convert the referenece to a path and retrieve the iterator;
    path = gtk_tree_row_reference_get_path(Row);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_TrackList), &iter, path);
    // We have our Iter now, so get the required information from the track treeview.
    gtk_tree_model_get(GTK_TREE_MODEL(playlist_TrackList), &iter, COL_ARTIST, &artist, COL_ALBUM, &album,
        COL_TRACKID, &item_id, COL_TRACKNAME, &title, COL_TRACKDURATION, & duration, -1);
    // Now store our information in the playlist treeview.
    playlist_track_count++;
    gtk_list_store_append(GTK_LIST_STORE(playlist_PL_List), &PL_rowIter);
    gtk_list_store_set(GTK_LIST_STORE(playlist_PL_List), &PL_rowIter, COL_PL_ORDER_NUM, playlist_track_count, COL_PL_ARTIST, artist,
        COL_PL_ALBUM, album, COL_PL_TRACKID, item_id, COL_PL_TRACKNAME, title, COL_PL_TRACKDURATION, duration, -1);

    //Need to free our string values
    g_free(artist);
    g_free(album);
    g_free(title);
    g_free(duration);
}

// ************************************************************************************************

/**
 * Reorder the tracks within the playlist.
 * @param direction
 * @return
 */
gboolean playlist_move_files(gint direction) {
    GList * playlist_files = NULL;
    GtkTreeIter iter;
    gint tracknumber = 1;
    // Get our files...
    playlist_files = playlist_PL_ListGetSelection();
    if (playlist_files == NULL)
        return FALSE;

    // If we are moving files down we need to reverse the rows references...
    if (direction == 1) {
        playlist_files = g_list_reverse(playlist_files);
        g_list_foreach(playlist_files, (GFunc) __playlist_move_files_down, NULL);
    } else {
        g_list_foreach(playlist_files, (GFunc) __playlist_move_files_up, NULL);
    }
    // Now reorder all tracks in this playlist.
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playlist_PL_List), &iter)) {
        gtk_list_store_set(GTK_LIST_STORE(playlist_PL_List), &iter, COL_PL_ORDER_NUM, tracknumber, -1);
        tracknumber++;
        while (gtk_tree_model_iter_next(GTK_TREE_MODEL(playlist_PL_List), &iter)) {
            gtk_list_store_set(GTK_LIST_STORE(playlist_PL_List), &iter, COL_PL_ORDER_NUM, tracknumber, -1);
            tracknumber++;
        }
    }
    return TRUE;
}

// ************************************************************************************************

/**
 * Move the selected track up in the playlist.
 * @param Row
 */
void __playlist_move_files_up(GtkTreeRowReference *Row) {
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeIter iter2;
    // convert the referenece to a path and retrieve the iterator;
    path = gtk_tree_row_reference_get_path(Row);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_PL_List), &iter, path);
    // We have our Iter now.
    // Now get it's prev path and turn it into a iter
    if (gtk_tree_path_prev(path) == TRUE) {
        // we have a previous entry...
        gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_PL_List), &iter2, path);
        gtk_list_store_swap(GTK_LIST_STORE(playlist_PL_List), &iter, &iter2);
    }

}

// ************************************************************************************************

/**
 * Move the selected track down in the playlist.
 * @param Row
 */
void __playlist_move_files_down(GtkTreeRowReference *Row) {
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeIter iter2;
    // convert the referenece to a path and retrieve the iterator;
    path = gtk_tree_row_reference_get_path(Row);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_PL_List), &iter, path);
    // We have our Iter now.
    iter2 = iter;
    if (gtk_tree_model_iter_next(GTK_TREE_MODEL(playlist_PL_List), &iter2) == TRUE) {
        // we have something to swap with...
        gtk_list_store_swap(GTK_LIST_STORE(playlist_PL_List), &iter, &iter2);
    }
}

// ************************************************************************************************

/**
 * Save the current selected playlist to the device.
 * @param PlayListID
 */
void playlist_SavePlaylist(gint PlayListID) {
    LIBMTP_playlist_t* tmpplaylist = devicePlayLists;
    gint tmpplaylistID = PlayListID;
    gint item_id = 0;
    GtkTreeIter iter;
    uint32_t *tmp = NULL;

    if (PlayListID > 0) {
        while (tmpplaylistID--)
            if (tmpplaylist->next != NULL)
                tmpplaylist = tmpplaylist->next;
    }
    // tmpplaylist points to our playlist;
    // So all we need to do is - update our current structure with the new details

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playlist_PL_List), &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(playlist_PL_List), &iter, COL_PL_TRACKID, &item_id, -1);
        tmpplaylist->no_tracks = 1;

        // item_id = our track num... so append to tmpplaylist->tracks
        if ((tmp = g_realloc(tmpplaylist->tracks, sizeof (uint32_t) * (tmpplaylist->no_tracks))) == NULL) {
            g_fprintf(stderr, _("realloc in savePlayList failed\n"));
            displayError(_("Updating playlist failed? 'realloc in savePlayList'\n"));
            return;
        }
        tmpplaylist->tracks = tmp;
        tmpplaylist->tracks[(tmpplaylist->no_tracks - 1)] = item_id;
        //tmpplaylist->no_tracks++;
        while (gtk_tree_model_iter_next(GTK_TREE_MODEL(playlist_PL_List), &iter)) {
            gtk_tree_model_get(GTK_TREE_MODEL(playlist_PL_List), &iter, COL_PL_TRACKID, &item_id, -1);
            tmpplaylist->no_tracks++;
            // item_id = our track num... so append to tmpplaylist->tracks
            if ((tmp = g_realloc(tmpplaylist->tracks, sizeof (uint32_t) * (tmpplaylist->no_tracks))) == NULL) {
                g_fprintf(stderr, _("realloc in savePlayList failed\n"));
                displayError(_("Updating playlist failed? 'realloc in savePlayList'\n"));
                return;
            }
            tmpplaylist->tracks = tmp;
            tmpplaylist->tracks[(tmpplaylist->no_tracks - 1)] = item_id;
            //tmpplaylist->no_tracks++;

        }
    }
    // get libmtp to save it.
    playlistUpdate(tmpplaylist);
    // Update our own metadata.
    devicePlayLists = getPlaylists();
}

// ************************************************************************************************

/**
 * Creates the Format Device Dialog box
 * @return Widget of completed dialog box.
 */
GtkWidget* create_windowFormat(void) {
    GtkWidget* windowFormat;
    GtkWidget* label1;
    GtkWidget* vbox1;

    windowFormat = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gchar * winTitle;
    winTitle = g_strconcat(PACKAGE_TITLE, " v", PACKAGE_VERSION, NULL);
    gtk_window_set_title(GTK_WINDOW(windowFormat), winTitle);
    gtk_window_set_position(GTK_WINDOW(windowFormat), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal(GTK_WINDOW(windowFormat), TRUE);
    //gtk_window_set_resizable(GTK_WINDOW(window1), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(windowFormat), GTK_WINDOW(windowMain));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(windowFormat), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(windowFormat), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_default_size(GTK_WINDOW(windowFormat), 200, 60);
    g_free(winTitle);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(windowFormat), vbox1);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 10);
    gtk_box_set_spacing(GTK_BOX(vbox1), 5);

    label1 = gtk_label_new("Formatting...");
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(vbox1), label1, TRUE, TRUE, 0);
    gtk_misc_set_padding(GTK_MISC(label1), 0, 5);
    gtk_misc_set_alignment(GTK_MISC(label1), 0, 0);

    formatDialog_progressBar1 = gtk_progress_bar_new();
    gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(formatDialog_progressBar1), 0.05);
    gtk_widget_show(formatDialog_progressBar1);
    gtk_box_pack_start(GTK_BOX(vbox1), formatDialog_progressBar1, TRUE, TRUE, 0);
 
    return windowFormat;
}

// ************************************************************************************************

/**
 * Displays the playlist selection dialog used to auto add track to playlist option.
 * @return The playlist MTP Object ID of the selected playlist, or GMTP_NO_PLAYLIST if none selected.
 */
uint32_t displayAddTrackPlaylistDialog(void){
    GtkWidget *dialog, *hbox, *label, *buttonNewPlaylist;
    LIBMTP_playlist_t* tmpplaylist = NULL;
    gint selectedPlaylist = 0;

#if GMTP_USE_GTK2
    GtkTooltips *tooltips;
    tooltips = gtk_tooltips_new();
#endif

    dialog = gtk_dialog_new_with_buttons(_("Playlists"), GTK_WINDOW(windowMain),
        (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_STOCK_OK, GTK_RESPONSE_OK,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);

#if GMTP_USE_GTK2
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
#else
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
#endif

    // Add in the label
    label = gtk_label_new(_("Playlist Name:"));
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(hbox), label);
    gtk_misc_set_padding(GTK_MISC(label), 5, 0);

    // Add in the combobox
#if GMTP_USE_GTK2
    combobox_AddTrackPlaylist = gtk_combo_box_new_text();
#else
    combobox_AddTrackPlaylist = gtk_combo_box_text_new();
#endif
    gtk_widget_show(combobox_AddTrackPlaylist);
    gtk_box_pack_start(GTK_BOX(hbox), combobox_AddTrackPlaylist, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(combobox_AddTrackPlaylist), 5);

    // Add in the new playlist button.
    buttonNewPlaylist = gtk_button_new_from_stock(GTK_STOCK_ADD);
    gtk_widget_show(buttonNewPlaylist);
    gtk_container_add(GTK_CONTAINER(hbox), buttonNewPlaylist);
    gtk_container_set_border_width(GTK_CONTAINER(buttonNewPlaylist), 5);

#if GMTP_USE_GTK2
    gtk_tooltips_set_tip(tooltips, buttonNewPlaylist, _("Add New Playlist"), NULL);
#else
    gtk_widget_set_tooltip_text(buttonNewPlaylist, _("Add New Playlist"));
#endif

    // Assign the callback for the new playlist button.
    g_signal_connect((gpointer) buttonNewPlaylist, "clicked",
        G_CALLBACK(on_TrackPlaylist_NewPlaylistButton_activate),
        NULL);

    // Populate the combobox with the current playlists.

    // We need to remove all entries in the combo box before starting.
    // This is a little bit of a hack - but does work.
    gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combobox_AddTrackPlaylist))));

    if (devicePlayLists != NULL) {
        // Populate the playlist dropdown box;
        //comboboxentry_playlist;
        tmpplaylist = devicePlayLists;
        while (tmpplaylist != NULL) {
            if (tmpplaylist->storage_id == DeviceMgr.devicestorage->id) {
#if GMTP_USE_GTK2
                gtk_combo_box_append_text(GTK_COMBO_BOX(combobox_AddTrackPlaylist), g_strdup(tmpplaylist->name));
#else
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox_AddTrackPlaylist), g_strdup(tmpplaylist->name));
#endif
            }
            tmpplaylist = tmpplaylist->next;
        }
    }
    if (devicePlayLists != NULL) {
        // Set our playlist to the first one.
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_AddTrackPlaylist), 0);
    }
   
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        // Get our playlist ID.
        selectedPlaylist = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_AddTrackPlaylist));
        // Now cycle through the playlists to get the correct one.
        tmpplaylist = devicePlayLists;

        if (selectedPlaylist > 0) {
            while (selectedPlaylist--)
                if (tmpplaylist->next != NULL)
                    tmpplaylist = tmpplaylist->next;
        }
        gtk_widget_destroy(dialog);
        return tmpplaylist->playlist_id;
    } else {
        gtk_widget_destroy(dialog);
        return GMTP_NO_PLAYLIST;
    }
}
