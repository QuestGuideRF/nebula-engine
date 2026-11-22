#pragma once
#include "core/types.h"
#include "platform.h"
#include "util/dictionary.h"
#include "io/uri.h"
namespace ToolkitUtil
{
class ProjectInfo
{
public:
    enum Result
    {
        Success = 1,
        NoProjPathInRegistry,
        NoProjectInfoFile,
        CouldNotOpenProjectInfoFile,
        ProjectFileContentInvalid,
    };
    ProjectInfo();
    ~ProjectInfo();
    Result Setup();
    void Discard();
    bool IsValid() const;
    Util::String GetErrorString(Result res) const;
    bool HasAttr(const Util::String& attrName) const;
    const Util::String& GetAttr(const Util::String& attrName) const;
    bool HasListAttr(const Util::String& attrName) const;
    const Util::Dictionary<Util::String, Util::String>& GetListAttr(const Util::String& attrName) const;
    Util::String GetPathAttr(const Util::String& attrName) const;
    static ProjectInfo SetupFromFile(const Util::String & path);
private:
    Util::String QueryProjectPathFromRegistry();
    Util::String QueryToolkitPathFromRegistry();
    Result ParseProjectInfoFile(const IO::URI & path);
    void AddAttrAssigns();
    Util::Dictionary<Util::String, Util::String> attrs;
    Util::Dictionary<Util::String, Util::Dictionary<Util::String, Util::String>> listAttrs;
};
}