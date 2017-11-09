#include "config.h"
#include "CDMPlayReady.h"

#if ENABLE(ENCRYPTED_MEDIA) && USE(PLAYREADY)

#include "CDMInstancePlayReady.h"
#include "CDMPrivate.h"
#include <wtf/UUID.h>

namespace WebCore {

class CDMPrivatePlayReady : public CDMPrivate {
public:
    CDMPrivatePlayReady();
    virtual ~CDMPrivatePlayReady();

    bool supportsInitDataType(const AtomicString&) const override;
    bool supportsConfiguration(const MediaKeySystemConfiguration&) const override;
    bool supportsConfigurationWithRestrictions(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const override;
    bool supportsSessionTypeWithConfiguration(MediaKeySessionType&, const MediaKeySystemConfiguration&) const override;
    bool supportsRobustness(const String&) const override;
    MediaKeysRequirement distinctiveIdentifiersRequirement(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const override;
    MediaKeysRequirement persistentStateRequirement(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const override;
    bool distinctiveIdentifiersAreUniquePerOriginAndClearable(const MediaKeySystemConfiguration&) const override;
    RefPtr<CDMInstance> createInstance() override;
    void loadAndInitialize() override;
    bool supportsServerCertificates() const override;
    bool supportsSessions() const override;
    bool supportsInitData(const AtomicString&, const SharedBuffer&) const override;
    RefPtr<SharedBuffer> sanitizeResponse(const SharedBuffer&) const override;
    std::optional<String> sanitizeSessionId(const String&) const override;
};

// CDMFactoryPlayReady

CDMFactoryPlayReady::CDMFactoryPlayReady() = default;
CDMFactoryPlayReady::~CDMFactoryPlayReady() = default;

std::unique_ptr<CDMPrivate> CDMFactoryPlayReady::createCDM(CDM&, const String&)
{
    return std::unique_ptr<CDMPrivate>(new CDMPrivatePlayReady);
}

bool CDMFactoryPlayReady::supportsKeySystem(const String& keySystem)
{
    return equalLettersIgnoringASCIICase(keySystem, "com.microsoft.playready") 
        || equalLettersIgnoringASCIICase(keySystem, "com.youtube.playready");
}

// CDMPrivatePlayReady

CDMPrivatePlayReady::CDMPrivatePlayReady() = default;
CDMPrivatePlayReady::~CDMPrivatePlayReady() = default;

bool CDMPrivatePlayReady::supportsInitDataType(const AtomicString& initDataType) const
{
    return equalLettersIgnoringASCIICase(initDataType, "cenc");
}

bool CDMPrivatePlayReady::supportsConfiguration(const MediaKeySystemConfiguration&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return true;
}

bool CDMPrivatePlayReady::supportsConfigurationWithRestrictions(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return true;
}

bool CDMPrivatePlayReady::supportsSessionTypeWithConfiguration(MediaKeySessionType&, const MediaKeySystemConfiguration&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return true;
}

bool CDMPrivatePlayReady::supportsRobustness(const String&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return false;
}

MediaKeysRequirement CDMPrivatePlayReady::distinctiveIdentifiersRequirement(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return MediaKeysRequirement::Optional;
}

MediaKeysRequirement CDMPrivatePlayReady::persistentStateRequirement(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return MediaKeysRequirement::Optional;
}

bool CDMPrivatePlayReady::distinctiveIdentifiersAreUniquePerOriginAndClearable(const MediaKeySystemConfiguration&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return false;
}

RefPtr<CDMInstance> CDMPrivatePlayReady::createInstance()
{
    return adoptRef(new CDMInstancePlayReady);
}

void CDMPrivatePlayReady::loadAndInitialize()
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
}

bool CDMPrivatePlayReady::supportsServerCertificates() const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return false;
}

bool CDMPrivatePlayReady::supportsSessions() const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return true;
}

bool CDMPrivatePlayReady::supportsInitData(const AtomicString& initDataType, const SharedBuffer&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return supportsInitDataType(initDataType);
}

RefPtr<SharedBuffer> CDMPrivatePlayReady::sanitizeResponse(const SharedBuffer& response) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return response.copy();
}

std::optional<String> CDMPrivatePlayReady::sanitizeSessionId(const String&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivatePlayReady::%s()\n", __func__);
    return { };
}

// CDMInstancePlayReady

CDMInstancePlayReady::CDMInstancePlayReady()
{
}

CDMInstancePlayReady::~CDMInstancePlayReady()
{
    for( std::list<PlayreadySession*>::iterator it = m_prSessions.begin(); it != m_prSessions.end(); ++it ) {
        delete *it;
    }
}

CDMInstance::SuccessValue CDMInstancePlayReady::initializeWithConfiguration(const MediaKeySystemConfiguration&)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
    return Succeeded;
}

CDMInstance::SuccessValue CDMInstancePlayReady::setDistinctiveIdentifiersAllowed(bool)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
    return Succeeded;
}

