//------------------------------------------------------------------------------
//  styleeditor.cc
//  (C) 2021 г. Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "styleeditor.h"
#include "editor/editor.h"
#include "editor/commandmanager.h"

using namespace Editor;

namespace Presentation
{
__ImplementClass(Presentation::StyleEditor, 'SEWn', Presentation::BaseWindow);

//------------------------------------------------------------------------------
/**
*/
StyleEditor::StyleEditor()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
StyleEditor::~StyleEditor()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
static void
HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

//------------------------------------------------------------------------------
/**
*/
static void
NodeFont(ImFont* font)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    bool font_details_opened = ImGui::TreeNode(font, "Шрифт: \"%s\"\n%.2f пикселей, %d глифов, %d файлов",
        font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size, font->ConfigDataCount);
    ImGui::SameLine(); if (ImGui::SmallButton("Установить по умолчанию")) { io.FontDefault = font; }
    if (!font_details_opened)
        return;

    ImGui::PushFont(font);
    ImGui::Text("Быстрая коричневая лиса перепрыгивает через ленивую собаку");
    ImGui::PopFont();
    ImGui::DragFloat("Масштаб шрифта", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");   // Масштабировать только этот шрифт
    ImGui::SameLine(); HelpMarker(
        "Обратите внимание: встроенный шрифт по умолчанию НЕ предназначен для масштабирования.\n\n"
        "Шрифты в настоящее время преобразуются в растровые изображения заданного размера во время создания атласа. "
        "Вы можете передискретизировать их, чтобы получить некоторую гибкость при масштабировании. "
        "Вы также можете выполнять рендеринг в нескольких размерах и выбирать, какой из них использовать во время выполнения.\n\n"
        "(Проблеск надежды: в будущем система атласов будет переписана, чтобы сделать масштабирование более гибким.)");
    ImGui::Text("Подъем: %f, Спуск: %f, Высота: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
    ImGui::Text("Резервный символ: '%c' (U+%04X)", font->FallbackChar, font->FallbackChar);
    ImGui::Text("Символ многоточия: '%c' (U+%04X)", font->EllipsisChar, font->EllipsisChar);
    const int surface_sqrt = (int)sqrtf((float)font->MetricsTotalSurface);
    ImGui::Text("Площадь текстуры: около %d пикселей ~%d x%d пикселей.", font->MetricsTotalSurface, surface_sqrt, surface_sqrt);
    for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
        if (font->ConfigData)
            if (const ImFontConfig* cfg = &font->ConfigData[config_i])
                ImGui::BulletText("Входные данные %d: \'%s\', передискретизация: (%d,%d), PixelSnapH: %d, смещение: (%.1f,%.1f)",
                    config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH, cfg->GlyphOffset.x, cfg->GlyphOffset.y);
    if (ImGui::TreeNode("Glyphs", "Глифы (%d)", font->Glyphs.Size))
    {
        // Отображать все глифы шрифтов на отдельных страницах по 256 символов.
        const ImU32 glyph_col = ImGui::GetColorU32(ImGuiCol_Text);
        for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
        {
            // Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
            // Это лишь небольшая оптимизация для уменьшения количества итераций при IM_UNICODE_MAX_CODEPOINT.
            // большой // (если ImWchar==ImWchar32, мы выполним здесь как минимум около 272 запросов)
            if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
            {
                base += 4096 - 256;
                continue;
            }

            int count = 0;
            for (unsigned int n = 0; n < 256; n++)
                if (font->FindGlyphNoFallback((ImWchar)(base + n)))
                    count++;
            if (count <= 0)
                continue;
            if (!ImGui::TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
                continue;
            float cell_size = font->FontSize * 1;
            float cell_spacing = style.ItemSpacing.y;
            ImVec2 base_pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            for (unsigned int n = 0; n < 256; n++)
            {
                // Мы используем ImFont::RenderChar в качестве ярлыка, поскольку у нас нет функций преобразования UTF-8.
                // доступен здесь и, следовательно, не может легко генерировать строку в кодировке UTF-8 с нулевым завершением.
                ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing), base_pos.y + (n / 16) * (cell_size + cell_spacing));
                ImVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
                const ImFontGlyph* glyph = font->FindGlyphNoFallback((ImWchar)(base + n));
                draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
                if (glyph)
                    font->RenderChar(draw_list, cell_size, cell_p1, glyph_col, (ImWchar)(base + n));
                if (glyph && ImGui::IsMouseHoveringRect(cell_p1, cell_p2))
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Код: U+%04X", base + n);
                    ImGui::Separator();
                    ImGui::Text("Видно: %d", glyph->Visible);
                    ImGui::Text("ПрогрессX: %.1f", glyph->AdvanceX);
                    ImGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
                    ImGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
                    ImGui::EndTooltip();
                }
            }
            ImGui::Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::TreePop();
}

//------------------------------------------------------------------------------
/**
*/
void
StyleEditor::Run(SaveMode save)
{
    ImGuiStyle* ref = nullptr;
    // Вы можете передать ссылочную структуру ImGuiStyle для сравнения, возврата и сохранения.
    // (без указателя ссылочного стиля мы будем использовать в качестве ссылки тот, который сравнивается локально)
    ImGuiStyle& style = ImGui::GetStyle();
    static ImGuiStyle ref_saved_style;

    // По умолчанию используется внутренняя память в качестве эталона.
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

    if (ImGui::ShowStyleSelector("Цвета##Селектор"))
        ref_saved_style = style;
    ImGui::ShowFontSelector("Шрифты##Селектор");

    // Упрощенные настройки (представляют размеры границ с плавающим указателем как логические значения, представляющие 0,0f или 1,0f)
    if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
        style.GrabRounding = style.FrameRounding; // Сделайте GrabRounding всегда таким же значением, как FrameRounding.
    { bool border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? 1.0f : 0.0f; } }
    ImGui::SameLine();
    { bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder",  &border)) { style.FrameBorderSize  = border ? 1.0f : 0.0f; } }
    ImGui::SameLine();
    { bool border = (style.PopupBorderSize > 0.0f);  if (ImGui::Checkbox("PopupBorder",  &border)) { style.PopupBorderSize  = border ? 1.0f : 0.0f; } }

    // Кнопка «Сохранить/Вернуть»
    if (ImGui::Button("Сохранить ссылку"))
        *ref = ref_saved_style = style;
    ImGui::SameLine();
    if (ImGui::Button("Восстановить ссылку"))
        style = *ref;
    ImGui::SameLine();
    HelpMarker(
        "Сохранить/вернуть в локальном непостоянном хранилище. Определение цветов по умолчанию не затрагивается. "
        "Используйте кнопку «Экспорт» ниже, чтобы сохранить их где-нибудь.");

    ImGui::Separator();

    if (ImGui::BeginTabBar("##вкладки", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Sizes"))
        {
            ImGui::Text("Main");
            ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
            ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
            ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
            ImGui::Text("Borders");
            ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::Text("Rounding");
            ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::Text("Alignment");
            ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
            int window_menu_button_position = style.WindowMenuButtonPosition + 1;
            if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
                style.WindowMenuButtonPosition = window_menu_button_position - 1;
            ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
            ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
            ImGui::SameLine(); HelpMarker("Выравнивание применяется, когда кнопка больше ее текстового содержимого.");
            ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
            ImGui::SameLine(); HelpMarker("Выравнивание применяется, когда выбираемый объект больше его текстового содержимого.");
            ImGui::Text("Обивка безопасной зоны");
            ImGui::SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
            ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Colors"))
        {
            static int output_dest = 0;
            static bool output_only_modified = true;
            if (ImGui::Button("Export"))
            {
                if (output_dest == 0)
                    ImGui::LogToClipboard();
                else
                    ImGui::LogToTTY();
                ImGui::LogText("ImVec4* цвета = ImGui::GetStyle().Colors;" "\n");
                for (int i = 0; i < ImGuiCol_COUNT; i++)
                {
                    const ImVec4& col = style.Colors[i];
                    const char* name = ImGui::GetStyleColorName(i);
                    if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
                        ImGui::LogText("цвета[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" "\n",
                            name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
                }
                ImGui::LogFinish();
            }
            ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "В буфер обмена\0В телетайп\0");
            ImGui::SameLine(); ImGui::Checkbox("Только измененные цвета", &output_only_modified);

            static ImGuiTextFilter filter;
            filter.Draw("Цвета фильтра", ImGui::GetFontSize() * 16);

            static ImGuiColorEditFlags alpha_flags = 0;
            if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None))             { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
            if (ImGui::RadioButton("Alpha",  alpha_flags == ImGuiColorEditFlags_AlphaPreview))     { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
            if (ImGui::RadioButton("Both",   alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
            HelpMarker(
                "В списке цветов:\n"
                "Щелкните левой кнопкой мыши на цветовом квадрате, чтобы открыть палитру цветов,\n"
                "Щелкните правой кнопкой мыши, чтобы открыть меню параметров редактирования.");

            ImGui::BeginChild("##цвета", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
            ImGui::PushItemWidth(-160);
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                const char* name = ImGui::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);
                ImGui::ColorEdit4("##цвет", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
                {
                    // Tips: in a real user application, you may want to merge and use an icon font into the main font,
                    // поэтому вместо «Сохранить»/«Вернуть» вы должны использовать значки!
                    // Прочтите FAQ и docs/FONTS.md об использовании иконочных шрифтов. Это действительно просто и супер удобно!
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
                }
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::TextUnformatted(name);
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Fonts"))
        {
            ImGuiIO& io = ImGui::GetIO();
            ImFontAtlas* atlas = io.Fonts;
            HelpMarker("Прочтите FAQ и docs/FONTS.md для получения подробной информации о загрузке шрифтов.");
            ImGui::PushItemWidth(120);
            for (int i = 0; i < atlas->Fonts.Size; i++)
            {
                ImFont* font = atlas->Fonts[i];
                ImGui::PushID(font);
                NodeFont(font);
                ImGui::PopID();
            }
            if (ImGui::TreeNode("Атлас текстуры", "Текстура атласа (%dx%d пикселей)", atlas->TexWidth, atlas->TexHeight))
            {
                ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
                ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
                ImGui::TreePop();
            }

            // Масштабирование шрифта после запекания. Обратите внимание, что это НЕ лучший способ масштабирования шрифтов, читайте ниже.
            // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click text to get out of bounds).
            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.0f;
            HelpMarker(
                "Это старые настройки, предоставленные для удобства.\n"
                "Однако _правильный_ способ масштабирования вашего пользовательского интерфейса в настоящее время заключается в перезагрузке шрифта заданного размера. "
                "перестройте атлас шрифтов и вызовите style.ScaleAllSizes() для ссылочной структуры ImGuiStyle.\n"
                "Использование этих настроек здесь даст вам результаты низкого качества.");
            static float window_scale = 1.0f;
            if (ImGui::DragFloat("масштаб окна", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Масштабировать только это окно
                ImGui::SetWindowFontScale(window_scale);
            ImGui::DragFloat("глобальный масштаб", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Масштабируйте все
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Rendering"))
        {
            ImGui::Checkbox("Сглаженные линии", &style.AntiAliasedLines);
            ImGui::SameLine();
            HelpMarker("При отключении сглаживания линий вы, вероятно, захотите также отключить границы в своем стиле.");

            ImGui::Checkbox("Сглаженные линии используют текстуру", &style.AntiAliasedLinesUseTex);
            ImGui::SameLine();
            HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering).");

            ImGui::Checkbox("Сглаженная заливка", &style.AntiAliasedFill);
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("Допуск тесселяции кривой", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
            if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;

            // При редактировании значения «Максимальная ошибка сегмента круга» нарисуйте предварительный просмотр его эффекта на кругах с автоматической тесселяцией.
            ImGui::DragFloat("Максимальная ошибка сегмента круга", &style.CircleTessellationMaxError, 0.01f, 0.10f, 10.0f, "%.2f");
            if (ImGui::IsItemActive())
            {
                ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
                ImGui::BeginTooltip();
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                float RAD_MIN = 10.0f, RAD_MAX = 80.0f;
                float off_x = 10.0f;
                for (int n = 0; n < 7; n++)
                {
                    const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (7.0f - 1.0f);
                    draw_list->AddCircle(ImVec2(p.x + off_x + rad, p.y + RAD_MAX), rad, ImGui::GetColorU32(ImGuiCol_Text), 0);
                    off_x += 10.0f + rad * 2.0f;
                }
                ImGui::Dummy(ImVec2(off_x, RAD_MAX * 2.0f));
                ImGui::EndTooltip();
            }
            ImGui::SameLine();
            HelpMarker("При рисовании примитивов круга с \"num_segments == 0\" тесселяция будет рассчитываться автоматически.");

            ImGui::DragFloat("Глобальная Альфа", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::PopItemWidth();
}

} // Презентация пространства имен
