#ifndef WidevineSession_h
#define WidevineSession_h

#if USE(WIDEVINE)
#include <string>
#include <string.h>
#include <runtime/Uint8Array.h>
#include <wtf/Forward.h>
#include <wtf/text/WTFString.h>
#include "cdm/cdm.h"

namespace WebCore
{

class WidevineSession : public widevine::Cdm::IEventListener
{

private:
    enum KeyState
    {
        // Has been initialized.
        KEY_INIT = 0,
        // Has a key message pending to be processed.
        KEY_PENDING = 1,
        // Has a usable key.
        KEY_READY = 2,
        // Has an error.
        KEY_ERROR = 3,
        // Has been closed.
        KEY_CLOSED = 4
    };

public:
    WidevineSession(const Vector<uint8_t> &initData, const void* pipeline);
    ~WidevineSession();

    RefPtr<Uint8Array> widevineGenerateKeyRequest(Uint8Array* initData, const String& customData, String& destinationURL, unsigned short& errorCode, uint32_t& systemCode);
    bool widevineProcessKey(Uint8Array* key, RefPtr<Uint8Array>& nextMessage, unsigned short& errorCode, uint32_t& systemCode);

    const RefPtr<ArrayBuffer>& key() const { return m_key; }
    bool keyRequested() const { return m_eKeyState == KEY_PENDING; }
    bool ready() const { return m_eKeyState == KEY_READY; }
    int processPayload(const void* iv, uint32_t ivSize, void* payloadData, uint32_t payloadDataSize, void** decrypted);
    static void freeDecrypted(void* decrypted);

    // Helper for WidevineSession clients.
    const Vector<uint8_t>& initData() { return m_initData; }
    const String& sessionId() { return m_sessionId; }
    bool hasPipeline(const void* pipeline) { return m_pipeline == pipeline; }

    virtual void onMessageUrl(const std::string& session_id, const std::string& server_url);
    virtual void onMessage(const std::string& session_id, widevine::Cdm::MessageType message_type, const std::string& message);
    virtual void onKeyStatusesChange(const std::string& session_id);
    virtual void onRemoveComplete(const std::string& session_id);

protected:
    RefPtr<ArrayBuffer> m_key;

    KeyState m_eKeyState;
    bool m_fCommit;

    String m_sessionId;
    Vector<uint8_t> m_initData;
    const void* m_pipeline;

    RefPtr<Uint8Array> *m_result;
    widevine::Cdm::KeyStatusMap m_keyMap;
};

}
#endif

#endif
