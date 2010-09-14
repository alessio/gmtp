
#include "config.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <libmtp.h>
#include <libgen.h>
#include <sys/stat.h>
#include <strings.h>
#include <id3tag.h>
#include <stdio.h>

#include "main.h"
#include "callbacks.h"
#include "interface.h"
#include "mtp.h"
#include "prefs.h"
#include "dnd.h"


// Array with file extensions matched to internal libmtp file types;
// See find_filetype() for usage;
MTP_file_ext_struct file_ext[] = {
    {"wav", LIBMTP_FILETYPE_WAV },
    {"mp3", LIBMTP_FILETYPE_MP3 },
    {"wma", LIBMTP_FILETYPE_WMA },
    {"ogg", LIBMTP_FILETYPE_OGG },
    {"mp4", LIBMTP_FILETYPE_MP4 },
    {"wmv", LIBMTP_FILETYPE_WMV },
    {"avi", LIBMTP_FILETYPE_AVI },
    {"mpeg", LIBMTP_FILETYPE_MPEG },
    {"mpg", LIBMTP_FILETYPE_MPEG },
    {"asf", LIBMTP_FILETYPE_ASF },
    {"qt", LIBMTP_FILETYPE_QT },
    {"mov", LIBMTP_FILETYPE_QT },
    {"wma", LIBMTP_FILETYPE_WMA },
    {"jpg", LIBMTP_FILETYPE_JPEG },
    {"jpeg", LIBMTP_FILETYPE_JPEG },
    {"jfif", LIBMTP_FILETYPE_JFIF },
    {"tif", LIBMTP_FILETYPE_TIFF },
    {"tiff", LIBMTP_FILETYPE_TIFF },
    {"bmp", LIBMTP_FILETYPE_BMP },
    {"gif", LIBMTP_FILETYPE_GIF },
    {"pic", LIBMTP_FILETYPE_PICT },
    {"pict", LIBMTP_FILETYPE_PICT },
    {"png", LIBMTP_FILETYPE_PNG },
    {"wmf", LIBMTP_FILETYPE_WINDOWSIMAGEFORMAT },
    {"ics", LIBMTP_FILETYPE_VCALENDAR2 },
    {"exe", LIBMTP_FILETYPE_WINEXEC },
    {"com", LIBMTP_FILETYPE_WINEXEC },
    {"bat", LIBMTP_FILETYPE_WINEXEC },
    {"dll", LIBMTP_FILETYPE_WINEXEC },
    {"sys", LIBMTP_FILETYPE_WINEXEC },
    {"aac", LIBMTP_FILETYPE_AAC },
    {"mp2", LIBMTP_FILETYPE_MP2 },
    {"flac", LIBMTP_FILETYPE_FLAC },
    {"m4a", LIBMTP_FILETYPE_M4A },
    {"doc", LIBMTP_FILETYPE_DOC },
    {"xml", LIBMTP_FILETYPE_XML },
    {"xls", LIBMTP_FILETYPE_XLS },
    {"ppt", LIBMTP_FILETYPE_PPT },
    {"mht", LIBMTP_FILETYPE_MHT },
    {"jp2", LIBMTP_FILETYPE_JP2 },
    {"jpx", LIBMTP_FILETYPE_JPX },
    {"bin", LIBMTP_FILETYPE_FIRMWARE },
    {"vcf", LIBMTP_FILETYPE_VCARD3}
};

