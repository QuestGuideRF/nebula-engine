#pragma once
#include "base/exporterbase.h"
#include "toolkitutil/platform.h"
#include "math/bbox.h"
namespace ToolkitUtil
{
class PhysicsExporter : public Base::ExporterBase
{
    __DeclareClass(PhysicsExporter);
public:
    PhysicsExporter();
    virtual ~PhysicsExporter();
    void Open();
    void Close();
    void ExportAll();
    void ExportDir(const Util::String& category);
    void ExportFile(const IO::URI& file);
private:
    bool NeedsDummyPhysics(const Util::String& category, const Util::String& file);
    bool NeedsConversion(const Util::String& path);
    bool NeedsConversion(const Util::String& src, const Util::String& dst);
    void CreateDummyPhysics(const Math::bbox& box, const Util::String& category, const Util::String& file);
};
}