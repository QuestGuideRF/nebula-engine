//------------------------------------------------------------------------------
//  shadercompiler.cc
//  (C) 2012 Густав Стербрант
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "shadercompiler.h"
#include "io/ioserver.h"
#include "io/xmlreader.h"
#include "coregraphics/config.h"
#include "app/application.h"

#if __DX11__
#include <d3dx11.h>
#include <D3Dcompiler.h>
#endif

#if __ANYFX__
#include "afxcompiler.h"
#endif
#include "toolkit-common/converters/binaryxmlconverter.h"

using namespace Util;
using namespace IO;
namespace ToolkitUtil
{

//------------------------------------------------------------------------------
/**
*/
ShaderCompiler::ShaderCompiler() :
    language("HLSL"),
    srcShaderCustomDir(""),
    srcFrameShaderCustomDir(""),
    srcMaterialCustomDir(""),    
    platform(Platform::Win32),
    force(false),
    debug(false),
    quiet(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ShaderCompiler::~ShaderCompiler()
{
    // empty
}


//------------------------------------------------------------------------------
/**
*/
bool
ShaderCompiler::CheckRecompile(const Util::String& srcPath, const Util::String& dstPath)
{
    if (!this->force)
    {
        // проверить штампы файлов
        IoServer* ioServer = IoServer::Instance();
        if (ioServer->FileExists(dstPath))
        {
            FileTime srcTime = ioServer->GetFileWriteTime(srcPath);
            FileTime dstTime = ioServer->GetFileWriteTime(dstPath);
            if (dstTime > srcTime)
            {
                return false;
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
/**
*/
bool 
ShaderCompiler::CompileShaders()
{
    n_assert(this->srcShaderBaseDir.IsValid());
    n_assert(this->dstShaderDir.IsValid());
    const Ptr<IoServer>& ioServer = IoServer::Instance();
    this->shaderNames.Clear();

    // проверьте, существует ли исходный каталог
    if (!ioServer->DirectoryExists(this->srcShaderBaseDir))
    {
        n_error("ОШИБКА: исходный каталог шейдера «%s» не найден!\n", this->srcShaderBaseDir.AsCharPtr());
        return false;
    }

    // убедитесь, что целевой каталог существует
    ioServer->CreateDirectory(this->dstShaderDir);

    // попытайтесь скомпилировать базовые шейдеры
    bool retval = false;
    if (this->language == "HLSL")
    {
        retval = this->CompileHLSL(this->srcShaderBaseDir);
    }
    else if (this->language == "GLSL")
    {
        retval = this->CompileGLSL(this->srcShaderBaseDir);
    }
    else if (this->language == "SPIRV")
    {
        retval = this->CompileSPIRV(this->srcShaderBaseDir);
    }

    if (this->srcShaderCustomDir.IsValid() && URI(this->srcShaderCustomDir) != URI(this->srcShaderBaseDir))
    {
        // попытаться скомпилировать собственные шейдеры
        if (this->language == "HLSL")
        {
            this->CompileHLSL(this->srcShaderCustomDir);
        }
        else if (this->language == "GLSL")
        {
            this->CompileGLSL(this->srcShaderCustomDir);
        }
        else if (this->language == "SPIRV")
        {
            this->CompileSPIRV(this->srcShaderCustomDir);
        }
    }    

    if (retval)
    {
        retval = this->WriteShaderDictionary();
    }

    return retval;
}

//------------------------------------------------------------------------------
/**
*/
bool 
ShaderCompiler::CompileFrameShaders()
{
    n_assert(this->srcFrameShaderBaseDir.IsValid());
    n_assert(this->dstFrameShaderDir.IsValid());
    const Ptr<IoServer>& ioServer = IoServer::Instance();

    // проверьте, существует ли исходный каталог
    if (!ioServer->DirectoryExists(this->srcFrameShaderBaseDir))
    {
        n_error("ОШИБКА: исходный каталог шейдера кадра «%s» не найден!\n", this->srcFrameShaderBaseDir.AsCharPtr());
        return false;
    }

    // убедитесь, что целевой каталог существует
    ioServer->CreateDirectory(this->dstFrameShaderDir);

    // for each base frame shader...
    bool success = true;
    Array<String> srcFiles = ioServer->ListFiles(this->srcFrameShaderBaseDir, "*.json");
    IndexT i;
    for (i = 0; i < srcFiles.Size(); i++)
    {
        // построить абсолютные исходные и целевые имена файлов
        String srcPath;
        srcPath.Format("%s/%s", this->srcFrameShaderBaseDir.AsCharPtr(), srcFiles[i].AsCharPtr());
        String dstPath;
        dstPath.Format("%s/%s", this->dstFrameShaderDir.AsCharPtr(), srcFiles[i].AsCharPtr());
        if (this->CheckRecompile(srcPath, dstPath))
        {
            success &= ioServer->CopyFile(srcPath, dstPath);
            n_printf("Преобразованный сценарий базового кадра: %s ---> %s \n", srcPath.AsCharPtr(), dstPath.AsCharPtr());
        }
    }

    if (this->srcFrameShaderCustomDir.IsValid() && URI(this->srcFrameShaderCustomDir) != URI(this->srcFrameShaderBaseDir))
    {
        // for each custom frame shader...
        srcFiles = ioServer->ListFiles(this->srcFrameShaderCustomDir, "*.json");
        for (i = 0; i < srcFiles.Size(); i++)
        {
            // построить абсолютные исходные и целевые имена файлов
            String srcPath;
            srcPath.Format("%s/%s", this->srcFrameShaderCustomDir.AsCharPtr(), srcFiles[i].AsCharPtr());
            String dstPath;
            dstPath.Format("%s/%s", this->dstFrameShaderDir.AsCharPtr(), srcFiles[i].AsCharPtr());
            if (this->CheckRecompile(srcPath, dstPath))
            {
                success &= ioServer->CopyFile(srcPath, dstPath);
                n_printf("Преобразованный сценарий пользовательского фрейма: %s ---> %s \n", srcPath.AsCharPtr(), dstPath.AsCharPtr());
            }
        }
    }
    
    return success;
}


//------------------------------------------------------------------------------
/**
*/
bool 
ShaderCompiler::CompileMaterials()
{
    n_assert(this->srcMaterialBaseDir.IsValid());
    n_assert(this->dstMaterialDir.IsValid());
    const Ptr<IoServer>& ioServer = IoServer::Instance();

    // проверьте, существует ли исходный каталог
    if (!ioServer->DirectoryExists(this->srcMaterialBaseDir))
    {
        n_warning("ВНИМАНИЕ: исходный каталог материалов '%s' не найден!\n", this->srcMaterialBaseDir.AsCharPtr());
        return false;
    }

    Util::String baseMaterialTemplateSrcDir = this->srcMaterialBaseDir;
    Util::String customMaterialTemplateSrcDir = this->srcMaterialCustomDir;
    Util::String materialTemplateDstDir = this->dstMaterialDir;

    // создать конвертер
    BinaryXmlConverter converter;
    ToolkitUtil::Logger logger;
    logger.SetVerbose(false);
    converter.SetPlatform(Platform::Win32);

    // удалить старые каталоги
    ioServer->DeleteDirectory(materialTemplateDstDir);

    // сделать новый
    ioServer->CreateDirectory(materialTemplateDstDir);

    // for each base material table...
    bool success = true;
    Array<String> srcFiles = ioServer->ListFiles(baseMaterialTemplateSrcDir, "*.xml");
    IndexT i;
    for (i = 0; i < srcFiles.Size(); i++)
    {
        // построить абсолютные исходные и целевые имена файлов
        String srcPath;
        srcPath.Format("%s/%s", baseMaterialTemplateSrcDir.AsCharPtr(), srcFiles[i].AsCharPtr());
        String dstPath;
        dstPath.Format("%s/%s", materialTemplateDstDir.AsCharPtr(), srcFiles[i].AsCharPtr());

        URI src(srcPath);
        URI dst(dstPath);

        if (this->CheckRecompile(srcPath, dstPath))
        {
            // конвертировать в двоичный XML
            success &= converter.ConvertFile(srcPath, dstPath, logger);
            n_printf("Преобразованная таблица шаблонов базового материала: %s ---> %s \n", src.LocalPath().AsCharPtr(), dst.LocalPath().AsCharPtr());
        }
    }

    if (this->srcMaterialCustomDir.IsValid() && URI(this->srcMaterialCustomDir) != URI(this->srcMaterialBaseDir))
    {
        // for each custom material table...
        srcFiles = ioServer->ListFiles(customMaterialTemplateSrcDir, "*.xml");
        for (i = 0; i < srcFiles.Size(); i++)
        {
            // построить абсолютные исходные и целевые имена файлов
            String srcPath;
            srcPath.Format("%s/%s", customMaterialTemplateSrcDir.AsCharPtr(), srcFiles[i].AsCharPtr());
            String dstPath;
            
            // отформатировать материал, который будет расширен с помощью _custom
            String file = srcFiles[i];
            file.StripFileExtension();
            dstPath.Format("%s/%s_custom.xml", materialTemplateDstDir.AsCharPtr(), file.AsCharPtr());

            URI src(srcPath);
            URI dst(dstPath);

            if (this->CheckRecompile(srcPath, dstPath))
            {
                // скопировать файл
                success &= converter.ConvertFile(srcPath, dstPath, logger);
                n_printf("Преобразованная таблица нестандартных материалов: %s ---> %s \n", src.LocalPath().AsCharPtr(), dst.LocalPath().AsCharPtr());
            }
        }
    }
    
    return success;
}


//------------------------------------------------------------------------------
/**
*/
bool 
ShaderCompiler::CompileHLSL(const Util::String& srcPath)
{
    const Ptr<IoServer>& ioServer = IoServer::Instance();

    // скопировать файлы SDH в место назначения
    bool retval = true;

    // список файлов в каталоге
    Array<String> srcFiles = ioServer->ListFiles(srcPath, "*.fx");

#ifndef __DX11__
    n_printf("Ошибка: невозможно скомпилировать шейдеры DX11 без поддержки DX11\n");
    return false;
#endif

    // просмотреть все файлы и скомпилировать
    IndexT j;
    for (j = 0; j < srcFiles.Size(); j++)
    {
        // получить файл
        String file = srcFiles[j];

        // форматировать строку с файлом
        String srcFile = srcPath + "/" + file;

        // формат назначения
        String destFile = this->dstShaderDir + "/" + file;
        destFile.StripFileExtension();
        file.StripFileExtension();

        URI src(srcFile);
        URI dst(destFile);


        // добавить в словарь
        this->shaderNames.Append(file);

        if (this->CheckRecompile(srcFile, destFile))
        {
            // compile
            n_printf("Компиляция:\n %s -> %s\n", src.LocalPath().AsCharPtr(), dst.LocalPath().AsCharPtr());

#if __DX11__
            ID3D10Blob* compiledShader = 0;
            ID3D10Blob* errorBlob = 0;

            HRESULT hr = D3DX11CompileFromFile(
                AssignRegistry::Instance()->ResolveAssignsInString(srcFile).AsCharPtr(),
                NULL,
                NULL,
                NULL,
                "fx_5_0",
                D3DCOMPILE_OPTIMIZATION_LEVEL3,
                NULL,
                NULL,
                &compiledShader,
                &errorBlob,
                &hr);

            if (FAILED(hr))
            {
                if (errorBlob)
                {
                    String error;
                    error.AppendRange((const char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
                    n_printf("%s\n", error.AsCharPtr());
                }
                else
                {
                    n_printf("Необработанная ошибка компиляции!\n");
                }
                retval = false;
            }
            else
            {


                // записать скомпилированный шейдер в файл
                Ptr<Stream> stream = ioServer->CreateStream(destFile);
                stream->SetAccessMode(Stream::WriteAccess);

                // открытый поток
                if (stream->Open())
                {
                    // написать шейдер
                    stream->Write(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize());

                    // закрыть файл
                    stream->Close();
                }
                else
                {
                    n_printf("Не удалось открыть файл %s\n", destFile.AsCharPtr());
                    retval = false;
                }
            }
#endif
        }
        else
        {
            retval = true;
        }

    }
    return retval;
}

//------------------------------------------------------------------------------
/**
    Реализовано с помощью AnyFX
*/
bool 
ShaderCompiler::CompileGLSL(const Util::String& srcPath)
{
    const Ptr<IoServer>& ioServer = IoServer::Instance();

    // список файлов в каталоге
    Array<String> srcFiles = ioServer->ListFiles(srcPath, "*.fx");

#ifndef __ANYFX__
    n_printf("Ошибка: невозможно скомпилировать шейдеры DX11 без поддержки DX11\n");
    return false;
#endif

#if __ANYFX__
    // запустить компиляцию AnyFX
    AnyFXBeginCompile();

    // просмотреть все файлы и скомпилировать
    IndexT j;
    for (j = 0; j < srcFiles.Size(); j++)
    {
        // получить файл
        String file = srcFiles[j];

        // форматировать строку с файлом
        String srcFile = srcPath + "/" + file;

        // формат назначения
        String destFile = this->dstShaderDir + "/" + file;
        destFile.StripFileExtension();
        file.StripFileExtension();

        URI src(srcFile);
        URI dst(destFile);

        // добавить в словарь
        this->shaderNames.Append(file);

        if (this->CheckRecompile(srcFile, destFile))
        {
            // compile
            n_printf("Компиляция:\n %s -> %s\n", src.LocalPath().AsCharPtr(), dst.LocalPath().AsCharPtr());

            URI shaderFolder("toolkit:work/shaders/gl");
            std::vector<std::string> defines;
            std::vector<std::string> flags;
            Util::String define;
            define.Format("-Д ГЛСЛ");
            defines.push_back(define.AsCharPtr());

            // сначала подключите эту папку
            define.Format("-Я%s/", URI(srcPath).LocalPath().AsCharPtr());
            defines.push_back(define.AsCharPtr());

            // затем подключите папку шейдеров N3 Toolkit
            define.Format("-Я%s/", shaderFolder.LocalPath().AsCharPtr());
            defines.push_back(define.AsCharPtr());

            // устанавливать флаги
            flags.push_back("/NOSUB");      // деактивировать использование подпрограммы
            flags.push_back("/GBLOCK");     // поместите все переменные шейдера вне явных буферов в один глобальный блок

            // if using debug, output raw shader code
            if (this->debug)
            {
                flags.push_back("/O");
            }
            
            AnyFXErrorBlob* errors = NULL;

            // это получит максимально возможное значение для версии GL, теперь закрепите минорное и мажорное значение до того, которое поддерживается glew
            int major = 4;
            int minor = 4;
            
            Util::String target;
            target.Format("гл%d%d", major, minor);
            Util::String escapedSrc = src.LocalPath();
            //escapedSrc.SubstituteString(" ", "\\ ");
            Util::String escapedDst = dst.LocalPath();
            //escapeddst.SubstituteString(" ", "\\ ");
            
            bool res = AnyFXCompile(escapedSrc.AsCharPtr(), escapedDst.AsCharPtr(), target.AsCharPtr(), nullptr, "Khronos", defines, flags, &errors);
            if (!res)
            {
                if (errors)
                {
                    n_printf("%s\n", errors->buffer);
                    delete errors;
                    errors = 0;
                }
                return false;
            }
            else if (errors)
            {
                n_printf("%s\n", errors->buffer);
                delete errors;
                errors = 0;
            }
        }
    }

    // остановить компиляцию AnyFX
    AnyFXEndCompile();
#else
#error "Компилятор GLSL не реализован! (используйте определение __ANYFX__, чтобы исправить это)"
#endif

    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
ShaderCompiler::CompileSPIRV(const Util::String& srcPath)
{
    const Ptr<IoServer>& ioServer = IoServer::Instance();

    // список файлов в каталоге
    Array<String> srcFiles = ioServer->ListFiles(srcPath, "*.fx");

#ifndef __ANYFX__
    n_printf("Ошибка: невозможно скомпилировать шейдеры DX11 без поддержки DX11\n");
    return false;
#endif

#if __ANYFX__
    // запустить компиляцию AnyFX
    AnyFXBeginCompile();

    // просмотреть все файлы и скомпилировать
    IndexT j;
    for (j = 0; j < srcFiles.Size(); j++)
    {
        // получить файл
        String file = srcFiles[j];

        // форматировать строку с файлом
        String srcFile = srcPath + "/" + file;

        // формат назначения
        String destFile = this->dstShaderDir + "/" + file;
        destFile.StripFileExtension();
        file.StripFileExtension();

        // добавить в словарь
        this->shaderNames.Append(file);

        URI shaderFolder(this->srcShaderBaseDir);
        std::vector<std::string> defines;
        std::vector<std::string> flags;
        Util::String define;
        define.Format("-Д ГЛСЛ");
        defines.push_back(define.AsCharPtr());

        // сначала подключите эту папку
        define.Format("-Я%s/", URI(srcPath).LocalPath().AsCharPtr());
        defines.push_back(define.AsCharPtr());

        // затем подключите папку шейдеров N3 Toolkit
        define.Format("-Я%s/", shaderFolder.LocalPath().AsCharPtr());
        defines.push_back(define.AsCharPtr());

        URI src(srcFile);
        URI dst(destFile);

        bool needRecompile = false;

        // генерировать зависимости
        std::vector<std::string> deps = AnyFXGenerateDependencies(src.LocalPath().AsCharPtr(), defines);

        uint i;
        for (i = 1; i < deps.size(); i++)
        {
            Util::String dep = deps[i].c_str();
            needRecompile |= this->CheckRecompile(dep, destFile);
        }
        needRecompile |= this->CheckRecompile(srcFile, destFile);
        needRecompile |= this->CheckRecompile(App::Application::Instance()->GetCmdLineArgs().GetCmdName(), destFile);

        if (needRecompile)
        {
            // compile
            n_printf("Компиляция %s:\n %s -> %s\n", srcFile.AsCharPtr(), src.LocalPath().AsCharPtr(), dst.LocalPath().AsCharPtr());

            // устанавливать флаги
            flags.push_back("/NOSUB");          // деактивировать использование подпрограмм, эффективно расширяет все подпрограммы как функции
            flags.push_back("/GBLOCK");         // поместите все переменные шейдера вне явного блока в один глобальный блок
            flags.push_back(Util::String::Sprintf("/DEFAULTSET %d", NEBULA_BATCH_GROUP).AsCharPtr());   // since we want the most frequently switched set as high as possible, we send the default set to 8, must match the NEBULAT_DEFAULT_GROUP in std.fxh and DEFAULT_GROUP in coregraphics/config.h

            // if using debug, output raw shader code
            if (this->debug)
            {
                flags.push_back("/O");
            }

            AnyFXErrorBlob* errors = NULL;

            // это получит максимально возможное значение для версии GL, теперь закрепите минорное и мажорное значение до того, которое поддерживается glew
            int major = 1;
            int minor = 0;

            Util::String target;
            target.Format("СПВ%d%d", major, minor);
            Util::String escapedSrc = src.LocalPath();
            Util::String escapedDst = dst.LocalPath();

            bool res = AnyFXCompile(escapedSrc.AsCharPtr(), escapedDst.AsCharPtr(), target.AsCharPtr(), nullptr, "Khronos", defines, flags, &errors);
            if (!res)
            {
                if (errors)
                {
                    n_printf("%s\n", errors->buffer);
                    delete errors;
                    errors = 0;
                }
                return false;
            }
            else if (errors)
            {
                n_printf("%s\n", errors->buffer);
                delete errors;
                errors = 0;
            }
        }
    }

    // остановить компиляцию AnyFX
    AnyFXEndCompile();
#else
#error "Компилятор SPIR-V не реализован! (используйте определение __ANYFX__, чтобы исправить это)"
#endif

    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool 
ShaderCompiler::WriteShaderDictionary()
{
    String filename;
    filename.Format("%s/shaders.dic", this->dstShaderDir.AsCharPtr());
    Ptr<Stream> stream = IoServer::Instance()->CreateStream(filename);
    Ptr<TextWriter> textWriter = TextWriter::Create();
    textWriter->SetStream(stream);
    if (textWriter->Open())
    {
        String prefix = "shd:";
        IndexT i;
        for (i = 0; i < this->shaderNames.Size(); i++)
        {
            textWriter->WriteLine(prefix + this->shaderNames[i]);        
        }
        textWriter->Close();
        return true;
    }
    return false;
}


} // пространство имен ToolkitUtil