guint deviceConnect(){
	gint error;
	if(DeviceMgr.deviceConnected == TRUE) {
		// We must be wanting to disconnect the device.
		return deviceDisconnect();
	} else {
		error = LIBMTP_Detect_Raw_Devices(&DeviceMgr.rawdevices, &DeviceMgr.numrawdevices);
		switch(error) {
		case LIBMTP_ERROR_NO_DEVICE_ATTACHED:
			g_print("Detect: No raw devices found.\n");
			displayError("Detect: No raw devices found.\n");
			return MTP_GENERAL_FAILURE;
		case LIBMTP_ERROR_CONNECTING:
			g_print("Detect: There has been an error connecting. \n");
			displayError("Detect: There has been an error connecting. \n");
			return MTP_GENERAL_FAILURE;
		case LIBMTP_ERROR_MEMORY_ALLOCATION:
			g_print("Detect: Encountered a Memory Allocation Error. \n");
			displayError("Detect: Encountered a Memory Allocation Error. \n");
			return MTP_GENERAL_FAILURE;
		}
		// We have at least 1 raw device, so we connect to the first device.
        if(DeviceMgr.numrawdevices > 1) {
            DeviceMgr.rawdeviceID = displayMultiDeviceDialog();
            DeviceMgr.device = LIBMTP_Open_Raw_Device(&DeviceMgr.rawdevices[DeviceMgr.rawdeviceID]);
        } else {
            // Connect to the first device.
            DeviceMgr.device = LIBMTP_Open_Raw_Device(&DeviceMgr.rawdevices[0]);
            DeviceMgr.rawdeviceID = 0;
        }
		if (DeviceMgr.device == NULL) {
			g_print("Detect: Unable to open raw device?\n");
			displayError("Detect: Unable to open raw device?\n");
			LIBMTP_Dump_Errorstack(DeviceMgr.device);
			LIBMTP_Clear_Errorstack(DeviceMgr.device);
			//LIBMTP_Dump_Device_Info(DeviceMgr.device);
			DeviceMgr.deviceConnected = FALSE;
			return MTP_GENERAL_FAILURE;
		}

		LIBMTP_Dump_Errorstack(DeviceMgr.device);
		LIBMTP_Clear_Errorstack(DeviceMgr.device);
		//LIBMTP_Dump_Device_Info(DeviceMgr.device);
		DeviceMgr.deviceConnected = TRUE;

        // We have a successful device connect, but lets check for multiple storageIDs.
        if(DeviceMgr.device->storage->next != NULL ){
            // Oops we have multiple storage IDs.
            DeviceMgr.storagedeviceID = displayDeviceStorageDialog();
        } else {
            DeviceMgr.storagedeviceID = MTP_DEVICE_SINGLE_STORAGE;
        }
        currentFolderID = 0;
		return MTP_SUCCESS;
	}
}

guint deviceDisconnect(){
	if (DeviceMgr.deviceConnected == FALSE)
	{
		DeviceMgr.deviceConnected = FALSE;
		return MTP_NO_DEVICE;
	}
	else
	{
		DeviceMgr.deviceConnected = FALSE;
		LIBMTP_Release_Device(DeviceMgr.device);
		g_free(DeviceMgr.rawdevices);
		return MTP_SUCCESS;
	}

}

