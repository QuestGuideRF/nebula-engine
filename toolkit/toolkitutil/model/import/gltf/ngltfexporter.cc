//------------------------------------------------------------------------------
//  ngltfexporter.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "ngltfexporter.h"
#include "io/ioserver.h"
#include "model/meshutil/meshbuildersaver.h"
#include "model/modelutil/modeldatabase.h"
#include "model/modelutil/modelattributes.h"
#include "model/animutil/animbuildersaver.h"
#include "model/skeletonutil/skeletonbuilder.h"
#include "model/skeletonutil/skeletonbuildersaver.h"
#include "util/bitfield.h"
#include "timing/timer.h"
#include "io/binarywriter.h"
#include "io/filestream.h"
#include "surface/surfacebuilder.h"
#include "ngltfmaterialexporter.h"
#include "jobs2/jobs2.h"

#define NEBULA_VALIDATE_GLTF 1

using namespace Util;
namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::NglTFExporter, 'glte', Base::ExporterBase);

//------------------------------------------------------------------------------
/**
*/
NglTFExporter::NglTFExporter() 
    : texConverter(nullptr)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
NglTFExporter::~NglTFExporter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool 
NglTFExporter::ParseScene()
{
    this->gltfScene = Gltf::Document();
    bool res = this->gltfScene.Deserialize(this->path.LocalPath().AsCharPtr());
    
    if (!res)
    {
        this->logger->Warning("Не удалось импортировать '%s'\n\n", this->file.AsCharPtr());
        return false;
    }

    auto scene = new NglTFScene();
    scene->SetName(this->file);
    scene->SetCategory(this->category);
    scene->Setup(&this->gltfScene, this->exportFlags, this->sceneScale);
    this->texConverter->SetLogger(this->logger);
    this->scene = scene;

    String fileExtension = this->path.LocalPath().GetFileExtension();
    {
        // Извлечение материалов в файлы .sur
        // Всегда делайте это перед экспортом текстур, так как имена текстур могут быть изменены в экстракторе.
        NglTFMaterialExtractor extractor;
        extractor.SetLogger(this->logger);
        extractor.SetCategoryName(this->category);
        extractor.SetDocument(&this->gltfScene);
        extractor.SetExportSubDirectory(this->file);
        Util::Array<IO::URI> outputs = extractor.ExportAll();
        this->outputFiles.AppendArray(outputs);
    }

    const Util::String tmpDir = Util::String::Sprintf("temp:textureconverter/%s", this->category.AsCharPtr());
    Util::String intermediateDir = Util::String::Sprintf("%s/%s/%s/temp", tmpDir.AsCharPtr(), this->category.AsCharPtr(), this->file.AsCharPtr());

    bool hasEmbedded = false;
    for (IndexT i = 0; i < gltfScene.materials.Size(); i++)
    {
        Gltf::Material const& material = gltfScene.materials[i];
        if (material.normalTexture.index != -1)
        {
            // Установить атрибуты текстур для обычных текстур
            TextureAttrs attrs;
            attrs.SetPixelFormat(TextureAttrs::BC5);
            attrs.SetFlipNormalY(false);
            attrs.SetMaxHeight(8192);
            attrs.SetMaxWidth(8192);
            
            int32_t imageIndex = gltfScene.textures[material.normalTexture.index].source;
            Gltf::Image const& image = gltfScene.images[imageIndex];
            if (image.embedded)
            {
                hasEmbedded = true;
                Util::String format = (image.type == Gltf::Image::Type::Jpg) ? "jpg" : "png";
                Util::String intermediateFile = Util::String::Sprintf("%s/%d.%s", intermediateDir.AsCharPtr(), imageIndex, format.AsCharPtr());
                this->texConverter->AddAttributeEntry(intermediateFile, attrs);
            }
            else
            {
                Util::String fileName = image.uri;
                fileName.StripFileExtension();
                Util::String fullFileName = Util::String::Sprintf("%s/%s", this->category.AsCharPtr(), fileName.AsCharPtr());
                this->texConverter->AddAttributeEntry(fullFileName, attrs);
            }
        }
        if (material.pbrMetallicRoughness.baseColorTexture.index != -1)
        {
            // Установите атрибуты текстуры для текстур baseColorTexture
            TextureAttrs attrs;
            attrs.SetMipMapFilter(TextureAttrs::Filter::Kaiser);
            attrs.SetScaleFilter(TextureAttrs::Filter::Kaiser);
            attrs.SetMaxHeight(8192);
            attrs.SetMaxWidth(8192);
            attrs.SetColorSpace(TextureAttrs::sRGB);

            int32_t imageIndex = gltfScene.textures[material.pbrMetallicRoughness.baseColorTexture.index].source;
            Gltf::Image const& image = gltfScene.images[imageIndex];
            if (image.embedded)
            {
                hasEmbedded = true;
                Util::String format = (image.type == Gltf::Image::Type::Jpg) ? "jpg" : "png";
                Util::String intermediateFile = Util::String::Sprintf("%s/%d.%s", intermediateDir.AsCharPtr(), imageIndex, format.AsCharPtr());
                this->texConverter->AddAttributeEntry(intermediateFile, attrs);
            }
            else
            {
                Util::String fileName = image.uri;
                fileName.StripFileExtension();
                Util::String fullFileName = Util::String::Sprintf("%s/%s", this->category.AsCharPtr(), fileName.AsCharPtr());
                this->texConverter->AddAttributeEntry(fullFileName, attrs);
            }
        }
        if (material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
        {
            // Установить атрибуты текстуры для текстур MetalRoughnessTexture
            TextureAttrs attrs;
            attrs.SetScaleFilter(TextureAttrs::Kaiser);
            attrs.SetMipMapFilter(TextureAttrs::Kaiser);
            attrs.SetMaxHeight(8192);
            attrs.SetMaxWidth(8192);

            int32_t imageIndex = gltfScene.textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index].source;
            Gltf::Image const& image = gltfScene.images[imageIndex];
            if (image.embedded)
            {
                hasEmbedded = true;
                Util::String format = (image.type == Gltf::Image::Type::Jpg) ? "jpg" : "png";
                Util::String intermediateFile = Util::String::Sprintf("%s/%d.%s", intermediateDir.AsCharPtr(), imageIndex, format.AsCharPtr());
                this->texConverter->AddAttributeEntry(intermediateFile, attrs);
            }
            else
            {
                Util::String fileName = image.uri;
                fileName.StripFileExtension();
                Util::String fullFileName = Util::String::Sprintf("%s/%s", this->category.AsCharPtr(), fileName.AsCharPtr());
                this->texConverter->AddAttributeEntry(fullFileName, attrs);
            }
        }
    }

    if (gltfScene.images.Size() > 0)
    {
        // Экспортируйте встроенные текстуры в папку, специфичную для файла.
        String embeddedPath = "tex:" + this->category + "/" + this->file;
        if (IO::IoServer::Instance()->DirectoryExists(embeddedPath))
        {
            // удалить все ранее созданные изображения
            if (!IO::IoServer::Instance()->DeleteDirectory(embeddedPath))
                this->logger->Warning("glTF - Не удалось удалить старый каталог для встроенных изображений gltf\n");
        }

        if (hasEmbedded)
        {
            IO::IoServer::Instance()->CreateDirectory(embeddedPath);


            TextureAttrTable texAttrTable;
            texAttrTable.Setup(tmpDir);

            // Создать промежуточный каталог
            IO::IoServer::Instance()->CreateDirectory(intermediateDir);

            Util::Array<Gltf::Image*> images;
            for (IndexT i = 0; i < gltfScene.images.Size(); i++)
            {
                if (gltfScene.images[i].embedded)
                {
                    images.Append(&gltfScene.images[i]);
                }
            }

            auto job = [
                image = images.ConstBegin()
                , scene = &gltfScene
                , category = &this->category
                , converter = this->texConverter
                , baseDir = &this->file
                , logger = this->logger
                , intermediateDir = &intermediateDir
                ](SizeT totalJobs, SizeT groupSize, IndexT groupIndex, SizeT invocationOffset)
            {
                for (int i = 0; i < groupSize; i++)
                {
                    int index = invocationOffset + i;
                    if (index >= totalJobs)
                        continue;

                    Gltf::Image const& image = scene->images[index];

                    void const* data;
                    size_t dataSize = 0;

                    if (!image.uri.IsEmpty())
                    {
                        data = image.data.GetPtr();
                        dataSize = image.data.Size();
                    }
                    else
                    {
                        auto const& bufferView = scene->bufferViews[image.bufferView];
                        data = (const char*)scene->buffers[bufferView.buffer].data.GetPtr() + bufferView.byteOffset;
                        dataSize = bufferView.byteLength;
                    }

                    Util::String format = (image.type == Gltf::Image::Type::Jpg) ? "jpg" : "png";

                    // экспортировать содержимое blob во временный файл
                    Ptr<IO::BinaryWriter> writer = IO::BinaryWriter::Create();
                    Util::String intermediateFile = Util::String::Sprintf("%s/%d.%s", intermediateDir->AsCharPtr(), index, format.AsCharPtr());
                    writer->SetStream(IO::IoServer::Instance()->CreateStream(intermediateFile));
                    if (!writer->Open())
                    {
                        logger->Warning("    [glTF - Не удалось открыть файловый поток для записи промежуточного формата изображения]\n");
                        return;
                    }
                    writer->GetStream()->Write(data, dataSize);
                    writer->Close();

                    auto dstFile = Util::String::Sprintf("текс:%s/%s/%d", category->AsCharPtr(), baseDir->AsCharPtr(), index);

                    // контент закодирован в формате Base 64 в uri
                    if (!converter->ConvertTexture(intermediateFile, dstFile, *intermediateDir))
                    {
                        logger->Error("    [glTF - Не удалось преобразовать текстуру]\n");
                    }
                }
            };
            Threading::Event event;
            Jobs2::JobDispatch(job, gltfScene.images.Size(), 1, nullptr, nullptr, &event);

            for (IndexT i = 0; i < gltfScene.images.Size(); i++)
            {
                auto dstFile = Util::String("tex:") + this->category + "/" + this->file + "/" + Util::String::FromInt(i);
                outputFiles.Append(dstFile);
            }

            event.Wait();

            // Удалить временный каталог
            if (IO::IoServer::Instance()->DirectoryExists(tmpDir))
            {
                if (!IO::IoServer::Instance()->DeleteDirectory(tmpDir))
                    this->logger->Warning("glTF - Не удалось удалить временный каталог texconverter\n");
            }

            // Сброс рабочей памяти
            Jobs2::JobNewFrame();
        }
    }

    return res;
}

} // пространство имен ToolkitUtil
//------------------------------------------------------------------------------