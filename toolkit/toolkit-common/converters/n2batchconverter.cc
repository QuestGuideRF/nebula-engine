//------------------------------------------------------------------------------
//  n2batchconverter.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "n2batchconverter.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace ToolkitUtil;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
N2BatchConverter::N2BatchConverter() :
    platform(Platform::Win32),
    force(false),
    verbose(true),
    isValid(false),
    logger(0),
    texAttrTable(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
N2BatchConverter::~N2BatchConverter()
{
    if (this->IsValid())
    {
        this->Discard();
    }
}

//------------------------------------------------------------------------------
/**
    NOTE: ProjectInfo must be set to the right platform already!
*/
bool
N2BatchConverter::CheckRequiredProjectInfoAttrs(const ProjectInfo& projInfo)
{
    if (!projInfo.HasAttr("DstDir")) return false;
    if (!projInfo.HasAttr("SrcDir")) return false;
    if (!projInfo.HasAttr("IntDir")) return false;
    if (!projInfo.HasAttr("N2ConverterSrcDir")) return false;
    if (!projInfo.HasAttr("N2ConverterDstDir")) return false;
    if (!projInfo.HasAttr("TextureTool")) return false;
    if (!projInfo.HasAttr("TextureAttrTable")) return false;
    if (!projInfo.HasAttr("TextureSrcDir")) return false;
    if (!projInfo.HasAttr("TextureSrcDir")) return false;
    if (!projInfo.HasAttr("TextureDstDir")) return false;
    if (!projInfo.HasAttr("AnimSrcDir")) return false;
    if (!projInfo.HasAttr("AnimDstDir")) return false;    
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
N2BatchConverter::Setup(Logger& logger, const ToolkitUtil::ProjectInfo& projInfo)
{
    n_assert(!this->IsValid());
    n_assert(0 == this->logger);
    this->isValid = true;
    this->logger = &logger;

    // инструменты настройки назначает
    AssignRegistry::Instance()->SetAssign(Assign("src", projInfo.GetAttr("SrcDir")));
    AssignRegistry::Instance()->SetAssign(Assign("dst", projInfo.GetAttr("DstDir")));
    AssignRegistry::Instance()->SetAssign(Assign("int", projInfo.GetAttr("IntDir")));

    // настройка конвертера N2
    this->n2Converter.SetPlatform(this->platform);
    this->n2Converter.SetVerbose(this->verbose);
    this->n2Converter.SetForceFlag(this->force);
    this->n2Converter.SetSrcDir(projInfo.GetAttr("N2ConverterSrcDir"));
    this->n2Converter.SetDstDir(projInfo.GetAttr("N2ConverterDstDir"));
    this->n2Converter.Setup();

    // настроить конвертер текстур
    this->texConverter.SetPlatform(this->platform);
    this->texConverter.SetQuietFlag(!this->verbose);
    this->texConverter.SetForceFlag(this->force);
    this->texConverter.SetToolPath(projInfo.GetPathAttr("TextureTool"));
    this->texConverter.SetSrcDir(projInfo.GetAttr("TextureSrcDir"));
    this->texConverter.SetDstDir(projInfo.GetAttr("TextureDstDir"));
    this->texConverter.SetExternalTextureAttrTable(this->texAttrTable);    
    this->texConverter.Setup(logger);

    // настроить конвертер анимаций
    this->animConverter.SetPlatform(this->platform);
    this->animConverter.SetForceFlag(this->force);
    this->animConverter.SetSrcDir(projInfo.GetAttr("AnimSrcDir"));
    this->animConverter.SetDstDir(projInfo.GetAttr("AnimDstDir"));
    this->animConverter.Setup(logger);
}

//------------------------------------------------------------------------------
/**
*/
void
N2BatchConverter::Discard()
{
    n_assert(this->IsValid());
    n_assert(0 != this->logger);
    this->texConverter.Discard();
    this->animConverter.Discard();
    this->n2Converter.Discard();
    this->logger = 0;
    this->isValid = false;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2BatchConverter::ConvertAll()
{
    n_assert(this->IsValid());

    // конвертировать файлы .n2 в .n3
    if (!this->n2Converter.ConvertAll(*this->logger))
    {
        this->logger->Error("N2BatchConverter::ConvertAll(): Ошибка N2Converter::ConvertAll()!\n");
        return false;
    }

    // преобразовать ресурсы, используемые при преобразовании N2>N3
    if (!this->ConvertResources(this->n2Converter.GetUsedResources()))
    {
        this->logger->Error("N2BatchConverter::ConvertAll(): Не удалось преобразовать ресурсы!\n");
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2BatchConverter::ConvertCategory(const String& category)
{
    n_assert(this->IsValid());

    // конвертировать файлы .n2 в .n3
    if (!this->n2Converter.ConvertCategory(*this->logger, category))
    {
        this->logger->Error("N2BatchConverter::ConvertCategory(%s): N2Converter::ConvertCategory() не удалось!\n", category.AsCharPtr());
        return false;
    }

    // преобразовать ресурсы, используемые при преобразовании N2>N3
    if (!this->ConvertResources(this->n2Converter.GetUsedResources()))
    {
        this->logger->Error("N2BatchConverter::ConvertCategory(%s): не удалось преобразовать ресурсы!\n", category.AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2BatchConverter::ConvertFile(const String& category, const String& srcFile)
{
    n_assert(this->IsValid());

    // конвертировать файлы .n2 в .n3
    if (!this->n2Converter.ConvertFile(*this->logger, category, srcFile))
    {
        this->logger->Error("N2BatchConverter::ConvertFile(%s, %s): N2Converter::ConvertFile(%s) не удалось!\n", category.AsCharPtr(), srcFile.AsCharPtr());
        return false;
    }

    // преобразовать ресурсы, используемые при преобразовании N2>N3
    if (!this->ConvertResources(this->n2Converter.GetUsedResources()))
    {
        this->logger->Error("N2BatchConverter::ConvertFile(%s, %s): не удалось преобразовать ресурсы!\n", category.AsCharPtr(), srcFile.AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2BatchConverter::ConvertResources(const Array<String>& resIds)
{
    n_assert(this->IsValid());

    // разделить ресурсы на массивы типов ресурсов
    String texPattern("текс:*");
    String aniPattern("годы:*");
    String mshPattern("мш:*");

    Array<String> texPaths;
    Array<String> animPaths;
    texPaths.Reserve(1024);
    animPaths.Reserve(1024);
    IndexT i;
    for (i = 0; i < resIds.Size(); i++)
    {
        const String& curResId = resIds[i];
        if (String::MatchPattern(curResId, texPattern))
        {
            String texPath = this->BuildSourceTexturePath(curResId);
            if (texPath.IsValid())
            {
                texPaths.Append(texPath);
            }
        }
        else if (String::MatchPattern(curResId, aniPattern))
        {
            String animPath = this->BuildSourceAnimPath(curResId);
            if (animPath.IsValid())
            {
                animPaths.Append(animPath);
            }
        }
        else if (String::MatchPattern(curResId, mshPattern))
        {
            // игнорируйте файлы сеток, они уже в нужном месте
        }
        else
        {
            this->logger->Warning("ИГНОРИРУЕМ НЕВЕРНЫЙ ПУТЬ К РЕСУРСУ '%s'!\n", curResId.AsCharPtr());
        }
    }

    // конвертировать текстуры
    if (!this->texConverter.ConvertFiles(texPaths))
    {
        this->logger->Warning("ОШИБКИ ПРИ ПРЕОБРАЗОВАНИИ ТЕКСТУР!\n");
    }

    // конвертировать анимацию
    if (!this->animConverter.ProcessFiles(animPaths))
    {
        this->logger->Warning("ОШИБКИ ПРИ ПРЕОБРАЗОВАНИИ АНИМАЦИИ!\n");
    }

    return true;
}

//------------------------------------------------------------------------------
/**
    Takes a target texture path (e.g. "tex:category/texture.dss") and 
    converts it to its source texture (.psd or .tga).
*/
String
N2BatchConverter::BuildSourceTexturePath(const String& resId) const
{
    // извлечь категорию и имя файла текстуры
    Array<String> tokens = resId.Tokenize(":/");
    n_assert(tokens.Size() >= 3);
    String texFileName = tokens[tokens.Size() - 1];
    String texCategoryName = tokens[tokens.Size() - 2];
    texFileName.StripFileExtension();

    // нужно проверить, был ли это файл PSD, TGA или DDS
    String tgaPath, psdPath, ddsPath;
    tgaPath.Format("%s/%s/%s.tga", this->texConverter.GetSrcDir().AsCharPtr(), texCategoryName.AsCharPtr(), texFileName.AsCharPtr());
    psdPath.Format("%s/%s/%s.psd", this->texConverter.GetSrcDir().AsCharPtr(), texCategoryName.AsCharPtr(), texFileName.AsCharPtr());
    ddsPath.Format("%s/%s/%s.dds", this->texConverter.GetSrcDir().AsCharPtr(), texCategoryName.AsCharPtr(), texFileName.AsCharPtr());

    IoServer* ioServer = IoServer::Instance();
    if (ioServer->FileExists(psdPath))
    {
        return psdPath;
    }
    else if (ioServer->FileExists(tgaPath))
    {
        return tgaPath;
    }
    else if (ioServer->FileExists(ddsPath))
    {
        return ddsPath;
    }
    else
    {
        this->logger->Warning("Could not find source texture for '%s' (psd, tga or dds)\n", resId.AsCharPtr());
        return "";
    }
}

//------------------------------------------------------------------------------
/**
    Создает путь к исходному файлу nax2 на основе пути назначения nax3.
*/
String
N2BatchConverter::BuildSourceAnimPath(const String& resId) const
{
    // извлечь категорию и имя файла анимации
    Array<String> tokens = resId.Tokenize(":/");
    n_assert(tokens.Size() >= 3);
    String animFileName = tokens[tokens.Size() - 1];
    String animCategoryName = tokens[tokens.Size() - 2];
    animFileName.StripFileExtension();

    // построить исходный путь
    String nax2Path;
    nax2Path.Format("%s/%s/%s.nax2", this->animConverter.GetSrcDir().AsCharPtr(), animCategoryName.AsCharPtr(), animFileName.AsCharPtr());
    if (IoServer::Instance()->FileExists(nax2Path))
    {
        return nax2Path;
    }
    else
    {
        return "";
    }
}

} // пространство имен ToolkitUtil