void deviceProperties(){
	gint ret;
	gchar *tmp_string;

	//g_print("You selected deviceProperites\n");

	// We first see if we have a connected device, and then extract the information from it.
	if(DeviceMgr.deviceConnected == TRUE) {
		// Lets get our information. Let's start with the raw information.
		if(DeviceMgr.rawdevices[DeviceMgr.rawdeviceID].device_entry.vendor == NULL) {
			DeviceMgr.Vendor = g_string_new("Unknown");
		} else {
			DeviceMgr.Vendor = g_string_new(DeviceMgr.rawdevices[DeviceMgr.rawdeviceID].device_entry.vendor);
		}
		if(DeviceMgr.rawdevices[DeviceMgr.rawdeviceID].device_entry.product == NULL) {
			DeviceMgr.Product = g_string_new("Unknown");
		} else {
			DeviceMgr.Product = g_string_new(DeviceMgr.rawdevices[DeviceMgr.rawdeviceID].device_entry.product);
		}
		DeviceMgr.VendorID = DeviceMgr.rawdevices[DeviceMgr.rawdeviceID].device_entry.vendor_id;
		DeviceMgr.ProductID = DeviceMgr.rawdevices[DeviceMgr.rawdeviceID].device_entry.product_id;
		DeviceMgr.BusLoc = DeviceMgr.rawdevices[DeviceMgr.rawdeviceID].bus_location;
		DeviceMgr.DeviceID = DeviceMgr.rawdevices[DeviceMgr.rawdeviceID].devnum;

		// Now lets get our other information.
		// Nice name:
		tmp_string = LIBMTP_Get_Friendlyname(DeviceMgr.device);
		if(tmp_string == NULL) {
			DeviceMgr.devicename = g_string_new("N/A");
		} else {
			DeviceMgr.devicename = g_string_new(tmp_string);
			g_free(tmp_string);
		}
		// Sync Partner
		tmp_string = LIBMTP_Get_Syncpartner(DeviceMgr.device);
		if(tmp_string == NULL) {
			DeviceMgr.syncpartner = g_string_new("N/A");
		} else {
			DeviceMgr.syncpartner = g_string_new(tmp_string);
			g_free(tmp_string);
		}
		// Battery Level
		ret = LIBMTP_Get_Batterylevel(DeviceMgr.device, &DeviceMgr.maxbattlevel, &DeviceMgr.currbattlevel);
		if (ret != 0) {
			// Silently ignore. Some devices does not support getting the
			// battery level.
            DeviceMgr.maxbattlevel = 0;
            DeviceMgr.currbattlevel = 0;
			LIBMTP_Clear_Errorstack(DeviceMgr.device);
		}
		// Manufacturer Name.
		tmp_string = LIBMTP_Get_Manufacturername(DeviceMgr.device);
		if(tmp_string == NULL) {
			DeviceMgr.manufacturername = g_string_new("N/A");
		} else {
			DeviceMgr.manufacturername = g_string_new(tmp_string);
			g_free(tmp_string);
		}
		// Model Number,
		tmp_string = LIBMTP_Get_Modelname(DeviceMgr.device);
		if(tmp_string == NULL) {
			DeviceMgr.modelname = g_string_new("N/A");
		} else {
			DeviceMgr.modelname = g_string_new(tmp_string);
			g_free(tmp_string);
		}
		// Serial Number.
		tmp_string = LIBMTP_Get_Serialnumber(DeviceMgr.device);
		if(tmp_string == NULL) {
			DeviceMgr.serialnumber = g_string_new("N/A");
		} else {
			DeviceMgr.serialnumber = g_string_new(tmp_string);
			g_free(tmp_string);
		}
		// Device Version.
		tmp_string = LIBMTP_Get_Deviceversion(DeviceMgr.device);
		if(tmp_string == NULL) {
			DeviceMgr.deviceversion = g_string_new("N/A");
		} else {
			DeviceMgr.deviceversion = g_string_new(tmp_string);
			g_free(tmp_string);
		}
		// Secure Time
		ret = LIBMTP_Get_Secure_Time(DeviceMgr.device, &tmp_string);
		if (ret == 0 && tmp_string != NULL) {
			// tmp_string is a XML fragment, and we need just the date/time out of it.
			DeviceMgr.sectime = g_string_new(tmp_string);
			g_free(tmp_string);
		} else {
			// Silently ignore - there may be devices not supporting secure time.
			DeviceMgr.sectime = g_string_new("N/A");
			LIBMTP_Clear_Errorstack(DeviceMgr.device);
		}

		// Storage.
		LIBMTP_Get_Storage(DeviceMgr.device, 0);
        if(DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE ){
            DeviceMgr.devicestorage = DeviceMgr.device->storage;
        } else {
            DeviceMgr.devicestorage = getCurrentDeviceStoragePtr(DeviceMgr.storagedeviceID);
        }

		// Supported filetypes;
		ret = LIBMTP_Get_Supported_Filetypes(DeviceMgr.device, &DeviceMgr.filetypes, &DeviceMgr.filetypes_len);
		if (ret != 0) {
			LIBMTP_Dump_Errorstack(DeviceMgr.device);
			LIBMTP_Clear_Errorstack(DeviceMgr.device);
		}

	} else {
		// Set to to none.
		g_print("DevicePropeties: How did I get called?\n");
		DeviceMgr.device = NULL;
	}
}

void deviceRescan(){
	gchar tmp_string[256];
	//g_print("You selected deviceRescan\n");
	// First we clear the file list...
	fileListClear();
	// Now clear the folder/file structures.
	LIBMTP_destroy_folder_t(deviceFolders);
	LIBMTP_destroy_file_t(deviceFiles);
	deviceFolders = NULL;
	deviceFiles = NULL;

	// Now get started.
	if(DeviceMgr.deviceConnected) {
		// Get a list of folder on the device.
		deviceFolders = LIBMTP_Get_Folder_List(DeviceMgr.device);
		if (deviceFolders == NULL) {
			LIBMTP_Dump_Errorstack(DeviceMgr.device);
			LIBMTP_Clear_Errorstack(DeviceMgr.device);
		}
		// Now get a list of files from the device.
		deviceFiles = LIBMTP_Get_Filelisting_With_Callback(DeviceMgr.device, NULL, NULL);
		if (deviceFiles == NULL) {
			LIBMTP_Dump_Errorstack(DeviceMgr.device);
			LIBMTP_Clear_Errorstack(DeviceMgr.device);
		}
		fileListAdd();
		// Now update the storage...
		LIBMTP_Get_Storage(DeviceMgr.device, 0);
		if(DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE ){
            DeviceMgr.devicestorage = DeviceMgr.device->storage;
        } else {
            DeviceMgr.devicestorage = getCurrentDeviceStoragePtr(DeviceMgr.storagedeviceID);
        }
		// Update the status bar.
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

	} else {
		g_print("Rescan: How did I get called?\n");
	}
}
// Find the ptr to the current storage structure.
LIBMTP_devicestorage_t* getCurrentDeviceStoragePtr(gint StorageID){
    LIBMTP_devicestorage_t* deviceStorage = DeviceMgr.device->storage;
    gint i = 0;
    // This is easy, as the gint is the number of hops we need to do to get to our target.
    if(StorageID == MTP_DEVICE_SINGLE_STORAGE)
        return DeviceMgr.device->storage;
    if(StorageID == 0)
        return DeviceMgr.device->storage;
    for(i = 0; i < StorageID; i++){
        deviceStorage = deviceStorage->next;
        if(deviceStorage == NULL)   // Oops, off the end
            return DeviceMgr.device->storage;
    }
    return deviceStorage;
}


