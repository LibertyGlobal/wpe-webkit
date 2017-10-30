#include "config.h"

#if (ENABLE(LEGACY_ENCRYPTED_MEDIA_V1) || ENABLE(LEGACY_ENCRYPTED_MEDIA) || ENABLE(ENCRYPTED_MEDIA)) && USE(GSTREAMER) && USE(WIDEVINE)
#include "WebKitWidevineDecryptorGStreamer.h"
#include "CDMProcessPayloadBase.h"
#include <gst/base/gstbytereader.h>
#include <gstsvpmeta.h>

#define WEBKIT_MEDIA_WIDEVINE_DECRYPT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WEBKIT_TYPE_MEDIA_WIDEVINE_DECRYPT, WebKitMediaWidevineDecryptPrivate))
struct _WebKitMediaWidevineDecryptPrivate
{
    WebCore::CDMProcessPayloadBase* sessionMetaData;
};

static void webKitMediaWidevineDecryptorFinalize(GObject*);
static gboolean webKitMediaWidevineDecryptorHandleKeyResponse(WebKitMediaCommonEncryptionDecrypt* self, GstEvent*);
static gboolean webKitMediaWidevineDecryptorDecrypt(WebKitMediaCommonEncryptionDecrypt*, GstBuffer* iv, GstBuffer* kid, GstBuffer* sample, unsigned subSamplesCount, GstBuffer* subSamples);

GST_DEBUG_CATEGORY(webkit_media_widevine_decrypt_debug_category);
#define GST_CAT_DEFAULT webkit_media_widevine_decrypt_debug_category

static GstStaticPadTemplate sinkTemplate = GST_STATIC_PAD_TEMPLATE("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("application/x-cenc, original-media-type=(string)video/webm, protection-system=(string)" WIDEVINE_PROTECTION_SYSTEM_UUID "; "
    "application/x-cenc, original-media-type=(string)video/mp4, protection-system=(string)" WIDEVINE_PROTECTION_SYSTEM_UUID "; "
    "application/x-cenc, original-media-type=(string)audio/webm, protection-system=(string)" WIDEVINE_PROTECTION_SYSTEM_UUID "; "
    "application/x-cenc, original-media-type=(string)audio/mp4, protection-system=(string)" WIDEVINE_PROTECTION_SYSTEM_UUID "; "
    "application/x-cenc, original-media-type=(string)video/x-h264, protection-system=(string)" WIDEVINE_PROTECTION_SYSTEM_UUID "; "
    "application/x-cenc, original-media-type=(string)audio/mpeg, protection-system=(string)" WIDEVINE_PROTECTION_SYSTEM_UUID ";"
    "application/x-cenc, original-media-type=(string)audio/x-opus, protection-system=(string)" WIDEVINE_PROTECTION_SYSTEM_UUID ";"
    "application/x-cenc, original-media-type=(string)video/x-vp9, protection-system=(string)" WIDEVINE_PROTECTION_SYSTEM_UUID ";"));

static GstStaticPadTemplate srcTemplate = GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("video/webm; audio/webm; video/mp4; audio/mp4; audio/mpeg; video/x-h264; audio/x-opus; video/x-vp9"));

#define webkit_media_widevine_decrypt_parent_class parent_class
G_DEFINE_TYPE(WebKitMediaWidevineDecrypt, webkit_media_widevine_decrypt, WEBKIT_TYPE_MEDIA_CENC_DECRYPT);

static void webkit_media_widevine_decrypt_class_init(WebKitMediaWidevineDecryptClass* klass)
{
    GstElementClass* elementClass = GST_ELEMENT_CLASS(klass);
    GObjectClass* gobjectClass = G_OBJECT_CLASS(klass);

    gobjectClass->finalize = webKitMediaWidevineDecryptorFinalize;

    gst_element_class_add_pad_template(elementClass, gst_static_pad_template_get(&sinkTemplate));
    gst_element_class_add_pad_template(elementClass, gst_static_pad_template_get(&srcTemplate));


    gst_element_class_set_static_metadata(elementClass,
        "Decrypt content encrypted using Widevine DRM system(SVP)",
        GST_ELEMENT_FACTORY_KLASS_DECRYPTOR,
        "Decrypts media that has been encrypted using Widevine DRM system(SVP)",
        "Zbigniew Holdys <zbigniew.holdys@redembedded.com>");

    GST_DEBUG_CATEGORY_INIT(webkit_media_widevine_decrypt_debug_category,
        "webkitwidevine", 0, "Widevine decryptor");

    WebKitMediaCommonEncryptionDecryptClass* cencClass = WEBKIT_MEDIA_CENC_DECRYPT_CLASS(klass);

    cencClass->protectionSystemId = WIDEVINE_PROTECTION_SYSTEM_UUID;
    cencClass->handleKeyResponse = GST_DEBUG_FUNCPTR(webKitMediaWidevineDecryptorHandleKeyResponse);
    cencClass->decrypt = GST_DEBUG_FUNCPTR(webKitMediaWidevineDecryptorDecrypt);

    g_type_class_add_private(klass, sizeof(WebKitMediaWidevineDecryptPrivate));
}

