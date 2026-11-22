//------------------------------------------------------------------------------
//  levelexporter.cc
//  (C) 2012–2015 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "game/levelexporter.h"
#include "io/ioserver.h"
#include "io/jsonreader.h"
#include "game/world.h"
#include "basegamefeature/levelparser.h"

using namespace IO;
using namespace Util;
using namespace MemDb;

namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::LevelExporter, 'LEXP', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
LevelExporter::LevelExporter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
LevelExporter::~LevelExporter()
{
    // empty
}


//------------------------------------------------------------------------------
/**
*/
void 
LevelExporter::Open()
{
    ExporterBase::Open();
}


//------------------------------------------------------------------------------
/**
*/
void 
LevelExporter::Close()
{
    ExporterBase::Close();
}

//------------------------------------------------------------------------------
/**
*/
void 
LevelExporter::ExportAll()
{
    String levelDir = "proj:work/levels";
    Array<String> files = IoServer::Instance()->ListFiles(IO::URI(levelDir), "*.json");
    for (int fileIndex = 0; fileIndex < files.Size(); fileIndex++)
    {
        this->ExportFile(levelDir + "/" + files[fileIndex]);
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
LevelExporter::ExportDir( const String& category )
{
    /// провал, мы хотим перегрузить базовый класс, но нас не волнует категория
    this->ExportAll();
}

//------------------------------------------------------------------------------
/**
*/
void 
LevelExporter::ExportFile( const URI& file )
{
    n_assert(this->isOpen);
   
    Ptr<Stream> levelStream = IoServer::Instance()->CreateStream(file);
    Ptr<JsonReader> jsonReader = JsonReader::Create();
    
    if(!levelStream->Open())
    {        
        this->logger->Error("Не удалось открыть уровень: %s\n",file.GetHostAndLocalPath().AsCharPtr());
        this->SetHasErrors(true);
        return;
    }
    if(!levelStream->GetSize())
    {
        this->logger->Error("Пустой файл уровня: %s\n", file.GetHostAndLocalPath().AsCharPtr());
        this->SetHasErrors(true);
        return;
    }
    jsonReader->SetStream(levelStream);
    if (!jsonReader->Open())
    {
        this->logger->Error("Не удалось открыть уровень: %s\n", file.GetHostAndLocalPath().AsCharPtr());
        this->SetHasErrors(true);
        return;
    }
    
    if (jsonReader->HasNode("/level"))
    {
        this->Progress(5, file.AsString());
        this->logger->Print("Экспорт: %s\n", file.GetHostAndLocalPath().ExtractFileName().AsCharPtr());
       
        Game::World* world = new Game::World(Game::WorldHash('TEMP'), 0);
        this->ExportLevel(jsonReader, world, file);
        delete world;
    }
    else
    {
        n_printf("Уровень: %s либо поврежден, либо не является уровнем.", file.GetHostAndLocalPath().AsCharPtr());
    }

    // очистка читателей
    levelStream->Close();
    jsonReader->Close();
}

//------------------------------------------------------------------------------
/**
*/
bool 
LevelExporter::ExportLevel(const Ptr<IO::JsonReader>& reader, Game::World* world, const IO::URI& outputFile)
{
    Ptr<BaseGameFeature::LevelParser> parser = BaseGameFeature::LevelParser::Create();
    parser->SetWorld(world);
    Util::Array<Game::Entity> entities = parser->LoadJsonLevel(reader);
    if (!entities.IsEmpty())
    {
        world->ExportLevel(outputFile.GetHostAndLocalPath());
        return true;
    }
    return false;
}

} // пространство имен ToolkitUtil