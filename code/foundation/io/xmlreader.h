#pragma once
#include "io/streamreader.h"
#include "tinyxml/tinyxml.h"
namespace IO
{
class XmlReader : public StreamReader
{
    __DeclareClass(XmlReader);
public:
    XmlReader();
    virtual ~XmlReader();
    virtual bool Open();
    virtual void Close();
    bool HasNode(const Util::String& path) const;
    Util::String GetCurrentNodeName() const;
    Util::String GetCurrentNodePath() const;
    int GetCurrentNodeLineNumber() const;
    void SetToRoot();
    void SetToNode(const Util::String& path);
    bool SetToFirstChild(const Util::String& name = "");
    bool SetToNextChild(const Util::String& name = "");
    bool SetToParent();
    bool HasAttr(const char* attr) const;
    Util::Array<Util::String> GetAttrs() const;
    bool HasContent() const;
    Util::String GetContent() const;
    bool HasComment() const;
    Util::String GetComment() const;
    Util::String GetString(const char* attr) const;
    bool GetBool(const char* attr) const;
    int GetInt(const char* attr) const;
    float GetFloat(const char* attr) const;
    #if !__OSX__
    Math::vec2 GetVec2(const char* attr) const;
    Math::vec4 GetVec4(const char* attr) const;
    Math::mat4 GetMat4(const char* attr) const;
    Math::transform44 GetTransform44(const char* attr) const;
    #endif
    template<typename T> T Get(const char* attr) const;
    Util::String GetOptString(const char* attr, const Util::String& defaultValue) const;
    bool GetOptBool(const char* attr, bool defaultValue) const;
    int GetOptInt(const char* attr, int defaultValue) const;
    float GetOptFloat(const char* attr, float defaultValue) const;
    #if !__OSX__
    Math::vec2 GetOptVec2(const char* attr, const Math::vec2& defaultValue) const;
    Math::vec4 GetOptVec4(const char* attr, const Math::vec4& defaultValue) const;
    Math::mat4 GetOptMat4(const char* attr, const Math::mat4& defaultValue) const;
    Math::transform44 GetOptTransform44(const char* attr, const Math::transform44& defaultValue) const;
    #endif
private:
    TiXmlNode* FindNode(const Util::String& path) const;
    class TinyXmlInitHelper
    {
    public:
        TinyXmlInitHelper()
        {
            TiXmlBase::SetCondenseWhiteSpace(true);
        }
    };
    static TinyXmlInitHelper initTinyXml;
    TiXmlDocument* xmlDocument;
    TiXmlElement* curNode;
};
}