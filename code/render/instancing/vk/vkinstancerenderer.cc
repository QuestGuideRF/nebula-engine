//------------------------------------------------------------------------------
// vkinstancerenderer.cc
// (C) 2016–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "vkinstancerenderer.h"
#include "coregraphics/shaderserver.h"
#include "coregraphics/config.h"
#include "coregraphics/vk/vkrenderdevice.h"

using namespace Math;
using namespace CoreGraphics;
namespace Vulkan
{

__ImplementClass(Vulkan::VkInstanceRenderer, 'VKIR', Base::InstanceRendererBase);
//------------------------------------------------------------------------------
/**
*/
VkInstanceRenderer::VkInstanceRenderer()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
VkInstanceRenderer::~VkInstanceRenderer()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
VkInstanceRenderer::Setup()
{
    InstanceRendererBase::Setup();

    this->shaderState = CoreGraphics::ShaderServer::Instance()->CreateShaderState("shd:shared", { NEBULA_OBJECT_GROUP });
    this->instancingBuffer = ConstantBuffer::Create();
    this->instancingBuffer->SetupFromBlockInShader(this->shaderState, "InstancingBlock", 16);
    this->instancingBlockVar = this->shaderState->GetVariableByName("InstancingBlock");
    this->instancingBlockVar->SetConstantBuffer(this->instancingBuffer);

    this->modelArrayVar = this->instancingBuffer->GetVariableByName("ModelArray");
    this->modelViewArrayVar = this->instancingBuffer->GetVariableByName("ModelViewArray");
    this->modelViewProjectionArrayVar = this->instancingBuffer->GetVariableByName("ModelViewProjectionArray");
    this->idArrayVar = this->instancingBuffer->GetVariableByName("IdArray");
}

//------------------------------------------------------------------------------
/**
*/
void
VkInstanceRenderer::Close()
{
    this->shaderState->Discard();
    this->shaderState = 0;
    this->modelArrayVar = 0;
    this->modelViewArrayVar = 0;
    this->modelViewProjectionArrayVar = 0;
    this->idArrayVar = 0;
    this->instancingBlockVar = 0;
    this->instancingBuffer->Discard();
    this->instancingBuffer = 0;

    InstanceRendererBase::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
VkInstanceRenderer::Render(const SizeT multiplier)
{
    n_assert(this->shader.isvalid());

    // получить устройство рендеринга
    Ptr<VkRenderDevice> renderDev = VkRenderDevice::Instance();

    // получить указатель на матричный массив
    matrix44* modelTrans = &this->modelTransforms[0];
    matrix44* modelViewTrans = &this->modelViewTransforms[0];
    matrix44* modelViewProjTrans = &this->modelViewProjectionTransforms[0];
    int* objectIdArray = &this->objectIds[0];

    // мы предполагаем, что все массивы одинаково велики
    SizeT instances = this->modelTransforms.Size();
    while (instances > 0)
    {
        // подсчитайте, сколько преобразований мы установим в этом пакете
        int numBatchInstances = n_min(instances, this->MaxInstancesPerBatch);

        // применять переменные
        this->instancingBuffer->CycleBuffers();
        this->instancingBuffer->BeginUpdateSync();
        this->modelArrayVar->SetMatrixArray(modelTrans, numBatchInstances);
        this->modelViewArrayVar->SetMatrixArray(modelViewTrans, numBatchInstances);
        this->modelViewProjectionArrayVar->SetMatrixArray(modelViewProjTrans, numBatchInstances);
        this->idArrayVar->SetIntArray(objectIdArray, numBatchInstances);
        this->instancingBuffer->EndUpdateSync();

        // commit shader, if it contains the instancing block (which requires shared.fxh) it will be updated here
        this->shader->Commit();

        // оказывать!
        renderDev->DrawIndexedInstanced(numBatchInstances * multiplier, 0);

        // уменьшить количество преобразований
        instances -= this->MaxInstancesPerBatch;

        // сместить все массивы
        modelTrans += this->MaxInstancesPerBatch;
        modelViewTrans += this->MaxInstancesPerBatch;
        modelViewProjTrans += this->MaxInstancesPerBatch;
    }
}

} // пространство имен Вулкан