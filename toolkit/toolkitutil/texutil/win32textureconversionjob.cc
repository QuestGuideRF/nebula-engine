//------------------------------------------------------------------------------
// win32textureconversionjob.cc
// (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "win32textureconversionjob.h"
#include "io/uri.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"

namespace ToolkitUtil
{
using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
Win32TextureConversionJob::Win32TextureConversionJob()
{
    this->SetDstFileExtension("dds");
}

//------------------------------------------------------------------------------
/**
    Запустите процесс конвертации. Возвращает false, если задание завершилось немедленно.   
*/
bool
Win32TextureConversionJob::Convert()
{
    n_assert(this->toolPath.IsValid());
    n_assert(0 != this->logger);
    if (TextureConversionJob::Convert())
    {  
        URI srcPathUri(this->srcPath);
        URI dstPathUri(this->dstPath);
        URI tmpDirUri(this->tmpDir);
        
        // построить аргументы командной строки для nvdxt
        String args = " -изменить масштаб lo -перезаписать ";
        bool isDXT5NormalMap = false;
        const TextureAttrs& attrs = this->textureAttrs;
        if ((attrs.GetRGBPixelFormat() == TextureAttrs::DXT5NM) ||
            (attrs.GetRGBAPixelFormat() == TextureAttrs::DXT5NM) ||
            (String::MatchPattern(this->srcPath, "*норма.*")) ||
            (String::MatchPattern(this->srcPath, "*нормальный.*")) ||
            (String::MatchPattern(this->srcPath, "*ударяться.*")))
        {
            args.Append(" -dxt5nm ");
            isDXT5NormalMap = true;
        }
        else
        {
            args.Append(" -24 ");
            args.Append(TextureAttrs::PixelFormatToString(attrs.GetRGBPixelFormat()));
            args.Append(" -32 ");
            args.Append(TextureAttrs::PixelFormatToString(attrs.GetRGBAPixelFormat()));
            args.Append(" ");
        }
        args.Append(" -clampScale ");
        args.AppendInt(attrs.GetMaxWidth());
        args.Append(" ");
        args.AppendInt(attrs.GetMaxHeight());
        if (attrs.GetQuality() == TextureAttrs::Normal)
        {
            args.Append(" -quality_normal ");
        }
        else if (attrs.GetQuality() == TextureAttrs::High)
        {
            args.Append(" -quality_production ");
        }
        else if (isDXT5NormalMap)
        {
            // Преобразование DXT5NM имеет ошибку в быстром режиме.
            args.Append(" -quality_normal ");
        }
        else
        {
            // быстрое качество
            args.Append(" -quick ");
        }
        if (!attrs.GetGenMipMaps())
        {
            args.Append(" -nomipmap ");
        }
        args.Append("-");
        args.Append(TextureAttrs::FilterToString(attrs.GetMipMapFilter()));
        args.Append(" ");
        if (isDXT5NormalMap)
        {
            args.Append(" -norm ");
        }
        args.Append(" -Rescale");
        args.Append(TextureAttrs::FilterToString(attrs.GetScaleFilter()));
        args.Append(" -файл \"");
        args.Append(srcPathUri.LocalPath());
        
        args.Append("\"-внешний каталог\"");
        args.Append(tmpDirUri.LocalPath());
        args.Append("\"");

        args.Append(" -outfile \"");
        args.Append(dstPathUri.LocalPath().ExtractFileName());
        args.Append("\"");

        // запустите nvdxt для выполнения преобразования        
        this->appLauncher.SetExecutable(this->toolPath);
        this->appLauncher.SetWorkingDirectory(this->srcPath.ExtractDirName());
        this->appLauncher.SetArguments(args);
#if __WIN32__
        this->appLauncher.SetNoConsoleWindow(this->quiet);
#endif        
        if (!this->appLauncher.LaunchWait())
        {
            this->logger->Warning("Не удалось запустить конвертер «%s»!\n", this->toolPath.AsCharPtr());
        }

        // скопировать преобразованную текстуру из временной папки в папку экспорта
        if (!this->CopyResult())
        {
            return false;
        }
    }
    return true;
}

}// пространство имен ToolkitUtil