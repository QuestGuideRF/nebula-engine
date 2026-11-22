//------------------------------------------------------------------------------
//  imguigraph.cc
//  (C) 2019-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "render/stdneb.h"
#include "imguigraph.h"
#include "imgui.h"


namespace Dynui
{


//------------------------------------------------------------------------------
/**
*/
Graph::Graph(const Util::String & _name, SizeT s) : _min(0.0f), _max(1000.0f), average(0.0f), name(_name), buffer(s), scroll(false)
{
    this->frame_max = -FLT_MAX;
    this->frame_min = FLT_MAX;
    Memory::Fill(this->buffer.GetBuffer(), s * sizeof(float), 0);
}


//------------------------------------------------------------------------------
/**
*/
void
Graph::Draw()
{
    header.Format("%s, среднее: %f", this->name.AsCharPtr(), this->average);
    this->averageSum = 0.0f;
        
    ImGui::PushID(static_cast<void*>(this));
    ImGui::Checkbox("Scroll", &this->scroll);
    ImGui::SameLine();
    ImGui::Text(Util::String::Sprintf("Мин/макс: %f, %f", this->_min, this->_max).AsCharPtr());
    ImGui::SameLine();
    if (ImGui::Button("Сброс Мин/Макс"))
    {
        this->frame_max = -FLT_MAX;
        this->frame_min = FLT_MAX;
    }
    ImVec2 region = ImGui::GetContentRegionAvail();
    ImGui::PlotLines("##сюжет", Graph::ValueGetter, (void*)this, this->buffer.Size(), 0, this->header.AsCharPtr(), 0.8f * this->_min, 1.2f * this->_max, region);
    ImGui::PopID();
    
    this->_max = this->frame_max;
    this->_min = this->frame_min;
    this->average = this->averageSum / (float)this->buffer.Size();    

}


}