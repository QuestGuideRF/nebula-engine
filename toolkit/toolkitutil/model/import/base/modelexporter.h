#pragma once
#include "toolkit-common/base/exporterbase.h"
#include "toolkit-common/base/exporttypes.h"
#include "toolkitutil/texutil/textureconverter.h"
#include "model/import/base/scene.h"
namespace ToolkitUtil
{
class ModelExporter : public Base::ExporterBase
{
public:
    ModelExporter();
    ~ModelExporter();
    virtual bool ParseScene();
    void SetLogger(ToolkitUtil::Logger* logger);
    void ExportFile(const IO::URI& file);
    void SetTextureConverter(TextureConverter* texConv);
    void SetExportFlags(const ToolkitUtil::ExportFlags& exportFlags);
protected:
    bool NeedsConversion(const Util::String& path);
    ToolkitUtil::ExportFlags exportFlags;
    float sceneScale;
    Util::String file;
    IO::URI path;
    Util::Array<IO::URI> outputFiles;
    Scene* scene;
    TextureConverter* texConverter;
    ToolkitUtil::Logger* logger;
};
inline void
ModelExporter::SetExportFlags(const ToolkitUtil::ExportFlags& exportFlags)
{
    this->exportFlags = exportFlags;
}
inline void
ModelExporter::SetTextureConverter(TextureConverter* texConv)
{
    this->texConverter = texConv;
}
inline void
ModelExporter::SetLogger(ToolkitUtil::Logger* logger)
{
    this->logger = logger;
}
}