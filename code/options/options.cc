//------------------------------------------------------------------------------
//  options.cc
//  (C) 2025 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "options.h"
namespace Options
{

App::ProjectSettingsT ProjectSettings;
App::LevelSettingsT LevelSettings;
//------------------------------------------------------------------------------
/**
*/
void InitOptions()
{
    Flat::FlatbufferInterface::DeserializeJsonFlatbuffer<App::ProjectSettings>(ProjectSettings, "proj:work/data/tables/projectsettings.json"_uri, "NPST");
}
}
