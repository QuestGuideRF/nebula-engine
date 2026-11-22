#pragma once
#include "core/refcounted.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "timing/time.h"
struct ufbx_node;
struct ufbx_matrix;
struct ufbx_vec4;
struct ufbx_vec3;
struct ufbx_vec2;
struct ufbx_anim_stack;
namespace ToolkitUtil
{
Math::mat4 FbxToMath(const ufbx_matrix& matrix);
Math::vec4 FbxToMath(const ufbx_vec4& vector);
Math::vec3 FbxToMath(const ufbx_vec3& vector);
Math::vec2 FbxToMath(const ufbx_vec2& vector);
class SceneNode;
class NFbxScene;
class NFbxMeshNode;
class ModelAttributes;
class AnimBuilder;
class AnimBuilderCurve;
class NFbxNode
{
public:
    static void Setup(SceneNode* node, SceneNode* parent, ufbx_node* fbxNode);
    static void ExtractAnimation(SceneNode* node, Util::Array<float>& keys, Util::Array<Timing::Tick>& keyTimes, ufbx_anim_stack* animStack);
    static void PrepareAnimation(SceneNode* node, ufbx_anim_stack* animStack);
protected:
    static void ExtractAnimationCurves(SceneNode* node, ufbx_node* fbxNode, Util::Array<float>& keys, Util::Array<Timing::Tick>& keyTimes, ufbx_anim_stack* animStack);
};
}