#ifndef _CDMProcessPayloadBase_h_
#define _CDMProcessPayloadBase_h_

namespace WebCore
{

class CDMProcessPayloadBase
{
public:
    virtual ~CDMProcessPayloadBase() { }
    virtual int processPayload(const void* iv, uint32_t ivSize, const void *kid, uint32_t kidSize, void* payloadData, uint32_t payloadDataSize, void** decrypted) = 0;

};

}

#endif /*_CDMProcessPayloadBase_h_*/
