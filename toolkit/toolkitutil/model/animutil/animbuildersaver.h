#pragma once
#include "io/uri.h"
#include "model/animutil/animbuilder.h"
#include "toolkit-common/platform.h"
#include "io/stream.h"
#include "system/byteorder.h"
namespace ToolkitUtil
{
class AnimBuilderSaver
{
public:
    static bool Save(const IO::URI& uri, const Util::Array<AnimBuilder>& animBuilders, Platform::Code platform);
private:
    static void WriteHeader(const Ptr<IO::Stream>& stream, const Util::Array<AnimBuilder>& animBuilders, const System::ByteOrder& byteOrder);
    static void WriteAnimations(const Ptr<IO::Stream>& stream, const Util::Array<AnimBuilder>& animBuilders, const System::ByteOrder& byteOrder);
};
}