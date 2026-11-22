//------------------------------------------------------------------------------
//  surfaceexporter.cc
//  (C) 2015–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "surfaceexporter.h"
#include "io/xmlreader.h"
#include "io/ioserver.h"
#include "toolkit-common/converters/binaryxmlconverter.h"
#include "toolkit-common/text.h"

using namespace Util;
using namespace IO;
namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::SurfaceExporter, 'SUEX', Base::ExporterBase);

//------------------------------------------------------------------------------
/**
*/
SurfaceExporter::SurfaceExporter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
SurfaceExporter::~SurfaceExporter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
SurfaceExporter::ExportFile(const IO::URI& file)
{
    // получить локальный путь
    String localPath = file.LocalPath();

    // вычесть имя файла из URL
    String fileName = localPath.ExtractFileName();
    fileName.StripFileExtension();
    String catName = localPath.ExtractLastDirName();
    String dst = String::Sprintf("sur:%s/%s.sur", catName.AsCharPtr(), fileName.AsCharPtr());

    // создать папку, если она не существует
    if (!IoServer::Instance()->DirectoryExists("sur:" + catName))
    {
        IoServer::Instance()->CreateDirectory("sur:" + catName);
    }

    // просто конвертируйте XML в двоичный формат
    this->logger->Print("%s -> %s\n", Text(file.LocalPath()).Color(TextColor::Blue).AsCharPtr(), Text(URI(Format("sur:%s/%s.sur", catName.AsCharPtr(), fileName.AsCharPtr())).LocalPath()).Color(TextColor::Green).Style(FontMode::Underline).AsCharPtr());
    BinaryXmlConverter converter;
    converter.ConvertFile(localPath, dst, *this->logger);
}

} // пространство имен ToolkitUtil