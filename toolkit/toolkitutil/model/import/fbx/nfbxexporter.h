#pragma once
#include "model/import/base/modelexporter.h"
#include "node/nfbxscene.h"
#include "toolkit-common/base/exporttypes.h"
#include "model/modelutil/modelphysics.h"
namespace ToolkitUtil
{
class NFbxExporter : public ModelExporter
{
    __DeclareClass(NFbxExporter);
public:
    NFbxExporter();
    virtual ~NFbxExporter();
    bool ParseScene() override;
};
}