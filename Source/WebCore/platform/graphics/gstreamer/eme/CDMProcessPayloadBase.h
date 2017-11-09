#ifndef _CDMProcessPayloadBase_h_
#define _CDMProcessPayloadBase_h_

#include <wtf/Vector.h>

namespace WebCore
{

class CDMProcessPayloadBase
{
public:
    virtual ~CDMProcessPayloadBase() { }
    virtual int processPayload(const void* iv, uint32_t ivSize, const void *kid, uint32_t kidSize, void* payloadData, uint32_t payloadDataSize, void** decrypted) = 0;
    virtual bool isCenc() { return true; }
    virtual const Vector<uint8_t>& initData() = 0;
};

}

#endif /*_CDMProcessPayloadBase_h_*/
