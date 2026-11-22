#pragma once
#include "gltfdata.h"
#include "surface/surfacebuilder.h"
#include "toolkit-common/logger.h"
namespace ToolkitUtil
{
class NglTFMaterialExtractor
{
public:
    NglTFMaterialExtractor();
    ~NglTFMaterialExtractor();
    void SetDocument(Gltf::Document const* document);
    void SetLogger(ToolkitUtil::Logger* logger);
    void SetCategoryName(Util::String const& categoryName);
    void SetExportSubDirectory(Util::String const& subDir);
    Util::Array<IO::URI> ExportAll();
    void ExtractMaterial(SurfaceBuilder& builder, Gltf::Material const& material);
private:
    Gltf::Document const* doc;
    Util::String catName;
    Util::String subDir;
    Util::String texCatDir;
    Util::String textureDir;
    ToolkitUtil::Logger* logger;
};
inline void
NglTFMaterialExtractor::SetDocument(Gltf::Document const* document)
{
    this->doc = document;
}
inline void
NglTFMaterialExtractor::SetLogger(ToolkitUtil::Logger* logger)
{
    this->logger = logger;
}
inline void
NglTFMaterialExtractor::SetCategoryName(Util::String const& categoryName)
{
    this->catName = categoryName;
}
inline void
NglTFMaterialExtractor::SetExportSubDirectory(Util::String const& subDir)
{
    this->subDir = subDir;
}
}