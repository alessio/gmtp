
#include "config.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
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
GtkWidget *toolbuttonProperties;
GtkWidget *properties1;
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

// Menu widget for Properties
GtkListStore *fileList;
GtkTreeSelection *fileSelection;
GList *fileSelection_RowReferences = NULL;

// Widgets for preferences buttons;
GtkWidget *checkbuttonDeviceConnect;
GtkWidget *entryDownloadPath;
GtkWidget *entryUploadPath;
GtkWidget *checkbuttonDownloadPath;
GtkWidget *checkbuttonConfirmFileOp;
GtkWidget *checkbuttonConfirmOverWriteFileOp;

// Widget for Progress Bar Dialog box.
GtkWidget *progressDialog;
GtkWidget *progressDialog_Text;
GtkWidget *progressDialog_Bar;
gchar *progressDialog_filename;

// Flags for overwriting files of host PC and device.
gint fileoverwriteop = MTP_ASK;
// Flag to allow overwrite of files on device.
gint deviceoverwriteop = MTP_ASK;

// AlbumArt Dialog global pointers
GtkWidget *AlbumArtDialog;
GtkWidget *AlbumArtFilename;

GtkWidget*
create_windowMain (void)
{
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
	GtkWidget *quit1;
	GtkWidget *menuitem2;
	GtkWidget *menuitem2_menu;
    GtkWidget *preferences1;
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
	//gchar tmp_string[256];

	accel_group = gtk_accel_group_new ();

	windowMain = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gchar * winTitle;
	winTitle = g_strconcat(PACKAGE," v", PACKAGE_VERSION, NULL);
	gtk_window_set_title (GTK_WINDOW (windowMain), (winTitle));
	gtk_window_set_default_size (GTK_WINDOW (windowMain), 760, 400);
    gtk_window_set_icon_from_file(GTK_WINDOW(windowMain), file_icon_png->str, NULL);
	g_free(winTitle);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (windowMain), vbox1);

	menubarMain = gtk_menu_bar_new ();
	gtk_widget_show (menubarMain);
	gtk_box_pack_start (GTK_BOX (vbox1), menubarMain, FALSE, FALSE, 0);

	menuitem1 = gtk_menu_item_new_with_mnemonic (("_File"));
	gtk_widget_show (menuitem1);
	gtk_container_add (GTK_CONTAINER (menubarMain), menuitem1);

	menuitem1_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem1), menuitem1_menu);

	fileConnect = gtk_image_menu_item_new_from_stock ("gtk-network", accel_group);
	menuText = gtk_bin_get_child(GTK_BIN(fileConnect));
	gtk_label_set_text(GTK_LABEL(menuText), "Connect Device");
	gtk_widget_show (fileConnect);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), fileConnect);

	menuseparator4 = gtk_separator_menu_item_new();
	gtk_widget_show (menuseparator4);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), menuseparator4);

	fileAdd = gtk_image_menu_item_new_from_stock ("gtk-add", accel_group);
	menuText = gtk_bin_get_child(GTK_BIN(fileAdd));
	gtk_label_set_text(GTK_LABEL(menuText), "Add Files");
	gtk_widget_show (fileAdd);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), fileAdd);

	fileRemove = gtk_image_menu_item_new_from_stock ("gtk-remove", accel_group);
	menuText = gtk_bin_get_child(GTK_BIN(fileRemove));
	gtk_label_set_text(GTK_LABEL(menuText), "Delete Files");
	gtk_widget_show (fileRemove);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), fileRemove);

	fileDownload = gtk_image_menu_item_new_from_stock ("gtk-goto-bottom", accel_group);
	menuText = gtk_bin_get_child(GTK_BIN(fileDownload));
	gtk_label_set_text(GTK_LABEL(menuText), "Download Files");
	gtk_widget_show (fileDownload);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), fileDownload);

	menuseparator1 = gtk_separator_menu_item_new();
	gtk_widget_show (menuseparator1);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), menuseparator1);

	fileNewFolder = gtk_image_menu_item_new_with_label ("Create Folder");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(fileNewFolder), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU ));
	gtk_widget_show (fileNewFolder);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), fileNewFolder);

	fileRemoveFolder = gtk_image_menu_item_new_from_stock ("gtk-delete", accel_group);
	menuText = gtk_bin_get_child(GTK_BIN(fileRemoveFolder));
	gtk_label_set_text(GTK_LABEL(menuText), "Delete Folder");
	gtk_widget_show (fileRemoveFolder);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), fileRemoveFolder);

	menuseparator2 = gtk_separator_menu_item_new();
	gtk_widget_show (menuseparator2);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), menuseparator2);

	fileRescan = gtk_image_menu_item_new_from_stock ("gtk-refresh", accel_group);
	menuText = gtk_bin_get_child(GTK_BIN(fileRescan));
	gtk_label_set_text(GTK_LABEL(menuText), "Refresh Device");
	gtk_widget_show (fileRescan);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), fileRescan);

	properties1 = gtk_image_menu_item_new_from_stock ("gtk-properties", accel_group);
	menuText = gtk_bin_get_child(GTK_BIN(properties1));
	gtk_label_set_text(GTK_LABEL(menuText), "Device Properties");
	gtk_widget_show (properties1);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), properties1);

	menuseparator3 = gtk_separator_menu_item_new();
	gtk_widget_show (menuseparator3);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), menuseparator3);

	quit1 = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
	gtk_widget_show (quit1);
	gtk_container_add (GTK_CONTAINER (menuitem1_menu), quit1);

	menuitem2 = gtk_menu_item_new_with_mnemonic (("_Edit"));
	gtk_widget_show (menuitem2);
	gtk_container_add (GTK_CONTAINER (menubarMain), menuitem2);

	menuitem2_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), menuitem2_menu);

    editDeviceName = gtk_image_menu_item_new_with_label("Change Device Name");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(editDeviceName), gtk_image_new_from_file(file_icon16_png->str));
    
	gtk_widget_show (editDeviceName);
	gtk_container_add (GTK_CONTAINER (menuitem2_menu), editDeviceName);

    editAddAlbumArt = gtk_image_menu_item_new_from_stock (GTK_STOCK_CDROM, accel_group);
    menuText = gtk_bin_get_child(GTK_BIN(editAddAlbumArt));
	gtk_label_set_text(GTK_LABEL(menuText), "Add Album Art");
	gtk_widget_show (editAddAlbumArt);
	gtk_container_add (GTK_CONTAINER (menuitem2_menu), editAddAlbumArt);

    menuseparator5 = gtk_separator_menu_item_new();
	gtk_widget_show (menuseparator5);
	gtk_container_add (GTK_CONTAINER (menuitem2_menu), menuseparator5);

	preferences1 = gtk_image_menu_item_new_from_stock ("gtk-preferences", accel_group);
	gtk_widget_show (preferences1);
	gtk_container_add (GTK_CONTAINER (menuitem2_menu), preferences1);

	menuitem4 = gtk_menu_item_new_with_mnemonic (("_Help"));
	gtk_widget_show (menuitem4);
	gtk_container_add (GTK_CONTAINER (menubarMain), menuitem4);

	menuitem4_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem4), menuitem4_menu);

	about1 = gtk_image_menu_item_new_with_mnemonic(("_About"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(about1), gtk_image_new_from_file(file_about_png->str));
	gtk_widget_show (about1);
	gtk_container_add (GTK_CONTAINER (menuitem4_menu), about1);

	handlebox1 = gtk_handle_box_new ();
	gtk_widget_show (handlebox1);
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox1), GTK_SHADOW_ETCHED_OUT);
	gtk_box_pack_start (GTK_BOX (vbox1), handlebox1, FALSE, FALSE, 0);

    tooltipsToolbar = gtk_tooltips_new();

	toolbarMain = gtk_toolbar_new ();
	gtk_widget_show (toolbarMain);
	gtk_container_add (GTK_CONTAINER (handlebox1), toolbarMain);
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbarMain), GTK_TOOLBAR_BOTH);
	tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbarMain));

    gtk_toolbar_set_tooltips(GTK_TOOLBAR(toolbarMain), TRUE);

	tmp_image = gtk_image_new_from_stock ("gtk-network", tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonConnect = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Connect"));
	gtk_widget_show (toolbuttonConnect);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonConnect);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonConnect), GTK_TOOLTIPS (tooltipsToolbar), "Connect/Disconnect to your device.", "Connect/Disconnect to your device.");

	toolbarSeparator = (GtkWidget*) gtk_separator_tool_item_new();
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbarSeparator), TRUE);
	gtk_widget_show(toolbarSeparator);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbarSeparator);

	tmp_image = gtk_image_new_from_stock ("gtk-add", tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonAddFile = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Add"));
	gtk_widget_show (toolbuttonAddFile);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonAddFile);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonAddFile), GTK_TOOLTIPS (tooltipsToolbar), "Add Files to your device.", "Add a varity of Files to your device in the current folder.");

    tmp_image = gtk_image_new_from_stock ("gtk-remove", tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonRemoveFile = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Delete"));
	gtk_widget_show (toolbuttonRemoveFile);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonRemoveFile);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonRemoveFile), GTK_TOOLTIPS (tooltipsToolbar), "Delete Files/Folders from your device.", "Permantly remove files/folders from your device. Note: Albums are stored as *.alb files.");

	tmp_image = gtk_image_new_from_stock ("gtk-goto-bottom", tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonRetrieve = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Download"));
	gtk_widget_show (toolbuttonRetrieve);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonRetrieve);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonRetrieve), GTK_TOOLTIPS (tooltipsToolbar), "Download Files from your device to your Host PC.", "Download files from your device to your PC. Default Download path is set in the prefernces dialog.");

    toolbarSeparator2 = (GtkWidget*) gtk_separator_tool_item_new();
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbarSeparator2), TRUE);
	gtk_widget_show(toolbarSeparator2);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbarSeparator2);

    tmp_image = gtk_image_new_from_stock (GTK_STOCK_CDROM, tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonAlbumArt = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Album Art"));
	gtk_widget_show (toolbuttonAlbumArt);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonAlbumArt);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonAlbumArt), GTK_TOOLTIPS (tooltipsToolbar), "Upload an image file as Album Art.", "Upload a JPG file and assign it as Album Art.");

	tmp_image = gtk_image_new_from_stock ("gtk-refresh", tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonRescan = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Refresh"));
	gtk_widget_show (toolbuttonRescan);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonRescan);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonRescan), GTK_TOOLTIPS (tooltipsToolbar), "Refresh File/Folder listing.", "Refresh File/Folder listing.");

	toolbarSeparator = (GtkWidget*) gtk_separator_tool_item_new();
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbarSeparator), TRUE);
	gtk_widget_show(toolbarSeparator);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbarSeparator);

	tmp_image = gtk_image_new_from_stock ("gtk-properties", tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonProperties = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Properties"));
	gtk_widget_show (toolbuttonProperties);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonProperties);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonProperties), GTK_TOOLTIPS (tooltipsToolbar), "View Device Properties.", "View Device Properties.");

	tmp_image = gtk_image_new_from_stock ("gtk-preferences", tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonPreferences = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Preferences"));
	gtk_widget_show (toolbuttonPreferences);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonPreferences);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonPreferences), GTK_TOOLTIPS (tooltipsToolbar), "View/Change gMTP Preferences.", "View/Change gMTP Preferences.");

	toolbarSeparator = (GtkWidget*) gtk_separator_tool_item_new();
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolbarSeparator), TRUE);
	gtk_widget_show(toolbarSeparator);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbarSeparator);

	tmp_image = gtk_image_new_from_stock ("gtk-quit", tmp_toolbar_icon_size);
	gtk_widget_show (tmp_image);
	toolbuttonQuit = (GtkWidget*) gtk_tool_button_new (tmp_image, ("Quit"));
	gtk_widget_show (toolbuttonQuit);
	gtk_container_add (GTK_CONTAINER (toolbarMain), toolbuttonQuit);
    gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(toolbuttonQuit), GTK_TOOLTIPS (tooltipsToolbar), "Quit gMTP.", "Quit");

    gtk_tooltips_enable (tooltipsToolbar);

	scrolledwindowMain = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindowMain);
	gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindowMain, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindowMain), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	treeviewFiles = gtk_tree_view_new ();
	gtk_widget_show (treeviewFiles);
	gtk_container_add (GTK_CONTAINER (scrolledwindowMain), treeviewFiles);
	gtk_container_set_border_width (GTK_CONTAINER (treeviewFiles), 5);
	fileSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeviewFiles));
	gtk_tree_selection_set_mode(fileSelection, GTK_SELECTION_MULTIPLE);

	fileList = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_UINT, G_TYPE_BOOLEAN, G_TYPE_UINT64);
	setupFileList(treeviewFiles);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeviewFiles), GTK_TREE_MODEL(fileList));
	g_object_unref(fileList);

	windowStatusBar = gtk_statusbar_new ();
	gtk_widget_show (windowStatusBar);
	gtk_box_pack_start (GTK_BOX (vbox1), windowStatusBar, FALSE, FALSE, 0);

    // Build our right-click context menu;
    contextMenu = create_windowMainContextMenu();

    // DnD functions
    //gmtp_drag_dest_set(windowMain); // This gets done in the connect callback and
    // only activates if we actually are connected to a device.

	gtk_signal_connect(GTK_OBJECT(windowMain), "drag-data-received",
                    GTK_SIGNAL_FUNC(gmtp_drag_data_received), NULL);
    // End Dnd functions

	g_signal_connect ((gpointer) windowMain, "destroy",
					  G_CALLBACK (on_quit1_activate),
					  NULL);

	g_signal_connect ((gpointer) properties1, "activate",
					  G_CALLBACK (on_deviceProperties_activate),
					  NULL);
	g_signal_connect ((gpointer) toolbuttonProperties, "clicked",
					  G_CALLBACK (on_deviceProperties_activate),
					  NULL);

	g_signal_connect ((gpointer) quit1, "activate",
					  G_CALLBACK (on_quit1_activate),
					  NULL);

	g_signal_connect ((gpointer) preferences1, "activate",
					  G_CALLBACK (on_preferences1_activate),
					  NULL);

    g_signal_connect ((gpointer) editDeviceName, "activate",
					  G_CALLBACK (on_editDeviceName_activate),
					  NULL);

    g_signal_connect ((gpointer) editAddAlbumArt, "activate",
					  G_CALLBACK (on_editAddAlbumArt_activate),
					  NULL);

	g_signal_connect ((gpointer) fileAdd, "activate",
					  G_CALLBACK (on_filesAdd_activate),
					  NULL);

	g_signal_connect ((gpointer) fileDownload, "activate",
					  G_CALLBACK (on_filesDownload_activate),
					  NULL);

	g_signal_connect ((gpointer) fileRemove, "activate",
					  G_CALLBACK (on_filesDelete_activate),
					  NULL);

	g_signal_connect ((gpointer) fileConnect, "activate",
					  G_CALLBACK (on_deviceConnect_activate),
					  NULL);

	g_signal_connect ((gpointer) fileNewFolder, "activate",
					  G_CALLBACK (on_fileNewFolder_activate),
					  NULL);

	g_signal_connect ((gpointer) fileRemoveFolder, "activate",
					  G_CALLBACK (on_fileRemoveFolder_activate),
					  NULL);

	g_signal_connect ((gpointer) fileRescan, "activate",
					  G_CALLBACK (on_deviceRescan_activate),
					  NULL);

	g_signal_connect ((gpointer) about1, "activate",
					  G_CALLBACK (on_about1_activate),
					  NULL);

	g_signal_connect ((gpointer) toolbuttonQuit, "clicked",
					  G_CALLBACK (on_quit1_activate),
					  NULL);

	g_signal_connect ((gpointer) toolbuttonRescan, "clicked",
					  G_CALLBACK (on_deviceRescan_activate),
					  NULL);

	g_signal_connect ((gpointer) toolbuttonAddFile, "clicked",
					  G_CALLBACK (on_filesAdd_activate),
					  NULL);

	g_signal_connect ((gpointer) toolbuttonRemoveFile, "clicked",
					  G_CALLBACK (on_filesDelete_activate),
					  NULL);

	g_signal_connect ((gpointer) toolbuttonRetrieve, "clicked",
					  G_CALLBACK (on_filesDownload_activate),
					  NULL);

    g_signal_connect ((gpointer) toolbuttonAlbumArt, "clicked",
					  G_CALLBACK (on_editAddAlbumArt_activate),
					  NULL);

	g_signal_connect ((gpointer) toolbuttonConnect, "clicked",
					  G_CALLBACK (on_deviceConnect_activate),
					  NULL);
	g_signal_connect ((gpointer) toolbuttonPreferences, "clicked",
					  G_CALLBACK (on_preferences1_activate),
					  NULL);

	g_signal_connect ((gpointer) treeviewFiles, "row-activated",
					  G_CALLBACK (fileListRowActivated),
					  NULL);

    g_signal_connect_swapped (treeviewFiles, "button_press_event",
                    G_CALLBACK (on_windowMainContextMenu_activate), contextMenu);


	gtk_window_add_accel_group (GTK_WINDOW (windowMain), accel_group);

	return windowMain;
}

