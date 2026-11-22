#pragma once
#include "toolkit-common/base/exporterbase.h"
#include "model/import/fbx/nfbxexporter.h"
#include "texutil/textureconverter.h"
#include "model/modelutil/modelbuilder.h"
#include "model/modelutil/modeldatabase.h"
#include "surface/surfaceexporter.h"
#include "toolkit-common/toolkitconsolehandler.h"
#include "model/import/gltf/ngltfexporter.h"
namespace ToolkitUtil
{
class AssetExporter : public Base::ExporterBase
{
    __DeclareClass(AssetExporter);
public:
    enum ExportModes
    {
        FBX = 1 << 0,
        Models = 1 << 1,
        Textures = 1 << 2,
        Surfaces = 1 << 3,
        GLTF = 1 << 4,
        Physics = 1 << 5,
        Audio = 1 << 6,
        All = FBX | Models | Textures | Surfaces | GLTF | Physics | Audio,
        ForceFBX = 1 << 7,
        ForceModels = 1 << 8,
        ForceTextures = 1 << 9,
        ForceSurfaces = 1 << 10,
        ForceGLTF = 1 << 11,
        ForcePhysics = 1 << 12,
        ForceAudio = 1 << 13,
        ForceAll = ForceFBX | ForceModels | ForceTextures | ForceGLTF | ForcePhysics | ForceAudio
    };
    AssetExporter();
    virtual ~AssetExporter();
    void Open();
    void Close();
    bool IsOpen() const;
    void UpdateSource();
    void ExportFile(const IO::URI& file) override;
    void ExportDir(const Util::String& category) override;
    void ExportAll() override;
    void ExportFolder(const Util::String& folder, const Util::String& category);
    void ExportList(const Util::Array<Util::String>& files);
    void SetExportMode(unsigned int mode);
    const Util::Array<ToolkitUtil::ToolLog> & GetMessages() const;
private:
    Ptr<ToolkitUtil::NFbxExporter> fbxExporter;
    Ptr<ToolkitUtil::NglTFExporter> gltfExporter;
    ToolkitUtil::TextureConverter textureExporter;
    Ptr<ToolkitUtil::SurfaceExporter> surfaceExporter;
    Ptr<ToolkitUtil::ModelBuilder> modelBuilder;
    ToolkitUtil::TextureAttrTable textureAttrTable;
    unsigned int mode;
    Util::Array<ToolLog> messages;
};
__ImplementEnumBitOperators(AssetExporter::ExportModes);
inline
const Util::Array<ToolkitUtil::ToolLog> &
AssetExporter::GetMessages() const
{
    return this->messages;
}
}