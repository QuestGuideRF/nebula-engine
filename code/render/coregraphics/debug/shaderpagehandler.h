#pragma once
#ifndef DEBUG_SHADERPAGEHANDLER_H
#define DEBUG_SHADERPAGEHANDLER_H
#include "http/httprequesthandler.h"
#include "resources/resourceid.h"
#include "http/html/htmlpagewriter.h"
#include "coregraphics/shader.h"
#include "models/nodes/modelnode.h"
namespace Debug
{
class ShaderPageHandler : public Http::HttpRequestHandler
{
    __DeclareClass(ShaderPageHandler);
public:
    ShaderPageHandler();
    virtual void HandleRequest(const Ptr<Http::HttpRequest>& request);
private:
    Http::HttpStatus::Code HandleShaderInfoRequest(const Util::String& resId, const Ptr<IO::Stream>& responseContentStream);
    void WriteShaderVariableTable(const Ptr<Http::HtmlPageWriter>& htmlWriter, const CoreGraphics::ShaderId shader);
};
}
#endif