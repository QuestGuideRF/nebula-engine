//------------------------------------------------------------------------------
//  shaderc.cc
//  (C) 2018-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "singleshadercompiler.h"
#include "io/ioserver.h"
#include "coregraphics/config.h"
#include "app/application.h"
#include "toolkit-common/converters/binaryxmlconverter.h"

#if __ANYFX__
#include "afxcompiler.h"
#endif

using namespace Util;
using namespace IO;
namespace ToolkitUtil
{

//------------------------------------------------------------------------------
/**
*/
SingleShaderCompiler::SingleShaderCompiler() :
    language("SPIRV"),          
    platform(Platform::Win32),  
    debug(false),
    quiet(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
SingleShaderCompiler::~SingleShaderCompiler()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool 
SingleShaderCompiler::CompileShader(const Util::String& src)
{
    if (!this->dstBinary.IsValid())
    {
        n_printf("Ошибка шейдера: нет места для компиляции шейдера");
        return false;
    }

    if (!this->dstHeader.IsValid())
    {
        n_printf("Ошибка shaperc: нет папки вывода заголовка для компиляции шейдера");
        return false;
    }
    
    const Ptr<IoServer>& ioServer = IoServer::Instance();
    
    // проверьте, есть ли источник 
    if (!ioServer->FileExists(src))
    {
        n_printf("Ошибка [shaderc]: источник шейдера «%s» не найден!\n", src.AsCharPtr());
        return false;
    }

    // убедитесь, что целевой каталог существует
    ioServer->CreateDirectory(this->dstBinary);
    ioServer->CreateDirectory(this->dstHeader);

    // попытайтесь скомпилировать базовые шейдеры
    bool retval = false;
    if (this->language == "GLSL")
    {
        retval = this->CompileGLSL(src);
    }
    else if (this->language == "SPIRV")
    {
        retval = this->CompileSPIRV(src);
    }
    
    return retval;
}

//------------------------------------------------------------------------------
/**
*/
bool
SingleShaderCompiler::CompileFrameShader(const Util::String& srcf)
{
    const Ptr<IoServer>& ioServer = IoServer::Instance();

    // проверьте, существует ли исходный каталог
    if (!ioServer->FileExists(URI(srcf)))
    {
        n_printf("Ошибка [shaderc]: источник шейдера кадра «%s» не найден!\n", srcf.AsCharPtr());
        return false;
    }

    // убедитесь, что целевой каталог существует
    Util::String frameDest = this->dstBinary + "/frame/";
    ioServer->CreateDirectory(frameDest);
    frameDest.Append(srcf.ExtractFileName());
    ioServer->CopyFile(srcf, frameDest);
    n_printf("[shaderc] Скрипт преобразованного базового кадра:\n %s ---> %s \n", srcf.AsCharPtr(), frameDest.AsCharPtr());
    
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool 
SingleShaderCompiler::CompileMaterial(const Util::String & srcf)
{   
    const Ptr<IoServer>& ioServer = IoServer::Instance();

    // создать конвертер
    BinaryXmlConverter converter;
    ToolkitUtil::Logger logger;
    logger.SetVerbose(false);
    converter.SetPlatform(Platform::Win32);
    
    // убедитесь, что вывод существует
    Util::String destDir = this->dstBinary + "/materials";
    ioServer->CreateDirectory(destDir);
    
    Util::String dest = destDir + "/" + srcf.ExtractFileName();
    
    URI src(srcf);
    URI dst(dest);
    Logger dummy;
    // конвертировать в двоичный XML
    n_printf("[shaderc] Преобразование таблицы шаблонов базового материала:\n %s ---> %s \n", src.LocalPath().AsCharPtr(), dst.LocalPath().AsCharPtr());
    return converter.ConvertFile(srcf, dest, dummy);
}

//------------------------------------------------------------------------------
/**
    Реализовано с помощью AnyFX
*/
bool 
SingleShaderCompiler::CompileGLSL(const Util::String& srcf)
{
    const Ptr<IoServer>& ioServer = IoServer::Instance();

#ifndef __ANYFX__
    n_printf("Ошибка: невозможно скомпилировать шейдеры DX11 без поддержки DX11\n");
    return false;
#endif

#if __ANYFX__
    // запустить компиляцию AnyFX
    AnyFXBeginCompile();

    Util::String file = srcf.ExtractFileName();
    file.StripFileExtension();
    // формат назначения
    String destFile = this->dstBinary + "/" + file;

    URI src(srcf);
    URI dst(destFile);

    // compile
    n_printf("[shaderc] Компиляция:\n %s -> %s\n", src.LocalPath().AsCharPtr(), dst.LocalPath().AsCharPtr());

    std::vector<std::string> defines;
    std::vector<std::string> flags;
    Util::String define;
    define.Format("-Д ГЛСЛ");
    defines.push_back(define.AsCharPtr());

    // сначала подключите эту папку
    define.Format("-Я%s/", URI(srcf).LocalPath().AsCharPtr());
    defines.push_back(define.AsCharPtr());

    for (auto inc = this->includeDirs.Begin(); inc != this->includeDirs.End(); inc++)
    {
        define.Format("-Я%s/", URI(*inc).LocalPath().AsCharPtr());
        defines.push_back(define.AsCharPtr());
    }

    // устанавливать флаги
    flags.push_back("/NOSUB");      // деактивировать использование подпрограммы
    flags.push_back("/GBLOCK");     // поместите все переменные шейдера вне явных буферов в один глобальный блок

    // if using debug, output raw shader code
    if (!this->debug)
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
    Util::String escapedDst = dst.LocalPath();
    std::vector<std::pair<unsigned, std::string>> resourceTableNames = {
        std::make_pair(NEBULA_TICK_GROUP, "Tick")
        , std::make_pair(NEBULA_FRAME_GROUP, "Frame")
        , std::make_pair(NEBULA_PASS_GROUP, "Pass")
        , std::make_pair(NEBULA_BATCH_GROUP, "Batch")
        , std::make_pair(NEBULA_INSTANCE_GROUP, "Instance")
        , std::make_pair(NEBULA_SYSTEM_GROUP, "System")
        , std::make_pair(NEBULA_DYNAMIC_OFFSET_GROUP, "DynamicOffset")
    };

    bool res = AnyFXCompile(
        escapedSrc.AsCharPtr()
        , escapedDst.AsCharPtr()
        , target.AsCharPtr()
        , nullptr
        , "Khronos"
        , defines
        , flags
        , resourceTableNames
        , &errors
    );

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
SingleShaderCompiler::CompileSPIRV(const Util::String& srcf)
{
    const Ptr<IoServer>& ioServer = IoServer::Instance();

#ifndef __ANYFX__
    n_printf("Ошибка: невозможно скомпилировать шейдеры DX11 без поддержки DX11\n");
    return false;
#endif

#if __ANYFX__
    // запустить компиляцию AnyFX
    AnyFXBeginCompile();

    
    Util::String file = srcf.ExtractFileName();
    Util::String folder = srcf.ExtractDirName();
    file.StripFileExtension();

    // формат назначения
    String destFile = this->dstBinary + "/" + file + ".fxb";
    String destHeader = this->dstHeader + "/" + file + ".h";

    URI src(srcf);
    URI dst(destFile);
    URI dstH(destHeader);

    // compile
    n_printf("[shaderc] \n Компиляция:\n %s -> %s", src.LocalPath().AsCharPtr(), dst.LocalPath().AsCharPtr());
    n_printf("          \n Генерация:\n %s -> %s\n", src.LocalPath().AsCharPtr(), dstH.LocalPath().AsCharPtr());

    
    std::vector<std::string> defines;
    std::vector<std::string> flags;
    Util::String define;
    define.Format("-Д ГЛСЛ");
    defines.push_back(define.AsCharPtr());

    // сначала подключите эту папку
    define.Format("-Я%s/", URI(folder).LocalPath().AsCharPtr());
    defines.push_back(define.AsCharPtr());

    for (auto inc = this->includeDirs.Begin(); inc != this->includeDirs.End(); inc++)
    {
        define.Format("-Я%s/", URI(*inc).LocalPath().AsCharPtr());
        defines.push_back(define.AsCharPtr());
    }

    // устанавливать флаги
    flags.push_back("/NOSUB");          // деактивировать использование подпрограмм, эффективно расширяет все подпрограммы как функции
    flags.push_back("/GBLOCK");         // поместите все переменные шейдера вне явного блока в один глобальный блок
    flags.push_back(Util::String::Sprintf("/DEFAULTSET %d", NEBULA_BATCH_GROUP).AsCharPtr());   // since we want the most frequently switched set as high as possible, we send the default set to 8, must match the NEBULAT_DEFAULT_GROUP in std.fxh and DEFAULT_GROUP in coregraphics/config.h

    // if using debug, output raw shader code
    if (!this->debug)
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
    Util::String escapedHeader = dstH.LocalPath();
    std::vector<std::pair<unsigned, std::string>> resourceTableNames = {
        std::make_pair(NEBULA_TICK_GROUP, "Tick")
        , std::make_pair(NEBULA_FRAME_GROUP, "Frame")
        , std::make_pair(NEBULA_PASS_GROUP, "Pass")
        , std::make_pair(NEBULA_BATCH_GROUP, "Batch")
        , std::make_pair(NEBULA_INSTANCE_GROUP, "Instance")
        , std::make_pair(NEBULA_SYSTEM_GROUP, "System")
        , std::make_pair(NEBULA_DYNAMIC_OFFSET_GROUP, "DynamicOffset")
    };

    bool res = AnyFXCompile(
        escapedSrc.AsCharPtr()
        , escapedDst.AsCharPtr()
        , escapedHeader.AsCharPtr()
        , target.AsCharPtr()
        , "Khronos"
        , defines
        , flags
        , resourceTableNames
        , &errors
    );
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
SingleShaderCompiler::CreateDependencies(const Util::String& srcf)
{
const Ptr<IoServer>& ioServer = IoServer::Instance();

#ifndef __ANYFX__
    n_printf("Ошибка: невозможно скомпилировать шейдеры DX11 без поддержки DX11\n");
    return false;
#endif

    // запустить компиляцию AnyFX
    //AnyFXBeginCompile();

    
    Util::String file = srcf.ExtractFileName();
    Util::String folder = srcf.ExtractDirName();
    file.StripFileExtension();
    // формат назначения
    String destDir = this->dstBinary;
    String destFile = destDir + file + ".dep";

    URI src(srcf);
    URI dst(destFile);


    // compile
    n_printf("[shaderc] Анализ:\n %s\n", src.LocalPath().AsCharPtr());
    
    std::vector<std::string> defines;
    
    Util::String define;
    define.Format("-Д ГЛСЛ");
    defines.push_back(define.AsCharPtr());

    // сначала подключите эту папку
    define.Format("-Я%s/", URI(folder).LocalPath().AsCharPtr());
    defines.push_back(define.AsCharPtr());

    for (auto inc = this->includeDirs.Begin(); inc != this->includeDirs.End(); inc++)
    {
        define.Format("-Я%s/", URI(*inc).LocalPath().AsCharPtr());
        defines.push_back(define.AsCharPtr());
    }
    
    Util::String escapedSrc = src.LocalPath();
    
    ioServer->CreateDirectory(destDir);
    Ptr<IO::Stream> output = ioServer->CreateStream(destFile);
    Ptr<IO::TextWriter> writer = IO::TextWriter::Create();
    writer->SetStream(output);
    if(writer->Open())
    {
        std::vector<std::string> deps = AnyFXGenerateDependencies(escapedSrc.AsCharPtr(), defines);
        for(auto str : deps)
        {                        
            writer->WriteString(str.c_str());
            writer->WriteChar(';');
        }
        writer->Close();
    }
    
    return true;
}


} // пространство имен ToolkitUtil