//------------------------------------------------------------------------------
//  meshpagehandler.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/debug/meshpagehandler.h"
#include "coregraphics/mesh.h"
#include "http/html/htmlpagewriter.h"
#include "resources/resourceserver.h"
#include "coregraphics/meshloader.h"
#include "coregraphics/indextype.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"

namespace Debug
{
__ImplementClass(Debug::MeshPageHandler, 'DMSH', Http::HttpRequestHandler);

using namespace IO;
using namespace CoreGraphics;
using namespace Util;
using namespace Http;
using namespace Resources;

//------------------------------------------------------------------------------
/**
*/
MeshPageHandler::MeshPageHandler()
{
    this->SetName("Meshes");
    this->SetDesc("показать отладочную информацию об общих ресурсах сетки");
    this->SetRootLocation("mesh");
}

//------------------------------------------------------------------------------
/**
*/
void
MeshPageHandler::HandleRequest(const Ptr<HttpRequest>& request)
{
    n_assert(HttpMethod::Get == request->GetMethod());

    // сначала проверьте, определена ли команда в URI
    Dictionary<String,String> query = request->GetURI().ParseQuery();
    if (query.Contains("meshinfo"))
    {
        request->SetStatus(this->HandleMeshInfoRequest((query["meshinfo"]), request->GetResponseContentStream()));
        return;
    }
    else if (query.Contains("vertexdump"))
    {
        Util::String resId = query["vertexdump"];
        IndexT minIndex = query["min"].AsInt();
        IndexT maxIndex = query["max"].AsInt();
        request->SetStatus(this->HandleVertexDumpRequest(resId, minIndex, maxIndex, request->GetResponseContentStream()));
        return;
    }

    // нет команды, отправить домашнюю страницу
    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(request->GetResponseContentStream());
    htmlWriter->SetTitle("Сетки туманностей");
    if (htmlWriter->Open())
    {
        htmlWriter->Element(HtmlElement::Heading1, "Общие ресурсы сетки");
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");
        htmlWriter->LineBreak();
        htmlWriter->LineBreak();

        // получить все загруженные потоком ресурсы сетки
        const MeshLoader* meshLoader = ResourceServer::Instance()->GetStreamLoader<MeshLoader>();
        const Util::Dictionary<Resources::ResourceName, Ids::Id32>& meshes = meshLoader->GetResources();
    
        // создать таблицу всех существующих сеток
        htmlWriter->AddAttr("border", "1");
        htmlWriter->AddAttr("rules", "cols");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->AddAttr("bgcolor", "lightsteelblue");
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableHeader, "ResId");
                htmlWriter->Element(HtmlElement::TableHeader, "UseCount");
                htmlWriter->Element(HtmlElement::TableHeader, "Dump");
            htmlWriter->End(HtmlElement::TableRow);
            
            // перебирать общие ресурсы
            IndexT i;
            for (i = 0; i < meshes.Size(); i++)
            {
                const Resources::ResourceName& name = meshes.KeyAtIndex(i);
                const Resources::ResourceId& id = meshLoader->GetId(name);
                const SizeT usage = meshLoader->GetUsage(id.resourceId);
                htmlWriter->Begin(HtmlElement::TableRow);
                    htmlWriter->Begin(HtmlElement::TableData);
                        htmlWriter->AddAttr("href", "/mesh?meshinfo=" + Util::String::FromLongLong(id.HashCode64()));
                        htmlWriter->Element(HtmlElement::Anchor, name.Value());
                    htmlWriter->End(HtmlElement::TableData);
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(usage));
                    htmlWriter->Begin(HtmlElement::TableData);
                        String str;
                        str.Format("/mesh?vertexdump=%s&min=0&max=100", name.Value());
                        htmlWriter->AddAttr("href", str);
                        htmlWriter->Element(HtmlElement::Anchor, "dump");
                    htmlWriter->End(HtmlElement::TableData);
                htmlWriter->End(HtmlElement::TableRow);
            }
        htmlWriter->End(HtmlElement::Table);
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
MeshPageHandler::HandleMeshInfoRequest(const Util::String& resId, const Ptr<Stream>& responseContentStream)
{
    // найдите сетку в ResourceServer
    const Ptr<ResourceServer>& resManager = ResourceServer::Instance();
    Resources::ResourceId id = resId.AsLongLong();

    if (!resManager->HasResource(id))
    {
        return HttpStatus::NotFound;
    }

    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(responseContentStream);
    htmlWriter->SetTitle("Информация о сетке туманности");
    if (htmlWriter->Open())
    {
        const Resources::ResourceName& name = resManager->GetName(id);
        const SizeT usage = resManager->GetUsage(id);
        htmlWriter->Element(HtmlElement::Heading1, name.Value());
        htmlWriter->AddAttr("href", "/index.html");
        htmlWriter->Element(HtmlElement::Anchor, "Home");
        htmlWriter->LineBreak();
        htmlWriter->AddAttr("href", "/mesh");
        htmlWriter->Element(HtmlElement::Anchor, "Сетки Главная");
        htmlWriter->LineBreak();
        htmlWriter->LineBreak();

        // отобразить некоторую информацию о сетке
        htmlWriter->Element(HtmlElement::Heading3, "Информация о ресурсе");
        htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Имя ресурса: ");
                htmlWriter->Element(HtmlElement::TableData, name.Value());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Решенный путь: ");
                htmlWriter->Element(HtmlElement::TableData, AssignRegistry::Instance()->ResolveAssigns(name.Value()).AsString());
            htmlWriter->End(HtmlElement::TableRow);
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, "Используйте количество: ");
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(usage));
            htmlWriter->End(HtmlElement::TableRow);
        htmlWriter->End(HtmlElement::Table);

