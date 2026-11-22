#pragma once
#include "base/exporterbase.h"
#include "db/database.h"
#include "db/dbfactory.h"
#include "io/xmlreader.h"
#include "texutil/textureconverter.h"
#include "logger.h"
namespace ToolkitUtil
{
class CEGuiExporter : public Base::ExporterBase
{
    __DeclareClass(CEGuiExporter);
public:
    CEGuiExporter();
    virtual ~CEGuiExporter();
    void Open();
    void Close();
    void ExportAll(ToolkitUtil::TextureConverter textureConverter, ToolkitUtil::Logger logger );
private:
    void CopyFiles(Util::String srcDir, Util::String dstDir, Util::String filter = "*.*");
    void CopyImagesetFiles(Util::String srcDir, Util::String dstDir );
};
}