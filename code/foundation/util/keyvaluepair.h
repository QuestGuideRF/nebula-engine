#pragma once
#include "core/types.h"
namespace Util
{
template<class KEYTYPE, class VALUETYPE> class KeyValuePair
{
public:
    KeyValuePair();
    KeyValuePair(const KEYTYPE& k, const VALUETYPE& v);
    explicit KeyValuePair(const KEYTYPE& k);
    KeyValuePair(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs);
    KeyValuePair(KeyValuePair<KEYTYPE, VALUETYPE>&& rhs) noexcept;
    void operator=(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs);
    void operator=(KeyValuePair<KEYTYPE, VALUETYPE>&& rhs) noexcept;
    bool operator==(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const;
    bool operator!=(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const;
    bool operator>(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const;
    bool operator>=(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const;
    bool operator<(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const;
    bool operator<=(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const;
    bool operator==(const KEYTYPE& rhs) const;
    bool operator!=(const KEYTYPE& rhs) const;
    bool operator>(const KEYTYPE& rhs) const;
    bool operator>=(const KEYTYPE& rhs) const;
    bool operator<(const KEYTYPE& rhs) const;
    bool operator<=(const KEYTYPE& rhs) const;
    VALUETYPE& Value();
    const KEYTYPE& Key() const;
    const VALUETYPE& Value() const;
    KEYTYPE keyData = KEYTYPE();
    VALUETYPE valueData = VALUETYPE();
};
template<class KEYTYPE, class VALUETYPE>
KeyValuePair<KEYTYPE, VALUETYPE>::KeyValuePair()
{
}
template<class KEYTYPE, class VALUETYPE>
KeyValuePair<KEYTYPE, VALUETYPE>::KeyValuePair(const KEYTYPE& k, const VALUETYPE& v) :
    keyData(k),
    valueData(v)
{
}
template<class KEYTYPE, class VALUETYPE>
KeyValuePair<KEYTYPE, VALUETYPE>::KeyValuePair(const KEYTYPE& k) :
    keyData(k)
{
}
template<class KEYTYPE, class VALUETYPE>
KeyValuePair<KEYTYPE, VALUETYPE>::KeyValuePair(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) :
    keyData(rhs.keyData),
    valueData(rhs.valueData)
{
}
template<class KEYTYPE, class VALUETYPE>
KeyValuePair<KEYTYPE, VALUETYPE>::KeyValuePair(KeyValuePair<KEYTYPE, VALUETYPE>&& rhs) noexcept:
    keyData(std::move(rhs.keyData)),
    valueData(std::move(rhs.valueData))
{
}
template<class KEYTYPE, class VALUETYPE>
void
KeyValuePair<KEYTYPE, VALUETYPE>::operator=(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs)
{
    this->keyData = rhs.keyData;
    this->valueData = rhs.valueData;
}
template<class KEYTYPE, class VALUETYPE>
void
KeyValuePair<KEYTYPE, VALUETYPE>::operator=(KeyValuePair<KEYTYPE, VALUETYPE>&& rhs) noexcept
{
    this->keyData = std::move(rhs.keyData);
    this->valueData = std::move(rhs.valueData);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator==(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const
{
    return (this->keyData == rhs.keyData);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator!=(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const
{
    return (this->keyData != rhs.keyData);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator>(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const
{
    return (this->keyData > rhs.keyData);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator>=(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const
{
    return (this->keyData >= rhs.keyData);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator<(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const
{
    return (this->keyData < rhs.keyData);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator<=(const KeyValuePair<KEYTYPE, VALUETYPE>& rhs) const
{
    return (this->keyData <= rhs.keyData);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator==(const KEYTYPE& rhs) const
{
    return (this->keyData == rhs);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator!=(const KEYTYPE& rhs) const
{
    return (this->keyData != rhs);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator>(const KEYTYPE& rhs) const
{
    return (this->keyData > rhs);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator>=(const KEYTYPE& rhs) const
{
    return (this->keyData >= rhs);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator<(const KEYTYPE& rhs) const
{
    return (this->keyData < rhs);
}
template<class KEYTYPE, class VALUETYPE>
bool
KeyValuePair<KEYTYPE, VALUETYPE>::operator<=(const KEYTYPE& rhs) const
{
    return (this->keyData <= rhs);
}
template<class KEYTYPE, class VALUETYPE>
VALUETYPE&
KeyValuePair<KEYTYPE, VALUETYPE>::Value()
{
    return this->valueData;
}
template<class KEYTYPE, class VALUETYPE>
const KEYTYPE&
KeyValuePair<KEYTYPE, VALUETYPE>::Key() const
{
    return this->keyData;
}
template<class KEYTYPE, class VALUETYPE>
const VALUETYPE&
KeyValuePair<KEYTYPE, VALUETYPE>::Value() const
{
    return this->valueData;
}
}