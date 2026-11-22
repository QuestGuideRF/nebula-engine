//------------------------------------------------------------------------------
//  settings.cc
//  (C) 2024 г. Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "settings.h"
#include "editor/editor.h"
#include "editor/commandmanager.h"
#include "editor/ui/uimanager.h"
#include "editor/cmds.h"
#include "editor/tools/selectiontool.h"
#include "imgui_internal.h"
#include "im3d/im3dcontext.h"
#include "io/ioserver.h"

using namespace Editor;

namespace Presentation
{
__ImplementClass(Presentation::Settings, 'SeWn', Presentation::BaseWindow);

//------------------------------------------------------------------------------
/**
*/
Settings::Settings()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Settings::~Settings()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
Settings::Update()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
Settings::Run(SaveMode save)
{
    //bool snapToGridIncrements = Инструменты::SelectionTool::SnapToGridIncrements();
    //if (ImGui::Checkbox("Grid snap", &snapToGridIncrements))
    //{
    //    //Инструменты::SelectionTool::SnapToGridIncrements(snapToGridIncrements);
    //}

    bool showGrid = Im3d::Im3dContext::GetGridStatus();
    if (ImGui::Checkbox("Показать сетку", &showGrid))
    {
        Im3d::Im3dContext::SetGridStatus(showGrid);
    }
    float cellSize;
    int cellCount;
    ImGui::BeginDisabled(!showGrid);
    Im3d::Im3dContext::GetGridSize(cellSize, cellCount);
    bool gridChanged = ImGui::DragFloat("Размер ячейки", &cellSize);
    gridChanged |= ImGui::DragInt("Количество ячеек", &cellCount);
    if (showGrid && gridChanged)
    {
        Im3d::Im3dContext::SetGridSize(cellSize, cellCount);
    }
    Math::vec4 gridColour = Im3d::Im3dContext::GetGridColor();
    if (ImGui::ColorEdit4("Цвет сетки", gridColour.v))
    {
        Im3d::Im3dContext::SetGridColor(gridColour);
    }
    Math::vec2 gridOffset = Im3d::Im3dContext::GetGridOffset();
    if (ImGui::DragFloat2("Смещение сетки", &gridOffset.x))
    {
        Im3d::Im3dContext::SetGridOffset(gridOffset);
    }
    ImGui::EndDisabled();

    //bool use3DGizmo = Инструменты::SelectionTool::UseIm3DGizmo();
    //if (ImGui::Checkbox("Use 3D Gizmo for selection", &use3DGizmo))
    //{
    //    Инструменты::SelectionTool::UseIm3DGizmo(use3DGizmo);
    //}
    //int gizmoSize;
    //интервал gizmoWidth;
    //ImGui::BeginDisabled(!use3DGizmo);
    //Im3d::Im3dContext::GetGizmoSize(gizmoSize, gizmoWidth);
    //bool sizeChange = ImGui::DragInt("Размер штуковины", &gizmoSize);
    //sizeChange |= ImGui::DragInt("Ширина гизмо", &gizmoWidth);
    //if (use3DGizmo && sizeChange)
    //{
    //    Im3d::Im3dContext::SetGizmoSize(gizmoSize, gizmoWidth);
    //}
    //ImGui::EndDisabled();
    const IO::URI path(Editor::UIManager::GetEditorUIIniPath());
    if (ImGui::Button("Сохранить макет редактора"))
    { 
        ImGui::SaveIniSettingsToDisk(path.LocalPath().c_str());
    }
    if (ImGui::Button("Загрузить макет редактора"))
    {
        ImGui::LoadIniSettingsFromDisk(path.LocalPath().c_str());
    }
    if (ImGui::Button("Восстановить макет по умолчанию"))
    {
        const Util::String defaultIni = "tool:syswork/data/editor/defaultui.ini";
        IO::IoServer::Instance()->CreateDirectory("user:nebula/editor/");
        if (IO::IoServer::Instance()->FileExists(defaultIni))
        {
            IO::IoServer::Instance()->CopyFile(defaultIni, path);
        }
        ImGui::LoadIniSettingsFromDisk(path.LocalPath().c_str());
    }
}

} // Презентация пространства имен
