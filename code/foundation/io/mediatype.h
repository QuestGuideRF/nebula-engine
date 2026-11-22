#pragma once
#include "util/string.h"
namespace IO
{
class MediaType
{
public:
    MediaType();
    MediaType(const Util::String& str);
    MediaType(const Util::String& type, const Util::String& subType);
    MediaType(const MediaType& rhs);
    void operator=(const MediaType& rhs);
    bool operator==(const MediaType& rhs);
    bool operator!=(const MediaType& rhs);
    bool IsValid() const;
    void Clear();
    void Set(const Util::String& str);
    void Set(const Util::String& type, const Util::String& subType);
    Util::String AsString() const;
    const Util::String& GetType() const;
    const Util::String& GetSubType() const;
private:
    Util::String type;
    Util::String subType;
};
inline
MediaType::MediaType()
{
}
inline
void
MediaType::Set(const Util::String& str)
{
    n_assert(str.IsValid());
    Util::Array<Util::String> tokens = str.Tokenize("/");
    n_assert(tokens.Size() == 2);
    this->type = tokens[0];
    this->subType = tokens[1];
}
inline
MediaType::MediaType(const Util::String& str)
{
    this->Set(str);
}
inline
MediaType::MediaType(const Util::String& t, const Util::String& s) :
    type(t),
    subType(s)
{
    n_assert(this->type.IsValid());
    n_assert(this->subType.IsValid());
}
inline
MediaType::MediaType(const MediaType& rhs) :
    type(rhs.type),
    subType(rhs.subType)
{
}
inline
void
MediaType::operator=(const MediaType& rhs)
{
    this->type = rhs.type;
    this->subType = rhs.subType;
}
inline bool
MediaType::operator==(const MediaType& rhs)
{
    return ((this->type == rhs.type) && (this->subType == rhs.subType));
}
inline bool
MediaType::operator!=(const MediaType& rhs)
{
    return !(*this == rhs);
}
inline void
MediaType::Set(const Util::String& t, const Util::String& s)
{
    n_assert(t.IsValid() && s.IsValid());
    this->type = t;
    this->subType = s;
}
inline Util::String
MediaType::AsString() const
{
    Util::String str;
    str.Reserve(128);
    str = this->type;
    str.Append("/");
    str.Append(this->subType);
    return str;
}
inline const Util::String&
MediaType::GetType() const
{
    return this->type;
}
inline const Util::String&
MediaType::GetSubType() const
{
    return this->subType;
}
inline bool
MediaType::IsValid() const
{
    return this->type.IsValid() && this->subType.IsValid();
}
inline void
MediaType::Clear()
{
    this->type.Clear();
    this->subType.Clear();
}
}