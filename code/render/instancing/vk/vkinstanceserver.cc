//------------------------------------------------------------------------------
// vkinstanceserver.cc
// (C) 2016–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "vkinstanceserver.h"
#include "instancing/instancerenderer.h"
#include "coregraphics/shaderserver.h"
#include "coregraphics/renderdevice.h"
#include "models/nodes/transformnodeinstance.h"
#include "models/modelinstance.h"

using namespace Util;
using namespace CoreGraphics;
using namespace Instancing;
using namespace Models;
namespace Vulkan
{

__ImplementSingleton(Vulkan::VkInstanceServer);
__ImplementClass(Vulkan::VkInstanceServer, 'VKIS', Base::InstanceServerBase);
//------------------------------------------------------------------------------
/**
*/
VkInstanceServer::VkInstanceServer()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
VkInstanceServer::~VkInstanceServer()
{
    __DestructSingleton
}

//------------------------------------------------------------------------------
/**
*/
bool
VkInstanceServer::Open()
{
    if (InstanceServerBase::Open())
    {
        this->renderer = InstanceRenderer::Create();
        this->renderer->Setup();
        this->instancingFeatureBits = ShaderServer::Instance()->FeatureStringToMask("Instanced");
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
VkInstanceServer::Close()
{
    this->renderer->Close();
    this->renderer = 0;
    InstanceServerBase::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
VkInstanceServer::Render(IndexT frameIndex)
{
    n_assert(this->IsOpen());
    n_assert(this->modelNode.isvalid());
    n_assert(this->renderer.isvalid());

    // получить устройство рендеринга
    RenderDevice* renderDev = RenderDevice::Instance();
    ShaderServer* shaderServer = ShaderServer::Instance();

    // прервать досрочно, если у нас нет экземпляров
    if (this->instancesByCode.Size() == 0)
    {
        return;
    }

    // установить шейдер в рендерере
    this->renderer->SetShader(this->shader);

    // получить строку активного варианта
    shaderServer->SetFeatureBits(this->instancingFeatureBits);
    this->shader->SelectActiveVariation(shaderServer->GetFeatureBits());

    // начать проход шейдера
    if (this->instancesByCode.Size() > 0) this->shader->Apply();

    // просматривать каждый экземпляр кода, пакетно обновлять преобразования на основе их кода
    IndexT codeIndex;
    for (codeIndex = 0; codeIndex < this->instancesByCode.Size(); codeIndex++)
    {
        // получить массив узлов
        const Array<Ptr<ModelNodeInstance> >& nodeInstances = this->instancesByCode.ValueAtIndex(codeIndex);

        // начни с активации рендерера
        this->renderer->BeginUpdate(nodeInstances.Size());

        // просмотреть каждый экземпляр модели и добавить его в средство рендеринга
        IndexT i;
        for (i = 0; i < nodeInstances.Size(); i++)
        {
            // получить узел
            const Ptr<ModelNodeInstance>& nodeInstance = nodeInstances[i];

            // преобразование для преобразования узла
            Ptr<TransformNodeInstance> transNode = nodeInstance.downcast<TransformNodeInstance>();

            // добавить преобразование в рендерер
            this->renderer->AddTransform(transNode->GetModelTransform());
            this->renderer->AddId(transNode->GetModelInstance()->GetPickingId());
        }

        // завершить обновление преобразований
        this->renderer->EndUpdate();

        // примените состояние для первого узла, тогда оно будет активно для всех из них
        nodeInstances[0]->ApplyState(frameIndex, this->pass);

        // теперь визуализируй
        this->renderer->Render(this->multiplier);
    }
}

} // пространство имен Вулкан