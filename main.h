/*
 * File:   main.h
 * Author: darran
 *
 * Created on October 29, 2009, 5:01 PM
 */

#ifndef _MAIN_H
#define _MAIN_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <libmtp.h>
    
typedef struct  {
	gboolean deviceConnected;
	gint numrawdevices;
    gint rawdeviceID;
    gint storagedeviceID;

	LIBMTP_raw_device_t * rawdevices;
	LIBMTP_mtpdevice_t *device;
	LIBMTP_devicestorage_t *devicestorage;
	LIBMTP_error_number_t err;

	GString *devicename;
	GString *manufacturername;
	GString *modelname;
	GString *serialnumber;
	GString *deviceversion;
	GString *syncpartner;
	GString *sectime;
	GString *devcert;

	// Raw device
	GString *Vendor;
	GString *Product;
	uint32_t VendorID;
	uint32_t ProductID;
	uint32_t DeviceID;
	uint32_t BusLoc;

	uint16_t *filetypes;
	uint16_t filetypes_len;
	uint8_t maxbattlevel;
	uint8_t currbattlevel;

} Device_Struct;

GtkWidget *windowMain;
GtkWidget *windowPrefsDialog;
GtkWidget *windowPropDialog;
GtkWidget *windowStatusBar;
GtkWidget *toolbuttonConnect;
GtkWidget *treeviewFiles;

Device_Struct DeviceMgr;

LIBMTP_file_t   *deviceFiles;
LIBMTP_folder_t *deviceFolders;
uint32_t currentFolderID;         // This is the ID of the current folder....

GString *file_icon_png;
GString *file_icon16_png;
GString *file_about_png;

// Misc Utility function;
void setFilePaths(int argc, char *argv[]);
gchar *getRuntimePath(int argc, char *argv[]);


#ifdef  __cplusplus
}
#endif

#endif  /* _MAIN_H */
