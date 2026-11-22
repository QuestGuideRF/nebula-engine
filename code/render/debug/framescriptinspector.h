#pragma once
#include "core/refcounted.h"
namespace Frame
{
class FrameScript;
}
namespace Debug
{
class FrameScriptInspector
{
public:
    static void Run(const Ptr<Frame::FrameScript>& script);
private:
};
}