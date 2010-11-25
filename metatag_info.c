/*
 * File:   metatag_info.c
 * Author: darran
 *
 * Created on November 11, 2010, 8:54 PM
 */

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
#include <string.h>
#include <id3tag.h>
#include <stdio.h>
#include <FLAC/all.h>
#include <vorbis/vorbisfile.h>

#include "main.h"
#include "callbacks.h"
#include "interface.h"
#include "mtp.h"
#include "prefs.h"
#include "dnd.h"
#include "metatag_info.h"

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

void get_id3_tags(gchar *filename, LIBMTP_track_t *trackinformation){
    gchar * tracknumber = 0;

    struct id3_file * id3_file_id = id3_file_open(filename, ID3_FILE_MODE_READONLY);
    
    if(id3_file_id != NULL){
        // We have a valid file, so lets get some data.
        struct id3_tag* id3_tag_id = id3_file_tag(id3_file_id);
        // We have our tag data, so now cycle through the fields.
        trackinformation->album = ID3_getFrameText(id3_tag_id, ID3_FRAME_ALBUM);
        trackinformation->title = ID3_getFrameText(id3_tag_id, ID3_FRAME_TITLE);
        trackinformation->artist = ID3_getFrameText(id3_tag_id, ID3_FRAME_ARTIST);
        trackinformation->date = ID3_getFrameText(id3_tag_id, ID3_FRAME_YEAR);
        trackinformation->genre = ID3_getFrameText(id3_tag_id, ID3_FRAME_GENRE);
        
        tracknumber = ID3_getFrameText(id3_tag_id, ID3_FRAME_TRACK);
        if (tracknumber != 0){
            trackinformation->tracknumber = atoi(tracknumber);
        } else {
            trackinformation->tracknumber = 0;
        }
        //trackinformation->tracknumber = atoi(ID3_getFrameText(id3_tag_id, ID3_FRAME_TRACK));

        // Need below if the default artist field is NULL
        if(trackinformation->artist == NULL)
            trackinformation->artist = ID3_getFrameText(id3_tag_id, "TPE2");
        if(trackinformation->artist == NULL)
            trackinformation->artist = ID3_getFrameText(id3_tag_id, "TPE3");
        if(trackinformation->artist == NULL)
            trackinformation->artist = ID3_getFrameText(id3_tag_id, "TPE4");
        if(trackinformation->artist == NULL)
            trackinformation->artist = ID3_getFrameText(id3_tag_id, "TCOM");
        // Need this if using different Year field.
        if(trackinformation->date == NULL)
            trackinformation->date = ID3_getFrameText(id3_tag_id, "TDRC");
        // Close our file for reading the fields.
        id3_file_close(id3_file_id);
    }
}

gchar * OGG_getFieldText(const vorbis_comment *comments, const char *name){
    gchar ** file_comments;
    gchar ** comments_split;
    gint file_comments_count = 0;
    // We simple cycle through our comments, looking for our name, and return it's value;

    if(comments->comments > 0){
        file_comments = comments->user_comments;
        file_comments_count = comments->comments;
        while(file_comments_count--){
            // We have our comment, now see if it is what we are after?
            comments_split = g_strsplit(*file_comments, "=", 2);
            if(*comments_split != NULL){
                //if(name == NULL) g_printf("OGG_getFieldText - name\n");
                //if(*comments_split == NULL) g_printf("OGG_getFieldText - *comments_split\n");
                if(g_ascii_strcasecmp(name, *comments_split) == 0){
                    // We have our desrired tag, so return it to the user.
                    comments_split++;
                    return g_strdup(*comments_split);
                }
            }
            // Increment our pointers accordingly.
            file_comments++;
        }
    } else {
        // No comments, so return a NULL value;
        return NULL;
    }
    // We didn't find our key, so return NULL
    return NULL;
}