void statusBarSet(gchar *text){
	statusBarClear();
	guint c_id1= gtk_statusbar_get_context_id(GTK_STATUSBAR(windowStatusBar), "");
	gtk_statusbar_push(GTK_STATUSBAR(windowStatusBar), c_id1, text);
}

void statusBarClear(){
	guint c_id1= gtk_statusbar_get_context_id(GTK_STATUSBAR(windowStatusBar), "");
	gtk_statusbar_pop(GTK_STATUSBAR(windowStatusBar), c_id1);
}

void SetToolbarButtonState(gboolean state){
	gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonAddFile), state);
	gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonRemoveFile), state);
	gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonRetrieve), state);
	gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonRescan), state);
    gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonAlbumArt), state);
	gtk_widget_set_sensitive(GTK_WIDGET(toolbuttonProperties), state);
	gtk_widget_set_sensitive(GTK_WIDGET(properties1), state);
	gtk_widget_set_sensitive(GTK_WIDGET(fileAdd), state);
	gtk_widget_set_sensitive(GTK_WIDGET(fileDownload), state);
	gtk_widget_set_sensitive(GTK_WIDGET(fileRemove), state);
	gtk_widget_set_sensitive(GTK_WIDGET(fileNewFolder), state);
	gtk_widget_set_sensitive(GTK_WIDGET(fileRemoveFolder), state);
	gtk_widget_set_sensitive(GTK_WIDGET(fileRescan), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editDeviceName), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editAddAlbumArt), state);
	gtk_widget_set_sensitive(GTK_WIDGET(treeviewFiles), state);
}

