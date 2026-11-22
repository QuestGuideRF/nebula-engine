//------------------------------------------------------------------------------
// textureconversionjob.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "textureconversionjob.h"
#include "io/ioserver.h"
#include "toolkit-common/text.h"

namespace ToolkitUtil
{
using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
    Конструктор 
*/
TextureConversionJob::TextureConversionJob() :
    textureAttrTable(0),
    logger(0),
    force(false),
    quiet(false),
    neverCopy(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Запустите задание преобразования. Переопределить в подклассах.   
*/
bool
TextureConversionJob::Convert()
{
    n_assert(this->srcPath.IsValid());
    n_assert(this->dstPath.IsValid());
    n_assert(this->tmpDir.IsValid());    
    n_assert(this->dstFileExt.IsValid());
    n_assert(0 != this->textureAttrTable);
    n_assert(0 != this->logger);

    // установить расширение файла dst
    if (this->PrepareConversion(srcPath, dstPath))
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
/**
    Prepares the actual conversion process, first checks if conversion
    is necessary, removes write protection on target file, and copies
    the file if the source is already in native format. Returns true
    if no further conversion is needed.
*/
bool
TextureConversionJob::PrepareConversion(const String& srcPath, const String& dstPath)
{
    IoServer* ioServer = IoServer::Instance();

    // сначала убедитесь, что целевой каталог существует
    IoServer::Instance()->CreateDirectory(this->dstPath.ExtractDirName());

    // проверьте, можем ли мы пропустить преобразование на основе меток времени файла и принудительного флага
    if (!this->NeedsConversion(srcPath, dstPath))
    {
        this->logger->Print("Пропуск %s\n", Text(URI(srcPath).LocalPath()).Color(TextColor::Blue).AsCharPtr());
        return true;
    }


    // удалить атрибут только для чтения из файла dst 
    if (ioServer->FileExists(dstPath))
    {
        ioServer->SetReadOnly(dstPath, false);
    }

    // настроить путь к временному файлу назначения
    this->tmpPath = this->tmpDir;
    this->tmpPath.Append("/");
    this->tmpPath.Append(this->dstPath.ExtractFileName());

    // убедитесь, что временный каталог существует
    IoServer::Instance()->CreateDirectory(this->tmpPath.ExtractDirName());

    // получить атрибуты преобразования текстур
    this->textureAttrs = this->textureAttrTable->GetEntry(srcPath);
    
    // if destination file is already in native format, do a plain copy
    if (!neverCopy && srcPath.GetFileExtension() == dstPath.GetFileExtension())
    {
        ioServer->CopyFile(srcPath, dstPath);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    Выполните проверку времени файла, чтобы решить, должна ли текстура быть
    преобразован.
*/
bool
TextureConversionJob::NeedsConversion(const String& srcPath, const String& dstPath)
{
    // проверка времени файла переопределена?
    if (this->force)
    {
        return true;
    }

    // в противном случае проверьте время файлов src и dst.
    IoServer* ioServer = IoServer::Instance();
    if (ioServer->FileExists(dstPath))
    {
        FileTime srcFileTime = ioServer->GetFileWriteTime(srcPath);
        FileTime dstFileTime = ioServer->GetFileWriteTime(dstPath);
        FileTime attrTime = srcFileTime;
        String texEntry;
        texEntry.Format("%s/%s", srcPath.ExtractLastDirName().AsCharPtr(), srcPath.ExtractFileName().AsCharPtr());
        texEntry.StripFileExtension();
        
        if (this->textureAttrTable->HasEntry(texEntry))
        {
            attrTime = this->textureAttrTable->GetEntry(texEntry).GetTime();
        }
        if (dstFileTime > srcFileTime && dstFileTime > attrTime)
        {
            // файл dst новее, чем файл src, конвертировать не нужно
            return false;
        }
    }

    // провал: файл dst не существует или он старше файла src
    return true;
}

//------------------------------------------------------------------------------
/**
    Copies the converted texture from the temp directory to the destination
    directory and deletes the temp file and directory.
*/
bool
TextureConversionJob::CopyResult()
{
    n_assert(0 != this->logger);
    bool retval = true;
    IoServer* ioServer = IoServer::Instance();
    if (ioServer->FileExists(this->tmpPath))
    {
        ioServer->CopyFile(this->tmpPath, this->dstPath);
    }
    else
    {
        this->logger->Error("    [TexConv — не удалось скопировать текстуру, поскольку файл src не существует: %s -> %s]\n", 
            this->tmpPath.AsCharPtr(), this->dstPath.AsCharPtr());
        retval = false;
    }
    ioServer->DeleteFile(this->tmpPath);
    ioServer->DeleteDirectory(this->tmpDir);
    return retval;
}

} // пространство имен ToolkitUtil