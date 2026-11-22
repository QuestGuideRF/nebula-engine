#pragma once
#include "core/types.h"
#include "core/sysfunc.h"
#include "util/array.h"
#include "util/dictionary.h"
#include "memory/heap.h"
#include "memory/poolarrayallocator.h"
namespace Math
{
    struct mat4;
    struct vec2;
    struct vec3;
    struct vec4;
    struct quat;
    class transform44;
};
namespace Util
{
class Blob;
class String
{
public:
    String();
    String(const String& rhs);
    String(String&& rhs) noexcept;
    String(const char* cStr);
    String(const char* cStr, size_t len);
    ~String();
    void operator=(const String& rhs);
    void operator=(String&& rhs) noexcept;
    void operator=(const char* cStr);
    void operator +=(const String& rhs);
    friend bool operator==(const String& a, const String& b);
    friend bool operator==(const String& a, const char* cStr);
    friend bool operator==(const char* cStr, const String& a);
    friend bool operator==(const String&, std::nullptr_t);
    friend bool operator !=(const String& a, const String& b);
    friend bool operator <(const String& a, const String& b);
    friend bool operator >(const String& a, const String& b);
    friend bool operator <=(const String& a, const String& b);
    friend bool operator >=(const String& a, const String& b);
    char operator[](IndexT i) const;
    char& operator[](IndexT i);
    void Reserve(SizeT newSize);
    SizeT Length() const;
    void Clear();
    bool IsEmpty() const;
    bool IsValid() const;
    bool CopyToBuffer(char* buf, SizeT bufSize) const;
    void Append(const String& str);
    void Append(const char* str);
    void AppendRange(const char* str, SizeT numChars);
    void ToLower();
    void ToUpper();
    void Capitalize();
    void CamelCaseToWords();
    SizeT Tokenize(const String& whiteSpace, Array<String>& outTokens) const;
    Array<String> Tokenize(const String& whiteSpace) const;
    SizeT Tokenize(const String& whiteSpace, char fence, Array<String>& outTokens) const;
    Array<String> Tokenize(const String& whiteSpace, char fence) const;
    String ExtractRange(IndexT fromIndex, SizeT numChars) const;
    String ExtractToEnd(IndexT fromIndex) const;
    void Strip(const String& charSet);
    IndexT FindStringIndex(const String& s, IndexT startIndex = 0) const;
    IndexT FindCharIndex(char c, IndexT startIndex = 0) const;
    bool BeginsWithString(const String& s) const;
    bool EndsWithString(const String& s) const;
    void TerminateAtIndex(IndexT index);
    bool ContainsCharFromSet(const String& charSet) const;
    void TrimLeft(const String& charSet);
    void TrimRight(const String& charSet);
    void Trim(const String& charSet);
    void SubstituteString(const String& str, const String& substStr);
    void SubstituteChar(char c, char subst);
    void __cdecl Format(const char* fmtString, ...);
    void __cdecl FormatArgList(const char* fmtString, va_list argList);
    static String Sprintf(const char* fmtString, ...);
    bool CheckValidCharSet(const String& charSet) const;
    void ReplaceChars(const String& charSet, char replacement);
    static String Concatenate(const Array<String>& strArray, const String& whiteSpace);
    static bool MatchPattern(const String& str, const String& pattern);
    uint32_t HashCode() const;
    void SetCharPtr(const char* s);
    void Set(const char* ptr, SizeT length);
    void Set(const char* ptr, size_t length);
    void SetByte(byte val);
    void SetUByte(ubyte val);
    void SetShort(short val);
    void SetUShort(ushort val);
    void SetInt(int val);
    void SetUInt(uint val);
    void SetLong(long val);
    void SetSizeT(size_t val);
    void SetLongLong(long long val);
    void SetFloat(float val);
    void SetDouble(double val);
    void SetBool(bool val);
    void Fill(SizeT length, unsigned char character);
    #if !__OSX__
    void SetVec2(const Math::vec2& v);
    void SetVec3(const Math::vec3& v);
    void SetVec4(const Math::vec4& v);
    void SetFloat2(const Math::float2& v);
    void SetFloat3(const Math::float3& v);
    void SetFloat4(const Math::float4& v);
    void SetQuaternion(const Math::quat& v);
    void SetMat4(const Math::mat4& v);
    void SetTransform44(const Math::transform44& v);
    #endif
    template<typename T> void Set(const T& t);
    void AppendChar(char val);
    void AppendInt(int val);
    void AppendByte(byte val);
    void AppendUByte(ubyte val);
    void AppendFloat(float val);
    void AppendBool(bool val);
    void AppendVec2(const Math::vec2& v);
    void AppendVec3(const Math::vec3& v);
    void AppendVec4(const Math::vec4& v);
    void AppendMat4(const Math::mat4& v);
    template<typename T> void Append(const T& t);
    const char* AsCharPtr() const;
    const char* Get() const;
    int AsInt() const;
    long long AsLongLong() const;
    float AsFloat() const;
    bool AsBool() const;
    Math::vec2 AsVec2() const;
    Math::vec3 AsVec3() const;
    Math::vec4 AsVec4() const;
    Math::float2 AsFloat2() const;
    Math::float3 AsFloat3() const;
    Math::float4 AsFloat4() const;
    Math::mat4 AsMat4() const;
    Math::transform44 AsTransform44() const;
    Util::Blob AsBlob() const;
    Util::String AsBase64() const;
    template<typename T> T As() const;
    bool IsValidInt() const;
    bool IsValidFloat() const;
    bool IsValidBool() const;
    bool IsValidVec2() const;
    bool IsValidVec4() const;
    bool IsValidMat4() const;
    bool IsValidTransform44() const;
    template<typename T> bool IsValid() const;
    static String FromByte(byte i);
    static String FromUByte(ubyte i);
    static String FromShort(short i);
    static String FromUShort(ushort i);
    static String FromInt(int i);
    static String FromUInt(uint i);
    static String FromLong(long i);
    static String FromSize(size_t i);
    static String FromLongLong(long long i);
    static String FromFloat(float f);
    static String FromDouble(double f);
    static String FromBool(bool b);
    static String FromVec2(const Math::vec2& v);
    static String FromVec3(const Math::vec3& v);
    static String FromVec4(const Math::vec4& v);
    static String FromFloat2(const Math::float2& v);
    static String FromFloat3(const Math::float3& v);
    static String FromFloat4(const Math::float4& v);
    static String FromQuat(const Math::quat& q);
    static String FromMat4(const Math::mat4& m);
    static String FromTransform44(const Math::transform44& m);
    static String FromBlob(const Util::Blob & b);
    static String FromBase64(const String&);
    template<typename T> static String From(const T& t);
    static constexpr uint Hash(const char* c, std::size_t s);
    template<typename INTEGER>
    static String Hex(INTEGER i);
    String GetFileExtension() const;
    bool CheckFileExtension(const String& ext) const;
    void ConvertBackslashes();
    void StripFileExtension();
    void ChangeFileExtension(const Util::String& newExt);
    void StripAssignPrefix();
    void ChangeAssignPrefix(const Util::String& newPref);
    String ExtractFileName() const;
    String ExtractLastDirName() const;
    String ExtractDirName() const;
    String ExtractToLastSlash() const;
    void ReplaceIllegalFilenameChars(char replacement);
    inline const char* c_str() const { return this->AsCharPtr(); }
    inline size_t length() const { return this->Length(); }
    inline bool empty() const { return this->IsEmpty(); }
    static bool IsDigit(char c);
    static bool IsAlpha(char c);
    static bool IsAlNum(char c);
    static bool IsLower(char c);
    static bool IsUpper(char c);
    static int StrCmp(const char* str0, const char* str1);
    static int StrLen(const char* str);
    static const char* StrChr(const char* str, int c);
    static Dictionary<String,String> ParseKeyValuePairs(const String& str);
private:
    void Delete();
    char* GetLastSlash() const;
    void Alloc(SizeT size);
    void Realloc(SizeT newSize);
    enum
    {
        LocalStringSize = 16,
    };
    char* heapBuffer;
    char localBuffer[LocalStringSize];
    SizeT strLen;
    SizeT heapBufferSize;
};
inline
String::String() :
    heapBuffer(nullptr),
    strLen(0),
    heapBufferSize(0)
{
    this->localBuffer[0] = 0;
}
inline void
String::Delete()
{
    if (this->heapBuffer)
    {
        Memory::Free(Memory::StringDataHeap, (void*) this->heapBuffer);
        this->heapBuffer = nullptr;
    }
    this->localBuffer[0] = 0;
    this->strLen = 0;
    this->heapBufferSize = 0;
}
inline
String::~String()
{
    this->Delete();
}
inline
String::String(const char* str) :
    heapBuffer(0),
    strLen(0),
    heapBufferSize(0)
{
    this->localBuffer[0] = 0;
    this->SetCharPtr(str);
}
inline
String::String(const char* str, size_t len) :
    heapBuffer(0),
    strLen(0),
    heapBufferSize(0)
{
    this->localBuffer[0] = 0;
    this->Set(str, (SizeT)len);
}
inline
String::String(const String& rhs) :
    heapBuffer(0),
    strLen(0),
    heapBufferSize(0)
{
    this->localBuffer[0] = 0;
    this->SetCharPtr(rhs.AsCharPtr());
}
inline
String::String(String&& rhs) noexcept:
    heapBuffer(rhs.heapBuffer),
    strLen(rhs.strLen),
    heapBufferSize(rhs.heapBufferSize)
{
    if (rhs.heapBuffer)
    {
        rhs.heapBuffer = nullptr;
        this->localBuffer[0] = 0;
    }
    else
    {
        if (this->strLen > 0)
        {
            Memory::Copy(rhs.localBuffer, this->localBuffer, this->strLen);
        }
        this->localBuffer[this->strLen] = 0;
    }
}
inline const char*
String::AsCharPtr() const
{
    if (this->heapBuffer)
    {
        return this->heapBuffer;
    }
    else
    {
        return this->localBuffer;
    }
}
inline const char*
String::Get() const
{
    return this->AsCharPtr();
}
inline void
String::operator=(const String& rhs)
{
    if (&rhs != this)
    {
        this->SetCharPtr(rhs.AsCharPtr());
    }
}
inline void
String::operator=(String&& rhs) noexcept
{
    if (&rhs != this)
    {
        this->Delete();
        this->strLen = rhs.strLen;
        rhs.strLen = 0;
        if (rhs.heapBuffer)
        {
            this->heapBuffer = rhs.heapBuffer;
            this->heapBufferSize = rhs.heapBufferSize;
            rhs.heapBuffer = nullptr;
            rhs.heapBufferSize = 0;
        }
        else
        {
            if (this->strLen > 0)
            {
                Memory::Copy(rhs.localBuffer, this->localBuffer, this->strLen);
            }
            this->localBuffer[this->strLen] = 0;
        }
    }
}
inline void
String::operator=(const char* cStr)
{
    this->SetCharPtr(cStr);
}
inline void
String::Append(const String& str)
{
    this->AppendRange(str.AsCharPtr(), str.strLen);
}
inline void
String::operator+=(const String& rhs)
{
    this->Append(rhs);
}
inline char
String::operator[](IndexT i) const
{
    n_assert(i <= this->strLen);
    return this->AsCharPtr()[i];
}
inline char&
String::operator[](IndexT i)
{
    n_assert(i <= this->strLen);
    return (char&)(this->AsCharPtr())[i];
}
inline SizeT
String::Length() const
{
    return this->strLen;
}
inline void
String::Clear()
{
    this->Delete();
}
inline bool
String::IsEmpty() const
{
    return (0 == this->strLen);
}
inline bool
String::IsValid() const
{
    return (0 != this->strLen);
}
inline uint32_t
String::HashCode() const
{
    const char* ptr = this->AsCharPtr();
    return Hash(ptr, this->strLen);
}
static inline String
operator+(const String& s0, const String& s1)
{
    String newString = s0;
    newString.Append(s1);
    return newString;
}
inline void
String::SubstituteChar(char c, char subst)
{
    char* ptr = const_cast<char*>(this->AsCharPtr());
    IndexT i;
    for (i = 0; i <= this->strLen; i++)
    {
        if (ptr[i] == c)
        {
            ptr[i] = subst;
        }
    }
}
inline void
String::ConvertBackslashes()
{
    this->SubstituteChar('\\', '/');
}
inline bool
String::CheckFileExtension(const String& ext) const
{
    return (this->GetFileExtension() == ext);
}
inline String
String::ExtractFileName() const
{
    String pathString;
    const char* lastSlash = this->GetLastSlash();
    if (lastSlash)
    {
        pathString = &(lastSlash[1]);
    }
    else
    {
        pathString = *this;
    }
    return pathString;
}
inline String
String::ExtractToLastSlash() const
{
    String pathString(*this);
    char* lastSlash = pathString.GetLastSlash();
    if (lastSlash)
    {
        lastSlash[1] = 0;
    }
    else
    {
        pathString = "";
    }
    return pathString;
}
inline bool
String::CheckValidCharSet(const String& charSet) const
{
    IndexT i;
    for (i = 0; i < this->strLen; i++)
    {
        if (InvalidIndex == charSet.FindCharIndex((*this)[i], 0))
        {
            return false;
        }
    }
    return true;
}
inline bool
String::IsValidInt() const
{
    return this->CheckValidCharSet(" \t-+01234567890");
}
inline bool
String::IsValidFloat() const
{
    return this->CheckValidCharSet(" \t-+.e1234567890");
}
inline void
String::ReplaceIllegalFilenameChars(char replacement)
{
    this->ReplaceChars("\\/:*?\"<>|", replacement);
}
inline String
String::FromByte(byte i)
{
    String str;
    str.SetByte(i);
    return str;
}
inline String
String::FromUByte(ubyte i)
{
    String str;
    str.SetUByte(i);
    return str;
}
inline String
String::FromShort(short i)
{
    String str;
    str.SetShort(i);
    return str;
}
inline String
String::FromUShort(ushort i)
{
    String str;
    str.SetUShort(i);
    return str;
}
inline String
String::FromInt(int i)
{
    String str;
    str.SetInt(i);
    return str;
}
inline String
String::FromUInt(uint i)
{
    String str;
    str.SetUInt(i);
    return str;
}
inline String
String::FromLong(long i)
{
    String str;
    str.SetLong(i);
    return str;
}
inline String
String::FromSize(size_t i)
{
    String str;
    str.SetSizeT(i);
    return str;
}
inline Util::String
String::FromLongLong(long long i)
{
    String str;
    str.SetLongLong(i);
    return str;
}
inline String
String::FromFloat(float f)
{
    String str;
    str.SetFloat(f);
    return str;
}
inline String
String::FromDouble(double i)
{
    String str;
    str.SetDouble(i);
    return str;
}
inline String
String::FromBool(bool b)
{
    String str;
    str.SetBool(b);
    return str;
}
#if !__OSX__
inline String
String::FromVec2(const Math::vec2& v)
{
    String str;
    str.SetVec2(v);
    return str;
}
inline String
String::FromVec3(const Math::vec3& v)
{
    String str;
    str.SetVec3(v);
    return str;
}
inline String
String::FromVec4(const Math::vec4& v)
{
    String str;
    str.SetVec4(v);
    return str;
}
inline String
String::FromFloat2(const Math::float2& v)
{
    String str;
    str.SetFloat2(v);
    return str;
}
inline String
String::FromFloat3(const Math::float3& v)
{
    String str;
    str.SetFloat3(v);
    return str;
}
inline String
String::FromFloat4(const Math::float4& v)
{
    String str;
    str.SetFloat4(v);
    return str;
}
inline String
String::FromQuat(const Math::quat& q)
{
    String str;
    str.SetQuaternion(q);
    return str;
}
inline String
String::FromMat4(const Math::mat4& m)
{
    String str;
    str.SetMat4(m);
    return str;
}
inline String
String::FromTransform44(const Math::transform44& t)
{
    String str;
    str.SetTransform44(t);
    return str;
}
#endif
template<typename INTEGER>
inline String
String::Hex(INTEGER i)
{
    constexpr SizeT hexLength = sizeof(INTEGER) << 1;
    static Util::String digits = "0123456789ABCDEF";
    String str("0x");
    str.Reserve(hexLength + 2);
    for (SizeT n = 0, j = (hexLength - 1) * 4; n < hexLength; ++n, j -= 4)
    {
        str.AppendChar(digits[(i >> j) & 0x0f]);
    }
    return str;
}
inline void
String::AppendChar(char val)
{
    this->AppendRange(&val, 1);
}
inline void
String::AppendInt(int val)
{
    this->Append(FromInt(val));
}
inline void
String::AppendByte(byte val)
{
    this->Append(FromByte(val));
}
inline void
String::AppendUByte(ubyte val)
{
    this->Append(FromUByte(val));
}
inline void
String::AppendFloat(float val)
{
    this->Append(FromFloat(val));
}
inline void
String::AppendBool(bool val)
{
    this->Append(FromBool(val));
}
#if !__OSX__
inline void
String::AppendVec2(const Math::vec2& val)
{
    this->Append(FromVec2(val));
}
inline void
String::AppendVec3(const Math::vec3& val)
{
    this->Append(FromVec3(val));
}
inline void
String::AppendVec4(const Math::vec4& val)
{
    this->Append(FromVec4(val));
}
inline void
String::AppendMat4(const Math::mat4& val)
{
    this->Append(FromMat4(val));
}
#endif
constexpr uint
String::Hash(const char* c, std::size_t s)
{
    uint hash = 0;
    std::size_t i;
    for (i = 0; i < s; i++)
    {
        hash += c[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}
inline auto Format = &String::Sprintf;
}
Util::String operator ""_str(const char* c, std::size_t s);
constexpr uint
operator ""_hash(const char* c, std::size_t s)
{
    return Util::String::Hash(c, s);
}