#include <stdio.h>
#include <stdlib.h>
#include <gst/gst.h>
#include "config.h"
#include "WidevineSession.h"

#if USE(WIDEVINE)
#include "wtf/UUID.h"

// #include <bkni.h>
#include <nexus_dma.h>

#include <runtime/JSCInlines.h>
#include <runtime/TypedArrayInlines.h>
#include <wtf/text/CString.h>

#include <nexus_dma.h>
#include "nexus_config.h"
#include "nexus_base_os.h"
#include <sage_srai.h>

GST_DEBUG_CATEGORY_EXTERN(webkit_media_widevine_decrypt_debug_category);
#define GST_CAT_DEFAULT webkit_media_widevine_decrypt_debug_category

namespace WebCore
{

#define ChkDRM(x) if((dr = (x)) != DRM_Prdy_ok) break;
#define ChkBOOL(x,y) if (!(x)) { dr = y; break;};

WidevineSession::WidevineSession(const Vector<uint8_t> &initData, const void* pipeline)
    : m_key()
    , m_eKeyState(KEY_INIT)
    , m_fCommit(false)
    , m_sessionId(createCanonicalUUIDString())
    , m_initData(initData)
    , m_pipeline(pipeline)
{
}

WidevineSession::~WidevineSession()
{
}

RefPtr<Uint8Array> WidevineSession::widevineGenerateKeyRequest(Uint8Array* initData, const String& customData, String& destinationURL, unsigned short& errorCode, uint32_t& systemCode)
{
    RefPtr<Uint8Array> result;
    return result;
}

bool WidevineSession::widevineProcessKey(Uint8Array* key, RefPtr<Uint8Array>& nextMessage, unsigned short& errorCode, uint32_t& systemCode)
{
    return false;
}

int WidevineSession::processPayload(const void* iv, uint32_t ivSize, void* payloadData, uint32_t payloadDataSize, void** decrypted)
{
    return 0;
}

void WidevineSession::freeDecrypted(void* decrypted)
{
    GST_DEBUG("%p", decrypted);
    SRAI_Memory_Free((uint8_t *)decrypted);
}


}

#endif
