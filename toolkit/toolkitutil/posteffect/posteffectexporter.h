#pragma once
#include "base/exporterbase.h"
#include "db/dbfactory.h"
#include "db/database.h"
#include "logger.h"
namespace ToolkitUtil
{
class PostEffectExporter : public Base::ExporterBase
{
    __DeclareClass(PostEffectExporter);
public:
    PostEffectExporter();
    virtual ~PostEffectExporter();
    void Open();
    void Close();
    void SetDb(const Ptr<Db::Database>& staticDb);
    void ExportAll();
    void SetLogger(Logger* logger);
private:
    void SetupTables();
    void CheckDefaultPreset();
    Ptr<Db::Database> staticDb;
    Logger* logger;
};
inline void
PostEffectExporter::SetDb(const Ptr<Db::Database>& staticDb)
{
    this->staticDb = staticDb;
}
inline void
PostEffectExporter::SetLogger(Logger* l)
{
    this->logger = l;
}
}