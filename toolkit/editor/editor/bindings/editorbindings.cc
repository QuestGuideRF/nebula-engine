//------------------------------------------------------------------------------
//  editorbindings.cc
//  (C) 2018–2019 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "editor/commandmanager.h"
#include "scripting/python/conversion.h"
#include "scripting/scriptserver.h"
#include "editor/ui/windowserver.h"

namespace py = nanobind;

/// @todo В каждом файле привязки не должно быть более одного модуля Python.
NB_MODULE(editor, m)
{
    // Начать макрос
    m.def("begin_macro", &Edit::CommandManager::BeginMacro, "Begin a new macro. Bundles subsequent commands into one undo stack entry. Remember to call 'edit.end_macro()'!");
    
    // Завершить макрос
    m.def("end_macro", &Edit::CommandManager::EndMacro);
    
    // Отменить последнюю команду
    m.def("undo", &Edit::CommandManager::Undo, "Отменить последнюю выполненную команду.");
    
    // Redo
    m.def("redo", &Edit::CommandManager::Redo, "Повторить последнюю невыполненную команду.");

    m.def("register_script_window", [](const char* scriptfile, const char* label)
    {
        n_assert(Presentation::WindowServer::HasInstance());
        Presentation::WindowServer::Instance()->RegisterWindowScript(scriptfile, label);
    });
}

namespace Scripting
{
void RegisterEditorBinds()
{
    Scripting::ScriptServer::RegisterModuleInit([]()
    {
        PyImport_AppendInittab("editor", PyInit_editor);
    });
}
}