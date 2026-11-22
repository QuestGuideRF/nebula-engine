//------------------------------------------------------------------------------
//  guiexporter.cc
//  (C) 2012-2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "gui/guiexporter.h"
#include "attr/attributedefinitionbase.h"
#include "db/dbfactory.h"
#include "attr/attrid.h"
#include "db/sqlite3/sqlite3factory.h"
#include "io/ioserver.h"
#include "db/sqlite3/sqlite3command.h"
#include "io/xmlreader.h"
#include "io/stream.h"
#include "db/valuetable.h"
#include "db/dataset.h"
#include "db/table.h"
#include "io/filestream.h"

using namespace Db;
using namespace IO;
using namespace Attr;
using namespace Util;

namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::CEGuiExporter, 'CGEX', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
CEGuiExporter::CEGuiExporter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
CEGuiExporter::~CEGuiExporter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void 
CEGuiExporter::Open()
{
    ExporterBase::Open();

    IO::AssignRegistry::Instance()->SetAssign(IO::Assign("ceui", "export:/ceui/datafiles/"));
    //IO::AssignRegistry::Instance()->SetAssign(IO::Assign("ceuiwork", "work:ceui/"));
}

//------------------------------------------------------------------------------
/**
*/
void 
CEGuiExporter::Close()
{
    ExporterBase::Close();
}

//------------------------------------------------------------------------------
/**
*/
void 
CEGuiExporter::ExportAll(ToolkitUtil::TextureConverter textureConverter, ToolkitUtil::Logger logger)
{
    Util::String dstFontsDir =      "ceui:fonts/";
    Util::String dstImagesetsDir =  "ceui:imagesets/";
    Util::String dstLayoutsDir =    "ceui:layouts/";
    Util::String dstLooknfeelsDir = "ceui:looknfeel/";
    Util::String dstSchemesDir =    "ceui:schemes/";
    Util::String dstLuaScrpitsDir = "ceui:lua_scripts/";
    Util::String dstXmlSchemasDir = "ceui:xml_schemas/";
    Util::String dstAnimationsDir = "ceui:animations/";

    Util::String srcFontsDir;
    Util::String srcImagesetsDir;
    Util::String srcLayoutsDir;
    Util::String srcLooknfeelsDir;
    Util::String srcSchemesDir;
    Util::String srcXmlSchemasDir;
    Util::String srcLuaScriptsDir;
    Util::String srcAnimationsDir;

    if(!IO::IoServer::Instance()->DirectoryExists("export:/ceui/")) //создать папку экспорта ceui, если она не существует
        IO::IoServer::Instance()->CreateDirectory("export:/ceui/");

    Util::Array<Util::String> projFiles = IO::IoServer::Instance()->ListFiles("proj:work/ceui/", "*.проект");

    if(projFiles.IsEmpty())
    {
        n_printf("ОШИБКА: файл .project не найден в папке \"proj:work/ceui/\"\n");
        this->SetHasErrors(true);
        return;
    }

    n_printf("Загрузка и извлечение данных из файла .project..\n");

    //предположим, что существует только один файл .project. Загрузить файл проекта
    IO::URI file = IO::URI("proj:work/ceui/" + projFiles[0]);
    Ptr<IO::Stream> stream = IO::IoServer::Instance()->CreateStream(file);
    Ptr<IO::XmlReader> xmlReader = IO::XmlReader::Create();

    Util::String s;
    s.Format("Не удалось открыть %s", file.GetHostAndLocalPath().AsCharPtr());
    n_assert2(stream->Open(), s.AsCharPtr());
    xmlReader->SetStream(stream);
    n_assert(xmlReader->Open());

    // извлечь каталоги src из файла .project
    xmlReader->SetToFirstChild("Project");

    Util::String baseDir = xmlReader->GetString("baseDirectory");

    srcFontsDir = "proj:work/ceui/" + baseDir + "/" + xmlReader->GetString("fontsPath") + "/";
    srcImagesetsDir = "proj:work/ceui/" + baseDir + "/" + xmlReader->GetString("imagesetsPath") + "/";
    srcLayoutsDir = "proj:work/ceui/" + baseDir + "/" + xmlReader->GetString("layoutsPath") + "/";
    srcLooknfeelsDir = "proj:work/ceui/" + baseDir + "/" + xmlReader->GetString("looknfeelsPath") + "/";
    srcSchemesDir = "proj:work/ceui/" + baseDir + "/" + xmlReader->GetString("schemesPath") + "/";
    srcXmlSchemasDir = "proj:work/ceui/" + baseDir + "/" + xmlReader->GetString("xmlSchemasPath") + "/";
    srcLuaScriptsDir = "proj:work/ceui/" + baseDir + "/" + "lua_scripts/"; //assume
    srcAnimationsDir = "proj:work/ceui/" + baseDir + "/" + "animations/"; //assume


    //копировать файлы
    n_printf("Экспорт файлов шрифтов...\n");
    this->CopyFiles(srcFontsDir, dstFontsDir);
    n_printf("Экспорт файлов набора изображений...\n");
    this->CopyImagesetFiles(srcImagesetsDir, dstImagesetsDir); //special case for .imageset files (edit imagefileextension to .dds)
    n_printf("Экспорт файлов макета...\n");
    this->CopyFiles(srcLayoutsDir, dstLayoutsDir);
    n_printf("Экспорт файлов Looknfeel...\n");
    this->CopyFiles(srcLooknfeelsDir, dstLooknfeelsDir);
    n_printf("Экспорт файлов схем...\n");
    this->CopyFiles(srcSchemesDir, dstSchemesDir);
    n_printf("Экспорт файлов Xmlschemas...\n");
    this->CopyFiles(srcXmlSchemasDir, dstXmlSchemasDir);
    n_printf("Экспорт файлов Luascript...\n");
    this->CopyFiles(dstLuaScrpitsDir, srcLuaScriptsDir);
    n_printf("Экспорт файлов анимации...\n");
    this->CopyFiles(srcAnimationsDir, dstAnimationsDir);


    // экспортировать все текстуры в папку набора изображений
    Util::Array<Util::String> textureFiles = IO::IoServer::Instance()->ListFiles(srcImagesetsDir, "*", true);

    textureConverter.SetSrcDir(srcImagesetsDir);
    textureConverter.SetDstDir("ceui:"); //исправлена ​​структура папок, в которых оказываются экспортированные изображения.
    if (!textureConverter.Setup(logger))
    {
        n_printf("ОШИБКА: не удалось настроить конвертер текстур!\n");
        this->SetHasErrors(true);
    }
    n_printf("Преобразование файлов текстур...\n");
    if(!textureConverter.ConvertFiles(textureFiles))
    {
        n_printf("ОШИБКА: текстуры не экспортированы.\n");
        this->SetHasErrors(true);
    }

    textureConverter.Discard();

    xmlReader->Close();
    stream->Close();

    n_printf("\nЭкспорт графического интерфейса готово!\n\n");

}