void setupFileList(GtkTreeView *treeviewFiles) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	// Filename column
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Filename", renderer,
													  "text", COL_FILENAME,
													  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
	gtk_tree_view_column_set_sort_column_id(column, COL_FILENAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_spacing(column, 5);

	// File Size column
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Size", renderer,
													  "text", COL_FILESIZE,
													  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
	gtk_tree_view_column_set_sort_column_id(column, COL_FILESIZE_HID);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_spacing(column, 5);

	// Folder/FileID column
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Object ID", renderer,
													  "text", COL_FILEID,
													  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
	gtk_tree_view_column_set_visible (column, FALSE);

	// isFolder column
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("isFolder", renderer,
													  "text", COL_ISFOLDER,
													  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
	gtk_tree_view_column_set_visible (column, FALSE);

    // File size column - hidden used for sorting the visible file size column
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("FileSize Hidden", renderer,
													  "text", COL_FILESIZE_HID,
													  NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeviewFiles), column);
	gtk_tree_view_column_set_visible (column, FALSE);


}

gboolean  fileListClear(){
	gtk_list_store_clear (GTK_LIST_STORE(fileList));
	return TRUE;
}

GSList* getFileGetList2Add(){
	GSList* files;
	GtkWidget *FileDialog;
	gchar *savepath;

	savepath = g_strndup("", 8192);
	FileDialog = gtk_file_chooser_dialog_new("Select Files to Add",
											 GTK_WINDOW(windowMain), GTK_FILE_CHOOSER_ACTION_OPEN,
											 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											 NULL);

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(FileDialog), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemUploadPath->str);
	if (gtk_dialog_run (GTK_DIALOG (FileDialog)) == GTK_RESPONSE_ACCEPT) {
		savepath = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (FileDialog));
		// Save our upload path.
		Preferences.fileSystemUploadPath = g_string_assign(Preferences.fileSystemUploadPath, savepath);

	}
	files = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER (FileDialog));
	gtk_widget_hide(FileDialog);
	gtk_widget_destroy (FileDialog);
	return files;
}

gboolean fileListAdd(){
	GtkTreeIter rowIter;
	gchar filename[8192];
	gchar filesize[256];
	LIBMTP_folder_t *tmpfolder;
	LIBMTP_file_t *tmpfile;
	guint parentID;

	// We start with the folder list...
	if(currentFolderID != 0) {
		// If we are not folderID = 0; then...
		// Scan the folder list for the current folderID, and set the parent ID,
		tmpfolder = deviceFolders;
		parentID = getParentFolderID(tmpfolder, currentFolderID);
		// Now add in the row information.
		gtk_list_store_append(GTK_LIST_STORE(fileList), &rowIter );
		gtk_list_store_set(GTK_LIST_STORE(fileList), &rowIter, COL_FILENAME, "< .. >", COL_FILESIZE, "",
						   COL_FILEID, parentID, COL_ISFOLDER, TRUE, COL_FILESIZE_HID, (guint64) 0, -1 );
	}
	// What we scan for is the folder's details where 'parent_id' == currentFolderID and display those.
	tmpfolder = getParentFolderPtr(deviceFolders, currentFolderID);
	while(tmpfolder != NULL) {
		if((tmpfolder->parent_id == currentFolderID) && (tmpfolder->storage_id == DeviceMgr.devicestorage->id)) {
			gtk_list_store_append(GTK_LIST_STORE(fileList), &rowIter );
			g_sprintf(filename, "< %s >", tmpfolder->name);
			gtk_list_store_set(GTK_LIST_STORE(fileList), &rowIter, COL_FILENAME, filename, COL_FILESIZE, "",
							   COL_FILEID, tmpfolder->folder_id, COL_ISFOLDER, TRUE, COL_FILESIZE_HID, (guint64) 0, -1 );
		}
		//g_printf("folder = %s\n", filename);
		tmpfolder = tmpfolder->sibling;
	}
	// We don't destroy the structure, only on a rescan operation.

	// We scan for files in the file details we 'parent_id' == currentFolderID and display those.
	tmpfile = deviceFiles;
	while(tmpfile != NULL) {
		//g_printf("file = %s, %d, %d\n", tmpfile->filename, tmpfile->parent_id, tmpfile->storage_id);
		if((tmpfile->parent_id == currentFolderID)&&(tmpfile->storage_id == DeviceMgr.devicestorage->id )) {
			gtk_list_store_append(GTK_LIST_STORE(fileList), &rowIter );

			if(tmpfile->filesize < 1000) {
				g_sprintf(filesize, "%d B", tmpfile->filesize);
			} else {
				if(tmpfile->filesize < (1000000)) {
					g_sprintf(filesize, "%.3f KB", (tmpfile->filesize / 1024.00));
				} else {
					g_sprintf(filesize, "%.3f MB", (tmpfile->filesize / (1024.00 * 1024.00)));
				}
			}
			gtk_list_store_set(GTK_LIST_STORE(fileList), &rowIter, COL_FILENAME, tmpfile->filename, COL_FILESIZE, &filesize,
							   COL_FILEID, tmpfile->item_id,  COL_ISFOLDER, FALSE, COL_FILESIZE_HID, tmpfile->filesize , -1 );
		}
		tmpfile = tmpfile->next;
	}
}

gboolean fileListDownload(GList *List){
	GtkWidget *FileDialog;
	gchar *savepath;
	savepath = g_strndup("", 8192);

	// Let's confirm our download path.

	//g_printf("Ask download = %x\n", Preferences.ask_download_path );
	if(Preferences.ask_download_path == TRUE) {

		FileDialog = gtk_file_chooser_dialog_new("Select Path to Download",
												 GTK_WINDOW(windowMain), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
												 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
												 NULL);

		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(FileDialog), Preferences.fileSystemDownloadPath->str);
		if (gtk_dialog_run (GTK_DIALOG (FileDialog)) == GTK_RESPONSE_ACCEPT) {
			savepath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (FileDialog));
			// Save our download path.
			Preferences.fileSystemDownloadPath = g_string_assign(Preferences.fileSystemDownloadPath, savepath);

		}
		gtk_widget_destroy (FileDialog);
	}
	g_free (savepath);

	// We do the deed.
	g_list_foreach(List, (GFunc)__fileDownload, NULL);
    fileoverwriteop = MTP_ASK;
	return TRUE;
}

