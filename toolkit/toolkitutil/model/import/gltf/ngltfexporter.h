#pragma once
#include "model/import/base/modelexporter.h"
#include "toolkit-common/base/exporttypes.h"
#include "gltfdata.h"
#include "node/ngltfscene.h"
#include "toolkitutil/texutil/textureconverter.h"
namespace ToolkitUtil
{
class NglTFExporter : public ModelExporter
{
    __DeclareClass(NglTFExporter);
public:
    NglTFExporter();
    virtual ~NglTFExporter();
    bool ParseScene() override;
    void SetTextureConverter(TextureConverter* texConv);
private:
    Gltf::Document gltfScene;
    TextureConverter* texConverter;
};
inline void
NglTFExporter::SetTextureConverter(TextureConverter* texConv)
{
    this->texConverter = texConv;
}
}