//------------------------------------------------------------------------------------
/**
*/
void
CEGuiExporter::CopyFiles( Util::String srcDir, Util::String dstDir, Util::String filter )
{
    Util::Array<Util::String> fileList = IO::IoServer::Instance()->ListFiles(srcDir, filter);

    if(!IO::IoServer::Instance()->DirectoryExists(dstDir))
        IO::IoServer::Instance()->CreateDirectory(dstDir);

    IndexT index;
    for (index = 0; index < fileList.Size(); index++)
    {
        Util::String fileName = fileList[index].ExtractFileName();
        n_assert(IO::IoServer::Instance()->CopyFile(srcDir + fileList[index], dstDir + fileName));
    }
}

//------------------------------------------------------------------------------------
/**
*/
void
CEGuiExporter::CopyImagesetFiles( Util::String srcDir, Util::String dstDir )
{

    ///редактировать файлы .imageset
    Util::Array<Util::String> fileList = IO::IoServer::Instance()->ListFiles(srcDir, "*.imageset");

    if(!IO::IoServer::Instance()->DirectoryExists(dstDir))
        IO::IoServer::Instance()->CreateDirectory(dstDir);

    IndexT index;
    for(index = 0; index < fileList.Size(); index++)
    {
        IO::URI file = IO::URI(srcDir + fileList[index]);
        Ptr<IO::Stream> stream = IO::IoServer::Instance()->CreateStream(file);
        Ptr<IO::XmlReader> xmlReader = IO::XmlReader::Create();
 
        Util::String s;
        s.Format("Не удалось открыть %s", file.AsString().AsCharPtr());
        n_assert2(stream->Open(), s.AsCharPtr());
        xmlReader->SetStream(stream);
        n_assert(xmlReader->Open());

        //открыть как XML и найти описание файла изображения
        xmlReader->SetToFirstChild("Imageset");
        Util::String replaceFrom = xmlReader->GetString("Imagefile");
        Util::String replaceTo = replaceFrom;
        replaceTo.StripFileExtension();
        replaceTo += ".dds";

        xmlReader->Close();

        //открыть/создать файл назначения
        IO::URI exportFile = IO::URI(dstDir + fileList[index]);
        Ptr<IO::FileStream> fileStream =  IO::IoServer::Instance()->CreateStream(exportFile).downcast<IO::FileStream>();
        fileStream->SetAccessMode(IO::Stream::ReadWriteAccess);
        s.Format("Не удалось открыть %s", exportFile.AsString().AsCharPtr());
        n_assert2(fileStream->Open(), s.AsCharPtr());

        //загрузить все данные в виде строки
        void* data = stream->Map();
        int size = stream->GetSize(); //Будем надеяться, что расширение рабочего файла (tga,psd,png,dds,tar и т. д.) будет того же размера, что и расширение экспортированного файла (dds). 3

        Util::String dataAsString((const char*)data);
        dataAsString.SubstituteString(replaceFrom, replaceTo); //замените расширение файла изображения на .dds

        // записать в файл
        fileStream->Write(dataAsString.AsCharPtr(), size);

        fileStream->Close();
        stream->Close();
    }
}

} // пространство имен ToolkitUtil
