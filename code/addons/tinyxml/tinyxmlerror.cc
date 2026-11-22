/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2002 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#include "foundation/stdneb.h"
#include "tinyxml.h"

// Целью создания отдельного файла ошибок является создание первого
// шаг к локализации. tinyxml (на данный момент) поддерживает только
// Сообщения об ошибках на английском языке, но теперь их можно перевести.
//
// Это также немного очищает код.
//

const char* TiXmlBase::errorString[ TIXML_ERROR_STRING_COUNT ] =
{
    "Нет ошибки",
    "Error",
    "Не удалось открыть файл",
    "Не удалось выделить память.",
    "Ошибка анализа элемента.",
    "Не удалось прочитать имя элемента.",
    "Ошибка чтения значения элемента.",
    "Ошибка чтения атрибутов.",
    "Ошибка: пустой тег.",
    "Ошибка чтения закрывающего тега.",
    "Ошибка анализа Неизвестно.",
    "Ошибка анализа комментария.",
    "Ошибка анализа объявления.",
    "Документ об ошибке пуст.",
    "Error null (0) or unexpected EOF found in input stream.",
    "Ошибка анализа CDATA.",
};
