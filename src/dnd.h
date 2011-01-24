/* 
*
*   File: dnd.h
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

#ifndef _DND_H
#define	_DND_H

#ifdef	__cplusplus
extern "C" {
#endif

/* Designate dropped data types that we know and care about */
enum {
	GMTP_DROP_STRING,
	GMTP_DROP_PLAINTEXT,
	GMTP_DROP_URLENCODED
};

/* Drag data format listing for gtk_drag_dest_set() */
const GtkTargetEntry _gmtp_drop_types[3];

#define gmtp_drag_dest_set(widget) gtk_drag_dest_set(widget, \
		GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP, \
		_gmtp_drop_types, 3, GDK_ACTION_COPY | GDK_ACTION_MOVE)

void gmtp_drag_data_received(GtkWidget * widget,
				       GdkDragContext * context,
				       gint x,
				       gint y,
				       GtkSelectionData * selection_data,
				       guint info,
				       guint time,
				       gpointer user_data);

GSList* getFilesListURI(gchar* rawdata);
void addFilesinFolder(gchar* foldername);

#ifdef	__cplusplus
}
#endif

#endif	/* _DND_H */

