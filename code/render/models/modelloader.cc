//------------------------------------------------------------------------------
// streammodelcache.cc
// (C) 2017-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "modelloader.h"
#include "model.h"
#include "core/refcounted.h"
#include "io/binaryreader.h"
#include "util/fourcc.h"
#include "nodes/modelnode.h"

using namespace Util;
using namespace IO;
namespace Models
{



__ImplementClass(Models::ModelLoader, 'SMPO', Resources::ResourceLoader);
//------------------------------------------------------------------------------
/**
*/
ModelLoader::ModelLoader()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ModelLoader::~ModelLoader()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
ModelLoader::Setup()
{
    this->placeholderResourceName = "sysmdl:placeholder.n3";
    this->failResourceName = "sysmdl:error.n3";

    IMPLEMENT_NODE_ALLOCATOR('TRFN', TransformNode);
    IMPLEMENT_NODE_ALLOCATOR('SPND', PrimitiveNode);
    IMPLEMENT_NODE_ALLOCATOR('SHSN', ShaderStateNode);
    IMPLEMENT_NODE_ALLOCATOR('CHSN', CharacterSkinNode);
    IMPLEMENT_NODE_ALLOCATOR('CHRN', CharacterNode);
    IMPLEMENT_NODE_ALLOCATOR('PSND', ParticleSystemNode);

    // никогда не забывай запустить это
    ResourceLoader::Setup();
}

//------------------------------------------------------------------------------
/**
*/
Resources::ResourceLoader::ResourceInitOutput
ModelLoader::InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream)
{
    // модель — это список ресурсов, ограничивающая рамка и словарь узлов.
    Math::bbox boundingBox;
    boundingBox.set(Math::vec3(0), Math::vec3(0));
    Util::Array<Models::ModelNode*> nodes;
    Ptr<BinaryReader> reader = BinaryReader::Create();
    Resources::ResourceLoader::ResourceInitOutput ret;

    // стек настройки для загрузки узлов
    Util::Stack<Models::ModelNode*> nodeStack;

    reader->SetStream(stream);
    if (reader->Open())
    {
        // убедитесь, что это действительно файл n3, и проверьте версию
        // Кроме того, мы предполагаем, что файл имеет собственный порядок байтов хоста (это
        // обеспечивается инструментами активов)

        auto streamData = (ModelStreamingData*)Memory::Alloc(Memory::ScratchHeap, sizeof(ModelStreamingData));
        memset(streamData, 0x0, sizeof(ModelStreamingData));
        streamData->requiredBits = LoadBits::NoBits;
        streamData->loadedBits = LoadBits::NoBits;

        ret.loaderStreamData = _StreamData{ .stream = stream, .data = streamData };

        FourCC magic = reader->ReadUInt();
        uint version = reader->ReadUInt();
        if (magic != FourCC('NEB3'))
        {
            n_error("StreamModelLoader: «%s» не является двоичным файлом N3!", stream->GetURI().AsString().AsCharPtr());
            return ret;
        }
        if (version != 1)
        {
            n_error("StreamModelLoader: «%s» имеет неверную версию!", stream->GetURI().AsString().AsCharPtr());
            return ret;
        }

        // начни читать теги
        bool done = false;
        while ((!stream->Eof()) && (!done))
        {
            FourCC fourCC = reader->ReadUInt();
            if (fourCC == FourCC('>лей'))
            {
                // начало модели
                UNUSED(FourCC) classFourCC = reader->ReadUInt();
                UNUSED(String) name = reader->ReadString();
            }
            else if (fourCC == FourCC('<лей'))
            {
                // конец модели, если мы перезагружаем, нам не следует снова загружать все ресурсы...
                done = true;

                // обновить глобальную ограничивающую рамку модели
                Math::bbox box;
                box.begin_extend();
                IndexT i;
                for (i = 0; i < nodes.Size(); i++)
                {
                    const ModelNode* node = nodes[i];
                    box.extend(node->boundingBox);
                }
                box.end_extend();
                boundingBox = box;
            }
            else if (fourCC == FourCC('>БДН'))
            {
                // начало ModelNode
                FourCC classFourCC = reader->ReadUInt();
                String name = reader->ReadString();
                ModelNode* node = this->nodeConstructors[classFourCC]();
                node->parent = nullptr;
                node->boundingBox = Math::bbox();
                node->name = name;
                node->tag = job.tag;
                if (!nodeStack.IsEmpty())
                {
                    Models::ModelNode* parent = nodeStack.Peek();
                    parent->children.Append(node);
                    node->parent = parent;
                }
                nodeStack.Push(node);
                nodes.Append(node);
            }
            else if (fourCC == FourCC('<БДН'))
            {
                // конец текущего ModelNode
                n_assert(!nodeStack.IsEmpty());
                Models::ModelNode* node = nodeStack.Pop();
                node->OnFinishedLoading(streamData);
            }
            else
            {
                // if not opening or closing a node, assume it's a data tag
                ModelNode* node = nodeStack.Peek();
                if (!node->Load(fourCC, job.tag, reader, job.immediate))
                {
                    break;
                }
            }
        }
        n_assert(nodeStack.IsEmpty());
        reader->Close();
    }

    ModelCreateInfo createInfo;
    createInfo.boundingBox = boundingBox;
    createInfo.nodes = nodes;
    ModelId id = CreateModel(createInfo);
    ret.id = id;
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
void
ModelLoader::Unload(const Resources::ResourceId id)
{
    ModelId model = id.resource;
    const Util::Array<Models::ModelNode*>& nodes = ModelGetNodes(model);

    // выгрузить узлы
    for (IndexT i = 0; i < nodes.Size(); i++)
    {
        nodes[i]->Unload();
    }

    DestroyModel(model);
}

//------------------------------------------------------------------------------
/**
*/
Resources::ResourceLoader::ResourceStreamOutput
ModelLoader::StreamResource(const ResourceLoadJob& job)
{
    ResourceLoader::ResourceStreamOutput ret;

    // Запрошенные биты уже выданы для загрузки, поэтому просто верните то, что было загружено.
    ModelStreamingData* modelStreamingData = static_cast<ModelStreamingData*>(job.streamData.data);
    ret.loadedBits = job.loadState.loadedBits;
    ret.loadedBits |= modelStreamingData->loadedBits;
    ret.pendingBits = 0x0;
    
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
uint
ModelLoader::LodMask(const _StreamData& stream, float lod, bool async) const
{
    ModelStreamingData* modelStreamingData = static_cast<ModelStreamingData*>(stream.data);
    return (uint)modelStreamingData->requiredBits;
}

} // Модели пространства имен
