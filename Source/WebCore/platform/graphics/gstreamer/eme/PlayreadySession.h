/* Playready Session management
 *
 * Copyright (C) 2016 Igalia S.L
 * Copyright (C) 2016 Metrological
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

#define TARGET_LITTLE_ENDIAN 1
#define BSTD_CPU_ENDIAN BSTD_ENDIAN_LITTLE

#ifndef PlayreadySession_h
#define PlayreadySession_h

#if USE(PLAYREADY)

//#include <drmtypes.h>
//#include <drmcommon.h>
//#include <drmmanager.h>
//#include <drmmathsafe.h>
#include <string>
#include <string.h>

#include <drm_prdy_http.h>
#include <drm_prdy.h>
#include <drm_prdy_types.h>
#include <drm_types.h>

#undef __in
#undef __out
#include <runtime/Uint8Array.h>
#include <wtf/Forward.h>
#include <wtf/text/WTFString.h>
#include "CDMProcessPayloadBase.h"

namespace WebCore
{

class PlayreadySession : public CDMProcessPayloadBase
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
    PlayreadySession(const Vector<uint8_t> &initData, const void* pipeline);
    ~PlayreadySession();

    RefPtr<Uint8Array> playreadyGenerateKeyRequest(Uint8Array* initData, const String& customData, String& destinationURL, unsigned short& errorCode, uint32_t& systemCode);
    bool playreadyProcessKey(Uint8Array* key, RefPtr<Uint8Array>& nextMessage, unsigned short& errorCode, uint32_t& systemCode);

    const RefPtr<ArrayBuffer>& key() const { return m_key; }
    bool keyRequested() const { return m_eKeyState == KEY_PENDING; }
    bool ready() const { return m_eKeyState == KEY_READY; }
    virtual int processPayload(const void* iv, uint32_t ivSize, const void *kid, uint32_t kidSize, void* payloadData, uint32_t payloadDataSize, void** decrypted);

    // Helper for PlayreadySession clients.
    Lock& mutex() { return m_prSessionMutex; }
    const Vector<uint8_t>& initData() { return m_initData; }
    const String& sessionId() { return m_sessionId; }
    bool hasPipeline(const void* pipeline) { return m_pipeline == pipeline; }

protected:
    RefPtr<ArrayBuffer> m_key;

private:
    /*
        static DRM_RESULT DRM_CALL _PolicyCallback(const DRM_VOID* f_pvOutputLevelsData, DRM_POLICY_CALLBACK_TYPE f_dwCallbackType, const DRM_VOID* f_pv);

        DRM_APP_CONTEXT* m_poAppContext { nullptr };
        DRM_DECRYPT_CONTEXT m_oDecryptContext;

        DRM_BYTE* m_pbOpaqueBuffer { nullptr };
        DRM_DWORD m_cbOpaqueBuffer;

        DRM_BYTE* m_pbRevocationBuffer { nullptr };
        KeyState m_eKeyState;
        DRM_CHAR m_rgchSessionID[CCH_BASE64_EQUIV(SIZEOF(DRM_ID)) + 1];
        DRM_BOOL m_fCommit;
    */

    DRM_Prdy_Handle_t   drmContext;
    DRM_Prdy_DecryptContext_t  pDecryptContext;
    KeyState m_eKeyState;
    bool m_fCommit;

    Lock m_prSessionMutex;
    String m_sessionId;
    Vector<uint8_t> m_initData;
    const void* m_pipeline;
};

}
#endif

#endif
