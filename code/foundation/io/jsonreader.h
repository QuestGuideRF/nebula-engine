#pragma once
#include "io/streamreader.h"
#include "math/vec4.h"
#include "math/vec2.h"
#include "math/mat4.h"
#include "util/stack.h"
#include "util/array.h"
#include "util/stringatom.h"
#include "util/bitfield.h"
#include "util/variant.h"
namespace pjson
{
    class value_variant;
    class document;
}
namespace IO
{
class JsonReader : public StreamReader
{
    __DeclareClass(JsonReader);
public:
    JsonReader();
    virtual ~JsonReader();
    virtual bool Open();
    virtual void Close();
    bool HasNode(const Util::String& path);
    void SetToRoot();
    bool SetToNode(const Util::String& path);
    bool SetToFirstChild(const Util::String& name = "");
    bool SetToNextChild();
    bool SetToParent();
    Util::String GetChildNodeName(SizeT childIndex);
    bool IsArray() const;
    bool IsObject() const;
    bool HasChildren() const;
    SizeT CurrentSize() const;
    bool HasAttr(const char* attr) const;
    Util::Array<Util::String> GetAttrs() const;
    Util::String GetCurrentNodeName() const;
    Util::String GetString(const char* attr =  0) const;
    Util::StringAtom GetStringAtom(const char* attr = 0) const;
    bool GetBool(const char* attr = 0) const;
    int GetInt(const char* attr = 0) const;
    uint GetUInt(const char* attr = 0) const;
    float GetFloat(const char* attr = 0) const;
    Math::vec2 GetVec2(const char* attr = 0) const;
    Math::vec3 GetVec3(const char* attr = 0) const;
    Math::vec4 GetVec4(const char* attr = 0) const;
    Math::mat4 GetMat4(const char* attr = 0) const;
    Math::transform44 GetTransform44(const char* attr = 0) const;
    template<typename T> void Get(T& target, const char* attr = 0);
    template<unsigned int N> void Get(Util::BitField<N>& target, const char* attr = 0);
    Util::String GetOptString(const char* attr, const Util::String& defaultValue) const;
    bool GetOptBool(const char* attr, bool defaultValue) const;
    int GetOptInt(const char* attr, int defaultValue) const;
    float GetOptFloat(const char* attr, float defaultValue) const;
    Math::vec2 GetOptVec2(const char* attr, const Math::vec2& defaultValue) const;
    Math::vec4 GetOptVec4(const char* attr, const Math::vec4& defaultValue) const;
    Math::mat4 GetOptMat4(const char* attr, const Math::mat4& defaultValue) const;
    Math::transform44 GetOptTransform44(const char* attr, const Math::transform44& defaultValue) const;
    template<typename T> bool GetOpt(T& target, const char* attr = 0);
    template<typename T> bool GetOpt(T& target, const char* attr, const T& _default);
private:
    const pjson::value_variant* GetChild(const char * key = 0) const;
    pjson::document* document;
    const pjson::value_variant* curNode;
    IndexT childIdx = -1;
    Util::Stack<const pjson::value_variant*> parents;
    Util::Stack<IndexT> parentIdx;
    char * buffer = nullptr;
};
template<> void JsonReader::Get<bool>(bool& ret, const char* attr);
template<> void JsonReader::Get<Util::Array<uint32_t>>(Util::Array<uint32_t>& ret, const char* attr);
template<> void JsonReader::Get<Util::Array<int>>(Util::Array<int>& ret, const char* attr);
template<> void JsonReader::Get<int64_t>(int64_t& ret, const char* attr);
template<> void JsonReader::Get<int32_t>(int32_t& ret, const char* attr);
template<> void JsonReader::Get<int16_t>(int16_t& ret, const char* attr);
template<> void JsonReader::Get<int8_t>(int8_t& ret, const char* attr);
template<> void JsonReader::Get<char>(char& ret, const char* attr);
template<> void JsonReader::Get<Math::mat4>(Math::mat4& ret, const char* attr);
template<> void JsonReader::Get<Math::int2>(Math::int2& ret, const char* attr);
template<> void JsonReader::Get<Math::vector>(Math::vector& ret, const char* attr);
template<> void JsonReader::Get<Math::vec4>(Math::vec4& ret, const char* attr);
template<> void JsonReader::Get<Math::vec3>(Math::vec3& ret, const char* attr);
template<> void JsonReader::Get<Math::vec2>(Math::vec2& ret, const char* attr);
template<> void JsonReader::Get<uint32_t>(uint32_t& ret, const char* attr);
template<> void JsonReader::Get<uint16_t>(uint16_t& ret, const char* attr);
template<> void JsonReader::Get<uint8_t>(uint8_t& ret, const char* attr);
template<> void JsonReader::Get<float>(float& ret, const char* attr);
template<> void JsonReader::Get<double>(double& ret, const char* attr);
template<> void JsonReader::Get<Util::Variant>(Util::Variant& ret, const char* attr);
template<> void JsonReader::Get<Util::String>(Util::String& ret, const char* attr);
template<> void JsonReader::Get<Util::FourCC>(Util::FourCC& ret, const char* attr);
template<> void JsonReader::Get<Util::StringAtom>(Util::StringAtom& ret, const char* attr);
template<> void JsonReader::Get<Util::Array<float>>(Util::Array<float>& ret, const char* attr);
template<> void JsonReader::Get<Util::Array<Util::String>>(Util::Array<Util::String>& ret, const char* attr);
template<> bool JsonReader::GetOpt<bool>(bool& ret, const char* attr);
template<> bool JsonReader::GetOpt<int>(int& ret, const char* attr);
template<> bool JsonReader::GetOpt<uint16_t>(uint16_t& ret, const char* attr);
template<> bool JsonReader::GetOpt<uint32_t>(uint32_t& ret, const char* attr);
template<> bool JsonReader::GetOpt<float>(float& ret, const char* attr);
template<> bool JsonReader::GetOpt<double>(double& ret, const char* attr);
template<> bool JsonReader::GetOpt<Math::vec4>(Math::vec4& ret, const char* attr);
template<> bool JsonReader::GetOpt<Math::quat>(Math::quat& ret, const char* attr);
template<> bool JsonReader::GetOpt<Math::mat4>(Math::mat4& ret, const char* attr);
template<> bool JsonReader::GetOpt<Util::String>(Util::String& ret, const char* attr);
template<> bool JsonReader::GetOpt<Util::Array<int>>(Util::Array<int>& target, const char* attr);
template<> bool JsonReader::GetOpt<Util::Array<uint32_t>>(Util::Array<uint32_t>& target, const char* attr);
template<> bool JsonReader::GetOpt<Util::Array<float>>(Util::Array<float>& target, const char* attr);
template<> bool JsonReader::GetOpt<Util::Array<Util::String>>(Util::Array<Util::String>& target, const char* attr);
template<typename T>
inline bool
JsonReader::GetOpt(T& target, const char* attr)
{
    if (this->HasAttr(attr))
    {
        this->Get(target, attr);
        return true;
    }
    return false;
}
template<typename T>
inline bool
JsonReader::GetOpt(T& target, const char* attr, const T& defaultVal)
{
    if (!this->GetOpt<T>(target, attr))
    {
        target = defaultVal;
        return false;
    }
    return true;
}
template<unsigned int N>
inline void
JsonReader::Get(Util::BitField<N>& ret, const char* attr)
{
    Util::Array<int> arr;
    this->Get<Util::Array<int>>(arr);
    unsigned int count = arr.Size();
    n_assert(count <= N);
    for (unsigned int i = 0; i < count; i++)
    {
        ret.SetBit(arr[i]);
    }
}
}