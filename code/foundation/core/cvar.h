#pragma once
namespace Core
{
struct CVar;
enum CVarType
{
    CVar_Int,
    CVar_Float,
    CVar_String
};
struct CVarCreateInfo
{
    const char* name = nullptr;
    const char* defaultValue = nullptr;
    CVarType type = CVar_Int;
    const char* description = nullptr;
};
CVar* CVarCreate(CVarCreateInfo const&);
CVar* CVarCreate(CVarType type, const char* name, const char* defaultValue, const char* description = nullptr);
CVar* CVarGet(const char* name);
void CVarParseWrite(CVar*, const char* value);
void CVarWriteFloat(CVar*, float value);
void CVarWriteInt(CVar*, int value);
void CVarWriteString(CVar*, const char* value);
int const CVarReadInt(CVar*);
float const CVarReadFloat(CVar*);
const char* CVarReadString(CVar*);
bool CVarModified(CVar*);
void CVarSetModified(CVar*, bool);
CVarType CVarGetType(CVar*);
const char* CVarGetName(CVar*);
const char* CVarGetDescription(CVar*);
int CVarNum();
CVar* CVarsBegin();
CVar* CVarsEnd();
CVar* CVarNext(CVar*);
}