// Find the ID of the parent folder.
uint32_t getParentFolderID(LIBMTP_folder_t *tmpfolder, uint32_t currentFolderID){
	uint32_t parentID;
	if(tmpfolder==NULL) {
		return 0;
	}
	if(tmpfolder->folder_id == currentFolderID) {
		parentID = tmpfolder->parent_id;
		return parentID;
	}
	parentID = getParentFolderID(tmpfolder->child, currentFolderID);
	if(parentID != 0)
		return parentID;
	parentID = getParentFolderID(tmpfolder->sibling, currentFolderID);
	return parentID;
}

// Find the structure of the parent MTP Folder based on the currentID.
LIBMTP_folder_t* getParentFolderPtr(LIBMTP_folder_t *tmpfolder, uint32_t currentFolderID){
	LIBMTP_folder_t* parentID;
	if(tmpfolder==NULL) {
		return tmpfolder;
	}
	if(tmpfolder->parent_id == currentFolderID) {
		return tmpfolder;
	}
	parentID = getParentFolderPtr(tmpfolder->child, currentFolderID);
	if(parentID != NULL)
		return parentID;
	parentID = getParentFolderPtr(tmpfolder->sibling, currentFolderID);
	return parentID;
}

// Find the structure of the MTP Folder based on the currentID.
LIBMTP_folder_t* getCurrentFolderPtr(LIBMTP_folder_t *tmpfolder, uint32_t FolderID){
	LIBMTP_folder_t* parentID;
	if(tmpfolder==NULL) {
		return tmpfolder;
	}
	if(tmpfolder->folder_id == FolderID) {
		return tmpfolder;
	}
	parentID = getCurrentFolderPtr(tmpfolder->child, FolderID);
	if(parentID != NULL)
		return parentID;
	parentID = getCurrentFolderPtr(tmpfolder->sibling, FolderID);
	return parentID;
}

