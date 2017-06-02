/* GStreamer
 * Copyright (C) <2016> Liberty Global
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_SVP_META_H__
#define __GST_SVP_META_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_SVP_SYSTEM_ID_CAPS_FIELD "svp-system"

typedef struct _GstSvpMeta GstSvpMeta;

typedef void GstSvpMetaCustomRelease(GstStructure *info);

struct _GstSvpMeta
{
  GstMeta meta;
  GstStructure            *info;
  GstSvpMetaCustomRelease *release;
};

GType gst_svp_meta_api_get_type (void);

#define GST_SVP_META_API_TYPE (gst_svp_meta_api_get_type())

#define gst_buffer_get_svp_meta(b) ((GstSvpMeta*)gst_buffer_get_meta ((b), GST_SVP_META_API_TYPE))

#define GST_SVP_META_INFO (gst_svp_meta_get_info())

const GstMetaInfo *gst_svp_meta_get_info (void);

/*
 *  GstBuffer               *buffer  - buffer to which meta data will be added
 *  GstStructure            *info    - metada structure 
 *                                     This structure should contain:
 *                                        "secure_buffer"  : G_TYPE_POINTER
 *                                        "chunks_cnt"     : G_TYPE_UINT
 *                                        "chunks_info"    : GST_TYPE_BUFFER
 *                                                           data of this buffer contains
 *                                                           for (i=0; i<chunks_cnt; i++) {
 *                                                              guint32 clear_size;
 *                                                              guint32 encrypted_size;
 *                                                              guint32 secure_buffer_offset;
 *                                                           }
 *                                                           "clear size"           : size of clear data part in "buffer.data"
 *                                                           "encrypted_size"       : size of encrypted data part in "buffer.data" available as clear in "secure_buffer"
 *                                                           "secure_buffer_offset" : offset from the begining of "secure_buffer" where decrypted data of encrypted part is stored
 *                                                                                    data in secure buffer can be stored with an alignment, so this offset should be used to access
 *                                                                                    begining of decrypted part
 *
 *                                                            buffer.data   = {clear_1, encrytped_1, clear_2, encrypted_2, ...}
 *                                                            secure_buffer = {decrypted_1,decrytped_2 , ...}
 *                                                                            "decrypted_N" is clear data after decrypting "encrypted_N"
 * 
 *  GstSvpMetaCustomRelease *release - custom fuction to release any custom specific data present in "info"
 */
GstSvpMeta *gst_buffer_add_svp_meta (GstBuffer * buffer,  GstStructure * info, GstSvpMetaCustomRelease *release);

G_END_DECLS
#endif /* __GST_SVP_META_H__ */