void get_ogg_tags(gchar *filename, LIBMTP_track_t *trackinformation){
    OggVorbis_File *mov_file = NULL;
    FILE *mfile;
    vorbis_comment *mov_file_comment = NULL;
    gchar * tracknumber = NULL;
    
    // Attempt to open the file, and init the OggVorbis_File struct for our file.
    // Yes I know about ov_fopen(), but Solaris 10 ships with vorbis 1.0.1 which
    // doesn't have this function.
    mfile = fopen(filename, "r");
    if (mfile == NULL)
        return;

    // Allocate memory to hold the OV file information.
    mov_file = g_malloc0(sizeof(OggVorbis_File));
    
    if (ov_open(mfile, mov_file, NULL, 0) != 0 ){
        fclose(mfile);
        return;
    }

    // Get or comment data;
    mov_file_comment = ov_comment(mov_file, -1);

    trackinformation->album = OGG_getFieldText(mov_file_comment, "ALBUM");
    trackinformation->title = OGG_getFieldText(mov_file_comment, "TITLE");
    trackinformation->artist = OGG_getFieldText(mov_file_comment, "ARTIST");
    trackinformation->date = OGG_getFieldText(mov_file_comment, "DATE");
    trackinformation->genre = OGG_getFieldText(mov_file_comment, "GENRE");
    tracknumber = OGG_getFieldText(mov_file_comment, "TRACKNUMBER");
    if (tracknumber != NULL) {
        trackinformation->tracknumber = atoi(tracknumber);
    } else {
        trackinformation->tracknumber = 0;
    }
    
    ov_clear(mov_file);
     
    return;
}

gchar *FLAC_getFieldText(const FLAC__StreamMetadata *tags, const char *name){

    int index = FLAC__metadata_object_vorbiscomment_find_entry_from(tags, 0, name);

    if (index < 0 ){
        return NULL;
    }
    else {
        return strchr((const char *)tags->data.vorbis_comment.comments[index].entry, '=')+1;
    }
}

void get_flac_tags(gchar *filename, LIBMTP_track_t *trackinformation){
    FLAC__StreamMetadata *tags = NULL;
    gchar * tracknumber = 0;
    
    // Load in our tag information stream
    if(!FLAC__metadata_get_tags(filename, &tags))
        return;
    // We have our tag data, get the individual fields.
    trackinformation->album = g_strdup(FLAC_getFieldText(tags, "ALBUM"));
    trackinformation->title = g_strdup(FLAC_getFieldText(tags, "TITLE"));
    trackinformation->artist = g_strdup(FLAC_getFieldText(tags, "ARTIST"));
    trackinformation->date = g_strdup(FLAC_getFieldText(tags, "DATE"));
    trackinformation->genre = g_strdup(FLAC_getFieldText(tags, "GENRE"));

    tracknumber = FLAC_getFieldText(tags, "TRACKNUMBER");
    if (tracknumber != 0){
        trackinformation->tracknumber = atoi(tracknumber);
    } else {
        trackinformation->tracknumber = 0;
    }
    //trackinformation->tracknumber = atoi(FLAC_getFieldText(tags, "TRACKNUMBER"));
    FLAC__metadata_object_delete(tags);
    return;
}

