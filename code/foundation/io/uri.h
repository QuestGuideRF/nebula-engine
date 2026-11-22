#pragma once
#ifndef IO_URI_H
#define IO_URI_H
#include "core/types.h"
#include "util/string.h"
#include "util/dictionary.h"
namespace IO
{
class URI
{
public:
    URI();
    URI(const Util::String& s);
    URI(const char* s);
    URI(const URI& rhs);
    void operator=(const URI& rhs);
    bool operator==(const URI& rhs) const;
    bool operator!=(const URI& rhs) const;
    void Set(const Util::String& s);
    Util::String AsString() const;
    bool IsEmpty() const;
    bool IsValid() const;
    void Clear();
    void SetScheme(const Util::String& s);
    const Util::String& Scheme() const;
    void SetUserInfo(const Util::String& s);
    const Util::String& UserInfo() const;
    void SetHost(const Util::String& s);
    const Util::String& Host() const;
    void SetPort(const Util::String& s);
    const Util::String& Port() const;
    void SetLocalPath(const Util::String& s);
    const Util::String& LocalPath() const;
    void AppendLocalPath(const Util::String& pathComponent);
    void SetFragment(const Util::String& s);
    const Util::String& Fragment() const;
    void SetQuery(const Util::String& s);
    const Util::String& Query() const;
    Util::Dictionary<Util::String,Util::String> ParseQuery() const;
    Util::String GetTail() const;
    Util::String GetHostAndLocalPath() const;
private:
    bool Split(const Util::String& s);
    Util::String Build() const;
    bool isEmpty;
    Util::String scheme;
    Util::String userInfo;
    Util::String host;
    Util::String port;
    Util::String localPath;
    Util::String fragment;
    Util::String query;
};
inline
URI::URI() :
    isEmpty(true)
{
}
inline
URI::URI(const Util::String& s) :
    isEmpty(true)
{
    bool validUri = this->Split(s);
    n_assert2(validUri, s.AsCharPtr());
}
inline
URI::URI(const char* s) :
    isEmpty(true)
{
    bool validUri = this->Split(s);
    n_assert2(validUri, s);
}
inline
URI::URI(const URI& rhs) :
    isEmpty(rhs.isEmpty),
    scheme(rhs.scheme),
    userInfo(rhs.userInfo),
    host(rhs.host),
    port(rhs.port),
    localPath(rhs.localPath),
    fragment(rhs.fragment),
    query(rhs.query)
{
}
inline
void
URI::operator=(const URI& rhs)
{
    this->isEmpty = rhs.isEmpty;
    this->scheme = rhs.scheme;
    this->userInfo = rhs.userInfo;
    this->host = rhs.host;
    this->port = rhs.port;
    this->localPath = rhs.localPath;
    this->fragment = rhs.fragment;
    this->query = rhs.query;
}
inline
bool
URI::operator==(const URI& rhs) const
{
    if (this->isEmpty && rhs.isEmpty)
    {
        return true;
    }
    else
    {
        return ((this->scheme == rhs.scheme) &&
                (this->userInfo == rhs.userInfo) &&
                (this->host == rhs.host) &&
                (this->port == rhs.port) &&
                (this->localPath == rhs.localPath) &&
                (this->fragment == rhs.fragment) &&
                (this->query == rhs.query));
    }
}
inline
bool
URI::operator!=(const URI& rhs) const
{
    return !(*this == rhs);
}
inline
bool
URI::IsEmpty() const
{
    return this->isEmpty;
}
inline
bool
URI::IsValid() const
{
    return !(this->isEmpty);
}
inline
void
URI::Clear()
{
    this->isEmpty = true;
    this->scheme.Clear();
    this->userInfo.Clear();
    this->host.Clear();
    this->port.Clear();
    this->localPath.Clear();
    this->fragment.Clear();
    this->query.Clear();
}
inline
void
URI::Set(const Util::String& s)
{
    this->Split(s);
}
inline
Util::String
URI::AsString() const
{
    return this->Build();
}
inline
void
URI::SetScheme(const Util::String& s)
{
    this->isEmpty = false;
    this->scheme = s;
}
inline
const Util::String&
URI::Scheme() const
{
    return this->scheme;
}
inline
void
URI::SetUserInfo(const Util::String& s)
{
    this->isEmpty = false;
    this->userInfo = s;
}
inline
const Util::String&
URI::UserInfo() const
{
    return this->userInfo;
}
inline
void
URI::SetHost(const Util::String& s)
{
    this->isEmpty = false;
    this->host = s;
}
inline
const Util::String&
URI::Host() const
{
    return this->host;
}
inline
void
URI::SetPort(const Util::String& s)
{
    this->isEmpty = false;
    this->port = s;
}
inline
const Util::String&
URI::Port() const
{
    return this->port;
}
inline
void
URI::SetLocalPath(const Util::String& s)
{
    this->isEmpty = false;
    this->localPath = s;
}
inline
const Util::String&
URI::LocalPath() const
{
    return this->localPath;
}
inline
void
URI::SetFragment(const Util::String& s)
{
    this->isEmpty = false;
    this->fragment = s;
}
inline
const Util::String&
URI::Fragment() const
{
    return this->fragment;
}
inline
void
URI::SetQuery(const Util::String& s)
{
    this->isEmpty = false;
    this->query = s;
}
inline
const Util::String&
URI::Query() const
{
    return this->query;
}
}
IO::URI operator ""_uri(const char* c, std::size_t s);
#endif