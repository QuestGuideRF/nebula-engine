#pragma once
#include "attributeid.h"
namespace MemDb
{
class FilterSet
{
public:
    FilterSet() = default;
    FilterSet(const std::initializer_list<AttributeId> inclusive, const std::initializer_list<AttributeId> exclusive);
    explicit FilterSet(const Util::FixedArray<AttributeId>& inclusive);
    explicit FilterSet(const Util::FixedArray<AttributeId>& inclusive, const Util::FixedArray<AttributeId>& exclusive);
    explicit FilterSet(const TableSignature& inclusive, const TableSignature& exclusive, const Util::FixedArray<AttributeId>& inclusiveComponents);
    TableSignature const& Inclusive() const;
    TableSignature const& Exclusive() const;
    Util::FixedArray<AttributeId> const& PropertyIds() const;
private:
    TableSignature inclusive;
    TableSignature exclusive;
    Util::FixedArray<AttributeId> inclusiveComponents;
};
inline
FilterSet::FilterSet(const std::initializer_list<AttributeId> inclusive, const std::initializer_list<AttributeId> exclusive) :
    inclusive(inclusive),
    exclusive(exclusive),
    inclusiveComponents(inclusive)
{
}
inline
FilterSet::FilterSet(const Util::FixedArray<AttributeId>& inclusive) :
    inclusive(inclusive),
    exclusive(),
    inclusiveComponents(inclusive)
{
};
inline
FilterSet::FilterSet(const Util::FixedArray<AttributeId>& inclusive, const Util::FixedArray<AttributeId>& exclusive) :
    inclusive(inclusive),
    exclusive(exclusive),
    inclusiveComponents(inclusive)
{
}
inline
FilterSet::FilterSet(const TableSignature& inclusive, const TableSignature& exclusive, const Util::FixedArray<AttributeId>& inclusiveComponents) :
    inclusive(inclusive),
    exclusive(exclusive),
    inclusiveComponents(inclusiveComponents)
{
}
;
inline TableSignature const&
FilterSet::Inclusive() const
{
    return this->inclusive;
}
inline TableSignature const&
FilterSet::Exclusive() const
{
    return this->exclusive;
}
inline Util::FixedArray<AttributeId> const&
FilterSet::PropertyIds() const
{
    return this->inclusiveComponents;
}
}