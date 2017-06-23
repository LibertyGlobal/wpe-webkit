#ifndef WebKitWidevineDecryptorGStreamer_h
#define WebKitWidevineDecryptorGStreamer_h

#if (ENABLE(LEGACY_ENCRYPTED_MEDIA_V1) || ENABLE(LEGACY_ENCRYPTED_MEDIA) || ENABLE(ENCRYPTED_MEDIA)) && USE(GSTREAMER) && USE(WIDEVINE)

#include "WebKitCommonEncryptionDecryptorGStreamer.h"

#define WIDEVINE_PROTECTION_SYSTEM_UUID "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"
#define WIDEVINE_PROTECTION_SYSTEM_ID "com.widevine.alpha"

G_BEGIN_DECLS

#define WEBKIT_TYPE_MEDIA_WIDEVINE_DECRYPT          (webkit_media_widevine_decrypt_get_type())
#define WEBKIT_MEDIA_WIDEVINE_DECRYPT(obj)          (G_TYPE_CHECK_INSTANCE_CAST((obj), WEBKIT_TYPE_MEDIA_WIDEVINE_DECRYPT, WebKitMediaWidevineDecrypt))
#define WEBKIT_MEDIA_WIDEVINE_DECRYPT_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass), WEBKIT_TYPE_MEDIA_WIDEVINE_DECRYPT, WebKitMediaWidevineDecryptClass))
#define WEBKIT_IS_MEDIA_WIDEVINE_DECRYPT(obj)       (G_TYPE_CHECK_INSTANCE_TYPE((obj), WEBKIT_TYPE_MEDIA_WIDEVINE_DECRYPT))
#define WEBKIT_IS_MEDIA_WIDEVINE_DECRYPT_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), WEBKIT_TYPE_MEDIA_WIDEVINE_DECRYPT))

typedef struct _WebKitMediaWidevineDecrypt        WebKitMediaWidevineDecrypt;
typedef struct _WebKitMediaWidevineDecryptClass   WebKitMediaWidevineDecryptClass;
typedef struct _WebKitMediaWidevineDecryptPrivate WebKitMediaWidevineDecryptPrivate;

GType webkit_media_widevine_decrypt_get_type(void);

struct _WebKitMediaWidevineDecrypt {
    WebKitMediaCommonEncryptionDecrypt parent;

    WebKitMediaWidevineDecryptPrivate* priv;
};

struct _WebKitMediaWidevineDecryptClass {
    WebKitMediaCommonEncryptionDecryptClass parentClass;
};

bool webkit_media_Widevine_decrypt_is_widevine_key_system_id(const gchar* keySystemId);

G_END_DECLS


#endif // (ENABLE(LEGACY_ENCRYPTED_MEDIA_V1) || ENABLE(LEGACY_ENCRYPTED_MEDIA) || ENABLE(ENCRYPTED_MEDIA)) && USE(GSTREAMER) && USE(WIDEVINE)
#endif // WebKitWidevineDecryptorGStreamer_h