#pragma once
#include "util/array.h"
#include "util/string.h"
#include "math/vec4.h"
#include "math/mat4.h"
namespace Util
{
class CommandLineArgs
{
public:
    CommandLineArgs();
    CommandLineArgs(const String& cmdLine);
    CommandLineArgs(int argc, const char** argv);
    const String& GetCmdName() const;
    bool HasArg(const String& arg, const IndexT start = 0) const;
    const String& GetString(const String& name, const String& defaultValue = "") const;
    Util::Array<Util::String> GetStrings(const String& name, const String& defaultValue = "") const;
    int GetInt(const String& name, int defaultValue = 0) const;
    Util::Array<int> GetInts(const String& name, int defaultValue = 0) const;
    float GetFloat(const String& name, float defaultValue = 0.0f) const;
    Util::Array<float> GetFloats(const String& name, float defaultValue = 0.0f) const;
    bool GetBool(const String& name, bool defaultValue=false) const;
    Util::Array<bool> GetBools(const String& name, bool defaultValue = false) const;
    bool GetBoolFlag(const String& name) const;
    #if !__OSX__
    Math::vec4 GetVec4(const String& name, const Math::vec4& defaultValue = Math::vec4()) const;
    Util::Array<Math::vec4> GetVec4s(const String& name, const Math::vec4& defaultValue = Math::vec4()) const;
    Math::mat4 GetMat4(const String& name, const Math::mat4& defaultValue = Math::mat4()) const;
    Util::Array<Math::mat4> GetMat4s(const String& name, const Math::mat4& defaultValue = Math::mat4()) const;
    #endif
    bool IsEmpty() const;
    SizeT GetNumArgs() const;
    const String& GetStringAtIndex(IndexT index) const;
    int GetIntAtIndex(IndexT index) const;
    float GetFloatAtIndex(IndexT index) const;
    bool GetBoolAtIndex(IndexT index) const;
    #if !__OSX__
    Math::vec4 GetVec4AtIndex(IndexT index) const;
    Math::mat4 GetMat4AtIndex(IndexT index) const;
    #endif
    void AppendCommandString(const CommandLineArgs& otherArgs);
    const Dictionary<String, String>& GetPairs() const;
private:
    void Error(const String& argName) const;
    int FindArgIndex(const String& argName, const IndexT start = 0) const;
    void ParseIntoGroups(const Array<String>& Tokenized);
    Array<String> args;
    Dictionary<String,String> keyPairs;
};
inline bool
CommandLineArgs::IsEmpty() const
{
    return this->args.Size() == 0;
}
inline const Dictionary<String, String>&
CommandLineArgs::GetPairs() const
{
    return this->keyPairs;
}
}