static void webkit_media_widevine_decrypt_init(WebKitMediaWidevineDecrypt*)
{
}

static void webKitMediaWidevineDecryptorFinalize(GObject* object)
{
    WebKitMediaWidevineDecrypt* self = WEBKIT_MEDIA_WIDEVINE_DECRYPT(object);
    WebKitMediaWidevineDecryptPrivate* priv = self->priv;

    priv->~WebKitMediaWidevineDecryptPrivate();

    GST_CALL_PARENT(G_OBJECT_CLASS, finalize, (object));
}

static gboolean webKitMediaWidevineDecryptorHandleKeyResponse(WebKitMediaCommonEncryptionDecrypt* self, GstEvent* event)
{
    WebKitMediaWidevineDecryptPrivate* priv = WEBKIT_MEDIA_WIDEVINE_DECRYPT_GET_PRIVATE(WEBKIT_MEDIA_WIDEVINE_DECRYPT(self));

    const GstStructure* structure = gst_event_get_structure(event);
    const char* label = "cdm-session";

    if (!gst_structure_has_name(structure, label))
        return FALSE;

    const GValue* value = gst_structure_get_value(structure, "session");
    priv->sessionMetaData = reinterpret_cast<WebCore::CDMProcessPayloadBase*>(g_value_get_pointer(value));

    GST_INFO_OBJECT(self, "received %s, %p", label, priv->sessionMetaData);

    return TRUE;
}

// static void buffer_release_fn(GstStructure *s)
// {
//     GST_DEBUG("struct=%p", s);
//     if (gst_structure_has_name(s, GST_SVP_SYSTEM_ID_CAPS_FIELD))
//     {
//         GstBuffer *samples;
//         void* secure;
//         gboolean valid = gst_structure_get(s, "secure_buffer", G_TYPE_POINTER, &secure, "chunks_info", GST_TYPE_BUFFER, &samples, nullptr);
//         GST_DEBUG("secure=%p, samples=%p", secure, samples);
//         if (valid)
//         {
//             if (secure != NULL)
//                 WebCore::WidevineSession::freeDecrypted(secure);
//             if (samples != NULL)
//                 gst_buffer_unref(samples);
//         }
//     }
//     else
//         GST_ERROR("no name " GST_SVP_SYSTEM_ID_CAPS_FIELD);
// }

