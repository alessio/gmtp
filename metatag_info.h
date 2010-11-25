/* 
 * File:   metatag_info.h
 * Author: darran
 *
 * Created on November 11, 2010, 8:54 PM
 */

#ifndef _METATAG_INFO_H
#define	_METATAG_INFO_H

#ifdef	__cplusplus
extern "C" {
#endif

    
typedef struct {
    uint32_t f1;
    uint16_t f2;
    uint16_t f3;
    uint16_t f4;
    uint8_t f5_1;
    uint8_t f5_2;
    uint8_t f5_3;
    uint8_t f5_4;
    uint8_t f5_5;
    uint8_t f5_6;
} GUID;

// We only include our ASF header objects we want, not all of them

static const GUID ASF_header = {
    0x75B22630, 0x668E, 0x11CF, 0xA6D9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C 
};

static const GUID ASF_comment_header = {
    0x75B22633, 0x668E, 0x11CF, 0xD9A6, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C 
};

static const GUID ASF_extended_content_header = {
        0xD2D0A440, 0xE307, 0x11D2, 0xF097, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50 
};

gchar * ID3_getFrameText(struct id3_tag *tag, char *frame_name);
gchar * FLAC_getFieldText(const FLAC__StreamMetadata *tags, const char *name);
//gchar * OGG_getFieldText(const vorbis_comment *comments, const char *name);

void get_id3_tags(gchar *filename, LIBMTP_track_t *trackinformation);
void get_ogg_tags(gchar *filename, LIBMTP_track_t *trackinformation);
void get_flac_tags(gchar *filename, LIBMTP_track_t *trackinformation);
void get_asf_tags(gchar *filename, LIBMTP_track_t *trackinformation);

#ifdef	__cplusplus
}
#endif

#endif	/* _METATAG_INFO_H */

