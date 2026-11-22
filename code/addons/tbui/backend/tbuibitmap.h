#pragma once
#include "coregraphics/texture.h"
#include "tb_renderer.h"
namespace TBUI
{
class TBUIRenderer;
class TBUIBitmap : public tb::TBBitmap
{
public:
    TBUIBitmap(TBUIRenderer* renderer);
    ~TBUIBitmap() override;
    bool Init(int width, int height, unsigned int* data);
    int Width() override;
    int Height() override;
    void SetData(unsigned int* data) override;
    CoreGraphics::TextureId GetTexture() const;
private:
    TBUIRenderer* renderer;
    int width = 0;
    int height = 0;
    CoreGraphics::TextureId texture;
};
inline CoreGraphics::TextureId
TBUIBitmap::GetTexture() const
{
    return this->texture;
}
}