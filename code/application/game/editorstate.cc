//------------------------------------------------------------------------------
//  @file editorstate.cc
//  @copyright (C) 2024 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "editorstate.h"
namespace Game
{

__ImplementSingleton(Game::EditorState);

//------------------------------------------------------------------------------
/**
*/
EditorState::EditorState()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
EditorState::~EditorState()
{
    __DestructSingleton;
}

} // пространство имен Игра
