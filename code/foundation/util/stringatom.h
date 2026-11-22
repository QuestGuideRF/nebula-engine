#pragma once
#include "util/string.h"
namespace Util
{
class StringAtom
{
public:
    StringAtom();
    StringAtom(const StringAtom& rhs);
    StringAtom(char* ptr);
    StringAtom(const char* ptr);
    StringAtom(const char* ptr, size_t len);
    StringAtom(unsigned char* ptr);
    StringAtom(const unsigned char* ptr);
    StringAtom(const String& str);
    StringAtom(std::nullptr_t);
    void operator=(const StringAtom& rhs);
    void operator=(const char* ptr);
    void operator=(const String& str);
    bool operator==(const StringAtom& rhs) const;
    bool operator==(std::nullptr_t) const;
    bool operator!=(const StringAtom& rhs) const;
    bool operator>(const StringAtom& rhs) const;
    bool operator<(const StringAtom& rhs) const;
    bool operator>=(const StringAtom& rhs) const;
    bool operator<=(const StringAtom& rhs) const;
    bool operator==(const char* rhs) const;
    bool operator!=(const char* rhs) const;
    bool operator==(const String& rhs) const;
    bool operator!=(const String& rhs) const;
    void Clear();
    bool IsValid() const;
    const char* Value() const;
    String AsString() const;
    uint32_t HashCode() const;
    uint32_t StringHashCode() const;
    const char* c_str() const;
    size_t length() const;
    bool empty() const;
private:
    void Setup(const char* str);
    const char* content;
};
__forceinline
StringAtom::StringAtom() :
    content(nullptr)
{
}
__forceinline
StringAtom::StringAtom(const StringAtom& rhs) :
    content(rhs.content)
{
}
inline
StringAtom::StringAtom(char* str)
{
    if (nullptr != str)
    {
        this->Setup(str);
    }
    else
    {
        this->content = nullptr;
    }
}
inline
StringAtom::StringAtom(const char* str)
{
    if (nullptr != str)
    {
        this->Setup(str);
    }
    else
    {
        this->content = nullptr;
    }
}
inline StringAtom::StringAtom(const char* str, size_t len)
{
    if (nullptr != str)
    {
        Util::String string(str, len);
        this->Setup(string.AsCharPtr());
    }
    else
    {
        this->content = nullptr;
    }
}
inline
StringAtom::StringAtom(unsigned char* str)
{
    if (nullptr != str)
    {
        this->Setup((const char*)str);
    }
    else
    {
        this->content = nullptr;
    }
}
inline
StringAtom::StringAtom(const unsigned char* str)
{
    if (nullptr != str)
    {
        this->Setup((const char*)str);
    }
    else
    {
        this->content = nullptr;
    }
}
inline
StringAtom::StringAtom(const String& str)
{
    this->Setup(str.AsCharPtr());
}
inline
StringAtom::StringAtom(std::nullptr_t)
{
    this->content = nullptr;
}
__forceinline void
StringAtom::operator=(const StringAtom& rhs)
{
    this->content = rhs.content;
}
inline void
StringAtom::operator=(const char* str)
{
    if (0 != str)
    {
        this->Setup(str);
    }
    else
    {
        this->content = 0;
    }
}
inline void
StringAtom::operator=(const String& str)
{
    this->Setup(str.AsCharPtr());
}
__forceinline bool
StringAtom::operator==(const StringAtom& rhs) const
{
    return this->content == rhs.content;
}
__forceinline bool
StringAtom::operator!=(const StringAtom& rhs) const
{
    return this->content != rhs.content;
}
__forceinline bool
StringAtom::operator>(const StringAtom& rhs) const
{
    return this->content > rhs.content;
}
__forceinline bool
StringAtom::operator<(const StringAtom& rhs) const
{
    return this->content < rhs.content;
}
__forceinline bool
StringAtom::operator>=(const StringAtom& rhs) const
{
    return this->content >= rhs.content;
}
__forceinline bool
StringAtom::operator<=(const StringAtom& rhs) const
{
    return this->content <= rhs.content;
}
inline bool
StringAtom::operator==(const String& rhs) const
{
    if (0 == this->content)
    {
        return false;
    }
    else
    {
        return (rhs == this->content);
    }
}
inline bool
StringAtom::operator!=(const String& rhs) const
{
    if (0 == this->content)
    {
        return false;
    }
    else
    {
        return (rhs != this->content);
    }
}
__forceinline void
StringAtom::Clear()
{
    this->content = 0;
}
__forceinline bool
StringAtom::IsValid() const
{
    return (0 != this->content) && (0 != this->content[0]);
}
__forceinline const char*
StringAtom::Value() const
{
    return this->content;
}
inline String
StringAtom::AsString() const
{
    return String(this->content);
}
inline uint32_t
StringAtom::HashCode() const
{
    return Math::pointerhash((void *)this->content);
}
__forceinline const char*
StringAtom::c_str() const
{
    return this->Value();
}
__forceinline size_t
StringAtom::length() const
{
    return strlen(this->content);
}
__forceinline bool
StringAtom::empty() const
{
    return (0 != this->content) && ('\0' != this->content[0]);
}
}
Util::StringAtom operator ""_atm(const char* c, std::size_t s);