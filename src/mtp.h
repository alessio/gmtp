/* 
 *
 *   File: mtp.h
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
    void clearDeviceFiles(LIBMTP_file_t * filelist);
    void clearAlbumStruc(LIBMTP_album_t * albumlist);
    void clearDevicePlaylist(LIBMTP_playlist_t * playlist_list);
    void clearDeviceTracks(LIBMTP_track_t * tracklist);
    void deviceRescan();
    void filesRename(gchar* filename, uint32_t ObjectID);
    void filesAdd(gchar* filename);
    void filesDelete(gchar* filename, uint32_t objectID);
    void filesDownload(gchar* filename, uint32_t objectID);
    gboolean fileExists(gchar* filename);
    guint32 folderAdd(gchar* foldername);
    void folderDelete(LIBMTP_folder_t* folderptr, guint level);
    void folderDeleteChildrenFiles(guint folderID);
    void folderDownload(gchar * foldername, uint32_t folderID, gboolean isParent);
    void albumAddTrackToAlbum(LIBMTP_album_t* albuminfo, LIBMTP_track_t* trackinfo);
    void albumAddArt(guint32 album_id, gchar* filename);
    void albumDeleteArt(guint32 album_id);
    LIBMTP_filesampledata_t * albumGetArt(LIBMTP_album_t* selectedAlbum);
    void setDeviceName(gchar* devicename);
    uint32_t getParentFolderID(LIBMTP_folder_t *tmpfolder, uint32_t currentFolderID);
    LIBMTP_folder_t* getParentFolderPtr(LIBMTP_folder_t *tmpfolder, uint32_t currentFolderID);
    LIBMTP_folder_t* getCurrentFolderPtr(LIBMTP_folder_t *tmpfolder, uint32_t FolderID);
    LIBMTP_filetype_t find_filetype(const gchar * filename);
    gchar* find_filetype_ext(LIBMTP_filetype_t filetype);
    LIBMTP_devicestorage_t* getCurrentDeviceStoragePtr(gint StorageID);

    // Playlist support.
    LIBMTP_playlist_t* getPlaylists(void);
    LIBMTP_track_t* getTracks(void);
    void playlistAdd(gchar* playlistname);
    void playlistDelete(LIBMTP_playlist_t * tmpplaylist);
    void playlistUpdate(LIBMTP_playlist_t * tmpplaylist);
    void playlistAddTrack(LIBMTP_playlist_t* playlist, LIBMTP_track_t* track);
    gchar* playlistImport(gchar * filename);
    void playlistExport(gchar * filename, LIBMTP_playlist_t * playlist);

    // Format device.
    void formatStorageDevice();

    // File operation helper.
    gchar* getFullFilename(uint32_t item_id);
    uint32_t getFileID(gchar* filename, gboolean ignorepath);
    uint32_t getFolderID(LIBMTP_folder_t* folderptr, gchar* foldername);

#ifdef  __cplusplus
}
#endif

#endif  /* _MTP_H */
