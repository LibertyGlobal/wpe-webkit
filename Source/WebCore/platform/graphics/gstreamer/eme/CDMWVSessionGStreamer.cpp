/*
 * Copyright (C) 2014-2015 FLUENDO S.A. All rights reserved.
 * Copyright (C) 2014-2015 METROLOGICAL All rights reserved.
 * Copyright (C) 2015 IGALIA S.L All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY FLUENDO S.A. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FLUENDO S.A. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CDMWVSessionGStreamer.h"

#if ENABLE(LEGACY_ENCRYPTED_MEDIA) && USE(GSTREAMER) && USE(WIDEVINE)

#include "CDM.h"
#include "MediaPlayerPrivateGStreamerBase.h"
#include "UUID.h"
#include "WebKitMediaKeyError.h"

#include <gst/gst.h>
#include <wtf/text/CString.h>

GST_DEBUG_CATEGORY_EXTERN(webkit_media_widevine_decrypt_debug_category);
#define GST_CAT_DEFAULT webkit_media_widevine_decrypt_debug_category

namespace WebCore {

CDMWVSessionGStreamer::CDMWVSessionGStreamer(CDMSessionClient* client, MediaPlayerPrivateGStreamerBase* playerPrivate)
    // FIXME: Create PlayreadySession with actual initData and pipeline values.
    : WidevineSession(Vector<uint8_t>(), nullptr)
    , m_client(client)
    , m_sessionId(createCanonicalUUIDString())
    , m_playerPrivate(playerPrivate)
{
}

CDMWVSessionGStreamer::~CDMWVSessionGStreamer()
{
}

CDMSessionType CDMWVSessionGStreamer::type()
{
    return CDMSessionTypePlayReady;
}

void CDMWVSessionGStreamer::setClient(CDMSessionClient* client)
{
    ASSERT((m_client == nullptr) ^ (client == nullptr));

    m_client = client;
}

const String& CDMWVSessionGStreamer::sessionId() const
{
    return m_sessionId;
}

RefPtr<Uint8Array> CDMWVSessionGStreamer::generateKeyRequest(const String& mimeType, Uint8Array* initData, String& destinationURL, unsigned short& errorCode, uint32_t& systemCode)
{
    GST_DEBUG("got request for %s", mimeType.utf8().data());
    m_playerPrivate->receivedGenerateKeyRequest(WIDEVINE_PROTECTION_SYSTEM_ID);
    return widevineGenerateKeyRequest(initData, String(), destinationURL, errorCode, systemCode);
}

bool CDMWVSessionGStreamer::update(Uint8Array* key, RefPtr<Uint8Array>& nextMessage, unsigned short& errorCode, uint32_t& systemCode)
{
    GST_DEBUG("processing key");
    return widevineProcessKey(key, nextMessage, errorCode, systemCode);
}

void CDMWVSessionGStreamer::releaseKeys()
{
}

RefPtr<ArrayBuffer> CDMWVSessionGStreamer::cachedKeyForKeyID(const String& sessionId) const
{
    return (sessionId == m_sessionId ? m_key : nullptr);
}

}

#endif
