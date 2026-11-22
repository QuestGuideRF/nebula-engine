#pragma once
#include "toolkit-common/base/exporterbase.h"
#include "toolkit-common/toolkitconsolehandler.h"
namespace ToolkitUtil
{
class GameExporter : public Base::ExporterBase
{
    __DeclareClass(GameExporter);
public:
    GameExporter();
    virtual ~GameExporter();
    void Open();
    void Close();
    void ExportAll();
    void SetLogger(Logger* logger);
    const Util::Array<ToolkitUtil::ToolLog> & GetLogs() const;
private:
    Logger* logger;
    Util::Array<ToolkitUtil::ToolLog> logs;
};
inline void
GameExporter::SetLogger(Logger* l)
{
    this->logger = l;
}
inline
const Util::Array<ToolkitUtil::ToolLog> &
GameExporter::GetLogs() const
{
    return this->logs;
}
}