void __fileDownload(GtkTreeRowReference *Row){
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *filename;
    gchar* fullfilename;
    
	gboolean isFolder;
	uint32_t objectID;


	fullfilename = g_strndup("", 8192);
	filename = g_strndup("", 8192);
	// First of all, lets set the download path.
	// TODO:    Set download path.

	//g_printf("Download path = %s\n", Preferences.fileSystemDownloadPath->str);
	// convert the referenece to a path and retrieve the iterator;
	path = gtk_tree_row_reference_get_path(Row);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
	// We have our Iter now.
	// Before we download, is it a folder ?
	gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_ISFOLDER, &isFolder, COL_FILENAME, &filename, COL_FILEID, &objectID, -1);
	if(isFolder == FALSE) {
		// Our strings are not equal, so we get to download the file.
		//g_printf("filename = %s, objectID = %d\n", filename, objectID);
        g_sprintf(fullfilename, "%s/%s", Preferences.fileSystemDownloadPath->str, filename);
		// Now download the actual file from the MTP device.
        // Check if file exists?
        if(access(fullfilename, F_OK) != -1){
            // We have that file already?
            if((Preferences.prompt_overwrite_file_op == TRUE)){
                if(fileoverwriteop == MTP_ASK){
                    fileoverwriteop = displayFileOverwriteDialog( filename);
                }
                switch(fileoverwriteop){
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
	}
	g_free(filename);
    g_free(fullfilename);
}

gboolean fileListRemove(GList *List){
	// Clear any selection that is present.
	fileListClearSelection();
	// List is a list of Iter's to be removed
	g_list_foreach(List, (GFunc)__fileRemove, NULL);
	// We have 2 options, manually scan the file structure for that file and manually fix up...
	// or do a rescan...
	// I'll be cheap, and do a full rescan of the device.
	deviceRescan();
	return TRUE;
}

void __fileRemove(GtkTreeRowReference *Row){
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar* filename;
	uint32_t objectID;
	gboolean isFolder;
	filename = g_strndup("", 8192);
	// convert the referenece to a path and retrieve the iterator;
	path = gtk_tree_row_reference_get_path(Row);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
	// We have our Iter now.
	gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_ISFOLDER, &isFolder, COL_FILENAME, &filename, COL_FILEID, &objectID, -1);
	if(isFolder == FALSE) {
		//g_printf("filename = %s, objectID = %d\n", filename, objectID);
		gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
		gtk_list_store_remove(GTK_LIST_STORE(fileList), &iter);
		// Now get rid of the actual file from the MTP device.
		filesDelete(filename, objectID);
	} else {
		//g_printf("I don't know how to delete folders\n");
        __folderRemove(Row);
	}
	g_free(filename);
}

gboolean folderListRemove(GList *List){
	// Clear any selection that is present.
	fileListClearSelection();
	// List is a list of Iter's to be removed
	g_list_foreach(List, (GFunc)__folderRemove, NULL);
	// We have 2 options, manually scan the file structure for that file and manually fix up...
	// or do a rescan...
	// I'll be cheap, and do a full rescan of the device.
	deviceRescan();
	return TRUE;
}

void __folderRemove(GtkTreeRowReference *Row){
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar* filename;
	uint32_t objectID;
	gboolean isFolder;
	filename = g_strndup("", 8192);
	// convert the referenece to a path and retrieve the iterator;
	path = gtk_tree_row_reference_get_path(Row);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
	// We have our Iter now.
	gtk_tree_model_get(GTK_TREE_MODEL(fileList), &iter, COL_ISFOLDER, &isFolder, COL_FILENAME, &filename, COL_FILEID, &objectID, -1);
	if(isFolder == TRUE) {
		//g_printf("folder = %s, objectID = %d\n", filename, objectID);
		if(g_ascii_strcasecmp(filename, "< .. >") != 0) {
			gtk_tree_model_get_iter(GTK_TREE_MODEL(fileList), &iter, path);
			gtk_list_store_remove(GTK_LIST_STORE(fileList), &iter);
			// Now get rid of the actual file from the MTP device.
            //g_print("__folderRemove: Here\n");
			folderDelete(getCurrentFolderPtr(deviceFolders, objectID), 0);
		} else {
			g_printf("I don't know how to delete a parent folder reference?\n");
		}
	} else {
		//g_printf("I don't know how to delete files\n");
        __fileRemove(Row);
	}
	g_free(filename);
}

void __filesAdd(gchar* filename){
    gchar* filename_stripped;

    filename_stripped = basename(filename);
    if(Preferences.prompt_overwrite_file_op == FALSE){
        filesAdd(filename);
        return;
    }
    // I guess we want to know if we should replace the file, but first
    if(deviceoverwriteop == MTP_ASK){
        if(fileExists(filename_stripped) == TRUE){
            deviceoverwriteop = displayFileOverwriteDialog( filename_stripped );
            switch(deviceoverwriteop){
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
        if(deviceoverwriteop == MTP_OVERWRITE_ALL)
            filesAdd(filename);
    }
}

// This will return a GList of the TREE ROW REFERENCES that are selected.
GList* fileListGetSelection(){
	GList *selectedFiles, *ptr;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	// Lets clear up the old list.
	//g_print("here8");
	g_list_free(fileSelection_RowReferences);
	fileSelection_RowReferences = NULL;

	//g_print("here9");
	if(gtk_tree_selection_count_selected_rows(fileSelection) == 0) {
		// We have no rows.
		return NULL;
	}
	//g_print("here10");
	// So now we must convert each selection to a row reference and store it in a new GList variable
	// which we will return below.
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeviewFiles));
	selectedFiles = gtk_tree_selection_get_selected_rows(fileSelection, &model);
	ptr = selectedFiles;
	//g_print("here12");
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(fileList), (GtkTreePath*) ptr->data);
		fileSelection_RowReferences = g_list_prepend(fileSelection_RowReferences, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}
	////g_print("here13");
	g_list_foreach (selectedFiles, (GFunc)gtk_tree_path_free, NULL);
	//g_print("here14");
	g_list_free (selectedFiles);
	//g_print("here15");
	return fileSelection_RowReferences;
}

gboolean fileListClearSelection(){
	if(fileSelection != NULL)
		gtk_tree_selection_unselect_all(fileSelection);
}

GtkWidget*
create_windowPreferences (void)
{
	GtkWidget *windowDialog;
	GtkWidget *vbox1;
	GtkWidget *frame1;
	GtkWidget *alignment1;

	GtkWidget *frame3;
	GtkWidget *alignment3;
    GtkWidget *alignment4;
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

	windowDialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gchar * winTitle;
	winTitle = g_strconcat(PACKAGE," v", PACKAGE_VERSION, " Preferences", NULL);
	gtk_window_set_title (GTK_WINDOW (windowDialog), winTitle);
	gtk_window_set_modal(GTK_WINDOW(windowDialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(windowDialog), GTK_WINDOW(windowMain));
	gtk_window_set_position (GTK_WINDOW (windowDialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable (GTK_WINDOW (windowDialog), FALSE);
	gtk_window_set_type_hint (GTK_WINDOW (windowDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	g_free(winTitle);

	vbox1 = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (windowDialog), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

	frame1 = gtk_frame_new (NULL);
	gtk_widget_show (frame1);
	gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);
	alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment1);
	gtk_container_add (GTK_CONTAINER (frame1), alignment1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 0, 12, 0);

	checkbuttonDeviceConnect = gtk_check_button_new_with_mnemonic (("Attempt to connect to Device on startup"));
	gtk_widget_show (checkbuttonDeviceConnect);
	gtk_container_add (GTK_CONTAINER (alignment1), checkbuttonDeviceConnect);

	labelDevice = gtk_label_new (("<b>Device</b>"));
	gtk_widget_show (labelDevice);
	gtk_frame_set_label_widget (GTK_FRAME (frame1), labelDevice);
	gtk_label_set_use_markup (GTK_LABEL (labelDevice), TRUE);

	frame3 = gtk_frame_new (NULL);
	gtk_widget_show (frame3);
	gtk_box_pack_start (GTK_BOX (vbox1), frame3, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_NONE);

    vbox2 = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (frame3), vbox2);

	alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment3);
	gtk_container_add (GTK_CONTAINER (vbox2), alignment3);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 0, 0, 12, 0);

    alignment4 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment4);
	gtk_container_add (GTK_CONTAINER (vbox2), alignment4);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment4), 0, 0, 12, 0);

	checkbuttonConfirmFileOp = gtk_check_button_new_with_mnemonic (("Confirm File/Folder Delete"));
	gtk_widget_show (checkbuttonConfirmFileOp);
	gtk_container_add (GTK_CONTAINER (alignment3), checkbuttonConfirmFileOp);

    checkbuttonConfirmOverWriteFileOp = gtk_check_button_new_with_mnemonic (("Prompt if to Overwrite file if already exists"));
	gtk_widget_show (checkbuttonConfirmOverWriteFileOp);
	gtk_container_add (GTK_CONTAINER (alignment4), checkbuttonConfirmOverWriteFileOp);

	labelDevice = gtk_label_new (("<b>File Operations</b>"));
	gtk_widget_show (labelDevice);
	gtk_frame_set_label_widget (GTK_FRAME (frame3), labelDevice);
	gtk_label_set_use_markup (GTK_LABEL (labelDevice), TRUE);

	frame2 = gtk_frame_new (NULL);
	gtk_widget_show (frame2);
	gtk_box_pack_start (GTK_BOX (vbox1), frame2, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);

	alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment2);
	gtk_container_add (GTK_CONTAINER (frame2), alignment2);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 0, 0, 12, 0);

	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox3);
	gtk_container_add (GTK_CONTAINER (alignment2), vbox3);

	checkbuttonDownloadPath = gtk_check_button_new_with_mnemonic (("Always show Download Path?"));
	gtk_widget_show (checkbuttonDownloadPath);
	gtk_box_pack_start (GTK_BOX (vbox3), checkbuttonDownloadPath, FALSE, FALSE, 0);

	table1 = gtk_table_new (2, 3, FALSE);
	gtk_widget_show (table1);
	gtk_box_pack_start (GTK_BOX (vbox3), table1, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
	gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
	gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

	labelDownloadPath = gtk_label_new (("Download Path:"));
	gtk_widget_show (labelDownloadPath);
	gtk_table_attach (GTK_TABLE (table1), labelDownloadPath, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelDownloadPath), 0, 0.5);

	labelUploadPath = gtk_label_new (("Upload Path:"));
	gtk_widget_show (labelUploadPath);
	gtk_table_attach (GTK_TABLE (table1), labelUploadPath, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelUploadPath), 0, 0.5);

	entryDownloadPath = gtk_entry_new ();
	gtk_widget_show (entryDownloadPath);
    gtk_editable_set_editable(GTK_EDITABLE(entryDownloadPath), FALSE);
	gtk_table_attach (GTK_TABLE (table1), entryDownloadPath, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	entryUploadPath = gtk_entry_new ();
	gtk_widget_show (entryUploadPath);
    gtk_editable_set_editable(GTK_EDITABLE(entryUploadPath), FALSE);
	gtk_table_attach (GTK_TABLE (table1), entryUploadPath, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	buttonDownloadPath = gtk_button_new_with_mnemonic (("..."));
	gtk_widget_show (buttonDownloadPath);
	gtk_table_attach (GTK_TABLE (table1), buttonDownloadPath, 2, 3, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	buttonUploadPath = gtk_button_new_with_mnemonic (("..."));
	gtk_widget_show (buttonUploadPath);
	gtk_table_attach (GTK_TABLE (table1), buttonUploadPath, 2, 3, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	labelFilePath = gtk_label_new (("<b>Filepaths on PC</b>"));
	gtk_widget_show (labelFilePath);
	gtk_frame_set_label_widget (GTK_FRAME (frame2), labelFilePath);
	gtk_label_set_use_markup (GTK_LABEL (labelFilePath), TRUE);

	// Now do the ask confirm delete...

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

	buttonClose = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (buttonClose);
	gtk_box_pack_end (GTK_BOX (hbox1), buttonClose, FALSE, FALSE, 0);

	g_signal_connect ((gpointer) windowDialog, "destroy",
					  G_CALLBACK (on_quitPrefs_activate),
					  NULL);

	g_signal_connect ((gpointer) buttonClose, "clicked",
					  G_CALLBACK (on_quitPrefs_activate),
					  NULL);

	g_signal_connect ((gpointer) checkbuttonDeviceConnect, "toggled",
					  G_CALLBACK (on_PrefsDevice_activate),
					  NULL);

	g_signal_connect ((gpointer) checkbuttonConfirmFileOp, "toggled",
					  G_CALLBACK (on_PrefsConfirmDelete_activate),
					  NULL);

    g_signal_connect ((gpointer) checkbuttonConfirmOverWriteFileOp, "toggled",
					  G_CALLBACK (on_PrefsConfirmOverWriteFileOp_activate),
					  NULL);

	g_signal_connect ((gpointer) checkbuttonDownloadPath, "toggled",
					  G_CALLBACK (on_PrefsAskDownload_activate),
					  NULL);

	g_signal_connect ((gpointer) buttonDownloadPath, "clicked",
					  G_CALLBACK (on_PrefsDownloadPath_activate),
					  NULL);

	g_signal_connect ((gpointer) buttonUploadPath, "clicked",
					  G_CALLBACK (on_PrefsUploadPath_activate),
					  NULL);

	// And now set the fields.

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonDeviceConnect), Preferences.attemptDeviceConnectOnStart);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonDownloadPath), Preferences.ask_download_path);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmFileOp), Preferences.confirm_file_delete_op);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonConfirmOverWriteFileOp), Preferences.prompt_overwrite_file_op);
	gtk_entry_set_text(GTK_ENTRY(entryDownloadPath), Preferences.fileSystemDownloadPath->str);
	gtk_entry_set_text(GTK_ENTRY(entryUploadPath), Preferences.fileSystemUploadPath->str);

	// To save the fields, we use callbacks on the widgets via gconf.

	return windowDialog;
}

