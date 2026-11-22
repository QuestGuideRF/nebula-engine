#pragma once
#include "core/singleton.h"
namespace Game
{
class EditorState
{
    __DeclareSingleton(EditorState)
public:
    EditorState();
    ~EditorState();
    bool isRunning = false;
    bool isPlaying = false;
};
}