void filesAdd(gchar* filename){
	uint64_t filesize;
	gchar *filename_stripped;
	struct stat sb;
	LIBMTP_file_t *genfile;
    LIBMTP_track_t *trackfile;
    LIBMTP_album_t *albuminfo;
	gint ret;

	//g_printf("You selected to add file %s\n", filename);

	if ( stat(filename, &sb) == -1 ) {
		//g_printf("%s: ", filename);
		perror("stat");
		return;
	}

	filesize = sb.st_size;
	if(filesize > DeviceMgr.devicestorage->FreeSpaceInBytes) {
		g_printf("Unable to add %s due to insufficient space: filesize = %l, freespace = %l\n", filename, filesize, DeviceMgr.devicestorage->FreeSpaceInBytes);
		displayError("Unable to add file due to insufficient space");
		return;
	}

    filename_stripped = basename(filename);
    displayProgressBar("File Upload");
	setProgressFilename(filename_stripped);

	// What we need to do is work what type of file we are sending
    // and either use the general file send, or
    // use the track send function.
    ret = find_filetype (filename_stripped);

    if ((ret == LIBMTP_FILETYPE_MP3) ||
        (ret == LIBMTP_FILETYPE_OGG) ||
        (ret == LIBMTP_FILETYPE_FLAC) )
    {
        // We have an MP3/Ogg/FLAC file.
        //g_printf("We have a supported audio file with metadata\n");
        trackfile = LIBMTP_new_track_t();

        trackfile->filesize = filesize;
        trackfile->filename = g_strdup(filename_stripped);
        trackfile->filetype = find_filetype (filename_stripped);
        trackfile->parent_id = currentFolderID;
        trackfile->storage_id = DeviceMgr.devicestorage->id;
        albuminfo = LIBMTP_new_album_t();
        albuminfo->parent_id = currentFolderID;
        albuminfo->storage_id = DeviceMgr.devicestorage->id;
        // Let's collect our metadata from the file, typically id3 tag data.
        if(ret == LIBMTP_FILETYPE_MP3){
            // We have an MP3 file, so use id3tag to read the metadata.
            struct id3_file * id3_file_id = id3_file_open(filename, ID3_FILE_MODE_READONLY);
            if(id3_file_id != NULL){
                // We have a valid file, so lets get some data.
                struct id3_tag* id3_tag_id = id3_file_tag(id3_file_id);
                // We have our tag data, so now cycle through the fields.
                trackfile->album = ID3_getFrameText(id3_tag_id, ID3_FRAME_ALBUM);
                trackfile->title = ID3_getFrameText(id3_tag_id, ID3_FRAME_TITLE);
                trackfile->artist = ID3_getFrameText(id3_tag_id, ID3_FRAME_ARTIST);
                trackfile->date = ID3_getFrameText(id3_tag_id, ID3_FRAME_YEAR);
                trackfile->genre = ID3_getFrameText(id3_tag_id, ID3_FRAME_GENRE);
                trackfile->tracknumber = atoi(ID3_getFrameText(id3_tag_id, ID3_FRAME_TRACK));
                // Need below if the default artist field is NULL
                if(trackfile->artist == NULL)
                    trackfile->artist = ID3_getFrameText(id3_tag_id, "TPE2");
                if(trackfile->artist == NULL)
                    trackfile->artist = ID3_getFrameText(id3_tag_id, "TPE3");
                if(trackfile->artist == NULL)
                    trackfile->artist = ID3_getFrameText(id3_tag_id, "TPE4");
                if(trackfile->artist == NULL)
                    trackfile->artist = ID3_getFrameText(id3_tag_id, "TCOM");
                // Need this if using different Year field.
                if(trackfile->date == NULL)
                    trackfile->date = ID3_getFrameText(id3_tag_id, "TDRC");
                // Close our file for reading the fields.
                id3_file_close(id3_file_id);
            }
        } else {
            // we don't support OGG or FLAC metadata yet?
            if(ret == LIBMTP_FILETYPE_OGG){
                // If we have OGG 

            } else {
                // If we have FLAC
                
            }
        }
        // Add some data if it's all blank so we don't freak out some players.
        if(trackfile->album == NULL)
            trackfile->album = NULL;
        if(trackfile->title == NULL)
            trackfile->title = g_strdup(filename_stripped);
        if(trackfile->artist == NULL)
            trackfile->artist = g_strdup("<Unknown>");
        if(trackfile->date == NULL)
            trackfile->date = g_strdup("<Unknown>");
        if(trackfile->genre == NULL)
            trackfile->genre = g_strdup("<Unknown>");

        // Update our album info, if we actually have an album.
        if(trackfile->album != NULL){
            albuminfo->name = g_strdup(trackfile->album);
            albuminfo->artist = g_strdup(trackfile->artist);
            albuminfo->composer = NULL;
            albuminfo->genre = g_strdup(trackfile->genre);
        }
        // Now send the track
        ret = LIBMTP_Send_Track_From_File(DeviceMgr.device, filename, trackfile, fileprogress, NULL);
        if (ret != 0) {
            g_print("Error sending track.\n");
            displayError(g_strconcat("Error sending track to device: <b>", filename, "</b>", NULL));
            LIBMTP_Dump_Errorstack(DeviceMgr.device);
            LIBMTP_Clear_Errorstack(DeviceMgr.device);
        }
        if(trackfile->album != NULL){
            albumAddTrackToAlbum(albuminfo, trackfile);
        }
        LIBMTP_destroy_track_t(trackfile);
        LIBMTP_destroy_album_t(albuminfo);
    } else {
        // Generic file upload.
        //g_printf("We have a generic file without metadata\n");
        genfile = LIBMTP_new_file_t();
        genfile->filesize = filesize;
        genfile->filename = g_strdup(filename_stripped);
        genfile->filetype = find_filetype (filename_stripped);
        genfile->parent_id = currentFolderID;
        genfile->storage_id = DeviceMgr.devicestorage->id;

        ret = LIBMTP_Send_File_From_File(DeviceMgr.device, filename, genfile, fileprogress, NULL);
        if (ret != 0) {
            g_printf("Error sending file %s.\n", filename);
            displayError(g_strconcat("Error sending file: <b>", filename, "</b>", NULL));
            LIBMTP_Dump_Errorstack(DeviceMgr.device);
            LIBMTP_Clear_Errorstack(DeviceMgr.device);
        }
        LIBMTP_destroy_file_t(genfile);
    }
	destroyProgressBar();
	// Now update the storage...
	LIBMTP_Get_Storage(DeviceMgr.device, 0);
	if(DeviceMgr.storagedeviceID == MTP_DEVICE_SINGLE_STORAGE ){
        DeviceMgr.devicestorage = DeviceMgr.device->storage;
    } else {
        DeviceMgr.devicestorage = getCurrentDeviceStoragePtr(DeviceMgr.storagedeviceID);
    }

}

