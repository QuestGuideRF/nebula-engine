#pragma once
#include "skeletonbuilder.h"
#include "toolkit-common/platform.h"
#include "io/stream.h"
#include "system/byteorder.h"
namespace ToolkitUtil
{
class SkeletonBuilderSaver
{
public:
    static bool Save(const IO::URI& uri, const Util::Array<SkeletonBuilder>& skeletonBuilders, Platform::Code platform);
private:
    static void WriteHeader(const Ptr<IO::Stream>& stream, const Util::Array<SkeletonBuilder>& skeletonBuilders, const System::ByteOrder& byteOrder);
    static void WriteSkeletons(const Ptr<IO::Stream>& stream, const Util::Array<SkeletonBuilder>& skeletonBuilders, const System::ByteOrder& byteOrder);
    static void WriteJoints(const Ptr<IO::Stream>& stream, const Util::Array<SkeletonBuilder>& skeletonBuilders, const System::ByteOrder& byteOrder);
};
}