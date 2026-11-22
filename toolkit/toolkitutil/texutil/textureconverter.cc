//------------------------------------------------------------------------------
//  textureconverter.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "textureconverter.h"
#include "io/ioserver.h"
#include "io/textwriter.h"
#include "io/xmlwriter.h"
#include "toolkitutil/texutil/imageconverter.h"
#include "util/guid.h"

#if (__WIN32__)
#include "toolkitutil/texutil/directxtexconversionjob.h"
#else
#include "toolkitutil/texutil/compressonatorconversionjob.h"
#endif
#include "timing/timer.h"

#include "toolkit-common/text.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
TextureConverter::TextureConverter() :
    logger(0),
    platform(Platform::Win32),
    force(false),
    quiet(false),
    valid(false),
    maxParallelJobs(1)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
TextureConverter::~TextureConverter()
{
    if (this->IsValid())
    {
        this->Discard();
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
TextureConverter::Setup()
{
    n_assert(!this->IsValid());
    n_assert(0 == this->logger);

    this->valid = true;

    // создать временный каталог
    IoServer::Instance()->CreateDirectory("temp:textureconverter");

    return true;
}

//------------------------------------------------------------------------------
/**
*/
void 
TextureConverter::SetLogger(Logger* logger)
{
    this->logger = logger;
}

//------------------------------------------------------------------------------
/**
*/
void
TextureConverter::Discard()
{
    n_assert(this->IsValid());
    this->valid = false;
    this->logger = 0;

    // удаляет временный каталог
    if (IoServer::Instance()->DirectoryExists("temp:textureconverter"))
    {
        IoServer::Instance()->DeleteDirectory("temp:textureconverter");
    }
}

//------------------------------------------------------------------------------
/**
    Конвертируйте все текстуры в заданном списке файлов.
*/
bool
TextureConverter::ConvertFiles(const Util::Array<Util::String>& files)
{
    bool success = true;

    // создать временный каталог из guid. чтобы другие работы не мешали
    Guid guid;
    guid.Generate();
    String tmpDir;
    tmpDir.Format("%s/%s", "temp:textureconverter", guid.AsString().AsCharPtr());

    // конвертировать каждую текстуру в списке файлов
    IndexT index;
    for(index = 0; index < files.Size(); index++)
    {
        String dirName = files[index].ExtractLastDirName();
        if (files[index].CheckFileExtension("tga") ||
            files[index].CheckFileExtension("bmp") ||
            files[index].CheckFileExtension("dds") ||
            files[index].CheckFileExtension("png") ||
            files[index].CheckFileExtension("exr") ||
            files[index].CheckFileExtension("jpg"))
        {
            success = this->ConvertTexture(files[index], "tex:", tmpDir);
        }
    }
    // удалить созданный временный каталог этого задания
    if (IoServer::Instance()->DirectoryExists(tmpDir))
    {
        IoServer::Instance()->DeleteDirectory(tmpDir);
    }
    return success;
}

//------------------------------------------------------------------------------
/**
    Convert a source texture defined by an absolute path into a destination
    texture defined by category name and texture name. The destination texture
    name may not have a file extension! This method will just branch to
    different platform-specific jobs based on the selected target platform.
*/
bool
TextureConverter::ConvertTexture(const String& srcTexPath, const String& dstTexPath, const String& tmpDir)
{
    n_assert(this->IsValid());
    n_assert(srcTexPath.IsValid());
    n_assert(tmpDir.IsValid());
    n_assert(0 != this->logger);

    // extract texture category and filename from path (last 2 components)
    // ПРИМЕЧАНИЕ. dstTexPath будет содержать расширение исходного файла, которое 
    // это плохой дизайн и он очень сбивает с толку - TextConversionJob будет
    // замените правильным расширением файла, специфичным для платформы, в его
    // Стартовый метод!
    Array<String> tokens = srcTexPath.Tokenize(":/");
    n_assert(tokens.Size() >= 3);
    String texCategory = tokens[tokens.Size() - 2];
    String texFilename = tokens[tokens.Size() - 1];


    // выберите метод конвертации в зависимости от целевой платформы
#if (__WIN32__)    
    DirectXTexConversionJob job;
    job.SetLogger(this->logger);
    job.SetSrcPath(srcTexPath);
    job.SetDstPath(dstTexPath);
    job.SetTmpDir(tmpDir);
    job.SetTexAttrTable(&this->textureAttrTable);
    job.SetForceFlag(this->force);
    job.SetQuietFlag(this->quiet);
    bool ret = job.Convert();
#else

    CompressonatorConversionJob job;
    job.SetLogger(this->logger);
    job.SetSrcPath(srcTexPath);
    job.SetDstPath(dstTexPath);
    job.SetTmpDir(tmpDir);
    job.SetTexAttrTable(&this->textureAttrTable);
    job.SetForceFlag(this->force);
    job.SetQuietFlag(this->quiet);
    bool ret = job.Convert();
#endif

    
    return ret;
}

//------------------------------------------------------------------------------
/**
    
*/
bool
TextureConverter::ConvertCubemap(const String& srcTexPath, const String& dstTexPath, const String& tmpDir)
{
    n_assert(this->IsValid());
    n_assert(srcTexPath.IsValid());
    n_assert(tmpDir.IsValid());
    n_assert(0 != this->logger);

    // extract texture category and filename from path (last 2 components)
    // ПРИМЕЧАНИЕ. dstTexPath будет содержать расширение исходного файла, которое 
    // это плохой дизайн и он очень сбивает с толку - TextConversionJob будет
    // замените правильным расширением файла, специфичным для платформы, в его
    // Стартовый метод!
    Array<String> tokens = srcTexPath.Tokenize(":/");
    n_assert(tokens.Size() >= 3);
    String texCategory = tokens[tokens.Size() - 2];
    String texFilename = tokens[tokens.Size() - 1];

    n_printf("Преобразование текстуры: %s\n", URI(srcTexPath).LocalPath().AsCharPtr());

    // выберите метод конвертации в зависимости от целевой платформы
#if (__WIN32__)
    DirectXTexConversionJob job;
    job.SetLogger(this->logger);
    job.SetSrcPath(srcTexPath);
    job.SetDstPath(dstTexPath);
    job.SetTmpDir(tmpDir);
    job.SetTexAttrTable(&this->textureAttrTable);
    job.SetForceFlag(this->force);
    job.SetQuietFlag(this->quiet);
    job.ConvertCube();
#else
/*
    Задание CompressonatorConversionJob;
    job.SetLogger(this->logger);
    job.SetSrcPath(srcTexPath);
    job.SetDstPath(dstTexPath);
    job.SetTmpDir(tmpDir);
    job.SetTexAttrTable(&this->textureAttrTable);
    job.SetForceFlag(this->force);
    job.SetQuietFlag(this->quiet);
    задание.Конвертировать();
    */
#endif

    if (this->platform != Platform::Win32 && this->platform != Platform::Linux) return false;
    else return true;
}

//------------------------------------------------------------------------------
/**
*/
void
TextureConverter::AddAttributeEntry(const Util::String& file, const TextureAttrs& attrs)
{
    this->textureAttrTable.SetEntry(file, attrs);
}

} // пространство имен ToolkitUtil