        htmlWriter->Element(HtmlElement::Heading3, "Meshes");
        SizeT numMeshes = MeshResourceGetNumMeshes(id);
        for (IndexT meshIndex = 0; meshIndex < numMeshes; meshIndex++)
        {
            MeshId mesh = MeshResourceGetMesh(id, meshIndex);
            // записать информацию о буфере вершин
            htmlWriter->Element(HtmlElement::Heading3, "Vertices");
            BufferId vbo = MeshGetVertexBuffer(mesh, 0);
            if (vbo != InvalidBufferId)
            {
                VertexLayoutId vlo = MeshGetVertexLayout(mesh);
                htmlWriter->Begin(HtmlElement::Table);
                    htmlWriter->Begin(HtmlElement::TableRow);
                        htmlWriter->Element(HtmlElement::TableData, "Являются ли вершины: ");
                        htmlWriter->Element(HtmlElement::TableData, String::FromInt(BufferGetSize(vbo)));
                    htmlWriter->End(HtmlElement::TableRow);
                    htmlWriter->Begin(HtmlElement::TableRow);
                        htmlWriter->Element(HtmlElement::TableData, "Вертексный шаг: ");
                        htmlWriter->Element(HtmlElement::TableData, String::FromInt(VertexLayoutGetSize(vlo)) + " bytes");
                    htmlWriter->End(HtmlElement::TableRow);
                htmlWriter->End(HtmlElement::Table);
                htmlWriter->Element(HtmlElement::Heading3, "Вершинные компоненты");
                htmlWriter->AddAttr("border", "1");
                htmlWriter->AddAttr("rules", "cols");
                htmlWriter->Begin(HtmlElement::Table);
                    htmlWriter->AddAttr("bgcolor", "lightsteelblue");
                        htmlWriter->Begin(HtmlElement::TableRow);
                            htmlWriter->Element(HtmlElement::TableHeader, "Semantic");
                            htmlWriter->Element(HtmlElement::TableHeader, "SemIdx");
                            htmlWriter->Element(HtmlElement::TableHeader, "Format");
                            htmlWriter->Element(HtmlElement::TableHeader, "Offset");
                            htmlWriter->Element(HtmlElement::TableHeader, "Size");
                        htmlWriter->End(HtmlElement::TableRow);

                    IndexT i;
                    const Util::Array<VertexComponent>& comps = VertexLayoutGetComponents(vlo);
                    for (i = 0; i < comps.Size(); i++)
                    {
                        const VertexComponent& comp = comps[i];
                        htmlWriter->Begin(HtmlElement::TableRow);
                            htmlWriter->Element(HtmlElement::TableData, String::FromInt(comp.GetIndex()));
                            htmlWriter->Element(HtmlElement::TableData, VertexComponent::FormatToString(comp.GetFormat()));
                            htmlWriter->Element(HtmlElement::TableData, String::FromInt(comp.GetByteOffset()));
                            htmlWriter->Element(HtmlElement::TableData, String::FromInt(comp.GetByteSize()));
                        htmlWriter->End(HtmlElement::TableRow);
                    }
                htmlWriter->End(HtmlElement::Table);
            }
            else
            {
                htmlWriter->Text("НЕТ ВЕРШИН");
                htmlWriter->LineBreak();
            }

            // записать информацию об индексном буфере
            htmlWriter->Element(HtmlElement::Heading3, "Indices");
            BufferId ibo = MeshGetIndexBuffer(mesh);
            if (ibo != InvalidBufferId)
            {
                htmlWriter->Begin(HtmlElement::Table);
                    htmlWriter->Begin(HtmlElement::TableRow);
                        htmlWriter->Element(HtmlElement::TableData, "Списки дел: ");
                        htmlWriter->Element(HtmlElement::TableData, String::FromInt(BufferGetSize(ibo)));
                    htmlWriter->End(HtmlElement::TableRow);
                    htmlWriter->Begin(HtmlElement::TableRow);
                        htmlWriter->Element(HtmlElement::TableData, "Тип индекса: ");
                        htmlWriter->Element(HtmlElement::TableData, IndexType::ToString(CoreGraphics::IndexType::Index32));
                    htmlWriter->End(HtmlElement::TableRow);
                htmlWriter->End(HtmlElement::Table);
            }
            else
            {
                htmlWriter->Text("БЕЗ ИНДЕКСОВ");
                htmlWriter->LineBreak();
            }

            // записать информацию о примитивной группе
            htmlWriter->Element(HtmlElement::Heading3, "Примитивные группы");
            htmlWriter->AddAttr("border", "1");
            htmlWriter->AddAttr("rules", "cols");
            htmlWriter->Begin(HtmlElement::Table);
                htmlWriter->AddAttr("bgcolor", "lightsteelblue");
            htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableHeader, "Число примитивов");
                htmlWriter->Element(HtmlElement::TableHeader, "Базовая вершина");
                htmlWriter->Element(HtmlElement::TableHeader, "Вершины?");
                htmlWriter->Element(HtmlElement::TableHeader, "Базовый индекс");
                htmlWriter->Element(HtmlElement::TableHeader, "Вы перечисляете?");
                htmlWriter->Element(HtmlElement::TableHeader, "Topology");
            htmlWriter->End(HtmlElement::TableRow);

