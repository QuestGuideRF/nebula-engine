#pragma once
#include "io/streamreader.h"
#include "io/util/bxmlloaderutil.h"
namespace IO
{
class BXmlReader : public StreamReader
{
    __DeclareClass(BXmlReader);
public:
    BXmlReader();
    virtual ~BXmlReader();
    virtual bool Open();
    virtual void Close();
    bool HasNode(const Util::String& path) const;
    Util::String GetCurrentNodeName() const;
    Util::String GetCurrentNodePath() const;
    void SetToNode(const Util::String& path);
    bool SetToFirstChild(const Util::String& name = "");
    bool SetToNextChild(const Util::String& name = "");
    bool SetToParent();
    bool HasAttr(const char* attr) const;
    Util::Array<Util::String> GetAttrs() const;
    Util::String GetString(const char* attr) const;
    bool GetBool(const char* attr) const;
    int GetInt(const char* attr) const;
    float GetFloat(const char* attr) const;
    #if !__OSX__
    Math::vec2 GetVec2(const char* attr) const;
    Math::vec4 GetVec4(const char* attr) const;
    Math::mat4 GetMat4(const char* attr) const;
    #endif
    template<typename T> T Get(const char* attr) const;
    Util::String GetOptString(const char* attr, const Util::String& defaultValue) const;
    bool GetOptBool(const char* attr, bool defaultValue) const;
    int GetOptInt(const char* attr, int defaultValue) const;
    float GetOptFloat(const char* attr, float defaultValue) const;
    #if !__OSX__
    Math::vec2 GetOptVec2(const char* attr, const Math::vec2& defaultValue) const;
    Math::vec4 GetOptVec4(const char* attr, const Math::vec4& defaultValue) const;
    Math::float2 GetOptVec2(const char* attr, const Math::float2& defaultValue) const;
    Math::float4 GetOptVec4(const char* attr, const Math::float4& defaultValue) const;
    Math::mat4 GetOptMat4(const char* attr, const Math::mat4& defaultValue) const;
    #endif
private:
    BXmlLoaderUtil loaderUtil;
};
}