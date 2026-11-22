#pragma once
#include "util/string.h"
namespace ToolkitUtil
{
enum class TextColor
{
    Default,
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};
enum class FontMode
{
    Normal,
    Bold,
    Dim,
    Italic,
    Underline,
    Blink,
    Inverse,
    Hidden,
    StrikeThrough
};
class Text
{
public:
    Text(const char* string, size_t size);
    Text(const Util::String& string);
    ~Text();
    Text& Color(const TextColor color);
    Text& Style(const FontMode mode);
    Text& CursorUp(SizeT lines);
    Text& CursorDown(SizeT lines);
    Text& CursorMove(SizeT line);
    Text& CursorEraseLine();
    const Util::String AsString();
    const char* AsCharPtr();
private:
    Util::String format;
    Util::String cursor;
    Util::String commands;
    Util::String string;
    Util::String formattedString;
};
extern Text operator""_text(const char* str, size_t size);
}