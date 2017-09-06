#define TARGET_LITTLE_ENDIAN 1
#define BSTD_CPU_ENDIAN BSTD_ENDIAN_LITTLE

#include <stdio.h>
#include <stdlib.h>
#include <gst/gst.h>

#include "config.h"
#include "WidevineSession.h"

#if USE(WIDEVINE)
#include "wtf/UUID.h"

#include <nexus_dma.h>
#include "nexus_config.h"
#include "nexus_memory.h"
#include "nexus_base_os.h"
#include <sage_srai.h>

GST_DEBUG_CATEGORY_EXTERN(webkit_media_widevine_decrypt_debug_category);
#define GST_CAT_DEFAULT webkit_media_widevine_decrypt_debug_category

namespace WebCore
{
widevine::Cdm* getWidevineCDM();

WidevineSession::WidevineSession(const Vector<uint8_t> &initData, const void* pipeline)
    : m_key()
    , m_eKeyState(KEY_INIT)
    , m_fCommit(false)
    , m_sessionId(createCanonicalUUIDString())
    , m_initData(initData)
    , m_pipeline(pipeline)
{
    std::string sessionId;
    if( getWidevineCDM()->createSession( widevine::Cdm::kTemporary, &sessionId ) != widevine::Cdm::kSuccess ) {
        m_eKeyState = KEY_ERROR;
        fprintf(stderr,"ERROR WidevineSession[%4d]::%s\n",__LINE__,__FUNCTION__);
    } else
        m_sessionId = sessionId.c_str();
}

WidevineSession::~WidevineSession() {
    getWidevineCDM()->close( m_sessionId.utf8().data() );
}

void WidevineSession::onMessageUrl(const std::string& session_id, const std::string& server_url) {
    fprintf(stderr,"Not Implemented - [widevine::Cdm::IEventListener] WidevineSession::%s()\n",__FUNCTION__);
}
void WidevineSession::onMessage(const std::string& session_id, widevine::Cdm::MessageType message_type, const std::string& message) {
    fprintf(stderr,"Not Implemented - [widevine::Cdm::IEventListener] WidevineSession::%s(), %d, %p\n",__FUNCTION__,message_type,m_result);
    if( m_result )
        *m_result = Uint8Array::create( reinterpret_cast<const unsigned char*>(message.c_str()), message.size() );
}
void WidevineSession::onKeyStatusesChange(const std::string& session_id) {
    fprintf(stderr,"Not Implemented - [widevine::Cdm::IEventListener] WidevineSession::%s()\n",__FUNCTION__);
    getWidevineCDM()->getKeyStatuses( session_id, &m_keyMap );
    fprintf(stderr,"Not Implemented - [widevine::Cdm::IEventListener] WidevineSession::%s()\n",__FUNCTION__);
    for( widevine::Cdm::KeyStatusMap::iterator it = m_keyMap.begin(); it != m_keyMap.end(); ++it ) {
        fprintf(stderr,"key found: \"%s\", %d\n",it->first.c_str(),it->second);
        if( it->second == widevine::Cdm::kUsable ) {
            m_key = ArrayBuffer::create( reinterpret_cast<const uint8_t*>(it->first.c_str()), it->first.size() );
            break;
        }
    }
}
void WidevineSession::onRemoveComplete(const std::string& session_id) {
    fprintf(stderr,"Not Implemented - [widevine::Cdm::IEventListener] WidevineSession::%s()\n",__FUNCTION__);
}

RefPtr<Uint8Array> WidevineSession::widevineGenerateKeyRequest(Uint8Array* initData, const String& customData, String& destinationURL, unsigned short& errorCode, uint32_t& systemCode)
{
    RefPtr<Uint8Array> result;
    m_result = &result;
    if( getWidevineCDM()->generateRequest( m_sessionId.utf8().data(), widevine::Cdm::kCenc, std::string(reinterpret_cast<char*>(initData->data()), initData->byteLength()) ) != widevine::Cdm::kSuccess ) {
        fprintf(stderr,"ERROR WidevineSession[%4d]::%s\n",__LINE__,__FUNCTION__);
    } else
        m_eKeyState = KEY_PENDING;
    fprintf(stderr,"WidevineSession: request created\n");
    m_result = nullptr;
    return result;
}

bool WidevineSession::widevineProcessKey(Uint8Array* key, RefPtr<Uint8Array>& nextMessage, unsigned short& errorCode, uint32_t& systemCode)
{
    m_result = &nextMessage;
    bool ret = true;
    if( getWidevineCDM()->update( m_sessionId.utf8().data(), std::string(reinterpret_cast<char*>(key->data()), key->byteLength()) ) != widevine::Cdm::kSuccess ) {
        fprintf(stderr,"WidevineSession: key FAIL\n");
        ret = false;
    } else if( m_key && m_key->byteLength() > 0 )
        m_eKeyState = KEY_READY;
    fprintf(stderr,"WidevineSession: key handled\n");
    m_result = nullptr;
    return ret;
}

int WidevineSession::processPayload(const void* iv, uint32_t ivSize, void* payloadData, uint32_t payloadDataSize, void** decrypted)
{
    widevine::Cdm::Status status = widevine::Cdm::kSuccess;
    uint8_t *nexus_heap = NULL;

    GST_DEBUG("payloadData=%p, size=%d", payloadData, payloadDataSize);
    //    fprintf(stderr,"payloadData=%p, size=%d", payloadData, payloadDataSize);
    do
    {
        widevine::Cdm::InputBuffer ib;
        ib.key_id           = reinterpret_cast<const uint8_t*>(m_key->data());
        ib.key_id_length    = m_key->byteLength();
        ib.iv               = reinterpret_cast<const uint8_t*>(iv);
        ib.iv_length        = ivSize;
        NEXUS_MemoryAllocationSettings memSettings;
        *decrypted = NULL;

        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
        memSettings.heap = NEXUS_Heap_Lookup(NEXUS_HeapLookupType_eMain);

        if( NEXUS_Memory_Allocate(payloadDataSize, &memSettings, (void**)&nexus_heap) !=  NEXUS_SUCCESS)
        {
            GST_ERROR("NEXUS_Memory_Allocate failed");
            break;
        }
        memcpy(nexus_heap, payloadData, payloadDataSize);

        GST_DEBUG("allocate srai");
        *decrypted = SRAI_Memory_Allocate(payloadDataSize, SRAI_MemoryType_SagePrivate);
        if (*decrypted == NULL)
        {
            GST_ERROR("SRAI_Memory_Allocate failed");
            break;
        }

        GST_DEBUG("*decrypted=%p, nexus_heap=%p, size=%d", *decrypted, nexus_heap, payloadDataSize);

        ib.data = nexus_heap;
        ib.data_length = payloadDataSize;

        NEXUS_FlushCache(nexus_heap, payloadDataSize);

        widevine::Cdm::OutputBuffer ob;

        ob.data = reinterpret_cast<uint8_t*>(*decrypted);
        ob.data_length = payloadDataSize;
        ob.is_secure = true;

        if( ( status = getWidevineCDM()->decrypt( ib, ob ) ) != widevine::Cdm::kSuccess ) {
            fprintf(stderr,"WidevineSession: decrypt FAIL\n");
            break;
        }

        NEXUS_FlushCache(nexus_heap, payloadDataSize);

        if (nexus_heap != NULL)
            NEXUS_Memory_Free(nexus_heap);

        return 0;
    }
    while (0);

    GST_DEBUG("failed in process payload %d", status);
    if (*decrypted != NULL)
        SRAI_Memory_Free((uint8_t *)*decrypted);
    *decrypted = NULL;
    return 1;
}

void WidevineSession::freeDecrypted(void* decrypted)
{
    GST_DEBUG("%p", decrypted);
    SRAI_Memory_Free((uint8_t *)decrypted);
}


}

#endif