void filesDelete(gchar* filename, uint32_t objectID){
	gint ret = 1;
	//g_print("You selected filesDelete\n");
	// Delete the file based on the object ID.
	ret = LIBMTP_Delete_Object(DeviceMgr.device, objectID);
	if (ret != 0) {
		LIBMTP_Dump_Errorstack(DeviceMgr.device);
		LIBMTP_Clear_Errorstack(DeviceMgr.device);
		g_printf("\nFailed to delete file %s\n", filename);
		displayError(g_strconcat("Failed to delete file <b>", filename, "</b>", NULL));
	}
}

void filesDownload(gchar* filename, uint32_t objectID){
	gchar* fullfilename;
	fullfilename = g_strndup("", 8192);
	//g_print("You selected filesDownload\n");
	displayProgressBar("File download");
	setProgressFilename(filename);
	// Download the file based on the objectID.
	g_sprintf(fullfilename, "%s/%s", Preferences.fileSystemDownloadPath->str, filename);
	//g_printf("Getting %d to %s (%s)\n", objectID, fullfilename, filename);
	if (LIBMTP_Get_File_To_File(DeviceMgr.device, objectID, fullfilename, fileprogress, NULL) != 0 ) {
		printf("\nError getting file from MTP device.\n");
		displayError("Error getting file from MTP device.");
		LIBMTP_Dump_Errorstack(DeviceMgr.device);
		LIBMTP_Clear_Errorstack(DeviceMgr.device);
	}
	destroyProgressBar();
	g_free(fullfilename);
}

guint32 folderAdd (gchar* foldername){
	guint32 res = LIBMTP_Create_Folder(DeviceMgr.device, foldername, currentFolderID, DeviceMgr.devicestorage->id);
	if (res == 0) {
		g_printf("Folder creation failed: %s\n", foldername);
		displayError(g_strconcat("Folder creation failed: <b>", foldername, "</b>", NULL));
		LIBMTP_Dump_Errorstack(DeviceMgr.device);
		LIBMTP_Clear_Errorstack(DeviceMgr.device);
	} else {
		//g_print("New folder created with ID: %d\n", res);
	}
    return res;
}

void folderDelete (LIBMTP_folder_t* folderptr, guint level){
    if(folderptr == NULL) {
		// Sanity check for rogue data or exit here operation, that is no child/sibling to work on.
		return;
	}
	// This is fun, as we have to find all child folder, delete those, as well as all files contained within...
	//g_printf("Removing folder: %s in level %x\n", folderptr->name, level);
	// First iteratate through all child folders and remove those files in those folders.
	// But first we need to get the folder structure pointer based on the objectID, so we know where to start.
	// So now we have our structure to the current select folder, so we need to cycle through all children and remove any files contained within.
	folderDeleteChildrenFiles(folderptr->folder_id);
	// Now cycle through folders contained in this folder and delete those;
	folderDelete(folderptr->child, level+1);
	if(level != 0)
		folderDelete(folderptr->sibling, level+1);
	// That should clear all the children.
	// Now do self.
	guint res = LIBMTP_Delete_Object(DeviceMgr.device, folderptr->folder_id);
	if (res != 0) {
		g_printf("Couldn't delete folder %s (%x)\n",folderptr->name,folderptr->folder_id);
		LIBMTP_Dump_Errorstack(DeviceMgr.device);
		LIBMTP_Clear_Errorstack(DeviceMgr.device);
	}
}

