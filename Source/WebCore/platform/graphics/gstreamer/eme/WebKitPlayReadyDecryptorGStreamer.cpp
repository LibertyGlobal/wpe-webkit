/* GStreamer PlayReady decryptor
 *
 * Copyright (C) 2015, 2016 Igalia S.L
 * Copyright (C) 2015, 2016 Metrological
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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */

#include "config.h"

#if (ENABLE(LEGACY_ENCRYPTED_MEDIA_V1) || ENABLE(LEGACY_ENCRYPTED_MEDIA) || ENABLE(ENCRYPTED_MEDIA)) && USE(GSTREAMER) && USE(PLAYREADY)
#include "WebKitPlayReadyDecryptorGStreamer.h"
#include "CDMProcessPayloadBase.h"
#include <gst/base/gstbytereader.h>
#include <gstsvpmeta.h>
#include "PlayreadySession.h"

#define WEBKIT_MEDIA_PLAYREADY_DECRYPT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WEBKIT_TYPE_MEDIA_PLAYREADY_DECRYPT, WebKitMediaPlayReadyDecryptPrivate))
struct _WebKitMediaPlayReadyDecryptPrivate
{
    WebCore::CDMProcessPayloadBase* sessionMetaData;
};

static void webKitMediaPlayReadyDecryptorFinalize(GObject*);
static gboolean webKitMediaPlayReadyDecryptorHandleKeyResponse(WebKitMediaCommonEncryptionDecrypt* self, GstEvent*);
static gboolean webKitMediaPlayReadyDecryptorDecrypt(WebKitMediaCommonEncryptionDecrypt*, GstBuffer* iv, GstBuffer* kid, GstBuffer* sample, unsigned subSamplesCount, GstBuffer* subSamples);

GST_DEBUG_CATEGORY(webkit_media_playready_decrypt_debug_category);
#define GST_CAT_DEFAULT webkit_media_playready_decrypt_debug_category

static GstStaticPadTemplate sinkTemplate = GST_STATIC_PAD_TEMPLATE("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("application/x-cenc, original-media-type=(string)video/x-h264, protection-system=(string)" PLAYREADY_PROTECTION_SYSTEM_UUID "; "
    "application/x-cenc, original-media-type=(string)audio/mpeg, protection-system=(string)" PLAYREADY_PROTECTION_SYSTEM_UUID
        ));

static GstStaticPadTemplate srcTemplate = GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("video/x-h264; audio/mpeg"));

#define webkit_media_playready_decrypt_parent_class parent_class
G_DEFINE_TYPE(WebKitMediaPlayReadyDecrypt, webkit_media_playready_decrypt, WEBKIT_TYPE_MEDIA_CENC_DECRYPT);

static void webkit_media_playready_decrypt_class_init(WebKitMediaPlayReadyDecryptClass* klass)
{
    WebKitMediaCommonEncryptionDecryptClass* cencClass = WEBKIT_MEDIA_CENC_DECRYPT_CLASS(klass);
    GstElementClass* elementClass = GST_ELEMENT_CLASS(klass);
    GObjectClass* gobjectClass = G_OBJECT_CLASS(klass);

    gobjectClass->finalize = webKitMediaPlayReadyDecryptorFinalize;

    gst_element_class_add_pad_template(elementClass, gst_static_pad_template_get(&sinkTemplate));
    gst_element_class_add_pad_template(elementClass, gst_static_pad_template_get(&srcTemplate));

    gst_element_class_set_static_metadata(elementClass,
        "Decrypt content encrypted using PlayReady Encryption",
        GST_ELEMENT_FACTORY_KLASS_DECRYPTOR,
        "Decrypts media that has been encrypted using PlayReady Encryption.",
        "Philippe Normand <philn@igalia.com>");

    GST_DEBUG_CATEGORY_INIT(webkit_media_playready_decrypt_debug_category,
        "webkitplayready", 0, "PlayReady decryptor");

    cencClass->protectionSystemId = PLAYREADY_PROTECTION_SYSTEM_UUID;
    cencClass->handleKeyResponse = GST_DEBUG_FUNCPTR(webKitMediaPlayReadyDecryptorHandleKeyResponse);
    cencClass->decrypt = GST_DEBUG_FUNCPTR(webKitMediaPlayReadyDecryptorDecrypt);

    g_type_class_add_private(klass, sizeof(WebKitMediaPlayReadyDecryptPrivate));
}

static void webkit_media_playready_decrypt_init(WebKitMediaPlayReadyDecrypt*)
{
}

static void webKitMediaPlayReadyDecryptorFinalize(GObject* object)
{
    WebKitMediaPlayReadyDecrypt* self = WEBKIT_MEDIA_PLAYREADY_DECRYPT(object);
    WebKitMediaPlayReadyDecryptPrivate* priv = self->priv;

    priv->~WebKitMediaPlayReadyDecryptPrivate();

    GST_CALL_PARENT(G_OBJECT_CLASS, finalize, (object));
}

