//------------------------------------------------------------------------------
//  @file modelexporter.cc
//  @copyright (C) 2022 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "modelexporter.h"
#include "io/ioserver.h"
#include "io/filestream.h"
#include "io/uri.h"
#include "timing/timer.h"

#include "model/animutil/animbuildersaver.h"
#include "model/skeletonutil/skeletonbuildersaver.h"

#include "model/modelwriter.h"
#include "model/meshutil/meshbuildersaver.h"

#include "model/import/gltf/node/ngltfscene.h"
#include "model/import/base/uniquestring.h"

#include "model/modelutil/modeldatabase.h"

#include "model/scenewriter.h"


#include "toolkit-common/text.h"

using namespace Util;
namespace ToolkitUtil
{

//------------------------------------------------------------------------------
/**
*/
ModelExporter::ModelExporter()
    : scene(nullptr)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ModelExporter::~ModelExporter()
{
    n_assert(this->scene == nullptr);
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelExporter::ParseScene()
{
    n_error("Реализовать в подклассе");
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ModelExporter::ExportFile(const IO::URI& file)
{
    IO::IoServer* ioServer = IO::IoServer::Instance();

    // Сбросить все уникальные строки
    UniqueString::Reset();

    outputFiles.Clear();

    this->path = file;
    String localPath = file.GetHostAndLocalPath();
    if (!this->NeedsConversion(localPath))
    {
        this->logger->Print("Пропуск %s\n", Text(localPath).Color(TextColor::Blue).AsCharPtr());
        return;
    }
    this->file = localPath.ExtractFileName();
    this->file.StripFileExtension();
    this->category = localPath.ExtractLastDirName();
    this->logger->Print("%s\n", Text(localPath).Color(TextColor::Blue).AsCharPtr());
    this->logger->Indent();

    // Получить анимацию со сцены
    Ptr<ModelAttributes> attributes = ModelDatabase::Instance()->LookupAttributes(this->category + "/" + this->file);
    this->exportFlags = attributes->GetExportFlags();
    this->sceneScale = attributes->GetScale();

    Timing::Timer timer;
    Timing::Timer totalTime;
    totalTime.Start();

    timer.Start();

    // Запуск синтаксического анализа сцены, специфичной для реализации
    if (!this->ParseScene())
        return;

    this->logger->Print("%s %s (%.2f мс)\n", "Parsing...", Text("done").Color(TextColor::Green).AsCharPtr(), timer.GetTime() * 1000.0f);
    //n_printf(" [Исходный код] (%.2f мс)\n", timer.GetTime() * 1000.0f);
    timer.Stop();

    timer.Reset();
    timer.Start();

    // вычислить глобальные преобразования узлов для всех узлов
    for (auto& node : this->scene->nodes)
    {
        if (node.base.parent == nullptr)
            node.CalculateGlobalTransforms();        
    }

    // Объединение сеток на основе вершинного компонента и материала
    Util::Array<SceneNode*> mergedMeshNodes;
    Util::Array<SceneNode*> mergedCharacterNodes;
    Util::Array<MeshBuilder*> mergedMeshes;
    this->scene->OptimizeGraphics(mergedMeshNodes, mergedCharacterNodes, mergedMeshes);

    Util::String physicsMeshExportName = String::Sprintf("msh:%s/%s_ph.nvx", this->category.AsCharPtr(), this->file.AsCharPtr());
    IO::URI destinationFiles[] =
    {
        IO::URI(String::Sprintf("msh:%s/%s.nvx", this->category.AsCharPtr(), this->file.AsCharPtr())) // mesh
        , IO::URI(physicsMeshExportName) // physics
        , IO::URI(String::Sprintf("ани:%s/%s.nax", this->category.AsCharPtr(), this->file.AsCharPtr())) // animation
        , IO::URI(String::Sprintf("ske:%s/%s.nsk", this->category.AsCharPtr(), this->file.AsCharPtr())) // skeleton
    };

    enum DestinationFile
    {
        Mesh
        , Physics
        , Animation
        , Skeleton
    };

    // сохранить сетку в файл
    if (!MeshBuilderSaver::Save(destinationFiles[DestinationFile::Mesh], mergedMeshes, this->platform))
    {
        this->logger->Error("Не удалось сохранить файл NVX: % s\n", destinationFiles[DestinationFile::Mesh].LocalPath().AsCharPtr());
    }
    else
    {
        outputFiles.Append(destinationFiles[DestinationFile::Mesh]);
    }
    

    // Удалить объединенные сетки после экспорта
    for (auto mesh : mergedMeshes)
        delete mesh;

    timer.Stop();

    // распечатать информацию
    this->logger->Print("%s %s\n", Format("Созданная сетка: %s", Text(destinationFiles[DestinationFile::Mesh].LocalPath()).Color(TextColor::Green).Style(FontMode::Underline).AsCharPtr()).AsCharPtr(), Format("(%.2f мс)", timer.GetTime() * 1000.0f).AsCharPtr());

    // получить физическую сетку
    Util::Array<SceneNode*> physicsNodes;
    MeshBuilder* physicsMesh = nullptr;
    MeshBuilderGroup group;
    scene->OptimizePhysics(physicsNodes, physicsMesh);

    // сохранять физическую сетку, только если она существует
    if (physicsMesh && physicsMesh->GetNumTriangles() > 0)
    {
        timer.Reset();
        timer.Start();

        // сохранить сетку
        if (!MeshBuilderSaver::Save(destinationFiles[DestinationFile::Physics], { physicsMesh }, this->platform))
        {
            this->logger->Error("Не удалось сохранить файл физики NVX: % s\n", destinationFiles[DestinationFile::Physics].LocalPath().AsCharPtr());
        }
        else
        {
            outputFiles.Append(destinationFiles[DestinationFile::Physics]);
        }

        timer.Stop();

        // распечатать информацию
        this->logger->Print("%s %s\n", Format("Сгенерированная физика: %s", Text(destinationFiles[DestinationFile::Physics].LocalPath()).Color(TextColor::Green).Style(FontMode::Underline).AsCharPtr()).AsCharPtr(), Format("(%.2f мс)", timer.GetTime() * 1000.0f).AsCharPtr());

    }

    if (this->scene->animations.Size() > 0)
    {
        timer.Reset();
        timer.Start();

        // Анимация очистки
        for (auto& anim : this->scene->animations)
        {
            anim.BuildVelocityCurves(AnimationFrameRate);
        }

        // теперь сохраните настоящую анимацию
        if (!AnimBuilderSaver::Save(destinationFiles[DestinationFile::Animation], this->scene->animations, this->platform))
        {
            this->logger->Error("Не удалось сохранить файл анимации: %s\n", destinationFiles[DestinationFile::Animation].LocalPath().AsCharPtr());
        }
        else
        {
            outputFiles.Append(destinationFiles[DestinationFile::Animation]);
        }

        timer.Stop();
        this->logger->Print("%s %s\n", Format("Созданная анимация: %s", Text(destinationFiles[DestinationFile::Animation].LocalPath()).Color(TextColor::Green).Style(FontMode::Underline).AsCharPtr()).AsCharPtr(), Format("(%.2f мс)", timer.GetTime() * 1000.0f).AsCharPtr());
    }

    if (this->scene->skeletons.Size() > 0)
    {
        timer.Reset();
        timer.Start();

        if (!SkeletonBuilderSaver::Save(destinationFiles[DestinationFile::Skeleton], this->scene->skeletons, this->platform))
        {
            this->logger->Error("Не удалось сохранить файл скелета: %s\n", destinationFiles[DestinationFile::Skeleton].LocalPath().AsCharPtr());
        }
        else
        {
            outputFiles.Append(destinationFiles[DestinationFile::Skeleton]);
        }

        timer.Stop(); 
        this->logger->Print("%s %s\n", Format("Сгенерированный скелет: %s", Text(destinationFiles[DestinationFile::Skeleton].LocalPath()).Color(TextColor::Green).Style(FontMode::Underline).AsCharPtr()).AsCharPtr(), Format("(%.2f мс)", timer.GetTime() * 1000.0f).AsCharPtr());
    }

    // Наконец, выведите иерархию модели в n3.
    SceneWriter::GenerateModels(
        Util::String::Sprintf("src:assets/%s/", this->category.AsCharPtr())
        , this->scene
        , this->platform
        , mergedMeshNodes
        , physicsNodes
        , mergedCharacterNodes
        , physicsMeshExportName
        , this->exportFlags
    );

    WriteIntermediateFile(file, outputFiles);

    totalTime.Stop();
    this->logger->Unindent();
    this->logger->Print("%s %s\n\n", "Done"_text.Color(TextColor::Green).Style(FontMode::Bold).AsCharPtr(), Format("(%.2f мс)", totalTime.GetTime() * 1000).AsCharPtr());

    delete this->scene;
    this->scene = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelExporter::NeedsConversion(const Util::String& path)
{
    Util::String category = path.ExtractLastDirName();
    Util::String file = path.ExtractFileName();
    file.StripFileExtension();

    // проверьте оба, если FBX новее, чем .n3
    Util::String model = "mdl:" + category + "/" + file + ".n3";
    Util::String physModel = "phys:" + category + "/" + file + ".actor";
    Util::String mesh = "msh:" + category + "/" + file + ".nvx";
    Util::String physMesh = "msh:" + category + "/" + file + "_ph.nvx";
    Util::String animation = "ani:" + category + "/" + file + ".nax";
    Util::String constants = "src:assets/" + category + "/" + file + ".constants";
    Util::String attributes = "src:assets/" + category + "/" + file + ".attributes";
    Util::String physics = "src:assets/" + category + "/" + file + ".physics";

    // проверьте, является ли fbx новее модели
    bool sourceNewer = ExporterBase::NeedsConversion(path, model);

    // и если .constants старше, чем fbx
    bool constantsNewer = ExporterBase::NeedsConversion(constants, model);

    // and if the .attributes is older than the n3 (attributes controls both model, and animation resource)
    bool attributesNewer = ExporterBase::NeedsConversion(attributes, model);
    // ...и если .физика старше, чем np3
    bool physicsNewer = ExporterBase::NeedsConversion(physics, physModel);

    // ...если сетка новее
    bool meshNewer = ExporterBase::NeedsConversion(path, mesh);

    // проверьте, были ли изменены настройки физики. нет способа узнать, есть ли у нас в нем новая физическая сетка, поэтому мы все равно просто экспортируем ее
    bool physicsMeshNewer = ExporterBase::NeedsConversion(physics, mesh);

    // return true if either is true
    return sourceNewer || constantsNewer || attributesNewer || physicsNewer || meshNewer || physicsMeshNewer;
}

} // пространство имен ToolkitUtil
