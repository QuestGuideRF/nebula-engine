#include <math.h>
#include "tb_font_renderer.h"
#include "tb_renderer.h"
#include "platform/tb_system_interface.h"
#include "tb_tempbuffer.h"
namespace TBUI
{
class TBUISTBFontRenderer : public tb::TBFontRenderer
{
public:
    TBUISTBFontRenderer();
    ~TBUISTBFontRenderer();
    bool Load(const char* filename, int size);
    virtual tb::TBFontFace* Create(tb::TBFontManager* font_manager, const char* filename, const tb::TBFontDescription& font_desc);
    virtual tb::TBFontMetrics GetMetrics();
    virtual bool RenderGlyph(tb::TBFontGlyphData* dst_bitmap, UCS4 cp);
    virtual void GetGlyphMetrics(tb::TBGlyphMetrics* metrics, UCS4 cp);
    virtual int GetAdvance(UCS4 cp1, UCS4 cp2);
private:
    stbtt_fontinfo font;
    tb::TBTempBuffer ttf_buffer;
    unsigned char* render_data;
    float scale;
};
}