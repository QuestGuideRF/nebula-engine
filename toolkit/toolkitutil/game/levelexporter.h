#pragma once
#include "toolkit-common/base/exporterbase.h"
#include "toolkit-common/platform.h"
#include "io/jsonreader.h"
#include "toolkit-common/toolkitconsolehandler.h"
#include "memdb/database.h"
namespace Game
{
class World;
}
namespace ToolkitUtil
{
class LevelExporter : public Base::ExporterBase
{
    __DeclareClass(LevelExporter);
public:
    LevelExporter();
    virtual ~LevelExporter();
    void Open();
    void Close();
    void ExportFile(const IO::URI& file) override;
    void ExportDir(const Util::String& category) override;
    void ExportAll() override;
    void SetLogger(ToolkitUtil::Logger* logger);
private:
    bool ExportLevel(const Ptr<IO::JsonReader>& reader, Game::World* world, const IO::URI& outputFile);
    ToolkitUtil::Logger* logger;
};
inline void
LevelExporter::SetLogger(ToolkitUtil::Logger* l)
{
    this->logger = l;
}
}