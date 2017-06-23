#pragma once

#include "CDMInstance.h"

#if ENABLE(ENCRYPTED_MEDIA) && USE(WIDEVINE)

#include "WidevineSession.h"

namespace WebCore {

class CDMInstanceWidevine : public CDMInstance {
public:
    CDMInstanceWidevine();
    virtual ~CDMInstanceWidevine();

    ImplementationType implementationType() const final { return  ImplementationType::Widevine; }

    SuccessValue initializeWithConfiguration(const MediaKeySystemConfiguration&) override;
    SuccessValue setDistinctiveIdentifiersAllowed(bool) override;
    SuccessValue setPersistentStateAllowed(bool) override;
    SuccessValue setServerCertificate(Ref<SharedBuffer>&&) override;

    void requestLicense(LicenseType, const AtomicString& initDataType, Ref<SharedBuffer>&& initData, LicenseCallback) override;
    void updateLicense(const String& sessionId, LicenseType, const SharedBuffer& response, LicenseUpdateCallback) override;
    void loadSession(LicenseType, const String& sessionId, const String& origin, LoadSessionCallback) override;
    void closeSession(const String& sessionId, CloseSessionCallback) override;
    void removeSessionData(const String& sessionId, LicenseType, RemoveSessionDataCallback) override;
    void storeRecordOfKeyUsage(const String& sessionId) override;
    void gatherAvailableKeys(AvailableKeysCallback) override;

    WidevineSession& wvSession() const { return *m_wvSession; }

private:
    std::unique_ptr<WidevineSession> m_wvSession;
};

} // namespace WebCore

#endif // ENABLE(ENCRYPTED_MEDIA) && USE(WIDEVINE)
