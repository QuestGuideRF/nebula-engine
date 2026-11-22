//------------------------------------------------------------------------------
//  texturebatcherapp.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "texturebatcherapp.h"
#include "toolkit-common/platform.h"
#include "io/ioserver.h"
#include "io/textreader.h"
#include "io/console.h"
#include "io/uri.h"
#include "timing/time.h"

namespace Toolkit
{
using namespace ToolkitUtil;
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
bool
TextureBatcherApp::ParseCmdLineArgs()
{   
    // анализ распределенных аргументов
    if (DistributedToolkitApp::ParseCmdLineArgs())
    {
        // всегда используйте выходной каталог в качестве каталога назначения, даже если
        // Приложение не распространяется. В этом случае выходной каталог
        // в любом случае установлено значение this->exportdirArg.
        this->textureConverter.SetDstDir(this->projectInfo.GetPathAttr("TextureDstDir"));
        this->textureConverter.SetForceFlag(this->forceArg);
        return true;
    }
    return false;
}


//------------------------------------------------------------------------------
/**
    derived from DistributedToolkitApp. Also checks for timestamps and force flags.
*/
Array<String>
TextureBatcherApp::CreateFileList()
{
    Array<String> res;

    // создать список из заданного списка файлов
    if (this->listfileArg.IsValid())
    {
        URI listUri(this->listfileArg);
        // открытый поток и читалка
        Ptr<Stream> readStream = IoServer::Instance()->CreateStream(listUri);
        readStream->SetAccessMode(Stream::ReadAccess);
        Ptr<TextReader> reader = TextReader::Create();
        reader->SetStream(readStream);
        if (reader->Open())
        {
            // прочитайте каждую строку и добавьте в список
            while(!reader->Eof())
            {
                String srcPath = reader->ReadLine();
                srcPath.Trim(" \r\n");
                if (this->NeedsConversion(srcPath) || this->forceArg)
                {
                    res.Append(srcPath);
                }
            }
            // закрыть поток и читатель
            reader->Close();
        }
        reader = nullptr;
        readStream = nullptr;
    }
    // прочитать содержимое каталога
    else if (this->dirArg.IsValid())
    {
        // просто добавьте один файл в список
        if (this->fileArg.IsValid())
        {
            String src;
            src.Format("src:assets/%s/%s",
                this->dirArg.AsCharPtr(),
                this->fileArg.AsCharPtr()
                );
            if (this->NeedsConversion(src) || this->forceArg)
            {
                res.Append(src);
            }
        }
        // добавить все файлы в этот каталог
        else
        {
            String directory;
            directory.Format("src:assets/%s",
                this->dirArg.AsCharPtr()
                );
            res.AppendArray(this->GetFileListFromDirectory(directory));
        }
    }
    // if there are no arguments whatsoever add all files in texture dir to list
    else
    {
        Array<String> dirs = IoServer::Instance()->ListDirectories("src:assets", "*");
        IndexT dirIndex;
        // перебрать каждый подкаталог «текстур»
        for (dirIndex = 0; dirIndex < dirs.Size(); dirIndex++)
        {
            if (dirs[dirIndex] != ".svn")
            {
                String directory;
                directory.Format("src:assets/%s", dirs[dirIndex].AsCharPtr());
                res.AppendArray(this->GetFileListFromDirectory(directory));
            }
        }
    }
    return res;
}

//------------------------------------------------------------------------------
/**
*/
bool
TextureBatcherApp::SetupProjectInfo()
{
    if (DistributedToolkitApp::SetupProjectInfo())
    {
        this->textureConverter.SetPlatform(this->platform);
        if (this->projectInfo.HasAttr("TextureTool"))
        {
            this->textureConverter.SetToolPath(this->projectInfo.GetPathAttr("TextureTool"));
        }
        this->textureConverter.SetTexAttrTablePath(this->projectInfo.GetAttr("TextureAttrTable"));
        this->textureConverter.SetDstDir(this->projectInfo.GetAttr("TextureDstDir"));
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
TextureBatcherApp::ShowHelp()
{
    n_printf("Пакетный экспортер текстур туманности.\n"
             "(C) Индивидуальные авторы 2013–2020 гг., см. файл АВТОРОВ.\n");
    n_printf(this->GetArgumentDescriptionString().AsCharPtr());
    n_printf("-force -- принудительный экспорт (не проверять отметки времени)\n"
             "-platform -- выберите платформу (win32, linux)\n\n");
}

//------------------------------------------------------------------------------
/**
*/
void
TextureBatcherApp::DoWork()
{    
    // получить список файлов для конвертации
    Array<String> files = this->CreateFileList();
    if (files.Size() > 0)
    {
        // настроить конвертер текстур
        Console::Instance()->Print("Настройте конвертер текстур\n");
        if (!this->textureConverter.Setup(this->logger))
        {
            n_printf("ОШИБКА: не удалось настроить конвертер текстур!\n");
            this->SetReturnCode(-1);
            return;
        }
        // выполнить преобразование текстур
        Console::Instance()->Print("Начать преобразование текстур\n");
        bool result = this->textureConverter.ConvertFiles(files);
        Console::Instance()->Print("Done\n");
        this->textureConverter.Discard();
    }
}

//------------------------------------------------------------------------------
/**
    Верните все файлы внутри этого каталога, которые нуждаются в преобразовании текстур.
*/
Array<String>
TextureBatcherApp::GetFileListFromDirectory(const String& directory)
{
    Array<String> res;
    Array<String> files = IoServer::Instance()->ListFiles(directory, "*");
    IndexT fileIndex;
    // проверьте каждый файл в этом каталоге, если его нужно конвертировать
    for (fileIndex = 0; fileIndex < files.Size(); fileIndex++)
    {
        String srcPath;
        srcPath.Format("%s/%s", directory.AsCharPtr(), files[fileIndex].AsCharPtr());
        if (this->NeedsConversion(srcPath))
        {
            res.Append(srcPath);
        }
    }
    return res;
}

//------------------------------------------------------------------------------
/**
    Возвращает расширение файла, зависящее от платформы, для преобразованных текстур.
*/
String
TextureBatcherApp::GetDstTextureFileExtension()
{
    if(Platform::Win32 == this->platform)
    {
        return "dds";
    }    
    else if (Platform::Linux == this->platform)
    {
        return "dds";
    }
    else
    {
        n_error("TextBatcherApp::GetDstTextureFileExtension(): неверная платформа!\n");
        return "<INVALID>";
    }
}

//------------------------------------------------------------------------------
/**
    Perform a file time check to decide whether a texture must be
    converted. Build dstPath from project settings and file name of srcPath.
*/
bool
TextureBatcherApp::NeedsConversion(const String& srcPath)
{
    // вырезать часть исходного каталога из srcPath
    String resolvedSrc = AssignRegistry::Instance()->ResolveAssignsInString(srcPath);
    String subPath = resolvedSrc.ExtractToEnd(AssignRegistry::Instance()->ResolveAssignsInString("src:assets").Length());
    subPath.Trim("/");
    String dstPath;
    dstPath.Format("%s/%s",
        this->projectInfo.GetPathAttr("TextureDstDir").AsCharPtr(),
        subPath.AsCharPtr()
    );
    // обязательно удалите все расширения файлов
    while ("" != dstPath.GetFileExtension())
    {
        dstPath.StripFileExtension();
    }
    dstPath.Format("%s.%s", dstPath.AsCharPtr(), this->GetDstTextureFileExtension().AsCharPtr());
    return this->NeedsConversion(srcPath, dstPath);
}

//------------------------------------------------------------------------------
/**
    Выполните проверку времени файла, чтобы решить, должна ли текстура быть
    преобразован. Используйте явный dstPath
*/
bool
TextureBatcherApp::NeedsConversion(const String& srcPath, const String& dstPath)
{
    // проверка времени файла переопределена?
    if (this->forceArg)
    {
        return true;
    }
    // в противном случае проверьте время файлов src и dst.
    IoServer* ioServer = IoServer::Instance();
    if (ioServer->FileExists(dstPath))
    {
        FileTime srcFileTime = ioServer->GetFileWriteTime(srcPath);
        FileTime dstFileTime = ioServer->GetFileWriteTime(dstPath);
        FileTime attrFileTime = srcFileTime;
        String xml = srcPath;
        xml.StripFileExtension();
        xml += ".xml";
        if(ioServer->FileExists(xml))
        {
            attrFileTime = ioServer->GetFileWriteTime(xml);
        }
        if(attrFileTime > dstFileTime)
        {
            return true;
        }
        if (dstFileTime > srcFileTime)
        {
            // файл dst новее, чем файл src, конвертировать не нужно
            return false;
        }
    }
    // провал: файл dst не существует или он старше файла src
    return true;
}
} // пространство имен ToolkitUtil