GtkWidget* create_windowProperties(){
	GtkWidget *windowDialog;
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *frame2;
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
	GtkWidget *frame1;
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
	GtkWidget *button1;

	GtkWidget *label50;

	GString *tmp_string2;
	gchar tmp_string[8192];

	windowDialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gchar * winTitle;
	winTitle = g_strconcat(DeviceMgr.devicename->str, " Properties", NULL);
	gtk_window_set_title (GTK_WINDOW (windowDialog), winTitle);
	gtk_window_set_modal(GTK_WINDOW(windowDialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(windowDialog), GTK_WINDOW(windowMain));
	gtk_window_set_position (GTK_WINDOW (windowDialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable (GTK_WINDOW (windowDialog), FALSE);
	gtk_window_set_type_hint (GTK_WINDOW (windowDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	g_free(winTitle);

	vbox1 = gtk_vbox_new (FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), 5);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (windowDialog), vbox1);

	frame2 = gtk_frame_new (NULL);
	gtk_widget_show (frame2);
	gtk_box_pack_start (GTK_BOX (vbox1), frame2, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);


	alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment2);
	gtk_container_add (GTK_CONTAINER (frame2), alignment2);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 0, 0, 12, 0);

	table2 = gtk_table_new (10, 2, FALSE);
	gtk_widget_show (table2);
	gtk_container_add (GTK_CONTAINER (alignment2), table2);
	gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
	gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
	gtk_table_set_col_spacings (GTK_TABLE (table2), 5);

	label15 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label15), "<b>Name:</b>");
	gtk_widget_show (label15);
	gtk_table_attach (GTK_TABLE (table2), label15, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label15), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label15), 0, 1);

	labelName = gtk_label_new (("label20"));
	gtk_widget_show (labelName);
	gtk_table_attach (GTK_TABLE (table2), labelName, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelName), 0, 0.5);

	labelModel = gtk_label_new (("label21"));
	gtk_widget_show (labelModel);
	gtk_table_attach (GTK_TABLE (table2), labelModel, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelModel), 0, 0.5);

	labelSerial = gtk_label_new (("label22"));
	gtk_widget_show (labelSerial);
	gtk_table_attach (GTK_TABLE (table2), labelSerial, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelSerial), 0, 0.5);

	labelBattery = gtk_label_new (("label24"));
	gtk_widget_show (labelBattery);
	gtk_table_attach (GTK_TABLE (table2), labelBattery, 1, 2, 5, 6,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelBattery), 0, 0.5);

	labelManufacturer = gtk_label_new (("label23"));
	gtk_widget_show (labelManufacturer);
	gtk_table_attach (GTK_TABLE (table2), labelManufacturer, 1, 2, 4, 5,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelManufacturer), 0, 0.5);

	labelDeviceVer = gtk_label_new (("label26"));
	gtk_widget_show (labelDeviceVer);
	gtk_table_attach (GTK_TABLE (table2), labelDeviceVer, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelDeviceVer), 0, 0.5);

	label26 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label26), "<b>Model Number:</b>");
	gtk_widget_show (label26);
	gtk_table_attach (GTK_TABLE (table2), label26, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label26), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label26), 0, 1);

	label29 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label29), "<b>Serial Number:</b>");
	gtk_widget_show (label29);
	gtk_table_attach (GTK_TABLE (table2), label29, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label29), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label29), 0, 1);

	label28 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label28), "<b>Device Version:</b>");
	gtk_widget_show (label28);
	gtk_table_attach (GTK_TABLE (table2), label28, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label28), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label28), 0, 1);

	label27 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label27), "<b>Manufacturer:</b>");
	gtk_widget_show (label27);
	gtk_table_attach (GTK_TABLE (table2), label27, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label27), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label27), 0, 1);

	label17 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label17), "<b>Battery Level:</b>");
	gtk_widget_show (label17);
	gtk_table_attach (GTK_TABLE (table2), label17, 0, 1, 5, 6,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label17), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label17), 0, 1);

	label25 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label25), "<b>Storage:</b>");
	gtk_widget_show (label25);
	gtk_table_attach (GTK_TABLE (table2), label25, 0, 1, 6, 7,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label25), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label25), 0, 1);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	//gtk_container_add (GTK_CONTAINER (windowDialog), vbox1);
	gtk_table_attach (GTK_TABLE (table2), vbox2, 0, 1, 7, 8,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	label18 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label18), "<b>Supported Formats:</b>");
	gtk_box_pack_start (GTK_BOX (vbox2), label18, FALSE, TRUE, 0);
	gtk_widget_show (label18);
	gtk_label_set_justify (GTK_LABEL (label18), GTK_JUSTIFY_RIGHT);
	//gtk_misc_set_alignment (GTK_MISC (label18), 1, 0);
	gtk_label_set_line_wrap (GTK_LABEL (label18), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label18), 0, 1);


	label50 = gtk_label_new ((""));
	gtk_box_pack_start (GTK_BOX (vbox2), label50, FALSE, TRUE, 0);
	gtk_widget_show (label50);


	label19 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label19), "<b>Secure Time:</b>");
	gtk_widget_show (label19);
	gtk_table_attach (GTK_TABLE (table2), label19, 0, 1, 8, 9,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label19), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label19), 0, 1);

	label16 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label16), "<b>Sync Partner:</b>");
	gtk_widget_show (label16);
	gtk_table_attach (GTK_TABLE (table2), label16, 0, 1, 9, 10,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label16), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label16), 0, 1);

	labelStorage = gtk_label_new (("label30"));
	gtk_widget_show (labelStorage);
	gtk_table_attach (GTK_TABLE (table2), labelStorage, 1, 2, 6, 7,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelStorage), 0, 0.5);

	labelSupportedFormat = gtk_label_new (("label31"));
	gtk_widget_show (labelSupportedFormat);
	gtk_table_attach (GTK_TABLE (table2), labelSupportedFormat, 1, 2, 7, 8,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_line_wrap (GTK_LABEL (labelSupportedFormat), TRUE);
	gtk_misc_set_alignment (GTK_MISC (labelSupportedFormat), 0, 0.5);

	labelSecTime = gtk_label_new (("label32"));
	gtk_widget_show (labelSecTime);
	gtk_table_attach (GTK_TABLE (table2), labelSecTime, 1, 2, 8, 9,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_line_wrap (GTK_LABEL (labelSecTime), TRUE);
	gtk_misc_set_alignment (GTK_MISC (labelSecTime), 0, 0.5);

	labelSyncPartner = gtk_label_new (("label33"));
	gtk_widget_show (labelSyncPartner);
	gtk_table_attach (GTK_TABLE (table2), labelSyncPartner, 1, 2, 9, 10,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelSyncPartner), 0, 0.5);

	label2 = gtk_label_new (("<b>MTP Device Properties</b>"));
	gtk_widget_show (label2);
	gtk_frame_set_label_widget (GTK_FRAME (frame2), label2);
	gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);

	frame1 = gtk_frame_new (NULL);
	gtk_widget_show (frame1);
	gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);

	alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment1);
	gtk_container_add (GTK_CONTAINER (frame1), alignment1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 0, 12, 0);

	table1 = gtk_table_new (6, 2, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (alignment1), table1);
	gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
	gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
	gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

	label3 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label3), "<b>Vendor:</b>");
	gtk_widget_show (label3);
	gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label3), 0, 1);

	label4 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label4), "<b>Product:</b>");
	gtk_widget_show (label4);
	gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label4), 0, 1);

	label5 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label5), "<b>Vendor ID:</b>");
	gtk_widget_show (label5);
	gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label5), 0, 1);

	label6 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label6), "<b>Product ID:</b>");
	gtk_widget_show (label6);
	gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label6), 0, 1);

	label7 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label7), "<b>Device Number:</b>");
	gtk_widget_show (label7);
	gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 5, 6,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label7), 0, 1);

	label8 = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label8), "<b>Bus Location:</b>");
	gtk_widget_show (label8);
	gtk_table_attach (GTK_TABLE (table1), label8, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (label8), 0, 1);

	labelDeviceVendor = gtk_label_new (("label9"));
	gtk_widget_show (labelDeviceVendor);
	gtk_table_attach (GTK_TABLE (table1), labelDeviceVendor, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelDeviceVendor), 0, 0.5);

	labelDeviceProduct = gtk_label_new (("label10"));
	gtk_widget_show (labelDeviceProduct);
	gtk_table_attach (GTK_TABLE (table1), labelDeviceProduct, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelDeviceProduct), 0, 0.5);

	labelVenID = gtk_label_new (("label11"));
	gtk_widget_show (labelVenID);
	gtk_table_attach (GTK_TABLE (table1), labelVenID, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelVenID), 0, 0.5);

	labelProdID = gtk_label_new (("label12"));
	gtk_widget_show (labelProdID);
	gtk_table_attach (GTK_TABLE (table1), labelProdID, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelProdID), 0, 0.5);

	labelBusLoc = gtk_label_new (("label13"));
	gtk_widget_show (labelBusLoc);
	gtk_table_attach (GTK_TABLE (table1), labelBusLoc, 1, 2, 4, 5,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelBusLoc), 0, 0.5);

	labelDevNum = gtk_label_new (("label14"));
	gtk_widget_show (labelDevNum);
	gtk_table_attach (GTK_TABLE (table1), labelDevNum, 1, 2, 5, 6,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelDevNum), 0, 0.5);

	label1 = gtk_label_new (("<b>Raw Device Information</b>"));
	gtk_widget_show (label1);
	gtk_frame_set_label_widget (GTK_FRAME (frame1), label1);
	gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

	button1 = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (button1);
	gtk_box_pack_end (GTK_BOX (hbox2), button1, FALSE, FALSE, 0);

	g_signal_connect ((gpointer) windowDialog, "destroy",
					  G_CALLBACK (on_quitProp_activate),
					  NULL);

	g_signal_connect ((gpointer) button1, "clicked",
					  G_CALLBACK (on_quitProp_activate),
					  NULL);

	// Now we need to update our strings for the information on the unit.
	gtk_label_set_text(GTK_LABEL(labelName), DeviceMgr.devicename->str);
	gtk_label_set_text(GTK_LABEL(labelModel), DeviceMgr.modelname->str);
	gtk_label_set_text(GTK_LABEL(labelSerial), DeviceMgr.serialnumber->str);
    if (DeviceMgr.maxbattlevel != 0){
        g_sprintf(tmp_string, "%d / %d (%d%%)", DeviceMgr.currbattlevel, DeviceMgr.maxbattlevel, (int) (((float) DeviceMgr.currbattlevel / (float) DeviceMgr.maxbattlevel) * 100.0));
    } else {
        g_sprintf(tmp_string, "%d / %d", DeviceMgr.currbattlevel, DeviceMgr.maxbattlevel);
    }
    gtk_label_set_text(GTK_LABEL(labelBattery), (gchar *)&tmp_string);
	gtk_label_set_text(GTK_LABEL(labelManufacturer), DeviceMgr.manufacturername->str);
	gtk_label_set_text(GTK_LABEL(labelDeviceVer), DeviceMgr.deviceversion->str);


    if(DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE){
        g_sprintf(tmp_string, "%d MB (free) / %d MB (total)", (int)( DeviceMgr.devicestorage->FreeSpaceInBytes  / 1048576 ), (int)( DeviceMgr.devicestorage->MaxCapacity / 1048576 ));
        gtk_label_set_text(GTK_LABEL(labelStorage), (gchar *)&tmp_string);
    } else {
        tmp_string2 = g_string_new("");
        // Cycle through each storage device and list the name and capacity.
        LIBMTP_devicestorage_t* deviceStorage = DeviceMgr.device->storage;
        while (deviceStorage != NULL) {
            if(tmp_string2->len > 0)
                tmp_string2 = g_string_append(tmp_string2, "\n");
            if (deviceStorage->StorageDescription != NULL){
                tmp_string2 = g_string_append(tmp_string2, deviceStorage->StorageDescription);
            } else {
                tmp_string2 = g_string_append(tmp_string2, deviceStorage->VolumeIdentifier);
            }
            g_sprintf(tmp_string, " : %d MB (free) / %d MB (total)", (int)( deviceStorage->FreeSpaceInBytes  / 1048576 ), (int)( deviceStorage->MaxCapacity / 1048576 ));
            tmp_string2 = g_string_append(tmp_string2, (gchar *)&tmp_string);
            deviceStorage = deviceStorage->next;
        }
        gtk_label_set_text(GTK_LABEL(labelStorage), tmp_string2->str);
        g_free(tmp_string2);
    }

	tmp_string2 = g_string_new("");
	// Build a string for us to use.
    gint i = 0;
	for (i = 0; i < DeviceMgr.filetypes_len; i++) {
        if(tmp_string2->len > 0)
                tmp_string2 = g_string_append(tmp_string2, "\n");
		tmp_string2 = g_string_append(tmp_string2, LIBMTP_Get_Filetype_Description(DeviceMgr.filetypes[i]));
	}

	gtk_label_set_text(GTK_LABEL(labelSupportedFormat), tmp_string2->str);

	gtk_label_set_text(GTK_LABEL(labelSecTime), DeviceMgr.sectime->str);
	gtk_label_set_text(GTK_LABEL(labelSyncPartner), DeviceMgr.syncpartner->str);

	// This is our raw information.
	gtk_label_set_text(GTK_LABEL(labelDeviceVendor), DeviceMgr.Vendor->str);
	gtk_label_set_text(GTK_LABEL(labelDeviceProduct), DeviceMgr.Product->str);
	g_sprintf(tmp_string, "0x%x", DeviceMgr.VendorID);
	gtk_label_set_text(GTK_LABEL(labelVenID), (gchar *)&tmp_string);
	g_sprintf(tmp_string, "0x%x", DeviceMgr.ProductID);
	gtk_label_set_text(GTK_LABEL(labelProdID), (gchar *)&tmp_string);
	g_sprintf(tmp_string, "0x%x", DeviceMgr.BusLoc);
	gtk_label_set_text(GTK_LABEL(labelBusLoc), (gchar *)&tmp_string);
	g_sprintf(tmp_string, "0x%x", DeviceMgr.DeviceID);
	gtk_label_set_text(GTK_LABEL(labelDevNum), (gchar *)&tmp_string);

    // Having the g_free here causes a segfault???
    //g_free(tmp_string2);
	return windowDialog;
}


