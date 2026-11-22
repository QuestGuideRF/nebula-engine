#pragma once
#include "util/dictionary.h"
#include "util/string.h"
#include "core/singleton.h"
#include "core/refcounted.h"
#include "modelattributes.h"
#include "modelconstants.h"
#include "modelphysics.h"
namespace ToolkitUtil
{
class ModelDatabase: public Core::RefCounted
{
    __DeclareSingleton(ModelDatabase);
    __DeclareClass(ModelDatabase);
public:
    ModelDatabase();
    ~ModelDatabase();
    void Open();
    void Close();
    bool IsOpen() const;
    void LoadAttributes(const Util::String& folder);
    Ptr<ModelAttributes> LookupAttributes(const Util::String& name, bool reload = false);
    bool AttributesExist(const Util::String& name);
    const Util::String& GetAttributesName(const Ptr<ModelAttributes>& attrs);
    Ptr<ModelPhysics> LookupPhysics(const Util::String& name, bool reload = false);
    bool PhysicsExist(const Util::String& name);
    const Util::String& GetPhysicsName(const Ptr<ModelPhysics>& attrs);
    Ptr<ModelConstants> LookupConstants(const Util::String& name, bool reload = false);
    bool ConstantsExist(const Util::String& name);
    const Util::String& GetConstantsName(const Ptr<ModelConstants>& constants);
private:
    bool isOpen;
    Util::Dictionary<Util::String, Ptr<ModelAttributes>> modelAttributes;
    Util::Dictionary<Util::String, Ptr<ModelConstants>> modelConstants;
    Util::Dictionary<Util::String, Ptr<ModelPhysics>> modelPhysics;
};
inline bool
ModelDatabase::IsOpen() const
{
    return this->isOpen;
}
}