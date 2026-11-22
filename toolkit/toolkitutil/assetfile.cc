//------------------------------------------------------------------------------
//  assetfile.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "assetfile.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
AssetFile::AssetFile() :
    checksum(0),
    state(Unknown)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
AssetFile::SetPath(const StringAtom& p)
{
    this->path = p;
}

//------------------------------------------------------------------------------
/**
*/
const StringAtom&
AssetFile::GetPath() const
{
    return this->path;
}

//------------------------------------------------------------------------------
/**
*/
void 
AssetFile::SetTimeStamp(FileTime t)
{
    this->timeStamp = t;
}

//------------------------------------------------------------------------------
/**
*/
FileTime
AssetFile::GetTimeStamp() const
{
    return this->timeStamp;
}

//------------------------------------------------------------------------------
/**
*/
void
AssetFile::SetChecksum(uint crc)
{
    this->checksum = crc;
}

//------------------------------------------------------------------------------
/**
*/
uint
AssetFile::GetChecksum() const
{
    return this->checksum;
}

//------------------------------------------------------------------------------
/**
*/
void
AssetFile::SetState(State s)
{
    this->state = s;
}

//------------------------------------------------------------------------------
/**
*/
AssetFile::State
AssetFile::GetState() const
{
    return this->state;
}

} // пространство имен ToolkitUtil