GtkWidget* create_windowProgressDialog (gchar* msg)
{
	GtkWidget *window1;
	GtkWidget *vbox1;
	GtkWidget *label_FileProgress;
	GtkWidget *label1;
	GtkWidget *progressbar_Main;

	window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gchar * winTitle;
	winTitle = g_strconcat(PACKAGE," v", PACKAGE_VERSION, NULL);
	gtk_window_set_title (GTK_WINDOW (window1), winTitle);
	gtk_window_set_position (GTK_WINDOW (window1), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_modal (GTK_WINDOW (window1), TRUE);
	gtk_window_set_resizable (GTK_WINDOW (window1), FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(window1), GTK_WINDOW(windowMain));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (window1), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (window1), GDK_WINDOW_TYPE_HINT_DIALOG);
	g_free(winTitle);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (window1), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 10);

	label1 = gtk_label_new (NULL);
	winTitle = g_strconcat("<b><big>", msg, "</big></b>", NULL);
	gtk_label_set_markup (GTK_LABEL(label1), winTitle);
	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (vbox1), label1, TRUE, TRUE, 0);
	gtk_misc_set_padding (GTK_MISC (label1), 0, 5);
	gtk_misc_set_alignment (GTK_MISC (label1), 0, 0);
	g_free(winTitle);

	label_FileProgress = gtk_label_new (("file = ( x / x ) x %"));
	gtk_widget_set_size_request(label_FileProgress, 320, -1);
	gtk_widget_show (label_FileProgress);
	gtk_box_pack_start (GTK_BOX (vbox1), label_FileProgress, TRUE, TRUE, 0);
	gtk_misc_set_padding (GTK_MISC (label_FileProgress), 0, 5);
	//gtk_misc_set_alignment (GTK_MISC (label_FileProgress), 0, 0);

	progressbar_Main = gtk_progress_bar_new ();
	gtk_widget_show (progressbar_Main);
	gtk_box_pack_start (GTK_BOX (vbox1), progressbar_Main, TRUE, TRUE, 0);

	progressDialog = window1;
	progressDialog_Text = label_FileProgress;
	progressDialog_Bar = progressbar_Main;
	return window1;
}

