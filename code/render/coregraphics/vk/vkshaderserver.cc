//------------------------------------------------------------------------------
// vkshaderserver.cc
// (C) 2016–2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "vkshaderserver.h"
#include "effectfactory.h"
#include "vkgraphicsdevice.h"
#include "vktexture.h"

#include "graphics/bindlessregistry.h"

#include "system_shaders/shared.h"

using namespace Resources;
using namespace CoreGraphics;
namespace Vulkan
{

__ImplementClass(Vulkan::VkShaderServer, 'VKSS', Base::ShaderServerBase);
__ImplementInterfaceSingleton(Vulkan::VkShaderServer);
//------------------------------------------------------------------------------
/**
*/
VkShaderServer::VkShaderServer()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
VkShaderServer::~VkShaderServer()
{
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
bool
VkShaderServer::Open()
{
    n_assert(!this->IsOpen());

    // создать фабрику AnyFX
    this->factory = new(AnyFX::EffectFactory);
    ShaderServerBase::Open();

    this->pendingViews.SetSignalOnEnqueueEnabled(false);
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
VkShaderServer::Close()
{
    n_assert(this->IsOpen());
    delete this->factory;

    // Нам нужно дождаться завершения работы графического процессора, поскольку мы
    // фактическая выгрузка шейдеров и пулов таблиц ресурсов после этого момента
    CoreGraphics::WaitAndClearPendingCommands();

    ShaderServerBase::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
VkShaderServer::UpdateResources()
{
    // Просто выделите память
    uint32_t bufferedFrameIndex = GetBufferedFrameIndex();

    VkDevice dev = GetCurrentDevice();

    // Настройте новые представления для новых потоков LOD
    Util::Array<_PendingView, 32> pendingViewsThisFrame;
    this->pendingViews.DequeueAll(pendingViewsThisFrame);

    // Удалить представления, которые были отклонены из-за потоковой передачи LOD.
    for (int i = this->pendingViewDeletes.Size() - 1; i >= 0; i--)
    {
        // if we have cycled through all our frames, safetly delete the view
        if (this->pendingViewDeletes[i].replaceCounter == bufferedFrameIndex)
        {
            vkDestroyImageView(dev, this->pendingViewDeletes[i].view, nullptr);
            this->pendingViewDeletes.EraseIndex(i);
        }
    }

    for (int i = 0; i < pendingViewsThisFrame.Size(); i++)
    {
        const _PendingView& pend = pendingViewsThisFrame[i];

        TextureIdLock _0(pend.tex);
        VkTextureRuntimeInfo& info = textureAllocator.Get<Texture_RuntimeInfo>(pend.tex.id);
        VkImageView oldView = info.view;
        VkResult res = vkCreateImageView(GetCurrentDevice(), &pend.createInfo, nullptr, &info.view);
        n_assert(res == VK_SUCCESS);

        // обновить записи текстур для всех таблиц
        Graphics::ReregisterTexture(pend.tex, info.type, info.bind);

        _PendingViewDelete pendingDelete;
        pendingDelete.view = oldView;
        pendingDelete.replaceCounter = bufferedFrameIndex;
        pendingViewDeletes.Append(pendingDelete);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
VkShaderServer::AddPendingImageView(CoreGraphics::TextureId tex, VkImageViewCreateInfo viewCreate, uint32_t bind)
{
    _PendingView pend;
    pend.tex = tex;
    pend.createInfo = viewCreate;
    pend.bind = bind;
    this->pendingViews.Enqueue(pend);
}

} // пространство имен Вулкан
