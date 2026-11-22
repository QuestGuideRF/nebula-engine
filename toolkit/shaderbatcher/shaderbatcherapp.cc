//------------------------------------------------------------------------------
//  shaderbatcherapp.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "shaderbatcherapp.h"
#include "timing/time.h"

namespace Toolkit
{
using namespace ToolkitUtil;

//------------------------------------------------------------------------------
/**
*/
bool
ShaderBatcherApp::ParseCmdLineArgs()
{
    if (ToolkitApp::ParseCmdLineArgs())
    {
        this->shaderCompiler.SetForceFlag(this->args.GetBoolFlag("-force"));
        this->shaderCompiler.SetDebugFlag(this->args.GetBoolFlag("-debug"));
        const Util::String& output = this->args.GetString("-out");
        if (!output.IsEmpty())
        {
            this->shaderCompiler.SetDstShaderDir(output + "/shaders");
            this->shaderCompiler.SetDstFrameShaderDir(output + "/frame");
            this->shaderCompiler.SetDstMaterialsDir(output + "/materials");
        }
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
ShaderBatcherApp::SetupProjectInfo()
{
    if (ToolkitApp::SetupProjectInfo())
    {
        this->shaderCompiler.SetPlatform(this->platform);
        if (this->projectInfo.HasAttr("ShaderToolParams"))
        {
            this->shaderCompiler.SetAdditionalParams(this->projectInfo.GetAttr("ShaderToolParams"));
        }

        // настроить необходимые вещи в компиляторе
        this->shaderCompiler.SetLanguage(this->projectInfo.GetAttr("ShaderLanguage"));
        this->shaderCompiler.SetSrcShaderBaseDir(this->projectInfo.GetAttr("ShaderSrcDir"));
        this->shaderCompiler.SetDstShaderDir(this->projectInfo.GetAttr("ShaderDstDir"));
        this->shaderCompiler.SetSrcFrameShaderBaseDir(this->projectInfo.GetAttr("FrameShaderSrcDir"));
        this->shaderCompiler.SetDstFrameShaderDir(this->projectInfo.GetAttr("FrameShaderDstDir"));
        this->shaderCompiler.SetSrcMaterialBaseDir(this->projectInfo.GetAttr("MaterialsSrcDir"));
        this->shaderCompiler.SetDstMaterialsDir(this->projectInfo.GetAttr("MaterialsDstDir"));

        // настроить пользовательские вещи
        if (this->projectInfo.HasAttr("ShaderSrcCustomDir")) this->shaderCompiler.SetSrcShaderCustomDir(this->projectInfo.GetAttr("ShaderSrcCustomDir"));
        if (this->projectInfo.HasAttr("FrameShaderSrcCustomDir")) this->shaderCompiler.SetSrcFrameShaderCustomDir(this->projectInfo.GetAttr("FrameShaderSrcCustomDir"));
        if (this->projectInfo.HasAttr("MaterialsSrcCustomDir")) this->shaderCompiler.SetSrcMaterialCustomDir(this->projectInfo.GetAttr("MaterialsSrcCustomDir"));
        
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderBatcherApp::ShowHelp()
{
    n_printf("Пакетный компилятор шейдеров туманности.\n"
             "(C) 2013–2020 Индивидуальные авторы, см. файл АВТОРОВ\n"
             "-help -- отобразить эту справку\n"
             "-platform -- выбор платформы (win32, linux)\n"
             "-waitforkey -- дождаться ключа после завершения\n"
             "-force -- принудительно перекомпилировать\n"
             "-debug -- компилировать с отладочной информацией\n");             
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderBatcherApp::Run()
{
    bool success = true;

    // настроить объект информации о проекте
    if (success && !this->SetupProjectInfo())
    {
        success = false;
    }

    // анализировать аргументы командной строки
    if (success && !this->ParseCmdLineArgs())
    {
        success = false;
    }

    // вызвать инструмент компилятора шейдеров
    if (success && !this->shaderCompiler.CompileShaders())
    {
        success = false;
        this->SetReturnCode(-1);
    }
    if (success && !this->shaderCompiler.CompileFrameShaders())
    {
        success = false;
        this->SetReturnCode(-1);
    }
    if (success && !this->shaderCompiler.CompileMaterials())
    {
        success = false;
        this->SetReturnCode(-1);
    }

    // дождаться ввода пользователя
    if (this->waitForKey)
    {
        n_printf("Нажмите <Enter>, чтобы продолжить!\n");
        while (!IO::Console::Instance()->HasInput())
        {
            Timing::Sleep(0.01);
        }
    }
}

} // Инструментарий пространства имен