static gboolean webKitMediaWidevineDecryptorDecrypt(WebKitMediaCommonEncryptionDecrypt* self, GstBuffer* ivBuffer, GstBuffer* kid, GstBuffer* buffer, unsigned subSampleCount, GstBuffer* subSamplesBuffer)
{
    WebKitMediaWidevineDecryptPrivate* priv = WEBKIT_MEDIA_WIDEVINE_DECRYPT_GET_PRIVATE(WEBKIT_MEDIA_WIDEVINE_DECRYPT(self));
    GstMapInfo map, ivMap, subSamplesMap, kidMap;
    unsigned position = 0, partitions = subSampleCount;
    GstByteReader* reader = nullptr;
    gboolean bufferMapped, subsamplesBufferMapped;
    int errorCode;
    guint32 offset = 0, last_offset = 0;
    guint32 totalEncrypted = 0;
    uint8_t* encryptedData;
    uint8_t* fEncryptedData;
    unsigned index = 0, diff = 0;
//    unsigned total = 0;
    void* decrypted = NULL;
    guint32 *svpSubsamplesBuffer = nullptr;
    if( !priv->sessionMetaData->isCenc() )
        subSampleCount = partitions / 2 + 1;

    GST_DEBUG_OBJECT(self, "subsamples: buffer=%p, count=%d", subSamplesBuffer, subSampleCount);

    if (!gst_buffer_map(ivBuffer, &ivMap, GST_MAP_READ))
    {
        GST_ERROR_OBJECT(self, "Failed to map IV");
        return false;
    }

    if (!gst_buffer_map(kid, &kidMap, GST_MAP_READ))
    {
        GST_ERROR_OBJECT(self, "Failed to map kid");
        gst_buffer_unmap(ivBuffer, &ivMap);
        return false;
    }

    bufferMapped = gst_buffer_map(buffer, &map, static_cast<GstMapFlags>(GST_MAP_READWRITE));
    if (!bufferMapped)
    {
        GST_ERROR_OBJECT(self, "Failed to map buffer");
        gst_buffer_unmap(ivBuffer, &ivMap);
        gst_buffer_unmap(kid, &kidMap);
        return false;
    }

    GST_DEBUG_OBJECT(self, "gstbuffer: %p %p %d", buffer, map.data, map.size);

    if (subSamplesBuffer)
    {
        subsamplesBufferMapped = gst_buffer_map(subSamplesBuffer, &subSamplesMap, GST_MAP_READ);
        if (!subsamplesBufferMapped)
        {
            GST_ERROR_OBJECT(self, "Failed to map subsample buffer");
            gst_buffer_unmap(ivBuffer, &ivMap);
            gst_buffer_unmap(kid, &kidMap);
            gst_buffer_unmap(buffer, &map);
            return false;
        }

        reader = gst_byte_reader_new(subSamplesMap.data, subSamplesMap.size);

        // Find out the total size of the encrypted data.
        guint32 *svpp = svpSubsamplesBuffer = (guint32*) g_malloc(subSampleCount * 3 * sizeof(guint32));

        ASSERT(priv->sessionMetaData);
        if( priv->sessionMetaData->isCenc() ) {
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
        } else {    // webm subsamples
            for (position = 0; position < partitions; position++)
            {
                gst_byte_reader_get_uint32_be(reader, &offset);
                diff = offset - last_offset;
                *svpp++ = diff;
                if( position % 2 == 0 ) {       // clear just added, svpp set on encrypted
                    svpp[1] = index;            // points to index;
                } else {
                    totalEncrypted += diff;     // encrypted just added
                    svpp++;                     // jump over index;
                }
                index  += diff;
                last_offset = offset;
            }
            if( partitions % 2 ) {
                *svpp = map.size - offset;      // remainder is encrypted
                totalEncrypted += *svpp;        // encrypted just added
            } else {
                svpp[ 0 ] = map.size - offset;  // remainder is clear
                svpp[ 1 ] = 0;                  // no encrypted part
                svpp[ 2 ] = index;              // index forwarded from last subsample
            }
        }

        // Build a new buffer storing the entire encrypted cipher.
        encryptedData = fEncryptedData = (uint8_t*) g_malloc(totalEncrypted);

        for (position = 0; position < subSampleCount; position++)
        {
            GST_DEBUG_OBJECT(self, "svpSubsamplesBuffer: i=%d  %d/%d/%d", position, svpSubsamplesBuffer[3*position], svpSubsamplesBuffer[3*position+1], svpSubsamplesBuffer[3*position+2]);
            memcpy(encryptedData, map.data + svpSubsamplesBuffer[3*position+2]+ svpSubsamplesBuffer[3*position], svpSubsamplesBuffer[3*position+1]);
            encryptedData += svpSubsamplesBuffer[3*position+1];
        }

        // Decrypt cipher.
        if ((errorCode = priv->sessionMetaData->processPayload(static_cast<const void*>(ivMap.data), static_cast<uint32_t>(ivMap.size), static_cast<const void*>(kidMap.data), static_cast<uint32_t>(kidMap.size), static_cast<void*>(fEncryptedData), static_cast<uint32_t>(totalEncrypted), &decrypted)))
        {
            GST_WARNING_OBJECT(self, "ERROR - packet decryption failed [%d]", errorCode);
            g_free(fEncryptedData);
            g_free(svpSubsamplesBuffer);
            gst_byte_reader_free(reader);
            gst_buffer_unmap(buffer, &map);
            gst_buffer_unmap(subSamplesBuffer, &subSamplesMap);
            gst_buffer_unmap(ivBuffer, &ivMap);
            gst_buffer_unmap(kid, &kidMap);
            return false;
        }
        if( !decrypted ) // decrypted in place!
        {
            encryptedData = fEncryptedData;
            for (position = 0; position < subSampleCount; position++)
            {
                memcpy(map.data + svpSubsamplesBuffer[3*position+2] + svpSubsamplesBuffer[3*position], encryptedData, svpSubsamplesBuffer[3*position+1]);
                encryptedData += svpSubsamplesBuffer[3*position+1];
            }

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
        if ((errorCode = priv->sessionMetaData->processPayload(static_cast<const void*>(ivMap.data), static_cast<uint32_t>(ivMap.size), static_cast<const void*>(kidMap.data), static_cast<uint32_t>(kidMap.size), static_cast<void*>(map.data), static_cast<uint32_t>(map.size), &decrypted)))
        {
            GST_WARNING_OBJECT(self, "ERROR - packet decryption failed [%d]", errorCode);
            g_free(fEncryptedData);
            g_free(svpSubsamplesBuffer);
            gst_buffer_unmap(buffer, &map);
            gst_buffer_unmap(ivBuffer, &ivMap);
            gst_buffer_unmap(kid, &kidMap);
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
                                nullptr/*buffer_release_fn*/);
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
    gst_buffer_unmap(kid, &kidMap);

    return true;
}

bool webkit_media_widevine_decrypt_is_widevine_key_system_id(const gchar* keySystemId)
{
    return g_strcmp0(keySystemId, WIDEVINE_PROTECTION_SYSTEM_UUID) == 0;
}

#endif