void folderDeleteChildrenFiles(guint folderID){
	LIBMTP_file_t* files = deviceFiles;
	while(files != NULL) {
        //g_printf("Testing file %s in folder %d\n", files->filename, files->parent_id);
		if(files->parent_id == folderID) {
            //g_printf("Removing file %s in folder %d\n", files->filename, files->parent_id);
			filesDelete(files->filename, files->item_id);
		}
		files = files->next;
	}
}

/* Find the file type based on extension */
LIBMTP_filetype_t find_filetype (const gchar * filename) {
	LIBMTP_filetype_t filetype = -1;
    gchar *fileext;
    gint i;
    gint j = sizeof(file_ext) / sizeof (MTP_file_ext_struct);

	fileext = rindex(filename,'.');
	// This accounts for the case with a filename without any "." (period).
	if (!fileext) {
		fileext = "";
	} else {
		++fileext;
	}
    // Now cycle through the array of extensions, and get the associated
    // libmtp filetype.
    for(i = 0; i < j; i++){
        if(g_ascii_strcasecmp (fileext, file_ext[i].file_extension) == 0){
            filetype = file_ext[i].file_type;
            break;
        }
    }
    if(filetype == -1){
        filetype = LIBMTP_FILETYPE_UNKNOWN;
    }
	return filetype;
}

void setDeviceName(gchar* devicename){
    gint res = 0;
    if(DeviceMgr.deviceConnected == TRUE){
        if(devicename != NULL)
            res = LIBMTP_Set_Friendlyname(DeviceMgr.device, devicename);
        if (res != 0) {
            g_printf("Error: Couldn't set device name to %s\n",devicename);
            LIBMTP_Dump_Errorstack(DeviceMgr.device);
            LIBMTP_Clear_Errorstack(DeviceMgr.device);
        }
    } else {
		// Set to to none.
		g_print("setDeviceName: How did I get called?\n");
	}
}

gchar * ID3_getFrameText(struct id3_tag *tag, char *frame_name)
{
    const id3_ucs4_t *id3_string;
    struct id3_frame *id3_frame;
    union id3_field *id3_field;
    gchar *rtn_string = NULL;
    enum id3_field_textencoding id3_field_encoding = ID3_FIELD_TEXTENCODING_ISO_8859_1;

    id3_frame = id3_tag_findframe (tag, frame_name, 0);
    if (id3_frame == NULL)
        return NULL;

    id3_field = id3_frame_field (id3_frame, 0);
    if (id3_field && (id3_field_type (id3_field) == ID3_FIELD_TYPE_TEXTENCODING)) {
        id3_field_encoding = id3_field->number.value;
    }
    if (frame_name == ID3_FRAME_COMMENT){
        id3_field = id3_frame_field (id3_frame, 3);
    } else {
        id3_field = id3_frame_field (id3_frame, 1);
    }
    if (id3_field == NULL)
        return NULL;
    if (frame_name == ID3_FRAME_COMMENT){
        id3_string = id3_field_getfullstring (id3_field);
    } else {
        id3_string = id3_field_getstrings (id3_field, 0);
    }
    if (id3_string == NULL)
        return NULL;
    if (frame_name == ID3_FRAME_GENRE)
        id3_string = id3_genre_name (id3_string);
    if (id3_field_encoding == ID3_FIELD_TEXTENCODING_ISO_8859_1) {
        rtn_string = (gchar *) id3_ucs4_latin1duplicate (id3_string);
    } else {
        rtn_string = (gchar *) id3_ucs4_utf8duplicate (id3_string);
    }
    return rtn_string;
}

gboolean fileExists(gchar* filename){
    // What we have to go is scan the entire file tree looking for
    // entries in the same folder as the current and the same
    // storage pool, then we do a string compare (since doing a string
    // compare is so much slower than comparing a few numbers).
    LIBMTP_file_t* tmpfile;
    tmpfile = deviceFiles;
    while (tmpfile != NULL){
        // Check for matching folder ID and storage ID.
        if((tmpfile->parent_id == currentFolderID) && (tmpfile->storage_id == DeviceMgr.devicestorage->id)){
            // Now test for the file name (do case insensitive cmp for those odd devices);
            if(g_ascii_strcasecmp(filename, tmpfile->filename) == 0)
                return TRUE;
        }
        tmpfile = tmpfile->next;
    }
    return FALSE;
}