CDMInstance::SuccessValue CDMInstancePlayReady::setPersistentStateAllowed(bool)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
    return Succeeded;
}

CDMInstance::SuccessValue CDMInstancePlayReady::setServerCertificate(Ref<SharedBuffer>&&)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
    return Failed;
}

const char msplrdyuuid[16] =    { 0x9a, 0x04, 0xf0, 0x79
                                , 0x98, 0x40
                                , 0x42, 0x86
                                , 0xab, 0x92
                                , 0xe6, 0x5b, 0xe0, 0x88, 0x5f, 0x95 };

inline bool getPSSHPayload( const uint8_t *initData, int initDataLength, const char uuid[16], const uint8_t **dataBlock, int *dataLength )
{
    while( initDataLength >= 4 )
    {
        int len = 0;
        len |= initData[0]; len <<= 8;
        len |= initData[1]; len <<= 8;
        len |= initData[2]; len <<= 8;
        len |= initData[3];
        if( initDataLength < len )                  // malformed block
            return false;
        if( len < 32 )                              // block with malformed uuid
            return false;
        if( !memcmp( initData + 12, uuid, 16 ) )    // found it
        {
            *dataBlock  = initData + 32;
            *dataLength = len - 32;
            return true;
        }
        initData        += len;                     // next block
        initDataLength  -= len;
    }
    return false;
}

void CDMInstancePlayReady::requestLicense(LicenseType, const AtomicString& /*initDataType*/, Ref<SharedBuffer>&& initData, LicenseCallback callback)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
    const uint8_t  *initBlock   = NULL;
    int             initLength  = 0;
    unsigned short  errorCode   = 0;
    uint32_t        systemCode  = 0;
    String              destinationURL;
    RefPtr<Uint8Array>  result;
    if( !getPSSHPayload( reinterpret_cast<const uint8_t*>(initData->data()), initData->size(), msplrdyuuid, &initBlock, &initLength ) )
    {
        initBlock = reinterpret_cast<const uint8_t*>(initData->data());
        initLength = initData->size();
    }
    Vector<uint8_t> _initData;
    _initData.append( reinterpret_cast<const uint8_t*>(initData->data()), initData->size() );
    PlayreadySession *session = new PlayreadySession( _initData, nullptr);
    RefPtr<Uint8Array> initDataArray = Uint8Array::create( initBlock, initLength );
    result = session->playreadyGenerateKeyRequest(initDataArray.get(), String(), destinationURL, errorCode, systemCode);
    if (!result) {
        delete session;
        callback(SharedBuffer::create(), String(), false, Failed);
        return;
    }
    m_prSessions.push_back( session );
    callback(SharedBuffer::create(result->data(), result->byteLength()), session->sessionId(), false, Succeeded);
}

void CDMInstancePlayReady::updateLicense(const String& sessionId, LicenseType, const SharedBuffer& response, LicenseUpdateCallback callback)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);

    PlayreadySession *session = NULL;
    for( std::list<PlayreadySession*>::iterator it = m_prSessions.begin(); it != m_prSessions.end(); ++it ) {
        if( (*it)->sessionId() == sessionId ) {
            session = *it;
            break;
        }
    }
    if( !session ) {
        callback(false, std::nullopt, std::nullopt, std::nullopt, Failed);
        return;
    }
    RefPtr<Uint8Array> message;
    unsigned short errorCode = 0;
    uint32_t systemCode = 0;
    RefPtr<Uint8Array> responseArray = Uint8Array::create(reinterpret_cast<const uint8_t*>(response.data()), response.size());
    bool result = session->playreadyProcessKey(responseArray.get(), message, errorCode, systemCode);

    if (!result || errorCode) {
        callback(false, std::nullopt, std::nullopt, std::nullopt, Failed);
        return;
    }

    std::optional<KeyStatusVector> changedKeys;
    if (session->ready()) {
        auto& key = session->key();
        if (key) {
            changedKeys = KeyStatusVector();
            Ref<SharedBuffer> keyData = SharedBuffer::create(reinterpret_cast<const char*>(key->data()), key->byteLength());
            changedKeys->append({ WTFMove(keyData), KeyStatus::Usable });
        }
    }

    callback(false, WTFMove(changedKeys), std::nullopt, std::nullopt, Succeeded);
}

void CDMInstancePlayReady::loadSession(LicenseType, const String& /*sessionId*/, const String& /*origin*/, LoadSessionCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
}

void CDMInstancePlayReady::closeSession(const String& /*sessionId*/, CloseSessionCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
}

void CDMInstancePlayReady::removeSessionData(const String& /*sessionId*/, LicenseType, RemoveSessionDataCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
}

void CDMInstancePlayReady::storeRecordOfKeyUsage(const String& /*sessionId*/)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);
}

void CDMInstancePlayReady::gatherAvailableKeys(AvailableKeysCallback)
{
}

} // namespace WebCore

#endif // ENABLE(ENCRYPTED_MEDIA) && USE(PLAYREADY)
