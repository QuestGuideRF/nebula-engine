//------------------------------------------------------------------------------
//  archiveapp.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "archiverapp.h"
#include "toolkitutil/applauncher.h"
#include "io/xmlwriter.h"
#include "zlib/zlib.h"
#include "io/binarywriter.h"

namespace Toolkit
{
using namespace ToolkitUtil;
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
ArchiverApp::ArchiverApp() :
    webDeployFlag(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
ArchiverApp::ShowHelp()
{
    n_printf("Архиватор NebulaT.\n"
             "(C) Радон Лабс ГмбХ\n"
             "Создает архивы ресурсов для конкретной платформы (например, Export.zip, Export_win32.zip)\n"
             "-help -- отобразить эту справку\n"
             "-webdeploy -- создать каталог веб-развертывания (только для платформы Win32)!\n");
}

//------------------------------------------------------------------------------
/**
*/
bool
ArchiverApp::ParseCmdLineArgs()
{
    if (ToolkitApp::ParseCmdLineArgs())
    {
        this->webDeployFlag = this->args.GetBoolFlag("-webdeploy");
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
ArchiverApp::SetupProjectInfo()
{
    if (ToolkitApp::SetupProjectInfo())
    {
        if (this->projectInfo.HasAttr("ArchiverTool"))
        {
            this->toolPath = this->projectInfo.GetPathAttr("ArchiverTool");
        }
        else
        {
            n_printf("ОШИБКА: в projectinfo.xml не установлен атрибут ArchiveTool!");
            return false;
        }
        if (this->projectInfo.HasAttr("ArchiverExcludePatterns"))
        {
            this->excludePatterns = this->projectInfo.GetAttr("ArchiverExcludePatterns").Tokenize("; ");
        }
        else
        {
            this->excludePatterns.Append("*.db4");
        }        
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ArchiverApp::Run()
{
    // анализировать аргументы командной строки
    if (!this->ParseCmdLineArgs())
    {
        return;
    }

    // настроить объект информации о проекте
    if (!this->SetupProjectInfo())
    {
        return;
    }
    
    // вызывать упаковщики, специфичные для платформы
    switch (this->platform)
    {
        case Platform::Win32:
            if (this->webDeployFlag)
            {
                this->PackWebDeploy(this->projectInfo.GetAttr("DstDir"), "webdeploy_data");
                break;
            }
            // провал!
        case Platform::Linux:
            this->PackDirectoryWin360(this->projectInfo.GetAttr("DstDir"));
            break;
    }
}

//------------------------------------------------------------------------------
/**
    Creates web-deployment data. This recursively every file and copies
    it into the webdeployment directory.
*/
void
ArchiverApp::PackWebDeploy(const String& srcDir, const String& webDeployDir)
{
    // рекурсивно копировать каталоги
    String srcPath = srcDir;
    String dstPath = "proj:" + webDeployDir + "/" + srcDir.ExtractFileName();
    this->RecursePackWebDeployDirectory(srcPath, dstPath);
}

//------------------------------------------------------------------------------
/**
    Рекурсивно обрабатывать один каталог для веб-развертывания.
*/
void
ArchiverApp::RecursePackWebDeployDirectory(const String& srcDir, const String& dstDir)
{
    IoServer* ioServer = IoServer::Instance();
    
    // сначала убедитесь, что каталог назначения существует
    ioServer->CreateDirectory(dstDir);

    // сжать-скопировать все файлы в каталоге
    Array<String> files = ioServer->ListFiles(srcDir, "*");
    IndexT fileIndex;
    for (fileIndex = 0; fileIndex < files.Size(); fileIndex++)
    {
        String srcPath = srcDir + "/" + files[fileIndex];
        String dstPath = dstDir + "/" + files[fileIndex] + ".nz";
        this->CompressCopyFile(srcPath, dstPath);
    }

    // рекурсия в подкаталоги
    Array<String> dirs = ioServer->ListDirectories(srcDir, "*");
    IndexT dirIndex;
    for (dirIndex = 0; dirIndex < dirs.Size(); dirIndex++)
    {
        const String& curDir = dirs[dirIndex];
        if ((curDir != "CVS") && (curDir != ".svn"))
        {
            String srcPath = srcDir + "/" + curDir;
            String dstPath = dstDir + "/" + curDir;
            this->RecursePackWebDeployDirectory(srcPath, dstPath);
        }
    }
}

//------------------------------------------------------------------------------
/**
    Сожмите и скопируйте один файл для веб-развертывания.
*/
void
ArchiverApp::CompressCopyFile(const String& srcPath, const String& dstPath)
{
    IoServer* ioServer = IoServer::Instance();

    // файл с открытым исходным кодом
    Ptr<Stream> srcStream = ioServer->CreateStream(srcPath);
    if (srcStream->Open())
    {
        if (srcStream->GetSize() == 0)
        {
            n_printf("* пропуск '%s' (файл пуст)\n", srcPath.AsCharPtr());
            return;
        }
        void* srcData = srcStream->Map();
        SizeT srcSize = srcStream->GetSize();

        // получить максимальную сжатую длину из zlib и выделить буфер сжатия
        uLong maxSize = compressBound(srcSize);
        void* buf = Memory::Alloc(Memory::ScratchHeap, maxSize);

        // сжать исходные данные в буфер сжатия
        uLongf ioDestLen = maxSize;
        int res = compress2((Bytef*)buf, &ioDestLen, (const Bytef*)srcData, srcSize, 9);
        n_assert(Z_OK == res);

        // записать файл назначения со сжатыми данными
        Ptr<Stream> dstStream = ioServer->CreateStream(dstPath);
        Ptr<BinaryWriter> writer = BinaryWriter::Create();
        writer->SetStream(dstStream);
        if (writer->Open())
        {
            writer->WriteUInt(FourCC('NZ__').AsUInt());
            writer->WriteUInt(srcSize);
            writer->WriteRawData(buf, ioDestLen);
            writer->Close();

            n_printf("-> %s (%d -> %d)\n", srcPath.AsCharPtr(), srcSize, ioDestLen);
        }
        else
        {
            n_error("ArchiverApp::CompressCopyFile(): не удалось открыть dst-файл '%s'!\n", dstPath.AsCharPtr());
        }

        // очистить все
        Memory::Free(Memory::ScratchHeap, buf);
        buf = 0;
        srcStream->Unmap();
        srcStream->Close();
        srcStream =  0;
    }
    else
    {
        n_error("ArchiverApp::CompressCopyFile: не удалось открыть исходный файл '%s'!\n", srcPath.AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Упаковывает один каталог с помощью zip.exe.
*/
void
ArchiverApp::PackDirectoryWin360(const String& dirPath)
{
    IoServer* ioServer = IoServer::Instance();

    // убедитесь, что каталог существует
    if (!ioServer->DirectoryExists(dirPath))
    {
        n_printf("ОШИБКА: каталог «%s» не существует!", dirPath.AsCharPtr());
        return;
    }

    // удалить целевой файл, если он существует
    String filePath = dirPath + ".zip";
    if (ioServer->FileExists(filePath))
    {
        ioServer->DeleteFile(filePath);
    }

    // вызвать инструмент zip
    String args;
    args.Format("-r %s %s\\* ", filePath.ExtractFileName().AsCharPtr(), dirPath.ExtractFileName().AsCharPtr());    
    IndexT i;
    for (i = 0; i < this->excludePatterns.Size(); i++)
    {
        args.Append("-x ");
        args.Append(this->excludePatterns[i]);
        args.Append(" ");
    }

    n_printf("Архивирование: %s %s", this->toolPath.AsCharPtr(), args.AsCharPtr());
    AppLauncher appLauncher;
    appLauncher.SetExecutable(this->toolPath);
    appLauncher.SetWorkingDirectory("proj:");
    appLauncher.SetArguments(args);
    if (!appLauncher.LaunchWait())
    {
        n_printf("ВНИМАНИЕ: не удалось запустить конвертер «%s»!\n", this->toolPath.AsCharPtr());
    }
}



} // Инструментарий пространства имен
