#pragma once
#include "attrid.h"
namespace Attr
{
DeclareGuid(Guid, 'auid', AccessMode::ReadOnly);
DeclareString(Name, 'anme', AccessMode::ReadWrite);
}