void albumAddTrackToAlbum(LIBMTP_album_t* albuminfo, LIBMTP_track_t* trackinfo){
    LIBMTP_album_t *album;
    LIBMTP_album_t *found_album = NULL;
    gint ret = 0;

    // Quick sanity check.
    if((albuminfo->name == NULL)||(albuminfo->artist == NULL))
        return;

    // Lets try to find the album.
    album = LIBMTP_Get_Album_List(DeviceMgr.device);
    while(album != NULL) {
        if((album->name != NULL)&&(album->artist != NULL))
        {
            // Lets test it. We attempt to match both album name and artist.
            if((g_ascii_strcasecmp(album->name, albuminfo->name) == 0) &&
                (g_ascii_strcasecmp(album->artist, albuminfo->artist) == 0)){
                    found_album = album;
            }
        }
        album = album->next;
        if(found_album != NULL)
            album = NULL; // exit the loop
    }

    if (found_album != NULL) {
        uint32_t *tracks;
        tracks = (uint32_t *)malloc((found_album->no_tracks+1) * sizeof(uint32_t));
        //g_printf("Album found: updating \"%s\"\n", found_album->name);
        if (!tracks) {
            g_printf("Failed memory allocation in albumAddTrackToAlbum()\n");
            return;
        }
        found_album->no_tracks++;
        if (found_album->tracks != NULL) {
            memcpy(tracks, found_album->tracks, found_album->no_tracks * sizeof(uint32_t));
            free(found_album->tracks);
        }
        tracks[found_album->no_tracks-1] = trackinfo->item_id;  // This ID is only set once the track is on the device.
        found_album->tracks = tracks;
        ret = LIBMTP_Update_Album(DeviceMgr.device, found_album);
        /*
        if(found_album->name != NULL)
            g_printf("Update Album Name: %s\n", found_album->name);
        if(found_album->artist != NULL)
            g_printf("Update Album Artist: %s\n", found_album->artist);*/
        LIBMTP_destroy_album_t(found_album);
    } else {
        uint32_t *trackid;
        trackid = (uint32_t *)malloc(sizeof(uint32_t));
        *trackid = trackinfo->item_id;
        albuminfo->tracks = trackid;
        albuminfo->no_tracks = 1;
        /*
        g_printf("Album doesn't exist: creating \"%s\"\n", albuminfo->name);
        if(albuminfo->name != NULL)
            g_printf("New Album Name: %s\n", albuminfo->name);
        if(albuminfo->artist != NULL)
            g_printf("New Album Artist: %s\n", albuminfo->artist);*/
        ret = LIBMTP_Create_New_Album(DeviceMgr.device, albuminfo);
    }
    if (ret != 0) {
        g_print("Error creating or updating album.\n(This could be due to that your device does not support albums.)\n");
        LIBMTP_Dump_Errorstack(DeviceMgr.device);
        LIBMTP_Clear_Errorstack(DeviceMgr.device);
    }
}

void albumAddArt(guint32 album_id, gchar* filename){
    LIBMTP_filesampledata_t *albumart;
    gint ret;
    uint64_t filesize;
    uint8_t *imagedata = NULL;
    struct stat statbuff;
    FILE* fd;

    if ( stat(filename, &statbuff) == -1 ) {
        //g_printf("%s: ", filename);
        perror("stat");
        return;
    }
    filesize = (uint64_t) statbuff.st_size;
    imagedata = g_malloc(filesize * sizeof(uint8_t));
    if (imagedata == NULL) {
        g_printf("Failed memory allocation in albumAddArt()\n");
        return;
    }
    fd = fopen(filename, "r");
    if (fd == NULL) {
        g_printf("Couldn't open image file %s\n",filename);
        g_free(imagedata);
        return;
    } else {
        fread(imagedata, filesize, 1, fd);
        fclose(fd);
    }

    albumart = LIBMTP_new_filesampledata_t();
    albumart->data = (gchar *)imagedata;
    albumart->size = filesize;
    albumart->filetype = find_filetype(basename(filename));

    //g_printf("Album info = %d, %s, %d\n", album_id, filename, albumart->filetype );

    ret = LIBMTP_Send_Representative_Sample(DeviceMgr.device, album_id, albumart);
    if (ret != 0) {
        g_printf("Error: Couldn't send album art\n");
        LIBMTP_Dump_Errorstack(DeviceMgr.device);
        LIBMTP_Clear_Errorstack(DeviceMgr.device);
    }
    g_free(imagedata);
}
