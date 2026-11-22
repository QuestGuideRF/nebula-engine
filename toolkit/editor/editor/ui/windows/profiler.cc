//------------------------------------------------------------------------------
//  @file Profiler.cc
//  @copyright (C) 2024 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "profiler.h"
#include "dynui/imguicontext.h"

#include "coregraphics/texture.h"
#include "input/inputserver.h"
#include "input/keyboard.h"

namespace Presentation
{
__ImplementClass(Presentation::Profiler, 'PrBw', Presentation::BaseWindow);

//------------------------------------------------------------------------------
/**
*/
Profiler::Profiler()
{
    this->pauseProfiling = false;
    this->fixedFps = 60.0f;
    this->profileFixedFps = false;
    this->additionalFlags = ImGuiWindowFlags_None;
}

//------------------------------------------------------------------------------
/**
*/
Profiler::~Profiler()
{
}


//------------------------------------------------------------------------------
/**
*/
int
RecursiveDrawScope(const Profiling::ProfilingScope& scope, ImDrawList* drawList, const ImVec2 start, const ImVec2 fullSize, ImVec2 pos, const ImVec2 canvas, const float frameTime, const int level)
{
    static const ImU32 colors[] =
    {
        IM_COL32(255, 200, 200, 255),
        IM_COL32(200, 255, 200, 255),
        IM_COL32(200, 200, 255, 255),
        IM_COL32(200, 200, 255, 255),
        IM_COL32(200, 255, 255, 255),
        IM_COL32(255, 255, 200, 255),
    };
    static const float YPad = ImGui::GetTextLineHeight();
    static const float TextPad = 5.0f;

    const uint32_t numColors = sizeof(colors) / sizeof(ImU32);
    uint32_t colorIndex = scope.category.HashCode() % numColors;

    // конвертировать в миллисекунды
    float startX = pos.x + (scope.start / frameTime) * canvas.x;
    float stopX = startX + Math::max((scope.duration / frameTime) * canvas.x, 1.0);
    float startY = pos.y;
    float stopY = startY + YPad;

    ImVec2 bbMin = ImVec2(startX, startY);
    ImVec2 bbMax = ImVec2(Math::min(stopX, startX + fullSize.x), Math::min(stopY, startY + fullSize.y));

    // нарисуйте заполненный прямоугольник для фона и обычный прямоугольник для контура
    drawList->PushClipRect(bbMin, bbMax, true);
    drawList->AddRectFilled(bbMin, bbMax, colors[colorIndex]);
    drawList->AddRect(bbMin, bbMax, IM_COL32(128, 128, 128, 128));

    // убедитесь, что текст появляется внутри поля
    Util::String text = Util::String::Sprintf("%s (%4.4f мс)", scope.name, scope.duration * 1000);
    drawList->AddText(ImVec2(startX + TextPad, pos.y), IM_COL32_BLACK, text.AsCharPtr());
    drawList->PopClipRect();

    if (ImGui::IsMouseHoveringRect(bbMin, bbMax))
    {
        ImGui::BeginTooltip();
        Util::String text = Util::String::Sprintf("%s [%s] (%4.4f мс) через %s (%d)", scope.name, scope.category.Value(), scope.duration * 1000, scope.file, scope.line);
        ImGui::Text(text.AsCharPtr());
        ImGui::EndTooltip();
        ImVec2 l1 = bbMax;
        l1.y = start.y;
        ImVec2 l2 = bbMax;
        l2.y = fullSize.y;
        drawList->PushClipRect(start, fullSize);
        drawList->AddLine(l1, l2, IM_COL32(255, 0, 0, 255), 1.0f);
        drawList->PopClipRect();
    }

    // переместить следующий элемент на один уровень вниз
    pos.y += YPad;
    int deepest = level + 1;
    for (IndexT i = 0; i < scope.children.Size(); i++)
    {
        int childLevel = RecursiveDrawScope(scope.children[i], drawList, start, fullSize, pos, canvas, frameTime, level + 1);
        deepest = Math::max(deepest, childLevel);
    }
    return deepest;
}

//------------------------------------------------------------------------------
/**
*/
int
RecursiveDrawGpuMarker(const CoreGraphics::FrameProfilingMarker& marker, ImDrawList* drawList, const ImVec2 start, const ImVec2 fullSize, ImVec2 pos, const ImVec2 canvas, const float frameTime, const int level)
{
    static const float YPad = ImGui::GetTextLineHeight();
    static const float TextPad = 5.0f;

    // конвертировать в миллисекунды
    float begin = marker.start / 1000000000.0f;
    float duration = marker.duration / 1000000000.0f;
    float startX = pos.x + begin / frameTime * canvas.x;
    float stopX = startX + Math::max(duration / frameTime * canvas.x, 1.0f);
    float startY = pos.y;
    float stopY = startY + YPad;

    ImVec2 bbMin = ImVec2(startX, startY);
    ImVec2 bbMax = ImVec2(Math::min(stopX, startX + fullSize.x), Math::min(stopY, startY + fullSize.y));

    // нарисуйте заполненный прямоугольник для фона и обычный прямоугольник для контура
    drawList->PushClipRect(bbMin, bbMax, true);
    ImU32 color = IM_COL32(marker.color.x * 255.0f, marker.color.y * 255.0f, marker.color.z * 255.0f, marker.color.w * 255.0f);
    drawList->AddRectFilled(bbMin, bbMax, color, 0.0f);
    drawList->AddRect(bbMin, bbMax, IM_COL32(128, 128, 128, 128), 0.0f);

    // убедитесь, что текст появляется внутри поля
    Util::String text = Util::String::Sprintf("%s (%4.4f мс)", marker.name.AsCharPtr(), duration * 1000);
    drawList->AddText(ImVec2(startX + TextPad, startY), IM_COL32_BLACK, text.AsCharPtr());
    drawList->PopClipRect();

    if (ImGui::IsMouseHoveringRect(bbMin, bbMax))
    {
        ImGui::BeginTooltip();
        Util::String text = Util::String::Sprintf("%s (%4.4f мс)", marker.name.AsCharPtr(), duration * 1000);
        ImGui::Text(text.AsCharPtr());
        ImGui::EndTooltip();
    }

    // переместить следующий элемент на один уровень вниз
    pos.y += YPad;
    int deepest = level + 1;
    for (IndexT i = 0; i < marker.children.Size(); i++)
    {
        int childLevel = RecursiveDrawGpuMarker(marker.children[i], drawList, start, fullSize, pos, canvas, frameTime, level + 1);
        deepest = Math::max(deepest, childLevel);
    }
    return deepest;
}

//------------------------------------------------------------------------------
/**
*/
void 
Profiler::Run(SaveMode save)
{
    if (!this->pauseProfiling)
    {
        this->currentFrameTime = Graphics::GraphicsServer::Instance()->GetFrameTime();
        this->averageFrameTime += this->currentFrameTime;
        this->frametimeHistory.Append(this->currentFrameTime);
        if (this->frametimeHistory.Size() > 120)
            this->frametimeHistory.EraseFront();

        if (Graphics::GraphicsServer::Instance()->GetFrameIndex() % 35 == 0)
        {
            this->prevAverageFrameTime = this->averageFrameTime / 35.0f;
            this->averageFrameTime = 0.0f;
        }
    }

    ImGui::Checkbox("Пауза (F3)", &this->pauseProfiling);
    if (ImGui::IsKeyPressed((ImGuiKey)Input::Key::F3))
        this->pauseProfiling = !this->pauseProfiling;
    if (!this->pauseProfiling)
    {
        this->ProfilingContexts = Profiling::ProfilingGetContexts();
        this->frameProfilingMarkers = CoreGraphics::GetProfilingMarkers();
    }

    if (!this->frametimeHistory.IsEmpty())
    {
        ImGui::Text("мс – %.2f\nFPS – %.2f", this->prevAverageFrameTime * 1000, 1 / this->prevAverageFrameTime);
        ImGui::PlotLines("Время кадра", &this->frametimeHistory[0], this->frametimeHistory.Size(), 0, 0, FLT_MIN, FLT_MAX, { ImGui::GetContentRegionAvail().x, 90 });
        ImGui::Separator();
        ImGui::Checkbox("Исправлен ФПС", &this->profileFixedFps);
        if (this->profileFixedFps)
        {
            ImGui::InputInt("FPS", &this->fixedFps);
            this->currentFrameTime = 1 / float(this->fixedFps);
        }
    }

#if NEBULA_ENABLE_PROFILING
    if (ImGui::CollapsingHeader("Profiler"))
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 start = ImGui::GetCursorScreenPos();
        ImVec2 fullSize = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y);
        static float timeWindow = 1.0f;
        ImGui::SliderFloat("Временное окно", &timeWindow, 1.0f, 1000.0f);
        if (ImGui::CollapsingHeader("Timeline"))
        {
            for (const Profiling::ProfilingContext& ctx : this->ProfilingContexts)
            {
                if (ImGui::CollapsingHeader(ctx.threadName.Value(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushFont(Dynui::ImguiContext::state.smallFont);

                    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
                    canvasSize.x *= timeWindow;
                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    int levels = 0;
                    if (ctx.topLevelScopes.Size() > 0) for (IndexT i = 0; i < ctx.topLevelScopes.Size(); i++)
                    {
                        const Profiling::ProfilingScope& scope = ctx.topLevelScopes[i];
                        int level = RecursiveDrawScope(scope, drawList, start, fullSize, pos, canvasSize, this->currentFrameTime, 0);
                        levels = Math::max(levels, level);
                    }
                    else
                    {
                        ImVec2 bbMin = ImVec2(pos.x, pos.y);
                        ImVec2 bbMax = ImVec2(pos.x, pos.y + ImGui::GetTextLineHeight());
                        drawList->PushClipRect(bbMin, bbMax);
                        drawList->AddRectFilled(bbMin, bbMax, IM_COL32(200, 50, 50, 0));
                        drawList->PopClipRect();
                        pos.y += ImGui::GetTextLineHeight();
                    }

                    // установите курсор назад, чтобы мы могли нарисовать нашу рамку
                    ImGui::SetCursorScreenPos(pos);
                    ImGui::InvisibleButton("canvas", ImVec2(canvasSize.x, Math::max(1.0f, levels * 20.0f)));
                    ImGui::PopFont();
                }
            }
            if (ImGui::CollapsingHeader("GPU"))
            {
                ImGui::PushFont(Dynui::ImguiContext::state.smallFont);

                ImVec2 canvasSize = ImGui::GetContentRegionAvail();
                ImVec2 pos = ImGui::GetCursorScreenPos();
                const Util::Array<CoreGraphics::FrameProfilingMarker>& frameMarkers = this->frameProfilingMarkers;

                // сделать графические маркеры очереди
                drawList->AddText(ImVec2(pos.x, pos.y), IM_COL32_WHITE, "Графическая очередь");
                pos.y += 20.0f;
                int levels = 0;
                for (int i = 0; i < frameMarkers.Size(); i++)
                {
                    const CoreGraphics::FrameProfilingMarker& marker = frameMarkers[i];
                    if (marker.queue != CoreGraphics::GraphicsQueueType)
                        continue;
                    int level = RecursiveDrawGpuMarker(marker, drawList, start, fullSize, pos, canvasSize, this->currentFrameTime, 0);
                    levels = Math::max(levels, level);
                }

                // установите курсор назад, чтобы мы могли нарисовать нашу рамку
                ImGui::SetCursorScreenPos(pos);
                ImGui::InvisibleButton("canvas", ImVec2(canvasSize.x, Math::max(1.0f, levels * 20.0f)));
                pos.y += levels * 20.0f;

                drawList->AddText(ImVec2(pos.x, pos.y), IM_COL32_WHITE, "Вычислить очередь");
                pos.y += 20.0f;
                levels = 0;
                for (int i = 0; i < frameMarkers.Size(); i++)
                {
                    const CoreGraphics::FrameProfilingMarker& marker = frameMarkers[i];
                    if (marker.queue != CoreGraphics::ComputeQueueType)
                        continue;
                    int level = RecursiveDrawGpuMarker(marker, drawList, start, fullSize, pos, canvasSize, this->currentFrameTime, 0);
                    levels = Math::max(levels, level);
                }

                // установите курсор назад, чтобы мы могли нарисовать нашу рамку
                ImGui::SetCursorScreenPos(pos);
                ImGui::InvisibleButton("canvas", ImVec2(canvasSize.x, Math::max(1.0f, levels * 20.0f)));
                ImGui::PopFont();
            }
        }
        if (ImGui::CollapsingHeader("Memory"))
        {
            ImGui::PushFont(Dynui::ImguiContext::state.smallFont);

            Util::Dictionary<const char*, uint64_t> counters = Profiling::ProfilingGetCounters();
            for (IndexT i = 0; i < counters.Size(); i++)
            {
                const char* name = counters.KeyAtIndex(i);
                uint64_t val = counters.ValueAtIndex(i);
                if (val >= 1_GB)
                    ImGui::LabelText(name, "Выделено %.2f ГБ", val / float(1_GB));
                else if (val >= 1_MB)
                    ImGui::LabelText(name, "Выделено %.2f МБ", val / float(1_MB));
                else if (val >= 1_KB)
                    ImGui::LabelText(name, "Выделено %.2f КБ", val / float(1_KB));
                else
                    ImGui::LabelText(name, "%lU B выделено", val);
            }

            const Util::Dictionary<const char*, Util::Pair<uint64_t, uint64_t>>& budgetCounters = Profiling::ProfilingGetBudgetCounters();
            for (IndexT i = 0; i < budgetCounters.Size(); i++)
            {
                const char* name = budgetCounters.KeyAtIndex(i);
                const Util::Pair<uint64_t, uint64_t>& val = budgetCounters.ValueAtIndex(i);
                if (val.first >= 1_GB)
                    ImGui::LabelText(name, "Выделено %.2f ГБ, осталось %.2f ГБ", val.second / float(1_GB), (val.first - val.second) / float(1_GB));
                else if (val.first >= 1_MB)
                    ImGui::LabelText(name, "Выделено %.2f МБ, осталось %.2f МБ", val.second / float(1_MB), (val.first - val.second) / float(1_MB));
                else if (val.first >= 1_KB)
                    ImGui::LabelText(name, "Выделено %.2f КБ, осталось %.2f КБ", val.second / float(1_KB), (val.first - val.second) / float(1_KB));
                else
                    ImGui::LabelText(name, "%lu B выделено, %lu B осталось", val.second, val.first - val.second);

                float weight = val.second / double(val.first);
                Math::vec4 green(0, 1, 0, 1);
                Math::vec4 red(1, 0, 0, 1);
                Math::vec4 color = Math::lerp(green, red, sqrt(weight));
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(color.x, color.y, color.z, color.w));
                ImGui::ProgressBar(weight);
                ImGui::PopStyleColor();
            }

            ImGui::PopFont();
        }
    }
#endif //NEBULA_ENABLE_PROFILING
}

} // Презентация пространства имен
