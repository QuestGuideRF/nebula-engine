#pragma once
#include "core/refcounted.h"
namespace Visibility
{
class VisibilityContainer : public Core::RefCounted
{
    __DeclareClass(VisibilityContainer);
public:
    VisibilityContainer();
    virtual ~VisibilityContainer();
private:
    Util::Array<bool> visible;
};
}