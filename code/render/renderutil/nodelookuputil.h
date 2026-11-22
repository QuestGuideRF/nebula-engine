#pragma once
#include "util/stringatom.h"
#include "models/nodes/shaderstatenode.h"
#include "models/modelcontext.h"
#include "models/nodes/modelnode.h"
#include "resources/resourceid.h"
namespace RenderUtil
{
class NodeLookupUtil
{
public:
    static Models::ShaderStateNode::Instance* LookupStateNodeInstance(const GraphicsEntityId& entity,
                                                                  const Util::String& nodeName,
                                                                  bool checkResourceState = true);
private:
    static Util::Array<Util::String> RecursiveGetNodeNames(const Ptr<Models::ModelNode>& modelNode , const Util::FourCC & fourcc);
    static Util::String currentNodeLayer;
};
}