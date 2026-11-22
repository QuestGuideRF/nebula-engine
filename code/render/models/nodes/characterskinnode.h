#pragma once
#include "primitivenode.h"
#include "characternode.h"
#include "coregraphics/graphicsdevice.h"
namespace Models
{
class CharacterSkinNode : public PrimitiveNode
{
public:
    CharacterSkinNode();
    virtual ~CharacterSkinNode();
    void ReserveFragments(SizeT numFragments);
    void AddFragment(IndexT primGroupIndex, const Util::Array<IndexT>& jointPalette);
    SizeT GetNumFragments() const;
    IndexT GetFragmentPrimGroupIndex(IndexT fragmentIndex) const;
    const Util::Array<IndexT>& GetFragmentJointPalette(IndexT fragmentIndex) const;
private:
    class Fragment
    {
    public:
        Fragment() : primGroupIndex(InvalidIndex) {};
        IndexT primGroupIndex;
        Util::Array<IndexT> jointPalette;
    };
protected:
    friend class Characters::CharacterContext;
    virtual bool Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader, bool immediate) override;
    virtual void OnFinishedLoading(ModelStreamingData* streamingData) override;
    Util::Array<Fragment> skinFragments;
};
inline SizeT
CharacterSkinNode::GetNumFragments() const
{
    return this->skinFragments.Size();
}
inline IndexT
CharacterSkinNode::GetFragmentPrimGroupIndex(IndexT fragmentIndex) const
{
    return this->skinFragments[fragmentIndex].primGroupIndex;
}
inline const Util::Array<IndexT>&
CharacterSkinNode::GetFragmentJointPalette(IndexT fragmentIndex) const
{
    return this->skinFragments[fragmentIndex].jointPalette;
}
}