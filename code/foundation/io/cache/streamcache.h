#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "io/uri.h"
#include "io/stream.h"
#include "util/dictionary.h"
namespace IO
{
class StreamCache : public Core::RefCounted
{
    __DeclareClass(StreamCache);
    __DeclareSingleton(StreamCache);
public:
    StreamCache();
    virtual ~StreamCache();
    bool IsCached(IO::URI const& uri) const;
    Util::KeyValuePair<void*,Ptr<IO::Stream>> GetCachedStream(IO::URI const& uri);
    bool OpenStream(IO::URI const& uri, Core::Rtti const& rtti);
    void RemoveStream(IO::URI const& uri);
private:
    void Discard();
    struct CacheEntry
    {
        Ptr<IO::Stream> stream;
        void * buffer = nullptr;
        SizeT useCount = 0;
    };
    Util::Dictionary<Util::String, CacheEntry> streams;
};
}