void displayProgressBar (gchar *msg){
	// No idea how this could come about, but we should take it into account so we don't have a memleak due to recreating the window multiple times.
	if(progressDialog != NULL) {
		destroyProgressBar();
	}
	progressDialog = create_windowProgressDialog (msg);
	gtk_widget_show_all (progressDialog);
}

void destroyProgressBar (void){
	gtk_widget_hide (progressDialog);
	gtk_widget_destroy (progressDialog);
	g_free(progressDialog_filename);
	progressDialog = NULL;
	progressDialog_Text = NULL;
	progressDialog_Bar = NULL;
}

void setProgressFilename(gchar* filename){
	progressDialog_filename = g_strdup(filename);
}

int fileprogress (const uint64_t sent, const uint64_t total, void const * const data)
{
	gchar tmp_string[8096];
	gint percent = (sent*100)/total;
	//g_printf("Progress: %llu of %llu (%d%%)\r", sent, total, percent);
	fflush(stdout);
	// Now update the progress dialog.
	if(progressDialog != NULL) {
		if(progressDialog_filename != NULL) {
			g_sprintf(tmp_string, "%s - %lluKB of %lluKB (%d%%)", progressDialog_filename, (sent / 1024), (total / 1024), percent);
		} else {
			g_sprintf(tmp_string, "%lluKB of %lluKB (%d%%)",  (sent / 1024), (total / 1024), percent);
		}
		gtk_label_set_text(GTK_LABEL(progressDialog_Text), (gchar *)&tmp_string);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressDialog_Bar), (double) percent / 100.00 );
		while (gtk_events_pending ())
			gtk_main_iteration ();
	}
	return 0;
}

void displayAbout(void){
	GtkWidget *dialog, *vbox, *label, *label2, *label3, *image;
    gchar *version_string;

	dialog = gtk_dialog_new_with_buttons ("About gMTP", GTK_WINDOW(windowMain),
									(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
									GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                    NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE );
    gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(vbox);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), vbox);

    // Add in our icon.
    image = gtk_image_new_from_file(file_icon_png->str);
    gtk_widget_show(image);
	gtk_container_add (GTK_CONTAINER (vbox), image);

    version_string = g_strconcat("<span size=\"xx-large\"><b>", PACKAGE," v", PACKAGE_VERSION, "</b></span>", NULL);

    label = gtk_label_new (version_string);
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_widget_show(label);
	gtk_container_add (GTK_CONTAINER (vbox), label);

    label2 = gtk_label_new ("A simple MP3 Player Client for Solaris 10\nand OpenSolaris\n");
    gtk_label_set_use_markup(GTK_LABEL(label2), TRUE);
    gtk_label_set_justify(GTK_LABEL(label2), GTK_JUSTIFY_CENTER);
    //gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(label2), 5, 0);
	gtk_widget_show(label2);
	gtk_container_add (GTK_CONTAINER (vbox), label2);

    label3 = gtk_label_new ("<small>Copyright 2009-2010, Darran Kartaschew\nReleased under the CDDL</small>\n");
    gtk_label_set_use_markup(GTK_LABEL(label3), TRUE);
	gtk_widget_show(label3);
	gtk_container_add (GTK_CONTAINER (vbox), label3);

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
    g_free(version_string);
}

void displayError(gchar* msg){
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW(windowMain),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_ERROR,
									 GTK_BUTTONS_OK,
									 msg);
	gtk_window_set_title(GTK_WINDOW (dialog), "Error");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

void displayInformation(gchar* msg){
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new (GTK_WINDOW(windowMain),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_INFO,
									 GTK_BUTTONS_OK,
									 msg);
	gtk_window_set_title(GTK_WINDOW (dialog), "Information");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}


gchar* displayFolderNewDialog(void){
	GtkWidget *dialog, *hbox, *label, *textbox;
	gchar* textfield;

	dialog = gtk_dialog_new_with_buttons("New Folder", GTK_WINDOW(windowMain),
										 (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										 GTK_STOCK_OK, GTK_RESPONSE_OK,
										 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										 NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK );

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);

	label = gtk_label_new ("Folder Name:");
	gtk_widget_show(label);
	gtk_container_add (GTK_CONTAINER (hbox), label);

	textbox = gtk_entry_new ();
	gtk_widget_show (textbox);
	gtk_entry_set_max_length (GTK_ENTRY (textbox), 64);
	gtk_entry_set_has_frame(GTK_ENTRY (textbox), TRUE);
	gtk_entry_set_activates_default(GTK_ENTRY (textbox), TRUE);
	gtk_container_add (GTK_CONTAINER (hbox), textbox);

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	if(result == GTK_RESPONSE_OK) {
		textfield = g_strdup(gtk_entry_get_text(GTK_ENTRY (textbox)));
		if(strlen(textfield) == 0) {
			// We have an emtpy string.
			gtk_widget_destroy (dialog);
			return NULL;
		} else {
			gtk_widget_destroy (dialog);
			return textfield;
		}
	} else {
		gtk_widget_destroy (dialog);
		return NULL;
	}
}

gchar* displayChangeDeviceNameDialog(gchar* devicename){
	GtkWidget *dialog, *hbox, *label, *textbox;
	gchar* textfield;

	dialog = gtk_dialog_new_with_buttons("Change Device Name", GTK_WINDOW(windowMain),
										 (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										 GTK_STOCK_OK, GTK_RESPONSE_OK,
										 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										 NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK );

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);

	label = gtk_label_new ("Device Name:");
	gtk_widget_show(label);
	gtk_container_add (GTK_CONTAINER (hbox), label);

	textbox = gtk_entry_new ();
	gtk_widget_show (textbox);
    if(devicename != NULL){
        gtk_entry_set_text(GTK_ENTRY (textbox), devicename);
    }
	gtk_entry_set_max_length (GTK_ENTRY (textbox), 64);
	gtk_entry_set_has_frame(GTK_ENTRY (textbox), TRUE);
	gtk_entry_set_activates_default(GTK_ENTRY (textbox), TRUE);
	gtk_container_add (GTK_CONTAINER (hbox), textbox);

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	if(result == GTK_RESPONSE_OK) {
		textfield = g_strdup(gtk_entry_get_text(GTK_ENTRY (textbox)));
		if(strlen(textfield) == 0) {
			// We have an emtpy string.
			gtk_widget_destroy (dialog);
			return NULL;
		} else {
			gtk_widget_destroy (dialog);
			return textfield;
		}
	} else {
		gtk_widget_destroy (dialog);
		return NULL;
	}
}

gint displayMultiDeviceDialog(void){
    GtkWidget *dialog, *hbox, *label, *textbox;
    gchar tmp_string[256];
    gint dialog_selection  = 0;

	dialog = gtk_dialog_new_with_buttons("Connect to which device?", GTK_WINDOW(windowMain),
										 (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										 GTK_STOCK_OK, GTK_RESPONSE_OK,
										 NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK );

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);

    label = gtk_label_new ("Device:");
	gtk_widget_show(label);
	gtk_container_add (GTK_CONTAINER (hbox), label);

    // Now create the combo box.
    textbox = gtk_combo_box_new_text();
    gtk_widget_show(textbox);
	gtk_container_add (GTK_CONTAINER (hbox), textbox);
    // Now add in our selection strings.
    // We should just take straight strings here, but this is quicker/easier.
    gint i = 0;
    for (i = 0; i < DeviceMgr.numrawdevices; i++) {
        if (DeviceMgr.rawdevices[i].device_entry.vendor != NULL ||
            DeviceMgr.rawdevices[i].device_entry.product != NULL) {
              g_sprintf(tmp_string, "   %s %s : (%04x:%04x) @ bus %d, dev %d",
                  DeviceMgr.rawdevices[i].device_entry.vendor,
                  DeviceMgr.rawdevices[i].device_entry.product,
                  DeviceMgr.rawdevices[i].device_entry.vendor_id,
                  DeviceMgr.rawdevices[i].device_entry.product_id,
                  DeviceMgr.rawdevices[i].bus_location,
                  DeviceMgr.rawdevices[i].devnum);
        } else {
              g_sprintf(tmp_string, "Unknown : %04x:%04x @ bus %d, dev %d",
                  DeviceMgr.rawdevices[i].device_entry.vendor_id,
                  DeviceMgr.rawdevices[i].device_entry.product_id,
                  DeviceMgr.rawdevices[i].bus_location,
                  DeviceMgr.rawdevices[i].devnum);
        }
        gtk_combo_box_append_text(GTK_COMBO_BOX(textbox), tmp_string);
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(textbox), 0);

    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	if(result == GTK_RESPONSE_OK) {
        dialog_selection = gtk_combo_box_get_active(GTK_COMBO_BOX(textbox));
    }
    gtk_widget_destroy (dialog);
    return dialog_selection;
}

