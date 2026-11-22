#pragma once
#include "modelwriter.h"
#include "io/binarywriter.h"
namespace ToolkitUtil
{
class BinaryModelWriter : public ModelWriter
{
    __DeclareClass(BinaryModelWriter);
public:
    BinaryModelWriter();
    virtual ~BinaryModelWriter();
    virtual bool Open();
    virtual void Close();
    virtual bool BeginModel(const Util::String& className, Util::FourCC classFourCC, const Util::String& name);
    virtual bool BeginModelNode(const Util::String& className, Util::FourCC classFourCC, const Util::String& name);
    virtual void BeginTag(const Util::String& name, Util::FourCC tagFourCC);
    virtual bool BeginPhysicsNode(const Util::String& name);
    virtual void BeginColliderNode();
    virtual void WriteBool(bool b);
    virtual void WriteInt(int i);
    virtual void WriteFloat(float f);
    virtual void WriteFloat2(const Math::vec2& f);
    virtual void WriteVec4(const Math::vec4& f);
    virtual void WriteString(const Util::String& s);
    virtual void WriteIntArray(const Util::Array<int>& a);
    virtual void EndTag();
    virtual void EndColliderNode();
    virtual void EndPhysicsNode();
    virtual void EndModelNode();
    virtual void EndModel();
private:
    Ptr<IO::BinaryWriter> writer;
};
}