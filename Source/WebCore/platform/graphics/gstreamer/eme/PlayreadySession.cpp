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

#include <stdio.h>

#include <stdlib.h>
#include <gst/gst.h>

#include "config.h"
#include "PlayreadySession.h"

#if USE(PLAYREADY)
//#include "MediaKeyError.h"
//#include "MediaPlayerPrivateGStreamer.h"
#include "wtf/UUID.h"

#include <bkni.h>
#include <nexus_dma.h>

#include <runtime/JSCInlines.h>
#include <runtime/TypedArrayInlines.h>
// #include <wtf/PassRefPtr.h>
#include <wtf/text/CString.h>

#include <nexus_dma.h>
#include "nexus_config.h"
#include "nexus_base_os.h"
//#include <nexus_platform.h>
#include <sage_srai.h>

GST_DEBUG_CATEGORY_EXTERN(webkit_media_playready_decrypt_debug_category);
#define GST_CAT_DEFAULT webkit_media_playready_decrypt_debug_category

namespace WebCore
{

#define ChkDRM(x) if((dr = (x)) != DRM_Prdy_ok) break;
#define ChkBOOL(x,y) if (!(x)) { dr = y; break;};

PlayreadySession::PlayreadySession(const Vector<uint8_t> &initData, const void* pipeline)
    : m_key()
    , m_eKeyState(KEY_INIT)
    , m_fCommit(false)
    , m_sessionId(createCanonicalUUIDString())
    , m_initData(initData)
    , m_pipeline(pipeline)
{
    DRM_Prdy_Init_t     prdyParamSettings;
    DRM_Prdy_GetDefaultParamSettings(&prdyParamSettings);
    const char *_env = getenv("WEBKIT_MSPR_DIR");
    if( _env && *_env )
        prdyParamSettings.defaultRWDirName = (char*)_env;
    prdyParamSettings.hdsFileName = (char*)"/tmp/drmstore.dat";
    BKNI_Memset(&pDecryptContext, 0, sizeof(DRM_Prdy_DecryptContext_t));

    drmContext =  DRM_Prdy_Initialize( &prdyParamSettings);
    if( drmContext == NULL )
    {
        m_eKeyState = KEY_ERROR;
        GST_ERROR("Playready initialization failed");
    }
    else
    {
        GST_DEBUG("Playready initialized %p", drmContext);
        GST_DEBUG(
            "hdsFileName: '%s', "
            "binFileName: '%s', "
            "keyFileName: '%s', "
            "keyHistoryFileName: '%s', "
            "revocationListFileName: '%s', "
            "defaultRWDirName: '%s'",
            prdyParamSettings.hdsFileName,
            prdyParamSettings.binFileName,
            prdyParamSettings.keyFileName,
            prdyParamSettings.keyHistoryFileName,
            prdyParamSettings.revocationListFileName,
            prdyParamSettings.defaultRWDirName);
    }
}

PlayreadySession::~PlayreadySession()
{
    GST_DEBUG("Releasing resources");
    DRM_Prdy_Reader_Close(&pDecryptContext);
    DRM_Prdy_Error_e e = DRM_Prdy_Uninitialize(drmContext);
    GST_DEBUG("Releasing resources, rv = %d ", e);
}

// PlayReady license policy callback which should be
// customized for platform/environment that hosts the CDM.
// It is currently implemented as a place holder that
// does nothing.
/*
DRM_RESULT DRM_CALL PlayreadySession::_PolicyCallback(const DRM_VOID *f_pvOutputLevelsData, DRM_POLICY_CALLBACK_TYPE f_dwCallbackType, const DRM_VOID *f_pv)
{
    return DRM_SUCCESS;
}*/

inline void dumpBuffer( const unsigned char *buf, int len ) {
    return;
    char out[128];
    int l, i, m, o = 0;
    while( len > 0 ) {
        l = 0;
        m = len > 16 ? 16 : len;
        l += snprintf(out+l,sizeof(out)-l," %04x: ", o);
        for( i = 0; i < m; ++i )
            l += snprintf(out+l,sizeof(out)-l,(i==8)?"   %02x ":"%02x ", buf[i]);
        while( l < 64 )
            out[l++] = ' ';
        for( i = 0; i < m; ++i )
            l += snprintf(out+l,sizeof(out)-l,"%c",((buf[i]<32)||(buf[i]>=128))?'.':buf[i]);
        buf += m;
        len -= m;
        o += m;
        fprintf(stderr,"%s\n",out);
    }
}

RefPtr<Uint8Array> PlayreadySession::playreadyGenerateKeyRequest(Uint8Array* initData, const String& customData, String& destinationURL, unsigned short& errorCode, uint32_t& systemCode)
{
    RefPtr<Uint8Array> result;
    DRM_Prdy_Error_e dr = DRM_Prdy_ok;
    do
    {
        GST_DEBUG("generating key request");
        GST_MEMDUMP("init data", (const guint8*) initData->data(), initData->byteLength());

        // The current state MUST be KEY_INIT otherwise error out.
        ChkBOOL(m_eKeyState == KEY_INIT, DRM_Prdy_invalid_parameter);
        ChkDRM(DRM_Prdy_Content_SetProperty(drmContext, DRM_Prdy_contentSetProperty_eAutoDetectHeader, (const unsigned char*)initData->data(), initData->byteLength()));
        GST_DEBUG("init data set on DRM context... %p", drmContext);
        dumpBuffer( (const unsigned char*)initData->data(), initData->byteLength() );
//         if (DRM_Prdy_Reader_Bind(drmContext, &pDecryptContext) == DRM_Prdy_ok)
//         {
//             GST_DEBUG("Play rights already acquired!");
//             m_eKeyState = KEY_READY;
//             systemCode = dr;
//             errorCode = 0;
//             return nullptr;
//         }
//         GST_DEBUG("DRM reader not bound");

        size_t urlLen, chLen;
        ChkDRM(DRM_Prdy_Get_Buffer_Size( drmContext, DRM_Prdy_getBuffer_licenseAcq_challenge, (const uint8_t*)(customData.length() == 0 ? NULL: customData.utf8().data()), customData.length(), &urlLen, &chLen));

        char* pCh_url = (char*)BKNI_Malloc(urlLen + 1);
        char* pCh_data = (char*)BKNI_Malloc(chLen);

        ChkDRM(DRM_Prdy_LicenseAcq_GenerateChallenge(drmContext, customData.length() == 0 ? NULL: customData.utf8().data(), customData.length(), pCh_url, &urlLen, pCh_data, &chLen));

        pCh_url[ urlLen ] = 0;

        GST_MEMDUMP("generated license request :", (const guint8*)pCh_data, chLen);
        result = Uint8Array::create((const guint8*)pCh_data, chLen);
        destinationURL = pCh_url;
        GST_DEBUG("destination URL : %s", destinationURL.utf8().data());

        m_eKeyState = KEY_PENDING;
        systemCode = dr;
        errorCode = 0;
        dumpBuffer( (const unsigned char*)pCh_data, chLen );
        if(pCh_url != NULL) BKNI_Free(pCh_url);
        if(pCh_data != NULL) BKNI_Free(pCh_data);

        return result;
    }
    while (0);

    GST_DEBUG("DRM key generation failed %d", dr);
    errorCode = -666; //MediaKeyError::MEDIA_KEYERR_CLIENT;
    return result;
}

//
// Expected synchronisation from caller. This method is not thread-safe!
//
bool PlayreadySession::playreadyProcessKey(Uint8Array* key, RefPtr<Uint8Array>& nextMessage, unsigned short& errorCode, uint32_t& systemCode)
{
    DRM_Prdy_Error_e dr = DRM_Prdy_ok;
    DRM_Prdy_License_Response_t pResponse;
    do
    {
        dumpBuffer( (unsigned char*)key->data(), key->byteLength() );
        uint8_t *m_pbKeyMessageResponse = (unsigned char*) key->data();
        uint32_t m_cbKeyMessageResponse = key->byteLength();
        GST_MEMDUMP("response received :", (const guint8*)key->data(), key->byteLength());
        GST_DEBUG("playreadyProcessKey %p, %d bytes", drmContext, m_cbKeyMessageResponse);
        // The current state MUST be KEY_PENDING

        ChkBOOL(m_pbKeyMessageResponse != NULL && m_cbKeyMessageResponse > 0, DRM_Prdy_invalid_parameter);

        if ((dr = DRM_Prdy_LicenseAcq_ProcessResponse(drmContext, (char *)m_pbKeyMessageResponse, m_cbKeyMessageResponse, &pResponse)) !=  DRM_Prdy_ok)
            ChkDRM(DRM_Prdy_LicenseAcq_ProcessResponseNonPersistent(drmContext, (char *)m_pbKeyMessageResponse, m_cbKeyMessageResponse, &pResponse));
        GST_DEBUG(" ");

        ChkDRM(DRM_Prdy_Reader_Bind(drmContext, &pDecryptContext));
//         m_key = key->buffer();
        m_key = ArrayBuffer::create(key->data(), key->byteLength());
        errorCode = 0;
        m_eKeyState = KEY_READY;
        GST_DEBUG("key processed, now ready for content decryption");
        systemCode = dr;
        return true;
    }
    while (0);

    GST_DEBUG("failed processing license response %d, %d, %d", dr, static_cast<int>(pResponse.dwResult), static_cast<int>(pResponse.eType));
    errorCode = -123; //!!!!!MediaKeyError::MEDIA_KEYERR_CLIENT;
    m_eKeyState = KEY_ERROR;
    return false;
}

int PlayreadySession::processPayload(const void* iv, uint32_t ivSize, const void */*kid*/, uint32_t /*kidSize*/, void* payloadData, uint32_t payloadDataSize, void** decrypted)
{
    DRM_Prdy_Error_e dr = DRM_Prdy_ok;
    uint8_t *nexus_heap = NULL;

    GST_DEBUG("payloadData=%p, size=%d", payloadData, payloadDataSize);
    do
    {
        DRM_Prdy_AES_CTR_Info_t     aesCtrInfo;
        uint8_t* ivData = (uint8_t*) iv;
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
        // FIXME: IV bytes need to be swapped ???
        uint8_t temp;
        for (uint32_t i = 0; i < ivSize / 2; i++)
        {
            temp = ivData[i];
            ivData[i] = ivData[ivSize - i - 1];
            ivData[ivSize - i - 1] = temp;
        }

        BKNI_Memcpy( &aesCtrInfo.qwInitializationVector, ivData, ivSize);
        aesCtrInfo.qwBlockOffset = 0;
        aesCtrInfo.bByteOffset = 0;

        NEXUS_DmaJobBlockSettings block;
        memset(&block, 0, sizeof(NEXUS_DmaJobBlockSettings));
        block.pSrcAddr = nexus_heap;
        block.pDestAddr = *decrypted;
        block.blockSize = payloadDataSize;
        block.cached = false;
        block.resetCrypto = block.scatterGatherCryptoStart = block.scatterGatherCryptoEnd = true;

        NEXUS_FlushCache(nexus_heap, payloadDataSize);

        GST_DEBUG("decrypt %p->%p %d", block.pSrcAddr, block.pDestAddr, block.blockSize);
        ChkDRM(DRM_Prdy_Reader_DecryptOpaque( &pDecryptContext, &aesCtrInfo, &block, 1) );

        NEXUS_FlushCache(nexus_heap, payloadDataSize);

        if (nexus_heap != NULL)
            NEXUS_Memory_Free(nexus_heap);

        // Call commit during the decryption of the first sample.
        if (!m_fCommit)
        {
            ChkDRM(DRM_Prdy_Reader_Commit(drmContext));
            GST_DEBUG("commit");
            //ChkDR(Drm_Reader_Commit(m_poAppContext, _PolicyCallback, NULL));
            m_fCommit = true;
        }
        return 0;
    }
    while (0);

    GST_DEBUG("failed in process payload %d", dr);
    if (*decrypted != NULL)
        SRAI_Memory_Free((uint8_t *)*decrypted);
    *decrypted = NULL;
    return 1;
}

}

#endif