void get_asf_tags(gchar *filename, LIBMTP_track_t *trackinformation){
    FILE *ASF_File;
    GUID Header_GUID ;
    uint32_t Header_Blocks;
    uint64_t Object_Size;
    long ASF_File_Position;

    // Content Object
    uint16_t Title_Length = 0;
    uint16_t Author_Length = 0;
    uint16_t Copyright_Length = 0;
    uint16_t Description_Length = 0;
    uint16_t Rating_Length = 0;
        
    gchar *Title = NULL;
    gchar *Author = NULL;

    // Extended Content Object
    uint16_t Content_Descriptors_Count = 0;
    uint16_t Descriptor_Name_Length = 0;
    gchar *Descriptor_Name = NULL;
    gchar *Descriptor_Name_UTF16 = NULL;
    uint16_t Descriptor_Value_Type = 0;
    uint16_t Descriptor_Value_Length = 0;
    uint64_t Descriptor_Value = 0;
    gchar *Descriptor_Value_Str = NULL;
    gchar *Descriptor_Value_Str_UTF16 = NULL;


    ASF_File = fopen(filename, "r");
    if (ASF_File == NULL)
        return;

    // Get our header GUID and make sure this is it.
    fread(&Header_GUID, sizeof(GUID), 1, ASF_File);
    if(!memcmp(&Header_GUID, &ASF_header, sizeof(GUID))){
        // If not exit.
        fclose(ASF_File);
        return;
    }
    // Skip the rest of the header area;
    fseek(ASF_File, 8, SEEK_CUR);
    fread(&Header_Blocks, sizeof(uint32_t), 1, ASF_File);
    fseek(ASF_File, 2, SEEK_CUR);

    // We should be at the start of the header blocks;
    // Header_blocks has the number of header objects that we can test.
    while(Header_Blocks--){
        fread(&Header_GUID, sizeof(GUID), 1, ASF_File);
        if(memcmp(&Header_GUID, &ASF_comment_header, sizeof(GUID)) == 0){
            // We have our standard comment header block;
            //g_printf("WMA: Found our comment block\n");
            // Get the size of the object, and the current file position.
            fread(&Object_Size, sizeof(uint64_t), 1, ASF_File);
            ASF_File_Position = ftell(ASF_File);
            // Get our field lengths.
            fread(&Title_Length, sizeof(uint16_t), 1, ASF_File);
            fread(&Author_Length, sizeof(uint16_t), 1, ASF_File);
            fread(&Copyright_Length, sizeof(uint16_t), 1, ASF_File);
            fread(&Description_Length, sizeof(uint16_t), 1, ASF_File);
            fread(&Rating_Length, sizeof(uint16_t), 1, ASF_File);
            // Since we only need Title and Author, we only need to alloc memory for those two.
            Title = g_malloc0(Title_Length + 0x10);
            Author = g_malloc0(Author_Length + 0x10);
            fread(Title, Title_Length, 1, ASF_File);
            fread(Author, Author_Length, 1, ASF_File);
            // Set our track information
            trackinformation->title = g_utf16_to_utf8 ((const gunichar2 *) Title, Title_Length, NULL, NULL, NULL );
            trackinformation->artist = g_utf16_to_utf8 ((const gunichar2 *) Author, Author_Length, NULL, NULL, NULL );
            // Free our memory that we used to load in the fields.
            g_free(Title);
            g_free(Author);
            Title = NULL;
            Author = NULL;
            // Set our file position so it's ready to read in the next GUID Header.
            fseek(ASF_File, ASF_File_Position, SEEK_SET);
            fseek(ASF_File, (Object_Size - sizeof(uint64_t) - sizeof(GUID)), SEEK_CUR);
        } else {
            if(memcmp(&Header_GUID, &ASF_extended_content_header, sizeof(GUID)) == 0){
                // We have our standard comment header block;
                //g_printf("WMA: Found our extended comment block\n");
                // Get the size of the object, and the current file position.
                fread(&Object_Size, sizeof(uint64_t), 1, ASF_File);
                ASF_File_Position = ftell(ASF_File);
                // Get the number of Descripions field we have, as we will need to cycle through them all.
                fread(&Content_Descriptors_Count, sizeof(uint16_t), 1, ASF_File);
                while(Content_Descriptors_Count--){
                    // These themselves are Objects within the main extended content header, which we need to handle.
                    // Format is:
                    // Descriptor Name Length (word)
                    // Descriptor Name (varies)
                    // Descriptor Value Type (word)
                    // Descriptor Value Length (word)
                    // Descriptor Value (varies - depend on Value Type).
                    Descriptor_Name_Length = 0;
                    Descriptor_Name = NULL;
                    Descriptor_Name_UTF16 = NULL;
                    Descriptor_Value_Type = 0;
                    Descriptor_Value_Length = 0;
                    Descriptor_Value = 0;
                    Descriptor_Value_Str = NULL;
                    Descriptor_Value_Str_UTF16 = NULL;
                    // Get our Descriptor Name.
                    fread(&Descriptor_Name_Length, sizeof(uint16_t), 1, ASF_File);
                    Descriptor_Name_UTF16 = g_malloc0(Descriptor_Name_Length + 0x10);
                    fread(Descriptor_Name_UTF16, Descriptor_Name_Length, 1, ASF_File);
                    Descriptor_Name = g_utf16_to_utf8 ((const gunichar2 *) Descriptor_Name_UTF16, Descriptor_Name_Length, NULL, NULL, NULL );
                    // Get our Value Type and Value Length
                    fread(&Descriptor_Value_Type, sizeof(uint16_t), 1, ASF_File);
                    fread(&Descriptor_Value_Length, sizeof(uint16_t), 1, ASF_File);
                    switch(Descriptor_Value_Type){
                        case 0: // String;
                        case 1: // Binary;
                            Descriptor_Value_Str_UTF16 = g_malloc0(Descriptor_Value_Length + 0x10);
                            fread(Descriptor_Value_Str_UTF16, Descriptor_Value_Length, 1, ASF_File);
                            Descriptor_Value_Str = g_utf16_to_utf8 ((const gunichar2 *) Descriptor_Value_Str_UTF16, Descriptor_Value_Length, NULL, NULL, NULL );
                            // We have out key=value pair so lets look for our desired  keys 'WM/AlbumTitle', 'WM/Genre' and 'WM/Year'
                            if(g_ascii_strcasecmp(Descriptor_Name, "WM/AlbumTitle\0") == 0){
                                // We have the album Title;
                                trackinformation->album = g_strdup(Descriptor_Value_Str);
                            } else {
                                if(g_ascii_strcasecmp(Descriptor_Name, "WM/Genre\0") == 0){
                                    // We have the album Genre;
                                    trackinformation->genre = g_strdup(Descriptor_Value_Str);
                                } else {
                                    if(g_ascii_strcasecmp(Descriptor_Name, "WM/Year\0") == 0){
                                        // We have the album Year;
                                        trackinformation->date = g_strdup(Descriptor_Value_Str);
                                    }
                                }
                            }
                            break;
                        case 2: // Boolean (DWORD)
                        case 3: // DWORD
                        case 4: // QWORD
                        case 5: // WORD
                            fread(&Descriptor_Value, Descriptor_Value_Length, 1, ASF_File);
                            if((g_ascii_strcasecmp(Descriptor_Name, "WM/Track\0") == 0)){
                                trackinformation->tracknumber = Descriptor_Value + 1;
                            } else {
                                if (g_ascii_strcasecmp(Descriptor_Name, "WM/TrackNumber\0") == 0)
                                    trackinformation->tracknumber = Descriptor_Value;
                            }
                            break;
                        default: // Unknown so skip it.
                            fseek(ASF_File, Descriptor_Value_Length, SEEK_CUR);
                            break;
                    }

                    // Free up our allocated memory;
                    g_free(Descriptor_Name);
                    g_free(Descriptor_Name_UTF16);
                    g_free(Descriptor_Value_Str);
                    g_free(Descriptor_Value_Str_UTF16);
                }

                // Set our file position so it's ready to read in the next GUID Header.
                fseek(ASF_File, ASF_File_Position, SEEK_SET);
                fseek(ASF_File, (Object_Size - sizeof(uint64_t) - sizeof(GUID)), SEEK_CUR);
            } else {
                // Skip this header;
                //g_printf("WMA: Unknown GUID - skipping\n");
                fread(&Object_Size, sizeof(uint64_t), 1, ASF_File);
                fseek(ASF_File, (Object_Size - sizeof(uint64_t) - sizeof(GUID)), SEEK_CUR);
            }
        }

    }
    fclose(ASF_File);
    return;
}
