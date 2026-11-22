#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Util
{
class FourCC
{
public:
    FourCC();
    constexpr FourCC(uint f);
    FourCC(const String& s);
    bool operator==(const FourCC& rhs) const;
    bool operator!=(const FourCC& rhs) const;
    bool operator<(const FourCC& rhs) const;
    bool operator<=(const FourCC& rhs) const;
    bool operator>(const FourCC& rhs) const;
    bool operator>=(const FourCC& rhs) const;
    bool IsValid() const;
    void SetFromUInt(uint f);
    uint AsUInt() const;
    void SetFromString(const String& s);
    String AsString() const;
    static String ToString(const FourCC& f);
    static FourCC FromString(const String& s);
    uint32_t HashCode() const;
private:
    uint fourCC;
};
inline
FourCC::FourCC() :
    fourCC(0)
{
}
constexpr
FourCC::FourCC(uint f) :
    fourCC(f)
{
}
inline
FourCC::FourCC(const String& s)
{
    this->SetFromString(s);
}
inline bool
FourCC::operator==(const FourCC& rhs) const
{
    return (this->fourCC == rhs.fourCC);
}
inline bool
FourCC::operator!=(const FourCC& rhs) const
{
    return (this->fourCC != rhs.fourCC);
}
inline bool
FourCC::operator>(const FourCC& rhs) const
{
    return (this->fourCC > rhs.fourCC);
}
inline bool
FourCC::operator>=(const FourCC& rhs) const
{
    return (this->fourCC >= rhs.fourCC);
}
inline bool
FourCC::operator<(const FourCC& rhs) const
{
    return (this->fourCC < rhs.fourCC);
}
inline bool
FourCC::operator<=(const FourCC& rhs) const
{
    return (this->fourCC <= rhs.fourCC);
}
inline bool
FourCC::IsValid() const
{
    return (0 != this->fourCC);
}
inline void
FourCC::SetFromUInt(uint f)
{
    n_assert(0 != f);
    this->fourCC = f;
}
inline uint
FourCC::AsUInt() const
{
    return this->fourCC;
}
inline void
FourCC::SetFromString(const String& s)
{
    *this = FromString(s);
}
inline String
FourCC::AsString() const
{
    return ToString(*this);
}
inline String
FourCC::ToString(const FourCC& f)
{
    n_assert(f.IsValid());
    String str("xzyw");
    str[0] = (char) ((f.fourCC & 0xFF000000) >> 24);
    str[1] = (char) ((f.fourCC & 0x00FF0000) >> 16);
    str[2] = (char) ((f.fourCC & 0x0000FF00) >> 8);
    str[3] = (char) (f.fourCC & 0x000000FF);
    return str;
}
inline FourCC
FourCC::FromString(const String& s)
{
    n_assert(s.IsValid() && (s.Length() == 4));
#if (__WIN32__ || __LINUX__ )
    return FourCC(uint(s[3] | s[2]<<8 | s[1]<<16 | s[0]<<24));
#else
    return FourCC(uint(s[0] | s[1]<<8 | s[2]<<16 | s[3]<<24));
#endif
}
inline uint32_t
FourCC::HashCode() const
{
    return this->fourCC;
}
}