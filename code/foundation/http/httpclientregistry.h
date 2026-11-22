#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "http/httpclient.h"
#include "io/uri.h"
#include "util/dictionary.h"
#include "util/stringatom.h"
namespace Http
{
class HttpClientRegistry : public Core::RefCounted
{
    __DeclareClass(HttpClientRegistry);
    __DeclareSingleton(HttpClientRegistry);
public:
    HttpClientRegistry();
    virtual ~HttpClientRegistry();
    void Setup();
    void Discard();
    bool IsValid() const;
    Ptr<HttpClient> ObtainConnection(const IO::URI& uri);
    void ReleaseConnection(const IO::URI& uri);
    bool IsConnected(const IO::URI& uri) const;
    void DisconnectIdle();
private:
    struct Connection
    {
        Ptr<HttpClient> httpClient;
        SizeT useCount;
    };
    Util::Dictionary<Util::StringAtom, Connection> connections;
    bool isValid;
};
inline bool
HttpClientRegistry::IsValid() const
{
    return this->isValid;
}
}