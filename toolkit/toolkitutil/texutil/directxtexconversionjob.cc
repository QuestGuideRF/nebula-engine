//------------------------------------------------------------------------------
// win32textureconversionjob.cc
// (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "directxtexconversionjob.h"
#include "io/uri.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "system/systeminfo.h"

#include "toolkit-common/text.h"

namespace ToolkitUtil
{
using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
DirectXTexConversionJob::DirectXTexConversionJob()
{
    System::SystemInfo systemInfo;
    this->toolPath.Format("набор инструментов:bin/%s/texconv", System::PlatformTypeAsString(System::Platform));
    this->cubeToolPath.Format("набор инструментов:bin/%s/texassemble", System::PlatformTypeAsString(System::Platform));
    this->SetDstFileExtension("dds");
}

//------------------------------------------------------------------------------
/**
*/
static const char*
GetTexConvFormat(TextureAttrs const& attrs)
{
    switch (attrs.GetPixelFormat())
    {
    case TextureAttrs::DXT1C:
    case TextureAttrs::DXT1A:  return "DXT1";
    case TextureAttrs::DXT3:   return attrs.GetColorSpace() == TextureAttrs::sRGB ? "BC2_UNORM_SRGB" : "BC2_UNORM";
    case TextureAttrs::DXT5:   return attrs.GetColorSpace() == TextureAttrs::sRGB ? "BC3_UNORM_SRGB" : "BC3_UNORM";
    case TextureAttrs::DXT5NM: return "BC5_UNORM";
    case TextureAttrs::U1555:  return "B5G5R5A1_UNORM";
    case TextureAttrs::U4444: return "B4G4R4A4_UNORM";
    case TextureAttrs::U565: return "B5G6R5_UNORM";
    case TextureAttrs::U8888: return "R8G8B8A8_UNORM_SRGB";
    case TextureAttrs::U888: return "BC7_UNORM";
    case TextureAttrs::BC4: return "BC4_UNORM";
    case TextureAttrs::BC5: "BC5_UNORM";
    case TextureAttrs::BC6H: return "BC6H_UF16";
    case TextureAttrs::BC7: return attrs.GetColorSpace() == TextureAttrs::sRGB ? "BC7_UNORM_SRGB" : "BC7_UNORM";
    case TextureAttrs::R8: return "R8_UNORM";
    case TextureAttrs::R16F: return "R16_FLOAT";
    case TextureAttrs::R16G16F: return "R16G16_FLOAT";
    case TextureAttrs::R16: return "R16_UNORM";
    case TextureAttrs::R16G16: return "R16G16_UNORM";
    case TextureAttrs::DXGI: return attrs.GetDxgi().AsCharPtr();
    }
    return "BC7_UNORM";
}

//------------------------------------------------------------------------------
/**
    Запустите процесс конвертации. Возвращает false, если задание завершилось немедленно.
*/
bool
DirectXTexConversionJob::Convert()
{
    n_assert(this->toolPath.IsValid());
    n_assert(0 != this->logger);
    if (TextureConversionJob::Convert())
    {
        URI srcPathUri(this->srcPath);
        URI dstPathUri(this->dstPath);
        URI tmpDirUri(this->tmpDir);

        enum TextureDataType
        {
            None,
            NormalMap,
            MaterialMap,
            HeightMap,
            
        };
        TextureDataType type = TextureDataType::None;

        // построить аргументы командной строки для TexConv
        String args = " -y -sepalpha -dx10 -nologo -timing ";
        const TextureAttrs& attrs = this->textureAttrs;
        if ((attrs.GetPixelFormat() == TextureAttrs::DXT5NM) ||
            (attrs.GetPixelFormat() == TextureAttrs::BC5) ||
            (String::MatchPattern(this->srcPath, "*норма.*")) ||
            (String::MatchPattern(this->srcPath, "*нормальный.*")) ||
            (String::MatchPattern(this->srcPath, "*Нормальный.*")) ||
            (String::MatchPattern(this->srcPath, "*ударяться.*")))
        {
            this->logger->Print("%s ", "Карта нормалей (BC5 UNORM)"_text.Color(TextColor::Yellow).AsCharPtr());
            type = TextureDataType::NormalMap;
        }
        else if (String::MatchPattern(this->srcPath, "*материал.*") ||
            String::MatchPattern(this->srcPath, "*норм.*") ||
            String::MatchPattern(this->srcPath, "*ОРМ.*") ||
            String::MatchPattern(this->srcPath, "*Металлическая шероховатость.*") ||
            String::MatchPattern(this->srcPath, "*Окклюзия.*"))
        {
            this->logger->Print("%s ", "Карта данных материала/окклюзии (BC7 UNORM)"_text.Color(TextColor::Yellow).AsCharPtr());
            type = TextureDataType::MaterialMap;
        }
        else if (String::MatchPattern(this->srcPath, "*высота.*"))
        {
            this->logger->Print("%s ", "Карта высот (R16 UNORM)"_text.Color(TextColor::Yellow).AsCharPtr());
            type = TextureDataType::HeightMap;
        }
        /*
                args.Append("-w");
                args.AppendInt(attrs.GetMaxWidth());
                args.Append("-h");
                args.AppendInt(attrs.GetMaxHeight());*/

        if (attrs.GetQuality() == TextureAttrs::High)
        {
            args.Append(" -BC х ");
        }
        else
        {
            args.Append(" -BC q ");
        }

        if (!attrs.GetGenMipMaps())
        {
            args.Append(" -m 1 ");
        }
        switch (type)
        {
            case None:
                if (attrs.GetColorSpace() == TextureAttrs::sRGB)
                {
                    args.Append(" -srgb ");
                }
                else
                {
                    args.Append(" -srgbo ");
                }
                args.Append(" -f ");
                args.Append(GetTexConvFormat(attrs));
                break;
            case NormalMap:
                if (attrs.GetFlipNormalY())
                {
                    args.Append(" -inverty ");
                }
                args.Append(" -f BC5_UNORM ");
                break;
            case HeightMap:
                args.Append(" -f R16_UNORM ");
                break;
            case MaterialMap:
                args.Append(" -f BC7_UNORM ");
                break;
        }

        args.Append(" \"");
        Util::String srcPath = IO::IoServer::NativePath(srcPathUri.LocalPath());
        //srcPath.ReplaceChars("/", '\\');
        args.Append(srcPath);
        args.Append("\" -o \"");
        Util::String tmpPath = IO::IoServer::NativePath(tmpDirUri.LocalPath());
        //tmpPath.ReplaceChars("/", '\\');
        args.Append(tmpPath);
        args.Append("\"");

        // запустите texconv ​​для выполнения преобразования        
        this->appLauncher.SetNoConsoleWindow(true);
        this->appLauncher.SetExecutable(this->toolPath);
        this->appLauncher.SetWorkingDirectory(this->srcPath.ExtractDirName());
        this->appLauncher.SetArguments(args);
#if __WIN32__
        //this->appLauncher.SetNoConsoleWindow(this->quiet);
#endif        
        if (!this->appLauncher.LaunchWait())
        {
            this->logger->Warning("Не удалось запустить конвертер «%s»!\n", this->toolPath.AsCharPtr());
            return false;
        }

        // скопировать преобразованную текстуру из временной папки в папку экспорта
        if (!this->CopyResult())
        {
            return false;
        }

        ToolkitUtil::Text print = Util::String::Sprintf("%s -> %s... ", Text(URI(this->srcPath).LocalPath()).Color(TextColor::Blue).AsCharPtr(), Text(Format("%s", URI(this->dstPath).LocalPath().AsCharPtr())).Color(TextColor::Green).Style(FontMode::Underline).AsCharPtr());
        this->logger->Print(Util::String::Sprintf("%s%s", print.AsCharPtr(), "done\n"_text.Color(TextColor::Green).AsCharPtr()).AsCharPtr());
    }

    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
DirectXTexConversionJob::ConvertCube()
{
    n_assert(this->toolPath.IsValid());
    n_assert(0 != this->logger);

    //FIXME, это какая-то хакерская херня
    this->tmpDir.Append("/");
    this->tmpDir.Append(this->srcPath.ExtractLastDirName().AsCharPtr());
    this->neverCopy = true;
    if (TextureConversionJob::Convert())
    {

        URI srcPathUri(this->srcPath);
        URI dstPathUri(this->dstPath);
        URI tmpDirUri(this->tmpDir);

        String args = "";
        Array<String> files = IoServer::Instance()->ListFiles(this->srcPath, "*.*");
        if (files.Size() != 6)
        {
            this->logger->Warning("Для кубической карты требуется ровно 6 изображений!\n");
            return false;
        }

        files.Sort();

        args.Append("куб -нолого -о \"");

        Util::String tmpPath = tmpDirUri.LocalPath();
        tmpPath.ReplaceChars("/", '\\');
        tmpPath.Append("\\");
        tmpPath.Append(this->dstPath.ExtractFileName());
        args.Append(tmpPath);
        args.Append("\"");

        for (int i = 0, n = files.Size(); i < n; i++)
        {
            args.Append(" ");
            args.Append(files[i].AsCharPtr());
        }

        this->appLauncher.SetExecutable(this->cubeToolPath);
        this->appLauncher.SetWorkingDirectory(this->srcPath);
        this->appLauncher.SetArguments(args);
#if __WIN32__
        //this->appLauncher.SetNoConsoleWindow(this->quiet);
#endif        
        if (!this->appLauncher.LaunchWait())
        {
            this->logger->Warning("Не удалось запустить конвертер «%s»!\n", this->cubeToolPath.AsCharPtr());
            return false;
        }

        this->srcPath = tmpPath;
        return this->Convert();
    }
    return false;
}
}// пространство имен ToolkitUtil