gint displayDeviceStorageDialog(void){
    GtkWidget *dialog, *hbox, *label, *textbox;
    LIBMTP_devicestorage_t *devicestorage;
    gchar tmp_string[256];
    gint dialog_selection  = 0;

    devicestorage = DeviceMgr.device->storage;
    
	dialog = gtk_dialog_new_with_buttons("Connect to which storage device?", GTK_WINDOW(windowMain),
										 (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										 GTK_STOCK_OK, GTK_RESPONSE_OK,
										 NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK );

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);

    label = gtk_label_new ("Storage Device:");
	gtk_widget_show(label);
	gtk_container_add (GTK_CONTAINER (hbox), label);

    // Now create the combo box.
    textbox = gtk_combo_box_new_text();
    gtk_widget_show(textbox);
	gtk_container_add (GTK_CONTAINER (hbox), textbox);
    // Now add in our selection strings.
    while (devicestorage != NULL) {
        if(devicestorage->StorageDescription != NULL) {
            gtk_combo_box_append_text(GTK_COMBO_BOX(textbox), devicestorage->StorageDescription);
        } else {
            g_sprintf(tmp_string, "Unknown id: %d, %d MB", devicestorage->id, (devicestorage->MaxCapacity / (1024 * 1024)));
            gtk_combo_box_append_text(GTK_COMBO_BOX(textbox), tmp_string);
        }
        devicestorage = devicestorage->next;
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(textbox), 0);

    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	if(result == GTK_RESPONSE_OK) {
        dialog_selection = gtk_combo_box_get_active(GTK_COMBO_BOX(textbox));
    }
    gtk_widget_destroy (dialog);
    return dialog_selection;

}

gint displayFileOverwriteDialog(gchar *filename){
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(windowMain),
                                    (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                    GTK_MESSAGE_WARNING,
                                    GTK_BUTTONS_NONE,
                                    "File <b>%s</b> already exists in target folder.\nDo you want to:", filename);
    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                                "Skip", MTP_SKIP,
                                "Skip All", MTP_SKIP_ALL,
                                "Overwrite", MTP_OVERWRITE,
                                "Overwrite All", MTP_OVERWRITE_ALL,
                                NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), "Question: Confirm Overwrite of Existing File?");
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), MTP_OVERWRITE );
    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    return result;
}

Album_Struct* displayAddAlbumArtDialog(void){
    Album_Struct* albumdetails;
    GtkWidget *hbox, *hbox2, *label, *label2, *textbox,  *buttonFilePath;
    LIBMTP_album_t * albuminfo, *albumtmp;

    albumdetails = g_malloc(sizeof(Album_Struct));
    albumdetails->album_id = 0;
    albumdetails->filename = NULL;

    AlbumArtDialog = gtk_dialog_new_with_buttons("Add Album Art", GTK_WINDOW(windowMain),
										 (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										 GTK_STOCK_OK, GTK_RESPONSE_OK,
										 NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(AlbumArtDialog), GTK_RESPONSE_OK );

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(AlbumArtDialog)->vbox), hbox);

    label = gtk_label_new ("Album:");
	gtk_widget_show(label);
	//gtk_container_add (GTK_CONTAINER (hbox), label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    // Now create the combo box.
    textbox = gtk_combo_box_new_text();
    gtk_widget_show(textbox);
	//gtk_container_add (GTK_CONTAINER (hbox), textbox);
    gtk_box_pack_start(GTK_BOX(hbox), textbox, TRUE, TRUE, 0);
    // Now add in our selection strings.
    albuminfo = LIBMTP_Get_Album_List(DeviceMgr.device);
    // Better check to see if we actually have anything?
    if(albuminfo == NULL){
        // we have no albums.
        displayInformation("No Albums available to set Album Art with. Either:\n1. You have no music files uploaded?\n2. Your device does not support Albums?\n3. Previous applications used to upload files do not autocreate albums for you or support the metadata for those files in order to create the albums for you?\n");
        gtk_widget_destroy (AlbumArtDialog);
        g_free(albumdetails);
        return NULL;
    }

    albumtmp = albuminfo;
    while(albuminfo != NULL) {
        if(albuminfo->name != NULL)
            gtk_combo_box_append_text(GTK_COMBO_BOX(textbox), albuminfo->name);
        albuminfo = albuminfo->next;
    }
    // End add selection.
    gtk_combo_box_set_active(GTK_COMBO_BOX(textbox), 0);

    hbox2 = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox2);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(AlbumArtDialog)->vbox), hbox2);

    label2 = gtk_label_new ("Filename:");
	gtk_widget_show(label2);
	gtk_container_add (GTK_CONTAINER (hbox2), label2);

    AlbumArtFilename = gtk_entry_new ();
	gtk_widget_show (AlbumArtFilename);
	gtk_entry_set_max_length (GTK_ENTRY (AlbumArtFilename), 8096);
	gtk_entry_set_has_frame(GTK_ENTRY (AlbumArtFilename), TRUE);
	gtk_entry_set_activates_default(GTK_ENTRY (AlbumArtFilename), FALSE);
	gtk_container_add (GTK_CONTAINER (hbox2), AlbumArtFilename);

    buttonFilePath = gtk_button_new_with_mnemonic (("..."));
	gtk_widget_show (buttonFilePath);
    gtk_container_add (GTK_CONTAINER (hbox2), buttonFilePath);

    g_signal_connect ((gpointer) buttonFilePath, "clicked",
					  G_CALLBACK (on_buttonFilePath_activate),
					  NULL);

    gint result = gtk_dialog_run (GTK_DIALOG (AlbumArtDialog));
	if(result == GTK_RESPONSE_OK) {
        // Now some sanity checks;
        if (g_ascii_strcasecmp(gtk_entry_get_text(GTK_ENTRY (AlbumArtFilename)), "") == 0){
            g_free(albumdetails);
            albumdetails = NULL;
        } else {
            // Filename is blank, now check the selection ID.
            if(gtk_combo_box_get_active(GTK_COMBO_BOX(textbox)) != -1){
                gint albumid =  gtk_combo_box_get_active(GTK_COMBO_BOX(textbox));
                gint i = 0;
                for(i = 0; i < albumid; i++){
                    if(albumtmp->name == NULL)
                        i++;
                    albumtmp = albumtmp->next;
                }
                albumdetails->album_id = albumtmp->album_id;
                albumdetails->filename = g_strdup(gtk_entry_get_text(GTK_ENTRY (AlbumArtFilename)));
            }
        }
    } else {
        // result != GTK_RESPONSE_OK
        g_free(albumdetails);
        albumdetails = NULL;
    }
    gtk_widget_destroy (AlbumArtDialog);
    LIBMTP_destroy_album_t(albumtmp);
    return albumdetails;
}

GtkWidget* create_windowMainContextMenu(void){
    GtkWidget* menu;
    GtkWidget* cfileAdd;
    GtkWidget* cfileRemove;
    GtkWidget* cfileDownload;
    GtkWidget* cfileNewFolder;
    GtkWidget* cfileRemoveFolder;
    GtkWidget* cfileRescan;
    GtkWidget* menuseparator1;
    GtkWidget* menuseparator2;

    menu = gtk_menu_new();

    cfileAdd = gtk_menu_item_new_with_label("Add Files");
	gtk_widget_show (cfileAdd);
	gtk_container_add (GTK_CONTAINER (menu), cfileAdd);

    cfileRemove = gtk_menu_item_new_with_label("Delete Files");
	gtk_widget_show (cfileRemove);
	gtk_container_add (GTK_CONTAINER (menu), cfileRemove);

	cfileDownload = gtk_menu_item_new_with_label("Download Files");
	gtk_widget_show (cfileDownload);
	gtk_container_add (GTK_CONTAINER (menu), cfileDownload);

    menuseparator1 = gtk_separator_menu_item_new();
	gtk_widget_show (menuseparator1);
	gtk_container_add (GTK_CONTAINER (menu), menuseparator1);

	cfileNewFolder =  gtk_menu_item_new_with_label("Create Folder");
	gtk_widget_show (cfileNewFolder);
	gtk_container_add (GTK_CONTAINER (menu), cfileNewFolder);

	cfileRemoveFolder = gtk_menu_item_new_with_label("Delete Folder");
	gtk_widget_show (cfileRemoveFolder);
	gtk_container_add (GTK_CONTAINER (menu), cfileRemoveFolder);

	menuseparator2 = gtk_separator_menu_item_new();
	gtk_widget_show (menuseparator2);
	gtk_container_add (GTK_CONTAINER (menu), menuseparator2);

	cfileRescan = gtk_menu_item_new_with_label("Refresh Device");
	gtk_widget_show (cfileRescan);
	gtk_container_add (GTK_CONTAINER (menu), cfileRescan);

    // Now our call backs.
    g_signal_connect ((gpointer) cfileAdd, "activate",
					  G_CALLBACK (on_filesAdd_activate),
					  NULL);

	g_signal_connect ((gpointer) cfileDownload, "activate",
					  G_CALLBACK (on_filesDownload_activate),
					  NULL);

	g_signal_connect ((gpointer) cfileRemove, "activate",
					  G_CALLBACK (on_filesDelete_activate),
					  NULL);

    g_signal_connect ((gpointer) cfileNewFolder, "activate",
					  G_CALLBACK (on_fileNewFolder_activate),
					  NULL);

	g_signal_connect ((gpointer) cfileRemoveFolder, "activate",
					  G_CALLBACK (on_fileRemoveFolder_activate),
					  NULL);

	g_signal_connect ((gpointer) cfileRescan, "activate",
					  G_CALLBACK (on_deviceRescan_activate),
					  NULL);

    return menu;

}

