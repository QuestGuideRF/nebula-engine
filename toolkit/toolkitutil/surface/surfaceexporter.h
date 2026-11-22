#pragma once
#include "toolkit-common/base/exporterbase.h"
namespace ToolkitUtil
{
class SurfaceExporter : public Base::ExporterBase
{
    __DeclareClass(SurfaceExporter);
public:
    SurfaceExporter();
    virtual ~SurfaceExporter();
    void ExportFile(const IO::URI& file);
};
}