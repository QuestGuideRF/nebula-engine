//------------------------------------------------------------------------------
//  shaderpagehandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/debug/shaderpagehandler.h"
#include "coregraphics/shaderserver.h"
#include "coregraphics/shader.h"
#include "http/html/htmlpagewriter.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"

namespace Debug
{
__ImplementClass(Debug::ShaderPageHandler, 'SPHL', Http::HttpRequestHandler);

using namespace IO;
using namespace CoreGraphics;
using namespace Graphics;
using namespace Util;
using namespace Http;
using namespace Resources;
using namespace Models;

//------------------------------------------------------------------------------
/**
*/
ShaderPageHandler::ShaderPageHandler()
{
    this->SetName("Shaders");
    this->SetDesc("показать информацию об отладке шейдера");
    this->SetRootLocation("shader");
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderPageHandler::HandleRequest(const Ptr<HttpRequest>& request)
{
    n_assert(HttpMethod::Get == request->GetMethod());

    // сначала проверьте, определена ли команда в URI
    Dictionary<String,String> query = request->GetURI().ParseQuery();
    if (query.Contains("shaderinfo"))
    {
        request->SetStatus(this->HandleShaderInfoRequest(query["shaderinfo"], request->GetResponseContentStream()));
        return;
    }

    // нет команды, отправить домашнюю страницу
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Шейдеры туманности");
    if (htmlWriter->Open())
    {
        ShaderServer* shdServer = ShaderServer::Instance();

        htmlWriter->Element(HtmlElement::Heading1, "Ресурсы по шейдерам");
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");
        htmlWriter->LineBreak();
        htmlWriter->LineBreak();

        // создать таблицу всех существующих шейдеров
        htmlWriter->Element(HtmlElement::Heading3, "Shaders");
        const Dictionary<ResourceName, Resources::ResourceId>& shaders = shdServer->GetAllShaders();
        htmlWriter->AddAttr("border", "1");
        htmlWriter->AddAttr("rules", "cols");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->AddAttr("bgcolor", "lightsteelblue");
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableHeader, "ResId");
                htmlWriter->Element(HtmlElement::TableHeader, "NumInstances");
            htmlWriter->End(HtmlElement::TableRow);

            IndexT i;
            for (i = 0; i < shaders.Size(); i++)
            {
                const ResourceName name = shaders.KeyAtIndex(i);
                htmlWriter->Begin(HtmlElement::TableRow);
                    htmlWriter->Begin(HtmlElement::TableData);
                        htmlWriter->AddAttr("href", "/shader?shaderinfo=" + name.AsString());
                        htmlWriter->Element(HtmlElement::Anchor, name.Value());
                    htmlWriter->End(HtmlElement::TableData);
                htmlWriter->End(HtmlElement::TableRow);
            }
        htmlWriter->End(HtmlElement::Table);

        // создать таблицу глобальных переменных
        htmlWriter->Element(HtmlElement::Heading3, "Общие переменные шейдера");

        htmlWriter->Close();
        request->SetStatus(HttpStatus::OK);
    }
    else
    {
        request->SetStatus(HttpStatus::InternalServerError);
    }
}

//------------------------------------------------------------------------------
/**
*/
HttpStatus::Code
ShaderPageHandler::HandleShaderInfoRequest(const Util::String& resId, const Ptr<IO::Stream>& responseContentStream)
{
    ShaderServer* shdServer = ShaderServer::Instance();
    Resources::ResourceId id = resId.AsLongLong();
    Resources::ResourceName name = ShaderGetName(id);

    // проверьте, существует ли на самом деле шейдер
    if (!shdServer->HasShader(resId))
    {
        return HttpStatus::NotFound;
    }

    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(responseContentStream);
    htmlWriter->SetTitle("Информация о шейдере туманности");
    if (htmlWriter->Open())
    {
        // нам нужно создать экземпляр временного шейдера, чтобы получить информацию об отражении
        const ShaderId& shd = id;
        SizeT numVars = ShaderGetConstantCount(shd);
        htmlWriter->Element(HtmlElement::Heading1, name.Value());
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");
        htmlWriter->LineBreak();
        htmlWriter->AddAttr("href", "/shader");
        htmlWriter->Element(HtmlElement::Anchor, "Главная страница шейдеров");
        htmlWriter->LineBreak();
        htmlWriter->LineBreak();

        // отобразить некоторую информацию о шейдере
        htmlWriter->Element(HtmlElement::Heading3, "Информация о ресурсе");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Идентификатор ресурса: ");
                htmlWriter->Element(HtmlElement::TableData, name.Value());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Решенный путь: ");
                htmlWriter->Element(HtmlElement::TableData, AssignRegistry::Instance()->ResolveAssigns(name.Value()).AsString());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Количество активных состояний: ");
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);

        // отображать переменные шейдера
        htmlWriter->Element(HtmlElement::Heading3, "Переменные шейдера");
#if __NEBULA_HTTP__
        if (numVars > 0)
        {
            this->WriteShaderVariableTable(htmlWriter, shd);
        }
        else
#endif
        {
            htmlWriter->Text("Никаких шейдерных переменных.");
            htmlWriter->LineBreak();
        }

        const Util::Dictionary<CoreGraphics::ShaderFeature::Mask, CoreGraphics::ShaderProgramId>& programs = ShaderGetPrograms(id);

        // отображать варианты шейдеров
        htmlWriter->Element(HtmlElement::Heading3, "Варианты шейдеров");
        if (programs.Size() > 0)
        {
            htmlWriter->AddAttr("border", "1");
            htmlWriter->AddAttr("rules", "cols");
            htmlWriter->Begin(HtmlElement::Table);
                htmlWriter->AddAttr("bgcolor", "lightsteelblue");
                htmlWriter->Begin(HtmlElement::TableRow);    
                    htmlWriter->Element(HtmlElement::TableHeader, "Name");
                    htmlWriter->Element(HtmlElement::TableHeader, "FeatureBits");
                htmlWriter->End(HtmlElement::TableRow);
                IndexT i;
                for (i = 0; i < programs.Size(); i++)
                {
                    const CoreGraphics::ShaderProgramId& id = programs.ValueAtIndex(i);
                    const CoreGraphics::ShaderFeature::Mask& mask = programs.KeyAtIndex(i);
                    htmlWriter->Begin(HtmlElement::TableRow);
                        htmlWriter->Element(HtmlElement::TableData, ShaderProgramGetName(id).AsString());
                        htmlWriter->Element(HtmlElement::TableData, shdServer->FeatureMaskToString(mask));
                    htmlWriter->End(HtmlElement::TableRow);
                }
            htmlWriter->End(HtmlElement::Table);
        }
        else
        {
            htmlWriter->Text("Никаких вариаций шейдеров.");
            htmlWriter->LineBreak();
        }

        htmlWriter->Element(HtmlElement::Heading3, "Материалы, использующие этот шейдер:");
        htmlWriter->AddAttr("border", "1");
        htmlWriter->AddAttr("rules", "cols");
        htmlWriter->Begin(HtmlElement::Table);
        htmlWriter->AddAttr("bgcolor", "lightsteelblue");
        htmlWriter->Begin(HtmlElement::TableRow);
        htmlWriter->Element(HtmlElement::TableHeader, "Id");
        htmlWriter->Element(HtmlElement::TableHeader, "Name");
        htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "РЕАЛИЗУЙТЕ МЕНЯ!");
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);


        htmlWriter->Close();
        return HttpStatus::OK;
    }
    return HttpStatus::InternalServerError;
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderPageHandler::WriteShaderVariableTable(const Ptr<HtmlPageWriter>& htmlWriter, const CoreGraphics::ShaderId shader)
{
    htmlWriter->AddAttr("border", "1");
    htmlWriter->AddAttr("rules", "cols");
    htmlWriter->Begin(HtmlElement::Table);
        htmlWriter->AddAttr("bgcolor", "lightsteelblue");
        htmlWriter->Begin(HtmlElement::TableRow);    
            htmlWriter->Element(HtmlElement::TableHeader, "Name");
            htmlWriter->Element(HtmlElement::TableHeader, "Type");
            htmlWriter->Element(HtmlElement::TableHeader, "IsArray");
            htmlWriter->Element(HtmlElement::TableHeader, "ArraySize");
        htmlWriter->End(HtmlElement::TableRow);

    SizeT numVars = ShaderGetConstantCount(shader);
    IndexT i;
    for (i = 0; i < numVars; i++)
    {
        const Util::StringAtom& name = ShaderGetConstantName(shader, i);
        const ShaderConstantType& type = ShaderGetConstantType(shader, i);
        htmlWriter->Begin(HtmlElement::TableRow);
            htmlWriter->Element(HtmlElement::TableData, name.AsString());
            htmlWriter->Element(HtmlElement::TableData, ConstantTypeToString(type));
        htmlWriter->End(HtmlElement::TableRow);
    }
    htmlWriter->End(HtmlElement::Table);
}

} // Отладка пространства имен
