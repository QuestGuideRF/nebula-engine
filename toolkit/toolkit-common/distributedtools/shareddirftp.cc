//------------------------------------------------------------------------------
//  shareddirftp.cc
//  (C) 2009 г. РадонЛабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "shareddirftp.h"
#include "io/memorystream.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "io/textwriter.h"
#include "io/textreader.h"
#include "applauncher.h"
#include "io/console.h"
#include "timing/timer.h"

using namespace IO;
using namespace Util;
using namespace ToolkitUtil;

namespace DistributedTools
{
    __ImplementClass(DistributedTools::SharedDirFTP,'SDTP',DistributedTools::SharedDirControl)
//------------------------------------------------------------------------------
/**
    Конструктор 
*/
SharedDirFTP::SharedDirFTP() :
    needVerboseCommand(false)
{
}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
SharedDirFTP::~SharedDirFTP()
{
}

//------------------------------------------------------------------------------
/**
    скопировать файл в общий каталог (src — абсолютный путь, dst — относительно контрольного каталога)
*/
void
SharedDirFTP::CopyFileToSharedDir(const Util::String & src, const Util::String & dst)
{
    // создать подкаталог, если он не существует
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    String dstPath;
    dstPath.Format("%s/%s",this->sharedDir.LocalPath().AsCharPtr(),dst.AsCharPtr());
    this->ScriptCreateSubDirectory(writer, dstPath);
    this->ScriptPutFile(writer, src);
    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    copy all files from src to shared dir (not recursive, only toplevel)
    (src being absolute path, dst relative to control dir)
*/
void
SharedDirFTP::CopyFilesToSharedDir(const Util::String & src, const Util::String & dst)
{
    // создать подкаталог, если он не существует
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    String dstPath;
    dstPath.Format("%s/%s",this->sharedDir.LocalPath().AsCharPtr(),dst.AsCharPtr());
    this->ScriptCreateSubDirectory(writer, dstPath);
    Array<String> files = IoServer::Instance()->ListFiles(URI(src),"*.*");
    String path;
    IndexT i;
    for(i = 0; i < files.Size(); i++)
    {
        path.Format("%s/%s",src.AsCharPtr(),files[i].AsCharPtr());
        this->ScriptPutFile(writer, path);  
    }
    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    copy everything (including sub dirs) from src into
    shared dirs guid folder (src being absolute path)
*/
void
SharedDirFTP::CopyDirectoryContentToSharedDir(const Util::String & src)
{
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    this->ScriptPutDirectoryContent(writer, src,this->sharedDir.LocalPath());
    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    copy file from shared dir to dst (dst being absolute, src relative to control dir)
*/
void
SharedDirFTP::CopyFileFromSharedDir(const Util::String & src, const Util::String & dst)
{
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    this->ScriptGetFiles(writer, src,dst);
    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    copy all files from shared dir sub dir to dst (not recursive, only toplevel)
    (dst being absolute, src relative to control dir)
*/
void
SharedDirFTP::CopyFilesFromSharedDir(const Util::String & src, const Util::String & dst)
{
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    this->ScriptGetFiles(writer, src,AssignRegistry::Instance()->ResolveAssignsInString(dst));
    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    copy everything (including sub dirs) from shared dirs guid folder
    into dst (dst being absolute path)
*/
void
SharedDirFTP::CopyDirectoryContentFromSharedDir(const Util::String & dst)
{
    // получить иерархию общего каталога
    FTPFile file = this->GetFtpDirectoryFileHierachy(this->sharedDir.LocalPath());
    
    // вызвать команду get для каждого файла в данной иерархии FTP
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    
    this->ScriptCreateSubDirectory(writer,this->sharedDir.LocalPath());
    this->ScriptGetFileHierachy(writer, file,AssignRegistry::Instance()->ResolveAssignsInString(dst));
    
    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    удаляет указанный файл (относительно управляющего каталога)
*/
void
SharedDirFTP::RemoveFileInSharedDir(const Util::String & filepath)
{
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    
    writer->WriteFormatted("удалить %s/%s\r\n",
        this->sharedDir.LocalPath().AsCharPtr(),
        AssignRegistry::Instance()->ResolveAssignsInString(filepath).AsCharPtr()
        );

    this->RunScript(this->CloseWriterAndGetStream(writer));
}
    
//------------------------------------------------------------------------------
/**
    removes specified directory if empty (relative to control dir)
*/
void
SharedDirFTP::RemoveDirectoryInSharedDir(const Util::String & dirpath)
{
    Ptr<TextWriter> writer = this->CreateScriptWriter();

    writer->WriteFormatted("rmdir %s/%s\r\n",
        this->sharedDir.LocalPath().AsCharPtr(),
        AssignRegistry::Instance()->ResolveAssignsInString(dirpath).AsCharPtr()
        );

    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    удалить все файлы и каталоги указанного
    каталог (относительно управляющего каталога) рекурсивно 
*/
void
SharedDirFTP::RemoveDirectoryContent(const Util::String & dir)
{
    // получить иерархию общего каталога
    FTPFile file = this->GetFtpDirectoryFileHierachy(URI(dir).LocalPath());

    Ptr<TextWriter> writer = this->CreateScriptWriter();

    String dirPath = URI(dir).LocalPath();
    // обрезать неожиданные косые черты на пути
    dirPath.Trim("\\/");
    writer->WriteFormatted("компакт-диск %s\r\n",dirPath.AsCharPtr());
    this->ScriptRemoveFileHierachy(writer, file);
    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    проверяет, существует ли подкаталог, имя которого соответствует заданному идентификатору
*/
bool
SharedDirFTP::ContainsGuidSubDir(const Util::Guid & guid)
{
    // получить иерархию общего каталога
    FTPFile file = this->GetFtpDirectoryFileHierachy(this->path.LocalPath(),false);
    
    IndexT idx;
    for(idx = 0; idx < file.content.Size(); idx++)
    {
        if(file.content[idx].name == guid.AsString())
        {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    checks if directory is empty (relative to control dir)
*/
bool
SharedDirFTP::DirIsEmpty(const Util::String & dir)
{
    // получить иерархию общего каталога
    String path;
    path.Format("%s/%s",this->sharedDir.LocalPath().AsCharPtr(),dir.AsCharPtr());
    
    FTPFile file = this->GetFtpDirectoryFileHierachy(path);
    return file.content.Size() == 0;

}
    
//------------------------------------------------------------------------------
/**
    возвращает все подкаталоги указанного каталога (относительно управляющего каталога)
*/
Util::Array<Util::String>
SharedDirFTP::ListDirectories(const Util::String & dir)
{
    // получить иерархию общего каталога
    String path;
    path.Format("%s/%s",this->sharedDir.LocalPath().AsCharPtr(),dir.AsCharPtr());
    
    FTPFile file = this->GetFtpDirectoryFileHierachy(path);
    
    Array<String> res;

    IndexT idx;
    for(idx = 0; idx < file.content.Size(); idx++)
    {
        if(file.content[idx].type == Directory)
        {
            res.Append(file.content[idx].name);
        }
    }
    return res;
}

//------------------------------------------------------------------------------
/**
    возвращает все файлы внутри указанного каталога (относительно управляющего каталога)
*/
Util::Array<Util::String>
SharedDirFTP::ListFiles(const Util::String & dir)
{
    // получить иерархию общего каталога
    String path;
    path.Format("%s/%s",this->sharedDir.LocalPath().AsCharPtr(),dir.AsCharPtr());

    FTPFile file = this->GetFtpDirectoryFileHierachy(path);

    Array<String> res;

    IndexT idx;
    for(idx = 0; idx < file.content.Size(); idx++)
    {
        if(file.content[idx].type == File)
        {
            res.Append(file.content[idx].name);
        }
    }
    return res;
}

//------------------------------------------------------------------------------
/**
    создает управляющий каталог из руководства
*/
void
SharedDirFTP::CreateControlDir()
{
    n_assert(this->sharedDir.IsValid());
    
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    this->ScriptCreateSubDirectory(writer, this->sharedDir.LocalPath());
    
    this->RunScript(this->CloseWriterAndGetStream(writer));
}
    
//------------------------------------------------------------------------------
/**
    удаляет управляющую директорию
*/
void
SharedDirFTP::RemoveControlDir()
{
    String rootPath = this->sharedDir.LocalPath().ExtractDirName();
    String guidDir = this->sharedDir.LocalPath().ExtractFileName();
    Ptr<TextWriter> writer = this->CreateScriptWriter();
    // обработать случай, когда корневой путь представляет собой «/». В этом случае команда «cd»
    // приведет к неправильному пути на сервере, поскольку "cd /" указывает на серверы
    // корневой каталог
    if(rootPath == "/")
    {
        writer->WriteFormatted("rmdir %s\r\n",guidDir.AsCharPtr());
    }
    else
    {
        writer->WriteFormatted("cd %s\r\nrmdir %s\r\n",rootPath.AsCharPtr(),guidDir.AsCharPtr());
    }
    this->RunScript(this->CloseWriterAndGetStream(writer));
}

//------------------------------------------------------------------------------
/**
    Возвращает объект иерархии файлов, описывающий структуру ниже
    указанный каталог ftp
*/
SharedDirFTP::FTPFile
SharedDirFTP::GetFtpDirectoryFileHierachy(const Util::String & path, bool searchSubdirectories)
{
    // перейдите в каталог и верните список
    bool receivedDir = false;
    FTPFile file;
    Timing::Timer timer;
    timer.Start();
    while(!receivedDir && timer.GetTime() < 90)
    {
        Ptr<TextWriter> dirListScript = this->CreateScriptWriter();
        this->ScriptCreateSubDirectory(dirListScript, path);
        dirListScript->WriteString("dir\r\n");

        Ptr<TextReader> reader = TextReader::Create();
        reader->SetStream(this->RunScript(this->CloseWriterAndGetStream(dirListScript)));
        
        file.name = path.ExtractFileName();
        file.type = Directory;
        if(reader->Open())
        {
            while(!reader->Eof())
            {
                String line = reader->ReadLine();
                if(line.FindStringIndex("d",0)==0)
                {
                    if(line.Length() > 57)
                    {
                        // имя каталога всегда начинается с символа 57
                        String dirName = line.ExtractToEnd(56);
                        // убрать пробелы
                        dirName.Trim(" \r\n");
                        if(searchSubdirectories)
                        {
                            String subPath;
                            subPath.Format("%s/%s",path.AsCharPtr(),dirName.AsCharPtr());
                            file.content.Append(this->GetFtpDirectoryFileHierachy(subPath));
                        }
                        else
                        {
                            FTPFile subFile;
                            subFile.type = Directory;
                            subFile.name = dirName;
                            file.content.Append(subFile);
                        }
                    }
                }
                else if(line.FindStringIndex("-",0)==0)
                {
                    // может быть только имя файла, если строка длиннее 57 символов....
                    if(line.Length() > 57)
                    {
                        // ... потому что имя файла всегда начинается с символа 57
                        String fileName = line.ExtractToEnd(56);
                        // убрать пробелы
                        fileName.Trim(" \r\n");
                        FTPFile subFile;
                        subFile.type = File;
                        subFile.name = fileName;
                        file.content.Append(subFile);
                    }
                }
                else if(line.FindStringIndex("226",0)==0)
                {
                    receivedDir = true;
                }
            }
            reader->Close();
        }
        
        // if no "Directory Sent OK" received, retry in 100 milliseconds...
        if(!receivedDir)
        {
            Timing::Sleep(0.1);
        }
    }
    timer.Stop();
    
    return file;
}

//------------------------------------------------------------------------------
/**
    This will call a status script on the current ftp executable. Based on
    the output of the executable it can be verified, if a verbose command
    is required for full output. This can happen in some cases:

    "For whatever reason the ftp client is "Verbose mode on" by default
    when started from the command line but is in "Verbose mode
    off" by default when started with Shell.exec."
    
    http://www.tech-archive.net/Archive/InetSDK/microsoft.public.inetsdk.programming.scripting.vbscript/2004-03/0004.htm
*/
void
SharedDirFTP::CheckExecutableStatus()
{
    this->needVerboseCommand = false;
    
    Ptr<TextReader> reader = TextReader::Create();
    Ptr<TextWriter> dirListScript = this->CreateScriptWriter();
    dirListScript->WriteLine("status");
    reader->SetStream(this->RunScript(this->CloseWriterAndGetStream(dirListScript)));
    if(reader->Open())
    {
        while(!reader->Eof())
        {
            String line = reader->ReadLine();
            line.ToLower();
            IndexT vIdx = line.FindStringIndex("verbose:");
            if(vIdx != InvalidIndex && line.Length() > vIdx+13)
            {
                String value = line.ExtractRange(vIdx + 8,4);
                if(
                    value.FindStringIndex("off") != InvalidIndex ||
                    value.FindStringIndex("aus") != InvalidIndex
                    )
                {
                    this->needVerboseCommand = true;
                }
            }
        }
        reader->Close();
    }
}

//------------------------------------------------------------------------------
/**
    Создает открытый объект textwriter для написания на нем сценариев.
*/
Ptr<TextWriter>
SharedDirFTP::CreateScriptWriter()
{
    Ptr<TextWriter> writer = TextWriter::Create();
    Ptr<MemoryStream> stream = MemoryStream::Create();
    stream->SetAccessMode(Stream::ReadWriteAccess);
    writer->SetStream(stream.upcast<Stream>());
    if(!writer->Open())
    {
        Console::Instance()->Error("Не удалось открыть поток памяти.");
    }
    return writer;
}

//------------------------------------------------------------------------------
/**
    Закрывает данный объект textwriter и возвращает поток с записанным текстом.
    сценарий в нем.
*/
const Ptr<Stream> &
SharedDirFTP::CloseWriterAndGetStream(const Ptr<IO::TextWriter> & writer)
{
    writer->Close();
    return writer->GetStream();
}

//------------------------------------------------------------------------------
/**
    записывает временный файл сценария ftp и запускает его.
    Возвращает выходной поток ftp-скрипта, включая команды.
*/
Ptr<Stream>
SharedDirFTP::RunScript(const Ptr<Stream> & script)
{
    Ptr<IoServer> ioServer = IoServer::Instance();
    
    // записать скрипт во временный текстовый файл
    URI uri;
    Guid filenameGuid;
    filenameGuid.Generate();
    uri.Set("temp:distributedtools/ftpscript/");
    URI dirUri(uri);
    // создать каталог скриптов
    ioServer->CreateDirectory(dirUri);
    
    String scriptName;
    scriptName.Format("%s.ftp",filenameGuid.AsString().AsCharPtr());
    uri.AppendLocalPath(scriptName);
    Ptr<Stream> stream = ioServer->CreateStream(uri);
    Ptr<TextWriter> writer = TextWriter::Create();
    writer->SetStream(stream);

    Ptr<TextReader> reader = TextReader::Create();
    reader->SetStream(script);

    if(writer->Open() && reader->Open())
    {
        // напишите часть входа
        writer->WriteLine(this->username);
        writer->WriteLine(this->password);
        
        // включите подробную информацию, если необходимо
        if(this->needVerboseCommand)
        {
            writer->WriteLine("verbose");
        }

        // переключение в бинарный режим
        writer->WriteLine("binary");
        
        // написать сценарий
        while(!reader->Eof())
        {
            writer->WriteLine(reader->ReadLine());
        }
        
        reader->Close();

        // написать часть выхода из системы
        writer->WriteLine("bye");
        
        writer->Close();
    }
    else
    {
        if(writer->IsOpen())
        {
            writer->Close();
        }
        if(reader->IsOpen())
        {
            reader->Close();
        }
    }
    writer = nullptr;
    reader = nullptr;
    // запустите ftp-приложение с созданным скриптом в качестве параметра
    AppLauncher appLauncher;
    appLauncher.SetWorkingDirectory(this->exeLocation.LocalPath().ExtractDirName());
    appLauncher.SetExecutable(this->exeLocation.LocalPath());
    String args;
    
    // Извлечение хоста из пути...
    // Схема://UserInfo@Host:Port/LocalPath#Fragment?Query
    args.Format("-i -w:12288 -s:%s %s",uri.LocalPath().AsCharPtr(),this->path.Host().AsCharPtr());
    
    appLauncher.SetArguments(args);
    Ptr<MemoryStream> output = MemoryStream::Create();
    output->SetAccessMode(Stream::ReadWriteAccess);
    appLauncher.SetStdoutCaptureStream(output.upcast<Stream>());
    // подождите, пока файл сценария не будет найден... подождите 90 секунд
    Timing::Timer timer;
    timer.Start();
    while(!ioServer->FileExists(uri) && timer.GetTime() < 90)
    {
        Timing::Sleep(0.1);
    }
    timer.Stop();
    if(ioServer->FileExists(uri))
    {
        appLauncher.LaunchWait();
    }
    else
    {
        Console::Instance()->Error("Не удалось найти ftp-скрипт:\n%s",uri.LocalPath().AsCharPtr());
    }
    
    // удалить файл сценария
    timer.Reset();
    timer.Start();
    while(ioServer->FileExists(uri) && timer.GetTime() < 90)
    {
        // try to delete the file within 90 seconds.
        if(!ioServer->DeleteFile(uri))
        {
            Timing::Sleep(0.1);
        }
    }
    timer.Stop();
    if(ioServer->FileExists(uri))
    {
        Console::Instance()->Warning("Could not delete script file from Temp directory:\n%s",uri.LocalPath().AsCharPtr());
    }

    // ... и каталог сценария, если возможно
    if(ioServer->DirectoryExists(dirUri))
    {
        ioServer->DeleteDirectory(dirUri);
    }

    return output.upcast<Stream>();
}

//------------------------------------------------------------------------------
/**
    Создает скрипт, который создает иерархию каталогов на текущем сервере.
*/
void
SharedDirFTP::ScriptCreateSubDirectory(const Ptr<IO::TextWriter> & writer, const Util::String & path)
{
    String localPath(path);
    localPath.ConvertBackslashes();
    Array<String> directories = localPath.Tokenize("/");

    String tmps;
    IndexT i;
    for(i = 0; i < directories.Size(); i++)
    {
        writer->WriteFormatted("mkdir %s\r\ncd %s\r\n",directories[i].AsCharPtr(),directories[i].AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Создает скрипт, который помещает файл в текущее местоположение сервера.
*/
void
SharedDirFTP::ScriptPutFile(const Ptr<IO::TextWriter> & writer, const Util::String &src)
{
    writer->WriteFormatted("вставьте \"%s\" \"%s\"\r\n",URI(src).LocalPath().AsCharPtr(),src.ExtractFileName().AsCharPtr());
}

//------------------------------------------------------------------------------
/**
    Создает скрипт, который помещает список файлов в текущее местоположение сервера.
*/
void
SharedDirFTP::ScriptPutFiles(const Ptr<IO::TextWriter> & writer, const Array<String> &srcArray)
{
    IndexT i;
    String putCmd;
    for(i = 0; i < srcArray.Size(); i++)
    {
        this->ScriptPutFile(writer, srcArray[i]);
    }
}

//------------------------------------------------------------------------------
/**
    Создает скрипт, который помещает все содержимое каталога в папку ftp,
    включая подкаталоги.
*/
void
SharedDirFTP::ScriptPutDirectoryContent(const Ptr<IO::TextWriter> & writer, const Util::String & src,const Util::String & dst)
{
    this->ScriptCreateSubDirectory(writer,dst);

    // список файлов
    Array<String> files = IoServer::Instance()->ListFiles(URI(src),"*.*");
    String filePath;
    Array<String> filePaths;
    IndexT i;
    for(i = 0; i < files.Size(); i++)
    {
        filePath.Format("%s/%s",src.AsCharPtr(),files[i].AsCharPtr());
        filePaths.Append(filePath);
    }
    this->ScriptPutFiles(writer,filePaths);

    // список подкаталогов
    Array<String> dirs = IoServer::Instance()->ListDirectories(URI(src),"*.*");
    for(i = 0; i < dirs.Size(); i++)
    {
        filePath.Format("%s/%s",src.AsCharPtr(),dirs[i].AsCharPtr());
        this->ScriptPutDirectoryContent(writer,filePath,dirs[i]);
        writer->WriteString("компакт-диск ..\r\n");
    }
}

//------------------------------------------------------------------------------
/**
    Скрипт, который получает файл в заданном расположении ftp
    и копирует его по заданному пути назначения
*/
void
SharedDirFTP::ScriptGetFile(const Ptr<IO::TextWriter> & writer, const Util::String &src, const Util::String &dst)
{
    this->ScriptCreateSubDirectory(writer, src);
    writer->WriteFormatted("\r\nlcd %s\r\nget %s\r\n",
        URI(dst).LocalPath().AsCharPtr(),
        src.ExtractFileName().AsCharPtr()
        );
}

//------------------------------------------------------------------------------
/**
    Скрипт, который получает все файлы в указанном расположении ftp
    и копирует содержимое по заданному пути назначения.
*/
void
SharedDirFTP::ScriptGetFiles(const Ptr<IO::TextWriter> & writer, const Util::String & src,const Util::String & dst)
{
    this->ScriptCreateSubDirectory(writer, src);
    writer->WriteFormatted("\r\nlcd %s\r\nmget *?\r\n",
        URI(dst).LocalPath().AsCharPtr()
        );
}

//------------------------------------------------------------------------------
/**
    Скрипт, копирующий файлы из заданной структуры ftp в локальный каталог.
    Создает недостающие каталоги.
*/
void
SharedDirFTP::ScriptGetFileHierachy(const Ptr<IO::TextWriter> & writer, const DistributedTools::SharedDirFTP::FTPFile &src, const Util::String &dst)
{
    // создать каталог, если он не существует
    if(IoServer::Instance()->CreateDirectory(URI(dst)))
    {
        // перейти в локальный каталог
        writer->WriteString("ЖК »");
        writer->WriteString(dst);
        writer->WriteString("\"\r\n");

        // добавить файлы
        IndexT idx;
        for(idx = 0; idx < src.content.Size(); idx++)
        {
            if(src.content[idx].type == File)
            {
                writer->WriteString("получать \"");
                writer->WriteString(src.content[idx].name);
                writer->WriteString("\"\r\n");
            }
        }

        // проверьте подкаталоги
        for(idx = 0; idx < src.content.Size(); idx++)
        {
            if(src.content[idx].type == Directory)
            {
                // сделай еще один шаг глубже
                writer->WriteFormatted("компакт-диск %s\r\n",src.content[idx].name.AsCharPtr());

                String dirName;
                dirName.Format("%s/%s",dst.AsCharPtr(),src.content[idx].name.AsCharPtr());
                this->ScriptGetFileHierachy(writer,src.content[idx],dirName);
                
                // return to parent dir
                writer->WriteString("компакт-диск ..\r\n");
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
    Script that removes all files and directories from the ftp server
    that are in the given file hierarchy
*/
void
SharedDirFTP::ScriptRemoveFileHierachy(const Ptr<IO::TextWriter> & writer, const FTPFile & src)
{
    // удалить файлы
    IndexT idx;
    for(idx = 0; idx < src.content.Size(); idx++)
    {
        if(src.content[idx].type == File)
        {
            writer->WriteString("удалить \"");
            writer->WriteString(src.content[idx].name);
            writer->WriteString("\"\r\n");
        }
    }

    // удалить каталоги
    for(idx = 0; idx < src.content.Size(); idx++)
    {
        if(src.content[idx].type == Directory)
        {
            writer->WriteFormatted("компакт-диск %s\r\n",src.content[idx].name.AsCharPtr());
            this->ScriptRemoveFileHierachy(writer, src.content[idx]);
            writer->WriteFormatted("\r\ncd ..\r\nrmdir %s\r\n",src.content[idx].name.AsCharPtr());
        }
    }
}

} // пространство имен DistributedTools
