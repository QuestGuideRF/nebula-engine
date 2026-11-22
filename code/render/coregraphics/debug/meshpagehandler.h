#pragma once
#ifndef DEBUG_MESHPAGEHANDLER_H
#define DEBUG_MESHPAGEHANDLER_H
#include "http/httprequesthandler.h"
#include "resources/resourceid.h"
namespace Debug
{
class MeshPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(MeshPageHandler);
public:
    MeshPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
private:
    Http::HttpStatus::Code HandleMeshInfoRequest(const Util::String& resId, const Ptr<IO::Stream>& responseContentStream);
    Http::HttpStatus::Code HandleVertexDumpRequest(const Util::String& resId, IndexT minIndex, IndexT maxIndex, const Ptr<IO::Stream>& responseContentStream);
};
}
#endif