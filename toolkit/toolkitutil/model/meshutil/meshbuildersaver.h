#pragma once
#include "io/uri.h"
#include "model/meshutil/meshbuilder.h"
#include "toolkit-common/platform.h"
#include "io/stream.h"
#include "system/byteorder.h"
namespace ToolkitUtil
{
class MeshBuilderSaver
{
public:
    static bool Save(const IO::URI& uri, const Util::Array<MeshBuilder*>& meshes, Platform::Code platform);
private:
    static void WriteHeader(const Ptr<IO::Stream>& stream, const Util::Array<MeshBuilder*>& meshes, const System::ByteOrder& byteOrder);
    static void WriteMeshes(const Ptr<IO::Stream>& stream, const Util::Array<MeshBuilder*>& meshes, const System::ByteOrder& byteOrder);
    static void WriteVertices(const Ptr<IO::Stream>& stream, const Util::Array<MeshBuilder*>& meshes, const System::ByteOrder& byteOrder);
    static void WriteTriangles(const Ptr<IO::Stream>& stream, const Util::Array<MeshBuilder*>& meshes, const System::ByteOrder& byteOrder);
    static void WriteMeshlets(const Ptr<IO::Stream>& stream, const Util::Array<MeshBuilder*>& meshes, const System::ByteOrder& byteOrder);
};
}