            const Util::Array<CoreGraphics::PrimitiveGroup>& primGroups = MeshGetPrimitiveGroups(mesh);
            IndexT i;
            for (i = 0; i < primGroups.Size(); i++)
            {
                const PrimitiveGroup& primGroup = primGroups[i];
                const PrimitiveTopology::Code& topo = MeshGetTopology(mesh);
                htmlWriter->Begin(HtmlElement::TableRow);
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(primGroup.GetNumPrimitives(topo)));
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(primGroup.GetBaseVertex()));
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(primGroup.GetNumVertices()));
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(primGroup.GetBaseIndex()));
                    htmlWriter->Element(HtmlElement::TableData, String::FromInt(primGroup.GetNumIndices()));
                    htmlWriter->Element(HtmlElement::TableData, PrimitiveTopology::ToString(topo));
                htmlWriter->End(HtmlElement::TableRow);
            }
            htmlWriter->End(HtmlElement::Table);
        }
        htmlWriter->Close();
        return HttpStatus::OK;
    }
    return HttpStatus::InternalServerError;
}

//------------------------------------------------------------------------------
/**
    Обработка запроса дампа сетки.
*/
HttpStatus::Code
MeshPageHandler::HandleVertexDumpRequest(const Util::String& resId, IndexT minVertexIndex, IndexT maxVertexIndex, const Ptr<Stream>& responseContentStream)
{
    // найдите сетку в ResourceServer
    const Ptr<ResourceServer>& resManager = ResourceServer::Instance();
    ResourceId id = resId.AsLongLong();
    if (!resManager->HasResource(id))
    {
        return HttpStatus::NotFound;
    }

    Ptr<HtmlPageWriter> htmlWriter = HtmlPageWriter::Create();
    htmlWriter->SetStream(responseContentStream);
    htmlWriter->SetTitle("Свалка сетки туманности");
    if (htmlWriter->Open())
    {
        SizeT numMeshes = MeshResourceGetNumMeshes(id);
        for (IndexT meshIndex = 0; meshIndex < numMeshes; meshIndex++)
        {
            MeshId mesh = MeshResourceGetMesh(id, meshIndex);
            const BufferId vb = MeshGetVertexBuffer(mesh, 0);
            //FIXME не работает с различными макетами вершин примитивных групп.
            const VertexLayoutId vl = MeshGetVertexLayout(mesh);

            // обрезать до допустимого диапазона
            SizeT numverts = BufferGetSize(vb);
            if (minVertexIndex > numverts)
            {
                minVertexIndex = numverts;
            }
            if (maxVertexIndex > numverts)
            {
                maxVertexIndex = numverts;
            }

            // написать заголовок
            htmlWriter->Element(HtmlElement::Heading1, resId.AsCharPtr());
            htmlWriter->AddAttr("href", "/index.html");
            htmlWriter->Element(HtmlElement::Anchor, "Home");
            htmlWriter->LineBreak();
            htmlWriter->AddAttr("href", "/mesh");
            htmlWriter->Element(HtmlElement::Anchor, "Сетки Главная");
            htmlWriter->LineBreak();
            htmlWriter->LineBreak();

            // сначала постройте простую таблицу диапазонов, поскольку мы не можем отобразить все вершины одновременно!   
            htmlWriter->Element(HtmlElement::Heading3, "Выберите диапазон");
            htmlWriter->AddAttr("border", "1");
            htmlWriter->AddAttr("rules", "cols");
            htmlWriter->Begin(HtmlElement::Table);
            IndexT rangeIndex;
            const SizeT rangeSize = 100;
            for (rangeIndex = 0; rangeIndex < numverts; rangeIndex += rangeSize)
            {
                IndexT maxRangeIndex = rangeIndex + rangeSize;
                if (maxRangeIndex > numverts)
                {
                    maxRangeIndex = numverts;
                }

                String link;
                link.Format("/mesh?vertexdump=%s&min=%d&max=%d", resId.AsCharPtr(), rangeIndex, maxRangeIndex);
                String str;
                str.Format("%d .. %d", rangeIndex, maxRangeIndex);
                htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Begin(HtmlElement::TableData);
                htmlWriter->AddAttr("href", link);
                htmlWriter->Element(HtmlElement::Anchor, str);
                htmlWriter->End(HtmlElement::TableData);
                htmlWriter->End(HtmlElement::TableRow);
            }
            htmlWriter->End(HtmlElement::Table);

            // написать фактическую таблицу дампа вершин        
            htmlWriter->Element(HtmlElement::Heading3, "Вершинный дамп");
            htmlWriter->AddAttr("border", "1");
            htmlWriter->AddAttr("rules", "cols");
            htmlWriter->Begin(HtmlElement::Table);
            htmlWriter->AddAttr("bgcolor", "lightsteelblue");
            htmlWriter->Begin(HtmlElement::TableRow);
            htmlWriter->Element(HtmlElement::TableHeader, "Index");
            const Util::Array<VertexComponent>& comps = VertexLayoutGetComponents(vl);
            IndexT compIndex;
            for (compIndex = 0; compIndex < comps.Size(); compIndex++)
            {
                const VertexComponent& c = comps[compIndex];
                String n;
                n.Format("%d (%s)",
                    c.GetIndex(),
                    VertexComponent::FormatToString(c.GetFormat()).AsCharPtr());
                htmlWriter->Element(HtmlElement::TableHeader, n);
            }
            htmlWriter->End(HtmlElement::TableRow);

            // for each vertex...
            SizeT vertexStride = VertexLayoutGetSize(vl);
            ubyte* ptr = (ubyte*)BufferMap(vb);
            IndexT vertexIndex;
            for (vertexIndex = minVertexIndex; vertexIndex < maxVertexIndex; vertexIndex++)
            {
                // начать новую строку и записать индекс вершины
                htmlWriter->Begin(HtmlElement::TableRow);
                htmlWriter->Element(HtmlElement::TableData, String::FromInt(vertexIndex));

                // for each vertex component...
                String str;
                ubyte* vertexPtr = ptr + (vertexIndex * vertexStride);
                for (compIndex = 0; compIndex < comps.Size(); compIndex++)
                {
                    const VertexComponent& c = comps[compIndex];
                    ubyte* cPtr = vertexPtr + c.GetByteOffset();
                    float* fltPtr = (float*)cPtr;
                    ubyte* ubPtr = (ubyte*)cPtr;
                    ushort* usPtr = (ushort*)cPtr;
                    switch (c.GetFormat())
                    {
                        case VertexComponent::Float:
                            str.Format("%.2f", fltPtr[0]);
                            break;

                        case VertexComponent::Float2:
                            str.Format("%.2f/%.2f", fltPtr[0], fltPtr[1]);
                            break;

                        case VertexComponent::Float3:
                            str.Format("%.2f/%.2f/%.2f", fltPtr[0], fltPtr[1], fltPtr[2]);
                            break;

                        case VertexComponent::Float4:
                            str.Format("%.2f/%.2f/%.2f/%.2f", fltPtr[0], fltPtr[1], fltPtr[2], fltPtr[3]);
                            break;

                        case VertexComponent::UByte4:
                        case VertexComponent::UByte4N:
                            str.Format("0x%02X/0x%02X/0x%02X/0x%02X", ubPtr[0], ubPtr[1], ubPtr[2], ubPtr[3]);
                            break;

                        case VertexComponent::Short2:
                        case VertexComponent::Short2N:
                            str.Format("0x%04X/0x%04X", usPtr[0], usPtr[1]);
                            break;

                        case VertexComponent::Short4:
                        case VertexComponent::UShort4:
                        case VertexComponent::Short4N:
                            str.Format("0x%04X/0x%04X/0x%04X/0x%04X", usPtr[0], usPtr[1], usPtr[2], usPtr[3]);
                            break;

                        case VertexComponent::UShort:
                            str.Format("0x%04X", usPtr[0]);
                            break;

                        default:
                            str = "<недействительно>";
                            break;

                    } // переключить формат
                    htmlWriter->Element(HtmlElement::TableData, str);
                } // for compIndex
                htmlWriter->End(HtmlElement::TableRow);
            } // for vertexIndex
            BufferUnmap(vb);
            htmlWriter->End(HtmlElement::Table);
        }

        htmlWriter->Close();
        return HttpStatus::OK;
    } // if Open
    return HttpStatus::InternalServerError;
}

} // Отладка пространства имен
