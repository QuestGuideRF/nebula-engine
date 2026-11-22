//------------------------------------------------------------------------------
//  modelbatcher3app.cc
//  (C) 2011–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "modelbatcher3app.h"
#include "io/ioserver.h"
#include "io/textreader.h"

using namespace Util;
using namespace ToolkitUtil;
using namespace IO;

namespace Toolkit
{

//------------------------------------------------------------------------------
/**
*/
ModelBuilder3App::ModelBuilder3App()
{
    // empty    
}

//------------------------------------------------------------------------------
/**
*/
ModelBuilder3App::~ModelBuilder3App()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool 
ModelBuilder3App::Open()
{   
    if (DistributedToolkitApp::Open())
    {
        return true;
    }
    
    return false;
}


//------------------------------------------------------------------------------
/**
*/
void 
ModelBuilder3App::DoWork()
{
    String dir = "";
    String file = "";
    String projectFolder = "proj:";

    if (this->args.HasArg("-projectdir"))
    {
        projectFolder = this->args.GetString("-projectdir") + "/";
    }    
    IO::AssignRegistry::Instance()->SetAssign(Assign("home", "proj:"));

    Array<String> fileList = CreateFileList();

    // объекты настройки
    this->builder = ModelBuilder::Create();
    this->database = ModelDatabase::Create();

    // открытая база данных
    this->database->Open();

    // экспортировать все файлы
    IndexT i;
    for (i = 0; i < fileList.Size(); i++)
    {
        // получить файл
        String file = fileList[i];
        file.StripFileExtension();

        // получить константы, атрибуты и физику
        Ptr<ModelConstants> constants = ModelDatabase::Instance()->LookupConstants(file);
        Ptr<ModelAttributes> attributes = ModelDatabase::Instance()->LookupAttributes(file);
        Ptr<ModelPhysics> physics = ModelDatabase::Instance()->LookupPhysics(file);

        // настройка построителя моделей
        this->builder->SetAttributes(attributes);
        this->builder->SetConstants(constants);
        this->builder->SetPhysics(physics);

        // создать путь модели
        String modelPath;
        modelPath.Format("mdl:%s.n3", file.AsCharPtr());

        // создать физический путь
        String physicsPath;
        physicsPath.Format("физ:%s.np3", file.AsCharPtr());
        
        // export
        this->builder->SaveN3(modelPath, this->platform);
        this->builder->SaveN3Physics(physicsPath, this->platform);
        n_printf("Модель здания: %s\n", file.AsCharPtr());
        n_printf("    Результат: \n [graphics = '%s'] \n [физика = '%s']\n\n", modelPath.AsCharPtr(), physicsPath.AsCharPtr());
    }

    // уничтожить базу данных
    this->database->Close();

    this->database = 0;
    this->builder = 0;
}

//------------------------------------------------------------------------------
/**
*/
bool 
ModelBuilder3App::ParseCmdLineArgs()
{
    return DistributedToolkitApp::ParseCmdLineArgs();   
}

//------------------------------------------------------------------------------
/**
*/
bool 
ModelBuilder3App::SetupProjectInfo()
{
    if (DistributedToolkitApp::SetupProjectInfo())
    {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelBuilder3App::ShowHelp()
{
    n_printf("Дозатор моделей Nebula Trifid.\n"
            "(C) Густав Стербрант, 2011 г.\n"
            "-help -- отобразить эту справку\n"
            "-platform -- выбрать платформу (win32)\n");
}

//------------------------------------------------------------------------------
/**
*/
Util::Array<Util::String> 
ModelBuilder3App::CreateFileList()
{
    Util::Array<Util::String> res;

    // создать список из заданного списка файлов
    if(this->listfileArg.IsValid())
    {
        // создать путь к списку аргументов
        URI listUri(this->listfileArg);

        // открытый поток и читалка
        Ptr<Stream> readStream = IoServer::Instance()->CreateStream(listUri);
        readStream->SetAccessMode(Stream::ReadAccess);

        // создать читателя
        Ptr<TextReader> reader = TextReader::Create();
        reader->SetStream(readStream);

        // открытый поток
        if (reader->Open())
        {
            // прочитайте каждую строку и добавьте в список
            while(!reader->Eof())
            {
                String srcPath = reader->ReadLine();
                srcPath.Trim(" \r\n");
                res.Append(srcPath);                
            }
            // закрыть поток и читатель
            reader->Close();
        }
        reader = 0;
        readStream = 0;
    }
    else
    {               
        String workDir = "proj:work/assets";
        Array<String> directories = IoServer::Instance()->ListDirectories(workDir, "*");
        for (int directoryIndex = 0; directoryIndex < directories.Size(); directoryIndex++)
        {
            String category = workDir + "/" + directories[directoryIndex];          
            Array<String> files = IoServer::Instance()->ListFiles(category, "*.константы");
            for (int fileIndex = 0; fileIndex < files.Size(); fileIndex++)
            {
                String file = directories[directoryIndex] + "/" + files[fileIndex];
                res.Append(file);           
            }
        }   
    }   
    return res;
}
} // пространство имен ToolkitUtil