static gboolean webKitMediaPlayReadyDecryptorHandleKeyResponse(WebKitMediaCommonEncryptionDecrypt* self, GstEvent* event)
{
    WebKitMediaPlayReadyDecryptPrivate* priv = WEBKIT_MEDIA_PLAYREADY_DECRYPT_GET_PRIVATE(WEBKIT_MEDIA_PLAYREADY_DECRYPT(self));

    const GstStructure* structure = gst_event_get_structure(event);
    const char* label = "cdm-session";

    if (!gst_structure_has_name(structure, label))
        return FALSE;

    GST_INFO_OBJECT(self, "received %s", label);

    const GValue* value = gst_structure_get_value(structure, "session");
    priv->sessionMetaData = reinterpret_cast<WebCore::CDMProcessPayloadBase*>(g_value_get_pointer(value));
    return TRUE;
}

static void buffer_release_fn(GstStructure *s)
{
    GST_DEBUG("struct=%p", s);
    if (gst_structure_has_name(s, GST_SVP_SYSTEM_ID_CAPS_FIELD))
    {
        GstBuffer *samples;
        void* secure;
        gboolean valid = gst_structure_get(s, "secure_buffer", G_TYPE_POINTER, &secure, "chunks_info", GST_TYPE_BUFFER, &samples, nullptr);
        GST_DEBUG("secure=%p, samples=%p", secure, samples);
        if (valid)
        {
//             freed internally - see patch 0018
            if (secure != NULL)
                WebCore::PlayreadySession::freeDecrypted(secure);
            if (samples != NULL)
                gst_buffer_unref(samples);
        }
    }
    else
        GST_ERROR("no name " GST_SVP_SYSTEM_ID_CAPS_FIELD);
}

