#include "config.h"
#include "CDMWidevine.h"

#if ENABLE(ENCRYPTED_MEDIA) && USE(WIDEVINE)

#include "CDMInstanceWidevine.h"
#include "CDMPrivate.h"
#include <wtf/UUID.h>

namespace WebCore {

class CDMPrivateWidevine : public CDMPrivate {
public:
    CDMPrivateWidevine();
    virtual ~CDMPrivateWidevine();

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

// CDMFactoryWidevine

CDMFactoryWidevine::CDMFactoryWidevine() = default;
CDMFactoryWidevine::~CDMFactoryWidevine() = default;

std::unique_ptr<CDMPrivate> CDMFactoryWidevine::createCDM(CDM&, const String&)
{
    return std::unique_ptr<CDMPrivate>(new CDMPrivateWidevine);
}

bool CDMFactoryWidevine::supportsKeySystem(const String& keySystem)
{
    return equalLettersIgnoringASCIICase(keySystem, "com.widevine.alpha");
}

// CDMPrivateWidevine

CDMPrivateWidevine::CDMPrivateWidevine() = default;
CDMPrivateWidevine::~CDMPrivateWidevine() = default;

bool CDMPrivateWidevine::supportsInitDataType(const AtomicString& initDataType) const
{
    return equalLettersIgnoringASCIICase(initDataType, "cenc");
}

bool CDMPrivateWidevine::supportsConfiguration(const MediaKeySystemConfiguration&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return true;
}

bool CDMPrivateWidevine::supportsConfigurationWithRestrictions(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return true;
}

bool CDMPrivateWidevine::supportsSessionTypeWithConfiguration(MediaKeySessionType&, const MediaKeySystemConfiguration&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return true;
}

bool CDMPrivateWidevine::supportsRobustness(const String&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return false;
}

MediaKeysRequirement CDMPrivateWidevine::distinctiveIdentifiersRequirement(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return MediaKeysRequirement::Optional;
}

MediaKeysRequirement CDMPrivateWidevine::persistentStateRequirement(const MediaKeySystemConfiguration&, const MediaKeysRestrictions&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return MediaKeysRequirement::Optional;
}

bool CDMPrivateWidevine::distinctiveIdentifiersAreUniquePerOriginAndClearable(const MediaKeySystemConfiguration&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return false;
}

RefPtr<CDMInstance> CDMPrivateWidevine::createInstance()
{
    return adoptRef(new CDMInstanceWidevine);
}

void CDMPrivateWidevine::loadAndInitialize()
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
}

bool CDMPrivateWidevine::supportsServerCertificates() const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return false;
}

bool CDMPrivateWidevine::supportsSessions() const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return true;
}

bool CDMPrivateWidevine::supportsInitData(const AtomicString& initDataType, const SharedBuffer&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return supportsInitDataType(initDataType);
}

RefPtr<SharedBuffer> CDMPrivateWidevine::sanitizeResponse(const SharedBuffer& response) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return response.copy();
}

std::optional<String> CDMPrivateWidevine::sanitizeSessionId(const String&) const
{
    fprintf(stderr, "NotImplemented: CDMPrivateWidevine::%s()\n", __func__);
    return { };
}

// CDMInstanceWidevine

CDMInstanceWidevine::CDMInstanceWidevine()
    : m_wvSession(std::make_unique<WidevineSession>(Vector<uint8_t>{ }, nullptr))
{
}

CDMInstanceWidevine::~CDMInstanceWidevine() = default;

CDMInstance::SuccessValue CDMInstanceWidevine::initializeWithConfiguration(const MediaKeySystemConfiguration&)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
    return Succeeded;
}

CDMInstance::SuccessValue CDMInstanceWidevine::setDistinctiveIdentifiersAllowed(bool)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
    return Succeeded;
}

CDMInstance::SuccessValue CDMInstanceWidevine::setPersistentStateAllowed(bool)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
    return Succeeded;
}

CDMInstance::SuccessValue CDMInstanceWidevine::setServerCertificate(Ref<SharedBuffer>&&)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
    return Failed;
}

void CDMInstanceWidevine::requestLicense(LicenseType, const AtomicString& initDataType, Ref<SharedBuffer>&& initData, LicenseCallback callback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
//    callback(SharedBuffer::create(result->data(), result->byteLength()), createCanonicalUUIDString(), false, SuccessValue::Succeeded);
}

void CDMInstanceWidevine::updateLicense(const String& sessionId, LicenseType, const SharedBuffer& response, LicenseUpdateCallback callback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
//    callback(false, WTFMove(changedKeys), std::nullopt, std::nullopt, Succeeded);
}

void CDMInstanceWidevine::loadSession(LicenseType, const String& sessionId, const String& origin, LoadSessionCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
}

void CDMInstanceWidevine::closeSession(const String& sessionId, CloseSessionCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
}

void CDMInstanceWidevine::removeSessionData(const String& sessionId, LicenseType, RemoveSessionDataCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
}

void CDMInstanceWidevine::storeRecordOfKeyUsage(const String& sessionId)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
}

void CDMInstanceWidevine::gatherAvailableKeys(AvailableKeysCallback)
{
}

} // namespace WebCore

#endif // ENABLE(ENCRYPTED_MEDIA) && USE(WIDEVINE)
