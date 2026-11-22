#pragma once
#include "io/streamwriter.h"
#include "tinyxml/tinyxml.h"
namespace IO
{
class XmlWriter : public IO::StreamWriter
{
    __DeclareClass(XmlWriter);
public:
    XmlWriter();
    virtual ~XmlWriter();
    virtual bool Open();
    virtual void Close();
    bool BeginNode(const Util::String& nodeName);
    void EndNode();
    void WriteContent(const Util::String& text);
    void WriteComment(const Util::String& comment);
    void WriteRaw(const Util::String& xml);
    void SetString(const Util::String& name, const Util::String& value);
    void SetBool(const Util::String& name, bool value);
    void SetInt(const Util::String& name, int value);
    void SetFloat(const Util::String& name, float value);
    #if !__OSX__
    void SetVec2(const Util::String& name, const Math::vec2& value);
    void SetVec4(const Util::String& name, const Math::vec4& value);
    void SetMat4(const Util::String& name, const Math::mat4& value);
    void SetTransform44(const Util::String& name, const Math::transform44& value);
    #endif
    template<typename T> void Set(const Util::String& name, const T &value);
private:
    TiXmlDocument* xmlDocument;
    TiXmlElement* curNode;
};
}