static gboolean webKitMediaPlayReadyDecryptorDecrypt(WebKitMediaCommonEncryptionDecrypt* self, GstBuffer* ivBuffer, GstBuffer* /*kid*/, GstBuffer* buffer, unsigned subSampleCount, GstBuffer* subSamplesBuffer)
{
    WebKitMediaPlayReadyDecryptPrivate* priv = WEBKIT_MEDIA_PLAYREADY_DECRYPT_GET_PRIVATE(WEBKIT_MEDIA_PLAYREADY_DECRYPT(self));
    GstMapInfo map, ivMap, subSamplesMap;
    unsigned position = 0;
    GstByteReader* reader = nullptr;
    gboolean bufferMapped, subsamplesBufferMapped;
    int errorCode;
    guint32 totalEncrypted = 0;
    uint8_t* encryptedData;
    uint8_t* fEncryptedData;
    unsigned index = 0;
//    unsigned total = 0;
    void* decrypted = NULL;
    guint32 *svpSubsamplesBuffer = nullptr;

    GST_DEBUG_OBJECT(self, "subsamples: buffer=%p, count=%d", subSamplesBuffer, subSampleCount);

    if (!gst_buffer_map(ivBuffer, &ivMap, GST_MAP_READ))
    {
        GST_ERROR_OBJECT(self, "Failed to map IV");
        return false;
    }

    bufferMapped = gst_buffer_map(buffer, &map, static_cast<GstMapFlags>(GST_MAP_READWRITE));
    if (!bufferMapped)
    {
        GST_ERROR_OBJECT(self, "Failed to map buffer");
        gst_buffer_unmap(ivBuffer, &ivMap);
        return false;
    }

    GST_DEBUG_OBJECT(self, "gstbuffer: %p %d", map.data, map.size);

    if (subSamplesBuffer)
    {
        subsamplesBufferMapped = gst_buffer_map(subSamplesBuffer, &subSamplesMap, GST_MAP_READ);
        if (!subsamplesBufferMapped)
        {
            GST_ERROR_OBJECT(self, "Failed to map subsample buffer");
            gst_buffer_unmap(ivBuffer, &ivMap);
            gst_buffer_unmap(buffer, &map);
            return false;
        }

        reader = gst_byte_reader_new(subSamplesMap.data, subSamplesMap.size);

        guint32 *svpp = svpSubsamplesBuffer = (guint32*) g_malloc(subSampleCount * 3 * sizeof(guint32));

        // Find out the total size of the encrypted data.
        for (position = 0; position < subSampleCount; position++)
        {
            guint16 inClear = 0;
            guint32 inEncrypted = 0;
            gst_byte_reader_get_uint16_be(reader, &inClear);
            gst_byte_reader_get_uint32_be(reader, &inEncrypted);
            *svpp++ = inClear;
            *svpp++ = inEncrypted;
            *svpp++ = index;
            totalEncrypted += inEncrypted;
            index += inClear + inEncrypted;
        }

        // Build a new buffer storing the entire encrypted cipher.
        encryptedData = fEncryptedData = (uint8_t*) g_malloc(totalEncrypted);

        for (position = 0; position < subSampleCount; position++)
        {
            memcpy(encryptedData, map.data + svpSubsamplesBuffer[3*position+2]+ svpSubsamplesBuffer[3*position], svpSubsamplesBuffer[3*position+1]);
            encryptedData += svpSubsamplesBuffer[3*position+1];
            GST_DEBUG_OBJECT(self, "svpSubsamplesBuffer: i=%d  %d/%d/%d", position, svpSubsamplesBuffer[3*position], svpSubsamplesBuffer[3*position+1], svpSubsamplesBuffer[3*position+2]);
        }

        // Decrypt cipher.
        ASSERT(priv->sessionMetaData);
        if ((errorCode = priv->sessionMetaData->processPayload(static_cast<const void*>(ivMap.data), static_cast<uint32_t>(ivMap.size), NULL, 0, static_cast<void*>(fEncryptedData), static_cast<uint32_t>(totalEncrypted), &decrypted)))
        {
            GST_WARNING_OBJECT(self, "ERROR - packet decryption failed [%d]", errorCode);
            g_free(fEncryptedData);
            g_free(svpSubsamplesBuffer);
            gst_byte_reader_free(reader);
            gst_buffer_unmap(buffer, &map);
            gst_buffer_unmap(subSamplesBuffer, &subSamplesMap);
            gst_buffer_unmap(ivBuffer, &ivMap);
            return false;
        }
        g_free(fEncryptedData);
        gst_buffer_unmap(subSamplesBuffer, &subSamplesMap);
    }
    else
    {

        svpSubsamplesBuffer = (guint32*) g_malloc(3 * sizeof(guint32));
        svpSubsamplesBuffer[0] = 0;
        svpSubsamplesBuffer[1] = map.size;
        svpSubsamplesBuffer[2] = 0;
        subSampleCount = 1;

        for (unsigned i = 0 ; i < subSampleCount; i++)
            GST_DEBUG_OBJECT(self, "svpSubsamplesBuffer: i=%d  %d/%d/%d", i, svpSubsamplesBuffer[3*i], svpSubsamplesBuffer[3*i+1], svpSubsamplesBuffer[3*i+2]);

        // Decrypt cipher.
//        ASSERT(priv->sessionMetaData);
        if ((errorCode = priv->sessionMetaData->processPayload(static_cast<const void*>(ivMap.data), static_cast<uint32_t>(ivMap.size), NULL, 0, static_cast<void*>(map.data), static_cast<uint32_t>(map.size), &decrypted)))
        {
            GST_WARNING_OBJECT(self, "ERROR - packet decryption failed [%d]", errorCode);
            g_free(fEncryptedData);
            g_free(svpSubsamplesBuffer);
            gst_buffer_unmap(buffer, &map);
            gst_buffer_unmap(ivBuffer, &ivMap);
            return false;
        }
    }


    if (decrypted != NULL)
    {
        GstBuffer* b = gst_buffer_new_wrapped(svpSubsamplesBuffer, subSampleCount * 3 * sizeof(guint32));
        GST_DEBUG_OBJECT(self, "decrypted=%p, subsample_buffer=%p, mem=%p, count=%d", decrypted, b, svpSubsamplesBuffer, subSampleCount);
//         fprintf(stderr, "decrypted=%p, subsample_buffer=%p, mem=%p, count=%d\n", decrypted, b, svpSubsamplesBuffer, subSampleCount);
        gst_buffer_add_svp_meta(buffer,
                                gst_structure_new(GST_SVP_SYSTEM_ID_CAPS_FIELD,
                                        "secure_buffer", G_TYPE_POINTER, decrypted,
                                        "chunks_info", GST_TYPE_BUFFER, b,
                                        "chunks_cnt", G_TYPE_UINT, subSampleCount, nullptr),
                                buffer_release_fn);
    }
    else
    {
        GST_WARNING_OBJECT(self, "decrypted=%p, mem=%p, count=%d", decrypted, svpSubsamplesBuffer, subSampleCount);
        g_free(svpSubsamplesBuffer);
    }

    if (reader)
        gst_byte_reader_free(reader);

    gst_buffer_unmap(buffer, &map);
    gst_buffer_unmap(ivBuffer, &ivMap);

    return true;
}

bool webkit_media_playready_decrypt_is_playready_key_system_id(const gchar* keySystemId)
{
    return g_strcmp0(keySystemId, PLAYREADY_PROTECTION_SYSTEM_UUID) == 0;
}

#endif
