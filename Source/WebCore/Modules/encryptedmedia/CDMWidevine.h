#pragma once

#if ENABLE(ENCRYPTED_MEDIA) && USE(WIDEVINE)

#include "CDM.h"

namespace WebCore {

class CDMFactoryWidevine : public CDMFactory {
public:
    CDMFactoryWidevine();
    virtual ~CDMFactoryWidevine();

    std::unique_ptr<CDMPrivate> createCDM(CDM&, const String&) override;
    bool supportsKeySystem(const String&) override;
};

} // namespace WebCore

#endif // ENABLE(ENCRYPTED_MEDIA) && USE(WIDEVINE)
