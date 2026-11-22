#pragma once
#include "toolkitutil/platform.h"
#include "io/uri.h"
#include "io/xmlwriter.h"
#include "util/string.h"
#include "util/stringatom.h"
#include "util/stack.h"
#include "io/stream.h"
#include "io/binaryreader.h"
#include "toolkitutil/modelwriter.h"
#include "util/variant.h"
#include "util/simpletree.h"
#include "toolkitutil/logger.h"
#include "n2util/nvx2orderfreestreamreader.h"
#include "toolkitutil/n2util/n2reflectioninfo.h"
#include "toolkitutil/n2util/n2sceneloader.h"
#include "math/transform44.h"
#include "math/rectangle.h"
namespace ToolkitUtil
{
class N2Converter
{
public:
    N2Converter();
    ~N2Converter();
    void Setup();
    void Discard();
    bool IsValid() const;
    void SetPlatform(Platform::Code platform);
    void SetSrcDir(const Util::String& srcDir);
    void SetDstDir(const Util::String& dstDir);
    void SetMeshDir(const Util::String& mshDir);
    void SetForceFlag(bool b);
    void SetBinaryFlag(bool b);
    void SetVerbose(bool b);
    void ResetUsedResources();
    const Util::Array<Util::String>& GetUsedResources() const;
    const Ptr<N2ReflectionInfo>& GetN2ReflectionInfo() const;
    bool ConvertFile(Logger& logger, const Util::String& category, const Util::String& srcFile);
    bool ConvertCategory(Logger& logger, const Util::String& category);
    bool ConvertAll(Logger& logger);
private:
    enum NodeType
    {
        TransformNode = 0,
        ShapeNode,
        Character3Node,
        Character3SkinAnimator,
        Character3SkinShapeNode,
        EmbeddedNodeType,
        AnimatorNode,
        ParticleSystemNode,
        InvalidNodeType,
    };
    enum AnimatorNodeType
    {
        IntAnimator = 0,
        FloatAnimator,
        Float4Animator,
        TransformAnimator,
        TransformCurveAnimator,
        UvAnimator,
        InvalidAnimatorNodeType,
    };
    struct KeyValues
    {
        float time;
        Util::Variant value;
    };
    struct AnimatorInfo
    {
        Util::String loopType;
        AnimatorNodeType animatorNode;
        Util::String animation;
        Util::Array<KeyValues> key;
        Util::String paramName;
        Util::String vectorName;
        Util::Array<KeyValues> posKey;
        Util::Array<KeyValues> eulerKey;
        Util::Array<KeyValues> scaleKey;
        int animationGroup;
        Util::Array<int> layer;
    };
    bool NeedsConversion(const Util::String& srcPath, const Util::String& dstPath);
    bool PerformConversion(const Util::String& modelName, const Ptr<IO::BinaryReader>& reader, const Ptr<ModelWriter>& writer);
    bool ParseNodes(const Ptr<IO::BinaryReader>& reader, const Ptr<ModelWriter>& writer);
    void BeginNode(const Util::String& objClass, const Util::String& objName, const Ptr<ModelWriter>& writer);
    void EndNode(const Ptr<ModelWriter>& writer);
    void ReadDataTag(Util::FourCC fourCC, const Ptr<IO::BinaryReader>& reader, const Ptr<ModelWriter>& writer);
    Util::String MapShaderNameToModelNodeType(const Util::String& shaderName);
    Util::String ConvertShaderName(const Util::String& shaderName);
    bool ConvertSkinLists(const Ptr<ModelWriter>& writer);
    void WriteParticleEnvelopeCurve(const Util::String& tag, Util::FourCC fourCC, const Ptr<IO::BinaryReader>& reader, const Ptr<ModelWriter>& writer);
    void WriteParticleRGBEnvelopeCurve(const Util::String& redTag, Util::FourCC redFourCC, const Util::String& greenTag, Util::FourCC greenFourCC, const Util::String& blueTag, Util::FourCC blueFourCC, const Ptr<IO::BinaryReader>& reader, const Ptr<ModelWriter>& writer);
    void WriteAnimatorNode(const Ptr<ModelWriter>& writer);
    void FillAnimatorInformation();
    void SetAnimatorPath(Util::String objName, Util::String objClass);
    AnimatorNodeType FromStringToAnimatorNode(Util::String animNode);
    Util::String ConvertRelNodePathToAbsolute(Util::String relPath, bool pathToAnimatorNode=false);
    Util::String ConvertNodePathFromArrayToString(Util::Array<Util::String> stringArray);
    Util::String ConvertShaderSemantic(const Util::String& rhs) const;
    Util::String ConvertAnimationName(const Util::String& anim) const;
    void AddUsedResource(const Util::String& resId);
    Util::String SubstituteResourceAssigns(const Util::String& rhs);
    Platform::Code platform;
    Util::String srcDir;
    Util::String dstDir;
    Util::String mshDir;
    Util::String srcPath;
    Util::String dstPath;
    bool force;
    bool binary;
    bool verbose;
    bool isValid;
    NodeType nodeType;
    Util::Stack<NodeType> nodeTypeStack;
    Ptr<N2ReflectionInfo> n2ReflectionInfo;
    Ptr<N2SceneLoader> n2SceneLoader;
    Util::FixedArray<IndexT> skinFragmentPrimGroupIndices;
    Util::FixedArray<Util::FixedArray<IndexT>> skinFragmentJointPalettes;
    Util::Dictionary<Util::String, Util::Array<Util::String> > animatedNodesToAnimator;
    Util::Dictionary<Util::String, AnimatorInfo> animatorInformation;
    Util::Array<Util::String> path;
    Util::String lastAnimatorName;
    AnimatorInfo animInfo;
    AnimatorNodeType animNodeType;
    Util::Array<Util::String> usedResources;
    class GuiElement
    {
    public:
        GuiElement();
        Util::String guiType;
        Util::String id;
        Util::String event;
        Util::String texture;
        Util::String pressedTexture;
        Util::String mouseOverTexture;
        Util::String disabledTexture;
        Util::String xAlignment;
        Util::String yAlignment;
        Math::bbox rawLocalBox;
        Math::transform44 rawTransform;
        Math::rectangle<float> rect;
        Math::float4 uv[4];
        bool ignoreInput;
    };
    Util::SimpleTree<GuiElement> windowTree;
    Util::Dictionary<Util::StringAtom, Util::StringAtom> alignmentTable;
    Ptr<Legacy::Nvx2OrderFreeStreamReader> nvx2Reader;
    bool IsGuiScene(const Ptr<IO::BinaryReader>& reader);
    void ConvertGuiScene(const Ptr<IO::BinaryReader>& reader, const Ptr<ModelWriter>& writer);
    Math::float4 ReadUvCoordinates(const IO::URI& uri, IndexT meshGroup);
    void WriteXmlGuiNode(const Ptr<IO::XmlWriter>& writer, Util::SimpleTree<GuiElement>::Node& node);
    Util::String MapGuiType(const Util::String& n2Type);
    void ComputeGUINodeDimensions(Util::SimpleTree<GuiElement>::Node& node);
};
inline void
N2Converter::SetVerbose(bool b)
{
    this->verbose = b;
}
inline void
N2Converter::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
inline void
N2Converter::SetSrcDir(const Util::String& dir)
{
    this->srcDir = dir;
}
inline void
N2Converter::SetDstDir(const Util::String& dir)
{
    this->dstDir = dir;
}
inline void
N2Converter::SetMeshDir(const Util::String& dir)
{
    this->mshDir = dir;
}
inline void
N2Converter::SetForceFlag(bool b)
{
    this->force = b;
}
inline void
N2Converter::SetBinaryFlag(bool b)
{
    this->binary = b;
}
inline void
N2Converter::ResetUsedResources()
{
    this->usedResources.Clear();
}
inline const Util::Array<Util::String>&
N2Converter::GetUsedResources() const
{
    return this->usedResources;
}
inline void
N2Converter::AddUsedResource(const Util::String& resId)
{
    if (InvalidIndex == this->usedResources.BinarySearchIndex(resId))
    {
        this->usedResources.InsertSorted(resId);
    }
}
inline const Ptr<N2ReflectionInfo>&
N2Converter::GetN2ReflectionInfo() const
{
    return this->n2ReflectionInfo;
}
}