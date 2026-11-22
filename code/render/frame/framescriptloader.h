#pragma once
#include "framescript.h"
#include "framepass.h"
#include "framesubpass.h"
#include "framesubmission.h"
#include "resources/resourceid.h"
#include "io/uri.h"
#include "jzon-c/jzon.h"
#include "coregraphics/shader.h"
namespace Frame
{
class FrameScriptLoader
{
public:
    static Ptr<FrameScript> LoadFrameScript(const IO::URI& path);
private:
    static void ParseFrameScript(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static void ParseTextureList(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static void ParseReadWriteBufferList(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParseBlit(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParseCopy(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParseResolve(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParseMipmap(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParseCompute(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParsePlugin(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParseSubgraph(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParseBarrier(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParseSwap(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParsePresent(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static FrameOp* ParsePass(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static void ParseSubmissionList(const Ptr<Frame::FrameScript>& script, JzonValue* node);
    static void ParseAttachmentList(const Ptr<Frame::FrameScript>& script, CoreGraphics::PassCreateInfo& pass, Util::Array<Resources::ResourceName>& attachmentNames, JzonValue* node);
    static void ParseSubpassDepthAttachment(Frame::FramePass* pass, CoreGraphics::Subpass& subpass, Util::Array<Resources::ResourceName>& attachmentNames, JzonValue* node);
    static void ParseSubpassResolves(Frame::FramePass* pass, CoreGraphics::Subpass& subpass, Util::Array<Resources::ResourceName>& attachmentNames, JzonValue* node);
    static void ParseSubpassList(const Ptr<Frame::FrameScript>& script, CoreGraphics::PassCreateInfo& pass, Frame::FramePass* framePass, Util::Array<Resources::ResourceName>& attachmentNames, JzonValue* node);
    static void ParseSubpassDependencyList(Frame::FramePass* pass, CoreGraphics::Subpass& subpass, JzonValue* node);
    static void ParseSubpassAttachmentList(Frame::FramePass* pass, CoreGraphics::Subpass& subpass, Util::Array<Resources::ResourceName>& attachmentNames, JzonValue* node);
    static void ParseSubpassInputList(Frame::FramePass* pass, CoreGraphics::Subpass& subpass, Util::Array<Resources::ResourceName>& attachmentNames, JzonValue* node);
    static void ParseSubpassPlugin(const Ptr<Frame::FrameScript>& script, Frame::FrameSubpass* subpass, JzonValue* node);
    static void ParseSubpassSubgraph(const Ptr<Frame::FrameScript>& script, Frame::FrameSubpass* subpass, JzonValue* node);
    static void ParseSubpassBatch(const Ptr<Frame::FrameScript>& script, Frame::FrameSubpass* subpass, JzonValue* node);
    static void ParseSubpassSortedBatch(const Ptr<Frame::FrameScript>& script, Frame::FrameSubpass* subpass, JzonValue* node);
    static void ParseSubpassFullscreenEffect(const Ptr<Frame::FrameScript>& script, Frame::FrameSubpass* subpass, JzonValue* node);
    static void ParseShaderState(const Ptr<Frame::FrameScript>& script, JzonValue* node, CoreGraphics::ShaderId& shd, CoreGraphics::ResourceTableId& table, Util::Dictionary<Util::StringAtom, CoreGraphics::BufferId>& constantBuffers, Util::Array<Util::Tuple<IndexT, CoreGraphics::BufferId, CoreGraphics::TextureId>>& textures);
    static void ParseShaderVariables(const Ptr<Frame::FrameScript>& script, const CoreGraphics::ShaderId& shd, CoreGraphics::ResourceTableId& table, Util::Dictionary<Util::StringAtom, CoreGraphics::BufferId>& constantBuffers, Util::Array<Util::Tuple<IndexT, CoreGraphics::BufferId, CoreGraphics::TextureId>>& textures, JzonValue* node);
    static void ParseResourceDependencies(const Ptr<Frame::FrameScript>& script, Frame::FrameOp* op, JzonValue* node);
    static Frame::FrameSubmission* LastSubmission[2];
};
}