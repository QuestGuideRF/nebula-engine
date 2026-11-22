#pragma once
#include "io/streamwriter.h"
#include "util/string.h"
#include "util/stack.h"
#include "util/bitfield.h"
#include "util/stringatom.h"
#include "util/variant.h"
#include "util/blob.h"
namespace pjson
{
    class value_variant;
    class document;
}
namespace IO
{
class JsonWriter : public IO::StreamWriter
{
    __DeclareClass(JsonWriter);
public:
    JsonWriter();
    virtual ~JsonWriter();
    virtual bool Open();
    virtual void Close();
    void BeginArray(const char * nodeName = nullptr);
    void BeginObject(const char * nodeName = nullptr);
    void End();
    template <typename T> void Add(const T & value, const Util::String & name = "");
	template<unsigned int N> void Add(Util::BitField<N> const& value, const Util::String& name = "");
    void Add(const char * value, const Util::String & name = "");
private:
    pjson::document * document;
    Util::Stack<pjson::value_variant*> hierarchy;
    Util::Stack<Util::String> nameHierarchy;
};
template<> void JsonWriter::Add(const Util::String& value, const Util::String& name);
template<> void JsonWriter::Add(const Util::StringAtom& value, const Util::String& name);
template<> void JsonWriter::Add(const Util::FourCC& value, const Util::String& name);
template<> void JsonWriter::Add(const bool& value, const Util::String& name);
template<> void JsonWriter::Add(const char& value, const Util::String& name);
template<> void JsonWriter::Add(const uchar& value, const Util::String& name);
template<> void JsonWriter::Add(const int& value, const Util::String& name);
template<> void JsonWriter::Add(const unsigned int& value, const Util::String& name);
template<> void JsonWriter::Add(const float& value, const Util::String& name);
template<> void JsonWriter::Add(const double& value, const Util::String& name);
template<> void JsonWriter::Add(const Math::vec3& value, const Util::String& name);
template<> void JsonWriter::Add(const Math::vec4& value, const Util::String& name);
template<> void JsonWriter::Add(const Math::vec2& value, const Util::String& name);
template<> void JsonWriter::Add(const Math::mat4& value, const Util::String& name);
template<> void JsonWriter::Add(const Math::transform44& value, const Util::String& name);
template<> void JsonWriter::Add(const Util::Variant& value, const Util::String& name);
template<> void JsonWriter::Add(const Util::Guid& value, const Util::String& name);
template<> void JsonWriter::Add(const Util::Array<int>& value, const Util::String& name);
template<> void JsonWriter::Add(const Util::Array<Util::String>& value, const Util::String& name);
template<unsigned int N>
inline void
JsonWriter::Add(Util::BitField<N> const& value, const Util::String& name)
{
	Util::Array<int> tempArr;
	for (uint i = 0; i < N; i++)
	{
		if (value.IsSet(i))
		{
			tempArr.Append(i);
		}
	}
	this->Add(tempArr, name);
}
}