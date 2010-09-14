/*
 * File:   mtp.h
 * Author: darran
 *
 * Created on October 29, 2009, 5:01 PM
 */

#ifndef _MTP_H
#define _MTP_H

#ifdef  __cplusplus
extern "C" {
#endif

enum MTP_ERROR {
	MTP_SUCCESS,
	MTP_NO_DEVICE,
	MTP_GENERAL_FAILURE,
	MTP_DEVICE_FULL,
	MTP_NO_MTP_DEVICE
};

#define MTP_DEVICE_SINGLE_STORAGE -1

typedef struct {
    gchar* file_extension;
    LIBMTP_filetype_t file_type;
} MTP_file_ext_struct;

guint deviceConnect();
guint deviceDisconnect();
void deviceProperties();
void deviceRescan();
void filesAdd(gchar* filename);
void filesDelete(gchar* filename, uint32_t objectID);
void filesDownload(gchar* filename, uint32_t objectID);
gboolean fileExists(gchar* filename);
guint32 folderAdd (gchar* foldername);
void folderDelete (LIBMTP_folder_t* folderptr, guint level);
void folderDeleteChildrenFiles(guint folderID);
void albumAddTrackToAlbum(LIBMTP_album_t* albuminfo, LIBMTP_track_t* trackinfo);
void albumAddArt(guint32 album_id, gchar* filename);
void setDeviceName(gchar* devicename);
uint32_t getParentFolderID(LIBMTP_folder_t *tmpfolder, uint32_t currentFolderID);
LIBMTP_folder_t* getParentFolderPtr(LIBMTP_folder_t *tmpfolder, uint32_t currentFolderID);
LIBMTP_folder_t* getCurrentFolderPtr(LIBMTP_folder_t *tmpfolder, uint32_t FolderID);
LIBMTP_filetype_t find_filetype (const gchar * filename);
LIBMTP_devicestorage_t* getCurrentDeviceStoragePtr(gint StorageID);

gchar * ID3_getFrameText(struct id3_tag *tag, char *frame_name);

#ifdef  __cplusplus
}
#endif

#endif  /* _MTP_H */
