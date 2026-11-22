#pragma once
#include "io/streamwriter.h"
#include "toolkit-common/platform.h"
namespace ToolkitUtil
{
class ModelWriter : public IO::StreamWriter
{
    __DeclareClass(ModelWriter);
public:
    ModelWriter();
    virtual ~ModelWriter();
    void SetPlatform(Platform::Code platform);
    void SetVersion(int version);
    virtual bool Open() = 0;
    virtual void Close() = 0;
    virtual bool BeginModel(const Util::String& className, Util::FourCC classFourCC, const Util::String& name) = 0;
    virtual bool BeginModelNode(const Util::String& className, Util::FourCC classFourCC, const Util::String& name) = 0;
    virtual bool BeginPhysicsNode(const Util::String& name) = 0;
    virtual void BeginColliderNode() = 0;
    virtual void BeginTag(const Util::String& name, Util::FourCC tagFourCC) = 0;
    virtual void WriteBool(bool b) = 0;
    virtual void WriteInt(int i) = 0;
    virtual void WriteFloat(float f) = 0;
    virtual void WriteFloat2(const Math::vec2& f) = 0;
    virtual void WriteVec4(const Math::vec4& f) = 0;
    virtual void WriteString(const Util::String& s) = 0;
    virtual void WriteIntArray(const Util::Array<int>& a) = 0;
    virtual void EndTag() = 0;
    virtual void EndColliderNode() = 0;
    virtual void EndPhysicsNode() = 0;
    virtual void EndModelNode() = 0;
    virtual void EndModel() = 0;
protected:
    Platform::Code platform;
    int version;
};
inline void
ModelWriter::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
inline void
ModelWriter::SetVersion(int v)
{
    this->version = v;
}
}