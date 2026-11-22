#pragma once
#include "core/types.h"
namespace Messaging
{
class Id
{
public:
    Id();
    bool operator==(const Id& rhs) const;
};
inline
Id::Id()
{
}
inline
bool
Id::operator==(const Id& rhs) const
{
    return (this == &rhs);
}
}