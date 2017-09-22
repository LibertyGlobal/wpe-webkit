#include "config.h"
#include "CDMWidevine.h"

#if ENABLE(ENCRYPTED_MEDIA) && USE(WIDEVINE)

#include "CDMInstanceWidevine.h"
#include "CDMPrivate.h"
#include <wtf/UUID.h>
#include <wtf/CurrentTime.h>
#include <wtf/RunLoop.h>
#include "cdm/cdm.h"
#include <list>
#include <map>
#include <sys/utsname.h>

namespace WebCore {

class WPEHost : public widevine::Cdm::IStorage,
                public widevine::Cdm::IClock,
                public widevine::Cdm::ITimer,
                public widevine::Cdm::IEventListener {
public:
    inline WPEHost() :
        m_cdm( nullptr ) {
    }

    inline void addSession( WidevineSession *wvSession ) {
        LockHolder holder( sessionsLock );
        sessions_.push_back( wvSession );
    }
    inline void delSession( WidevineSession *wvSession ) {
        LockHolder holder( sessionsLock );
        sessions_.remove( wvSession );
    }

    inline widevine::Cdm *cdm() {
        if (!m_cdm ) {
            widevine::Cdm::ClientInfo client = { "HorizonBox", "Liberty Global", "STB", "EOS", "", "" };
            {
                struct utsname name;
                if (uname(&name) == 0) {
                    client.arch_name = name.machine;
                }
            }
            client.build_info = __DATE__;
            widevine::Cdm::DeviceCertificateRequest cert_request;
            widevine::Cdm::SecureOutputType         output_type = widevine::Cdm::kOpaqueHandle;
#ifdef WIDEVINE_BCM_NO_SVP
            const char *_env = getenv("WEBKIT_WV_NO_SVP");
            if( _env && *_env )
                output_type = widevine::Cdm::kNoSecureOutput;
#endif
            widevine::Cdm::Status status = widevine::Cdm::initialize( output_type, client, this, this, this, &cert_request, widevine::Cdm::kVerbose );
            if( widevine::Cdm::kSuccess == status )
                m_cdm = widevine::Cdm::create( this, true );
        }
        return m_cdm;
    }

private:
    virtual bool read(const std::string& name, std::string* data) {
        StorageMap::iterator it = files_.find(name);
        if( it == files_.end() )
            return false;
        *data = it->second;
        return true;
    }
    virtual bool write(const std::string& name, const std::string& data) {
        files_[name] = data;
        return true;
    }
    virtual bool exists(const std::string& name) {
        return ( files_.find(name) != files_.end() );
    }
    virtual bool remove(const std::string& name) {
        StorageMap::iterator it = files_.find(name);
        if( it == files_.end() )
            return false;
        files_.erase( it );
        return true;
    }
    virtual int32_t size(const std::string& name) {
        StorageMap::iterator it = files_.find(name);
        if( it == files_.end() )
            return -1;
        return it->second.size();
    }

    virtual int64_t now() {
        return WTF::currentTimeMS();
    }

    virtual void setTimeout(int64_t delay_ms, IClient* client, void* context) {
        TimerWrap *wrap = new TimerWrap{ true, client, context, this };
        {
            LockHolder holder( timersLock );
            timers_.push_back( wrap );
        }
        RunLoop::main().dispatchAfter( Seconds( 0.001 * delay_ms ), [wrap] {
            {
                LockHolder holder( wrap->host->timersLock );
                for( TimerStack::iterator it = wrap->host->timers_.begin(); it != wrap->host->timers_.end(); ++it ) {
                    if( *it == wrap ) {
                        wrap->host->timers_.erase( it );
                        break;
                    }
                }
            }
            if( wrap->alive ) {
                wrap->client->onTimerExpired( wrap->context );
            }
            delete wrap;
        } );
    }
    virtual void cancel(IClient* client) {
        LockHolder holder( timersLock );
        for( TimerStack::iterator it = timers_.begin(); it != timers_.end(); ++it ) {
            if( (*it)->client == client )
                (*it)->alive = false;
        }
    }

    virtual void onMessageUrl(const std::string& session_id, const std::string& server_url) {
        LockHolder holder( sessionsLock );
        for( SessionsList::iterator it = sessions_.begin(); it != sessions_.end(); ++it ) {
            if( (*it)->sessionId() == session_id.c_str() ) {
                (*it)->onMessageUrl( session_id, server_url );
                break;
            }
        }
    }
    virtual void onMessage(const std::string& session_id, widevine::Cdm::MessageType message_type, const std::string& message) {
        LockHolder holder( sessionsLock );
        for( SessionsList::iterator it = sessions_.begin(); it != sessions_.end(); ++it ) {
            if( (*it)->sessionId() == session_id.c_str() ) {
                (*it)->onMessage( session_id, message_type, message );
                break;
            }
        }
    }
    virtual void onKeyStatusesChange(const std::string& session_id) {
        LockHolder holder( sessionsLock );
        for( SessionsList::iterator it = sessions_.begin(); it != sessions_.end(); ++it ) {
            if( (*it)->sessionId() == session_id.c_str() ) {
                (*it)->onKeyStatusesChange( session_id );
                break;
            }
        }
    }
    virtual void onRemoveComplete(const std::string& session_id) {
        LockHolder holder( sessionsLock );
        for( SessionsList::iterator it = sessions_.begin(); it != sessions_.end(); ++it ) {
            if( (*it)->sessionId() == session_id.c_str() ) {
                (*it)->onRemoveComplete( session_id );
                break;
            }
        }
    }

private:
    struct TimerWrap {
        bool        alive;
        IClient    *client;
        void       *context;
        WPEHost    *host;
    };

    typedef std::map<std::string, std::string>  StorageMap;
    typedef std::list<TimerWrap*>               TimerStack;
    StorageMap                  files_;
    TimerStack                  timers_;
    Lock                        timersLock;

    typedef std::list<WidevineSession*>         SessionsList;
    SessionsList                sessions_;
    Lock                        sessionsLock;

    widevine::Cdm              *m_cdm;
};

static WPEHost host;

widevine::Cdm* getWidevineCDM() {
    return host.cdm();
}

class CDMPrivateWidevine :  public CDMPrivate {
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
    host.addSession( &*m_wvSession );
}

CDMInstanceWidevine::~CDMInstanceWidevine() {
    host.delSession( &*m_wvSession ) ;
}

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

void CDMInstanceWidevine::requestLicense(LicenseType, const AtomicString& /*initDataType*/, Ref<SharedBuffer>&& initData, LicenseCallback callback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s(), %d\n", __func__,initData->size());
    unsigned short  errorCode   = 0;
    uint32_t        systemCode  = 0;
    String              destinationURL;
    RefPtr<Uint8Array>  result;
    RefPtr<Uint8Array> initDataArray = Uint8Array::create( reinterpret_cast<const uint8_t*>(initData->data()), initData->size() );
    result = m_wvSession->widevineGenerateKeyRequest(initDataArray.get(), String(), destinationURL, errorCode, systemCode);

    if (!result) {
        callback(SharedBuffer::create(), m_wvSession->sessionId(), false, Failed);
        return;
    }

    callback(SharedBuffer::create(result->data(), result->byteLength()), m_wvSession->sessionId(), false, Succeeded);
}

void CDMInstanceWidevine::updateLicense(const String& /*sessionId*/, LicenseType, const SharedBuffer& response, LicenseUpdateCallback callback)
{
    fprintf(stderr, "NotImplemented: CDMInstancePlayReady::%s()\n", __func__);

    RefPtr<Uint8Array> message;
    unsigned short errorCode = 0;
    uint32_t systemCode = 0;
    RefPtr<Uint8Array> responseArray = Uint8Array::create(reinterpret_cast<const uint8_t*>(response.data()), response.size());
    bool result = m_wvSession->widevineProcessKey(responseArray.get(), message, errorCode, systemCode);

    if (!result || errorCode) {
        callback(false, std::nullopt, std::nullopt, std::nullopt, Failed);
        return;
    }

    if( message && (message->byteLength() > 0) ) {
        callback( false, std::nullopt, std::nullopt, Message( MediaKeyMessageType::LicenseRequest, SharedBuffer::create( message->data(), message->byteLength() ) ), Succeeded );
        return;
    }

    std::optional<KeyStatusVector> changedKeys;
    if (m_wvSession->ready()) {
        auto& key = m_wvSession->key();
        if (key) {
            changedKeys = KeyStatusVector();
            Ref<SharedBuffer> keyData = SharedBuffer::create(reinterpret_cast<const char*>(key->data()), key->byteLength());
            changedKeys->append({ WTFMove(keyData), KeyStatus::Usable });
        }
    }

    callback(false, WTFMove(changedKeys), std::nullopt, std::nullopt, Succeeded);
}

void CDMInstanceWidevine::loadSession(LicenseType, const String& /*sessionId*/, const String& /*origin*/, LoadSessionCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
}

void CDMInstanceWidevine::closeSession(const String& /*sessionId*/, CloseSessionCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
}

void CDMInstanceWidevine::removeSessionData(const String& /*sessionId*/, LicenseType, RemoveSessionDataCallback)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
}

void CDMInstanceWidevine::storeRecordOfKeyUsage(const String& /*sessionId*/)
{
    fprintf(stderr, "NotImplemented: CDMInstanceWidevine::%s()\n", __func__);
}

void CDMInstanceWidevine::gatherAvailableKeys(AvailableKeysCallback)
{
}

} // namespace WebCore

#endif // ENABLE(ENCRYPTED_MEDIA) && USE(WIDEVINE)
