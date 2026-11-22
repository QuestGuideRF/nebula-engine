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
#include <ctype.h>
#include <stddef.h>

//#define DEBUG_PARSER
#if defined( DEBUG_PARSER )
#   if defined( DEBUG ) && defined( _MSC_VER )
#       include <windows.h>
#       define TIXML_LOG OutputDebugString
#   else
#       define TIXML_LOG printf
#   endif
#endif

// Обратите внимание, что «PutString» жестко закодирует тот же список. Этот
// менее гибок, чем кажется. Изменение записей
// или ордер нарушит пут-стринг.   
TiXmlBase::Entity TiXmlBase::entity[ NUM_ENTITY ] = 
{
    { "&amp;",  5, '&' },
    { "&lt;",   4, '<' },
    { "&gt;",   4, '>' },
    { "&quot;", 6, '\"' },
    { "&апос;", 6, '\'' }
};

// Куча информации о Юникоде по адресу:
//      http://www.unicode.org/faq/utf_bom.html
// Включая основную часть этой таблицы, которая определяет #байты в
// последовательность из ведущего байта. 1 место для недопустимых последовательностей --
// хотя результат будет мусорным, пропустите его как можно больше.
// Остерегайтесь несимволов в UTF-8:   
//              ef bb bf («ведущие байты» Microsoft)
//              эф быть бойфрендом
//              эф, парень, парень 

const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

const int TiXmlBase::utf8ByteTable[256] = 
{
    //  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x00
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x10
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x20
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x30
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x40
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x50
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x60
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x70 Конец диапазона ASCII
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x80 от 0x80 до 0xc1 недействительно
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0x90 
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0xa0 
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0xb0 
        1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  // 0xc0 от 0xc2 до 0xdf 2 байта
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  // 0xd0
        3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  // 0xe0 от 0xe0 до 0xef 3 байта
        4,  4,  4,  4,  4,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // 0xf0 от 0xf0 до 0xf4 4 байта, 0xf5 и выше недействительны
};


void TiXmlBase::ConvertUTF32ToUTF8( unsigned long input, char* output, int* length )
{
    const unsigned long BYTE_MASK = 0xBF;
    const unsigned long BYTE_MARK = 0x80;
    const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    if (input < 0x80) 
        *length = 1;
    else if ( input < 0x800 )
        *length = 2;
    else if ( input < 0x10000 )
        *length = 3;
    else if ( input < 0x200000 )
        *length = 4;
    else
        { *length = 0; return; }    // В любом случае этот код не будет правильно это отражать.

    output += *length;

    // Страшно, страшно, провалиться.
    switch (*length) 
    {
        case 4:
            --output; 
            *output = (char)((input | BYTE_MARK) & BYTE_MASK); 
            input >>= 6;
        case 3:
            --output; 
            *output = (char)((input | BYTE_MARK) & BYTE_MASK); 
            input >>= 6;
        case 2:
            --output; 
            *output = (char)((input | BYTE_MARK) & BYTE_MASK); 
            input >>= 6;
        case 1:
            --output; 
            *output = (char)(input | FIRST_BYTE_MARK[*length]);
    }
}


/*static*/ int TiXmlBase::IsAlpha( unsigned char anyByte, TiXmlEncoding /*encoding*/ )
{
    // Это будет работать только для low-ascii, все остальное считается допустимым.
    // письмо. Я не уверен, что это лучший подход, но попробовать его довольно сложно.
    // чтобы выяснить алфавитное и неперекодированное кодирование. Так что возьмите очень 
    // консервативный подход.

//  if ( encoding == TIXML_ENCODING_UTF8 )
//  {
        if ( anyByte < 127 )
            return isalpha( anyByte );
        else
            return 1;   // Что еще делать? Набор юникода огромен... поймите английский правильно.
//  }
//  else
//  {
//      return isalpha( anyByte );
//  }
}


/*static*/ int TiXmlBase::IsAlphaNum( unsigned char anyByte, TiXmlEncoding /*encoding*/ )
{
    // Это будет работать только для low-ascii, все остальное считается допустимым.
    // письмо. Я не уверен, что это лучший подход, но попробовать его довольно сложно.
    // чтобы выяснить алфавитное и неперекодированное кодирование. Так что возьмите очень 
    // консервативный подход.

//  if ( encoding == TIXML_ENCODING_UTF8 )
//  {
        if ( anyByte < 127 )
            return isalnum( anyByte );
        else
            return 1;   // Что еще делать? Набор юникода огромен... поймите английский правильно.
//  }
//  else
//  {
//      return isalnum( anyByte );
//  }
}


class TiXmlParsingData
{
    friend class TiXmlDocument;
  public:
    void Stamp( const char* now, TiXmlEncoding encoding );

    const TiXmlCursor& Cursor() { return cursor; }

  private:
    // Используется только документом!
    TiXmlParsingData( const char* start, int _tabsize, int row, int col )
    {
        assert( start );
        stamp = start;
        tabsize = _tabsize;
        cursor.row = row;
        cursor.col = col;
    }

    TiXmlCursor     cursor;
    const char*     stamp;
    int             tabsize;
};


void TiXmlParsingData::Stamp( const char* now, TiXmlEncoding encoding )
{
    assert( now );

    // Ничего не делайте, если размер табуляции равен 0.
    if ( tabsize < 1 )
    {
        return;
    }

    // Получить текущую строку, столбец.
    int row = cursor.row;
    int col = cursor.col;
    const char* p = stamp;
    assert( p );

    while ( p < now )
    {
        // Считайте p беззнаковым, и у нас будет счастливый компилятор.
        const unsigned char* pU = (const unsigned char*)p;

        // Код предоставлен Флетчером Данном: (изменен Ли)
        switch (*pU) {
            case 0:
                // Мы *должны* никогда не попасть сюда, но если мы это сделаем, не надо.
                // пройти мимо завершающего нулевого символа, когда-либо
                return;

            case '\r':
                // перейти на следующую строку
                ++row;
                col = 0;                
                // Съешь персонажа
                ++p;

                // Проверьте последовательность \r\n и рассматривайте ее как один символ.
                if (*p == '\n') {
                    ++p;
                }
                break;

            case '\n':
                // перейти на следующую строку
                ++row;
                col = 0;

                // Съешь персонажа
                ++p;

                // Проверьте последовательность \n\r и рассматривайте ее как одну
                // характер.  (Да, эта странная вещь происходит до сих пор.
                // на каких-то загадочных платформах...)
                if (*p == '\r') {
                    ++p;
                }
                break;

            case '\t':
                // Съешь персонажа
                ++p;

                // Перейти к следующей позиции табуляции
                col = (col / tabsize + 1) * tabsize;
                break;

            case TIXML_UTF_LEAD_0:
                if ( encoding == TIXML_ENCODING_UTF8 )
                {
                    if ( *(p+1) && *(p+2) )
                    {
                        // В этих случаях не продвигайте колонну вперед. Это
                        // Пробелы шириной 0.
                        if ( *(pU+1)==TIXML_UTF_LEAD_1 && *(pU+2)==TIXML_UTF_LEAD_2 )
                            p += 3; 
                        else if ( *(pU+1)==0xbfU && *(pU+2)==0xbeU )
                            p += 3; 
                        else if ( *(pU+1)==0xbfU && *(pU+2)==0xbfU )
                            p += 3; 
                        else
                            { p +=3; ++col; }   // Нормальный персонаж.
                    }
                }
                else
                {
                    ++p;
                    ++col;
                }
                break;

            default:
                if ( encoding == TIXML_ENCODING_UTF8 )
                {
                    // Съешьте символ utf8 длиной от 1 до 4 байт.
                    int step = TiXmlBase::utf8ByteTable[*((unsigned char*)p)];
                    if ( step == 0 )
                        step = 1;       // Случай ошибки из-за неправильного кодирования, но обрабатывайте его корректно.
                    p += step;

                    // Конечно, просто продвиньте одну колонку вперед.
                    ++col;
                }
                else
                {
                    ++p;
                    ++col;
                }
                break;
        }
    }
    cursor.row = row;
    cursor.col = col;
    assert( cursor.row >= -1 );
    assert( cursor.col >= -1 );
    stamp = p;
    assert( stamp );
}


const char* TiXmlBase::SkipWhiteSpace( const char* p, TiXmlEncoding encoding )
{
    if ( !p || !*p )
    {
        return 0;
    }
    if ( encoding == TIXML_ENCODING_UTF8 )
    {
        while ( *p )
        {
            const unsigned char* pU = (const unsigned char*)p;
            
            // Пропустите глупые знаки порядка байтов Microsoft UTF-8.
            if (    *(pU+0)==TIXML_UTF_LEAD_0
                 && *(pU+1)==TIXML_UTF_LEAD_1 
                 && *(pU+2)==TIXML_UTF_LEAD_2 )
            {
                p += 3;
                continue;
            }
            else if(*(pU+0)==TIXML_UTF_LEAD_0
                 && *(pU+1)==0xbfU
                 && *(pU+2)==0xbeU )
            {
                p += 3;
                continue;
            }
            else if(*(pU+0)==TIXML_UTF_LEAD_0
                 && *(pU+1)==0xbfU
                 && *(pU+2)==0xbfU )
            {
                p += 3;
                continue;
            }

            if ( IsWhiteSpace( *p ) || *p == '\n' || *p =='\r' )        // Все еще использую старые правила для пустого пространства.
                ++p;
            else
                break;
        }
    }
    else
    {
        while ( *p && IsWhiteSpace( *p ) || *p == '\n' || *p =='\r' )
            ++p;
    }

    return p;
}

#ifdef TIXML_USE_STL
/*static*/ bool TiXmlBase::StreamWhiteSpace( TIXML_ISTREAM * in, TIXML_STRING * tag )
{
    for( ;; )
    {
        if ( !in->good() ) return false;

        int c = in->peek();
        // В этой области мы не можем добраться до документа. Так что терпите неудачу молча.
        if ( !IsWhiteSpace( c ) || c <= 0 )
            return true;

        *tag += (char) in->get();
    }
}

/*static*/ bool TiXmlBase::StreamTo( TIXML_ISTREAM * in, int character, TIXML_STRING * tag )
{
    //assert( character > 0 && character < 128 );   // else it won't work in utf-8
    while ( in->good() )
    {
        int c = in->peek();
        if ( c == character )
            return true;
        if ( c <= 0 )       // Тихий сбой: невозможно получить документ в этой области
            return false;

        in->get();
        *tag += (char) c;
    }
    return false;
}
#endif

const char* TiXmlBase::ReadName( const char* p, TIXML_STRING * name, TiXmlEncoding encoding )
{
    *name = "";
    assert( p );

    // Имена начинаются с букв или подчеркиваний.
    // Конечно, в Юникоде tinyxml понятия не имеет, что такое буква.
    // алгоритм щедрый.
    //
    // После этого это могут быть буквы, символы подчеркивания, цифры,
    // дефисы или двоеточия. (Двоеточия действительны только для пространств имен,
    // но tinyxml не может отличать пространства имен от имен.)
    if (    p && *p 
         && ( IsAlpha( (unsigned char) *p, encoding ) || *p == '_' ) )
    {
        while(      p && *p
                &&  (       IsAlphaNum( (unsigned char ) *p, encoding ) 
                         || *p == '_'
                         || *p == '-'
                         || *p == '.'
                         || *p == ':' ) )
        {
            (*name) += *p;
            ++p;
        }
        return p;
    }
    return 0;
}

const char* TiXmlBase::GetEntity( const char* p, char* value, int* length, TiXmlEncoding encoding )
{
    // Предположите сущность и вытащите ее.
    TIXML_STRING ent;
    int i;
    *length = 0;

    if ( *(p+1) && *(p+1) == '#' && *(p+2) )
    {
        unsigned long ucs = 0;
        ptrdiff_t delta = 0;
        unsigned mult = 1;

        if ( *(p+2) == 'x' )
        {
            // Hexadecimal.
            if ( !*(p+3) ) return 0;

            const char* q = p+3;
            q = strchr( q, ';' );

            if ( !q || !*q ) return 0;

            delta = q-p;
            --q;

            while ( *q != 'x' )
            {
                if ( *q >= '0' && *q <= '9' )
                    ucs += mult * (*q - '0');
                else if ( *q >= 'a' && *q <= 'f' )
                    ucs += mult * (*q - 'a' + 10);
                else if ( *q >= 'A' && *q <= 'F' )
                    ucs += mult * (*q - 'A' + 10 );
                else 
                    return 0;
                mult *= 16;
                --q;
            }
        }
        else
        {
            // Decimal.
            if ( !*(p+2) ) return 0;

            const char* q = p+2;
            q = strchr( q, ';' );

            if ( !q || !*q ) return 0;

            delta = q-p;
            --q;

            while ( *q != '#' )
            {
                if ( *q >= '0' && *q <= '9' )
                    ucs += mult * (*q - '0');
                else 
                    return 0;
                mult *= 10;
                --q;
            }
        }
        if ( encoding == TIXML_ENCODING_UTF8 )
        {
            // конвертировать UCS в UTF-8
            ConvertUTF32ToUTF8( ucs, value, length );
        }
        else
        {
            *value = (char)ucs;
            *length = 1;
        }
        return p + delta + 1;
    }

    // Теперь попробуйте соответствовать этому.
    for( i=0; i<NUM_ENTITY; ++i )
    {
        if ( strncmp( entity[i].str, p, entity[i].strLength ) == 0 )
        {
            assert( strlen( entity[i].str ) == entity[i].strLength );
            *value = entity[i].chr;
            *length = 1;
            return ( p + entity[i].strLength );
        }
    }

    // Так что это была не сущность, она непризнанная или что-то в этом роде.
    *value = *p;    // Не ставьте назад последний, так как мы его возвращаем!
    return p+1;
}


bool TiXmlBase::StringEqual( const char* p,
                             const char* tag,
                             bool ignoreCase,
                             TiXmlEncoding encoding )
{
    assert( p );
    assert( tag );
    if ( !p || !*p )
    {
        assert( 0 );
        return false;
    }

    const char* q = p;

    if ( ignoreCase )
    {
        while ( *q && *tag && ToLower( *q, encoding ) == ToLower( *tag, encoding ) )
        {
            ++q;
            ++tag;
        }

        if ( *tag == 0 )
            return true;
    }
    else
    {
        while ( *q && *tag && *q == *tag )
        {
            ++q;
            ++tag;
        }

        if ( *tag == 0 )        // Мы нашли конец тега и все равно?
            return true;
    }
    return false;
}

const char* TiXmlBase::ReadText(    const char* p, 
                                    TIXML_STRING * text, 
                                    bool trimWhiteSpace, 
                                    const char* endTag, 
                                    bool caseInsensitive,
                                    TiXmlEncoding encoding )
{
    *text = "";
    if (    !trimWhiteSpace         // некоторые теги всегда оставляют пробелы
         || !condenseWhiteSpace )   // if true, whitespace is always kept
    {
        // Сохраняйте все пустое пространство.
        while (    p && *p
                && !StringEqual( p, endTag, caseInsensitive, encoding )
              )
        {
            int len;
            char cArr[4] = { 0, 0, 0, 0 };
            p = GetChar( p, cArr, &len, encoding );
            text->append( cArr, len );
        }
    }
    else
    {
        bool whitespace = false;

        // Удалите ведущие пробелы:
        p = SkipWhiteSpace( p, encoding );
        while (    p && *p
                && !StringEqual( p, endTag, caseInsensitive, encoding ) )
        {
            if ( *p == '\r' || *p == '\n' )
            {
                whitespace = true;
                ++p;
            }
            else if ( IsWhiteSpace( *p ) )
            {
                whitespace = true;
                ++p;
            }
            else
            {
                // Если мы нашли пробел, добавьте его перед
                // новый персонаж. Любой пробел просто становится пробелом.
                if ( whitespace )
                {
                    (*text) += ' ';
                    whitespace = false;
                }
                int len;
                char cArr[4] = { 0, 0, 0, 0 };
                p = GetChar( p, cArr, &len, encoding );
                if ( len == 1 )
                    (*text) += cArr[0]; // более эффективный
                else
                    text->append( cArr, len );
            }
        }
    }
    return p + strlen( endTag );
}

#ifdef TIXML_USE_STL

void TiXmlDocument::StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag )
{
    // Основная проблема с документом заключается в том, что мы не знаем, что мы
    // потоковая передача. Прочитайте что-то, что предположительно является тегом (и надеждой), а затем
    // идентифицируйте его и вызовите соответствующий метод потока для тега.
    //
    // Эта «предварительная потоковая передача» никогда не будет читать закрывающий «>», поэтому
    // субтег может ориентироваться самостоятельно.

    if ( !StreamTo( in, '<', tag ) ) 
    {
        SetError( TIXML_ERROR_PARSING_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
        return;
    }

    while ( in->good() )
    {
        int tagIndex = (int) tag->length();
        while ( in->good() && in->peek() != '>' )
        {
            int c = in->get();
            if ( c <= 0 )
            {
                SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
                break;
            }
            (*tag) += (char) c;
        }

        if ( in->good() )
        {
            // Теперь у нас есть нечто, что мы считаем узлом 
            // какой-то. Определите его и вызовите узел, чтобы
            // продолжить трансляцию.
            TiXmlNode* node = Identify( tag->c_str() + tagIndex, TIXML_DEFAULT_ENCODING );

            if ( node )
            {
                node->StreamIn( in, tag );
                bool isElement = node->ToElement() != 0;
                delete node;
                node = 0;

                // Если это корневой элемент, все готово. Разбор будет
                // выполняется оператором >>.
                if ( isElement )
                {
                    return;
                }
            }
            else
            {
                SetError( TIXML_ERROR, 0, 0, TIXML_ENCODING_UNKNOWN );
                return;
            }
        }
    }
    // Нам следовало вернуться раньше.
    SetError( TIXML_ERROR, 0, 0, TIXML_ENCODING_UNKNOWN );
}

#endif

const char* TiXmlDocument::Parse( const char* p, TiXmlParsingData* prevData, TiXmlEncoding encoding )
{
    ClearError();

    // Анализируйте на уровне документа. Поскольку документ
    // не содержит ничего, кроме других тегов, большая часть того, что происходит
    // здесь пропуск пробелов.
    if ( !p || !*p )
    {
        SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
        return 0;
    }

    // Обратите внимание, что для документа это должно быть
    // перед пропуском пробела, чтобы синтаксический анализ
    // начинается с указателя, который нам дан.
    location.Clear();
    if ( prevData )
    {
        location.row = prevData->cursor.row;
        location.col = prevData->cursor.col;
    }
    else
    {
        location.row = 0;
        location.col = 0;
    }
    TiXmlParsingData data( p, TabSize(), location.row, location.col );
    location = data.Cursor();

    if ( encoding == TIXML_ENCODING_UNKNOWN )
    {
        // Проверьте ведущие байты Microsoft UTF-8.
        const unsigned char* pU = (const unsigned char*)p;
        if (    *(pU+0) && *(pU+0) == TIXML_UTF_LEAD_0
             && *(pU+1) && *(pU+1) == TIXML_UTF_LEAD_1
             && *(pU+2) && *(pU+2) == TIXML_UTF_LEAD_2 )
        {
            encoding = TIXML_ENCODING_UTF8;
            useMicrosoftBOM = true;
        }
    }

    p = SkipWhiteSpace( p, encoding );
    if ( !p )
    {
        SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
        return 0;
    }

    while ( p && *p )
    {
        TiXmlNode* node = Identify( p, encoding );
        if ( node )
        {
            p = node->Parse( p, &data, encoding );
            LinkEndChild( node );
        }
        else
        {
            break;
        }

        // Мы получили информацию о кодировке?
        if (    encoding == TIXML_ENCODING_UNKNOWN
             && node->ToDeclaration() )
        {
            TiXmlDeclaration* dec = node->ToDeclaration();
            const char* enc = dec->Encoding();
            assert( enc );

            if ( *enc == 0 )
                encoding = TIXML_ENCODING_UTF8;
            else if ( StringEqual( enc, "UTF-8", true, TIXML_ENCODING_UNKNOWN ) )
                encoding = TIXML_ENCODING_UTF8;
            else if ( StringEqual( enc, "UTF8", true, TIXML_ENCODING_UNKNOWN ) )
                encoding = TIXML_ENCODING_UTF8; // неправильно, но будь милым
            else 
                encoding = TIXML_ENCODING_LEGACY;
        }

        p = SkipWhiteSpace( p, encoding );
    }

    // Здесь было пусто?
    if ( !firstChild ) {
        SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding );
        return 0;
    }

    // Все хорошо.
    return p;
}

void TiXmlDocument::SetError( int err, const char* pError, TiXmlParsingData* data, TiXmlEncoding encoding )
{   
    // Первая ошибка в цепочке точнее - больше не ставить!
    if ( error )
        return;

    assert( err > 0 && err < TIXML_ERROR_STRING_COUNT );
    error   = true;
    errorId = err;
    errorDesc = errorString[ errorId ];

    errorLocation.Clear();
    if ( pError && data )
    {
        data->Stamp( pError, encoding );
        errorLocation = data->Cursor();
    }
}


TiXmlNode* TiXmlNode::Identify( const char* p, TiXmlEncoding encoding )
{
    TiXmlNode* returnNode = 0;

    p = SkipWhiteSpace( p, encoding );
    if( !p || !*p || *p != '<' )
    {
        return 0;
    }

    TiXmlDocument* doc = GetDocument();
    p = SkipWhiteSpace( p, encoding );

    if ( !p || !*p )
    {
        return 0;
    }

    // Что это за вещь? 
    // – Элементы начинаются с буквы или подчеркивания, но xml зарезервирован.
    // - Комментарии: <!--
    // - Объявление: <?xml
    // - Все остальное tinyxml неизвестно.
    //

    const char* xmlHeader = { "<?xml" };
    const char* commentHeader = { "<!--" };
    const char* dtdHeader = { "<!" };
    const char* cdataHeader = { "<![CDATA[" };

    if ( StringEqual( p, xmlHeader, true, encoding ) )
    {
        #ifdef DEBUG_PARSER
            TIXML_LOG( "Объявление синтаксического анализа XML\n" );
        #endif
        returnNode = new TiXmlDeclaration();
    }
    else if ( StringEqual( p, commentHeader, false, encoding ) )
    {
        #ifdef DEBUG_PARSER
            TIXML_LOG( "Комментарий синтаксического анализа XML\n" );
        #endif
        returnNode = new TiXmlComment();
    }
    else if ( StringEqual( p, cdataHeader, false, encoding ) )
    {
        #ifdef DEBUG_PARSER
            TIXML_LOG( "Анализ XML CDATA\n" );
        #endif
        TiXmlText* text = new TiXmlText( "" );
        text->SetCDATA( true );
        returnNode = text;
    }
    else if ( StringEqual( p, dtdHeader, false, encoding ) )
    {
        #ifdef DEBUG_PARSER
            TIXML_LOG( "Анализ XML Неизвестно(1)\n" );
        #endif
        returnNode = new TiXmlUnknown();
    }
    else if (    IsAlpha( *(p+1), encoding )
              || *(p+1) == '_' )
    {
        #ifdef DEBUG_PARSER
            TIXML_LOG( "Элемент синтаксического анализа XML\n" );
        #endif
        returnNode = new TiXmlElement( "" );
    }
    else
    {
        #ifdef DEBUG_PARSER
            TIXML_LOG( "Анализ XML Неизвестно(2)\n" );
        #endif
        returnNode = new TiXmlUnknown();
    }

    if ( returnNode )
    {
        // Установите родителя, чтобы он мог сообщать об ошибках
        returnNode->parent = this;
    }
    else
    {
        if ( doc )
            doc->SetError( TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN );
    }
    return returnNode;
}

#ifdef TIXML_USE_STL

void TiXmlElement::StreamIn (TIXML_ISTREAM * in, TIXML_STRING * tag)
{
    // Нас вызывают с некоторым предварительным анализом. То есть что-то из «этого»
    // элемент находится в «теге». Идем дальше и транслируем до закрывающего ">"
    while( in->good() )
    {
        int c = in->get();
        if ( c <= 0 )
        {
            TiXmlDocument* document = GetDocument();
            if ( document )
                document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
            return;
        }
        (*tag) += (char) c ;
        
        if ( c == '>' )
            break;
    }

    if ( tag->length() < 3 ) return;

    // Хорошо... если мы тег «/>», то все готово. Мы прочитали полный тег.
    // Если нет, определите и транслируйте.

    if (    tag->at( tag->length() - 1 ) == '>' 
         && tag->at( tag->length() - 2 ) == '/' )
    {
        // Все хорошо!
        return;
    }
    else if ( tag->at( tag->length() - 1 ) == '>' )
    {
        // Это еще не все. Может быть:
        //      text
        //      закрывающий тег
        //      еще один узел.
        for ( ;; )
        {
            StreamWhiteSpace( in, tag );

            // У нас есть текст?
            if ( in->good() && in->peek() != '<' ) 
            {
                // Да, текст.
                TiXmlText text( "" );
                text.StreamIn( in, tag );

                // За текстом следует закрывающий тег или другой узел.
                // Пройдите еще раз и разберитесь.
                continue;
            }

            // Теперь у нас есть либо закрывающий тег... либо другой узел.
            // В любом случае мы должны быть в положении «<».
            if ( !in->good() ) return;
            assert( in->peek() == '<' );
            int tagIndex = (int) tag->length();

            bool closingTag = false;
            bool firstCharFound = false;

            for( ;; )
            {
                if ( !in->good() )
                    return;

                int c = in->peek();
                if ( c <= 0 )
                {
                    TiXmlDocument* document = GetDocument();
                    if ( document )
                        document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
                    return;
                }
                
                if ( c == '>' )
                    break;

                *tag += (char) c;
                in->get();

                if ( !firstCharFound && c != '<' && !IsWhiteSpace( c ) )
                {
                    firstCharFound = true;
                    if ( c == '/' )
                        closingTag = true;
                }
            }
            // Если это был закрывающий тег, прочитайте закрывающий «>», чтобы очистить входной поток.
            // Если этого не было, то стриминг будет осуществляться по тегу.
            if ( closingTag )
            {
                if ( !in->good() )
                    return;

                int c = in->get();
                if ( c <= 0 )
                {
                    TiXmlDocument* document = GetDocument();
                    if ( document )
                        document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
                    return;
                }
                assert( c == '>' );
                *tag += (char) c;

                // Мы закончили, как только нашли закрывающий тег.
                return;
            }
            else
            {
                // Если это не закрывающий тег, укажите его и выполните потоковую передачу.
                const char* tagloc = tag->c_str() + tagIndex;
                TiXmlNode* node = Identify( tagloc, TIXML_DEFAULT_ENCODING );
                if ( !node )
                    return;
                node->StreamIn( in, tag );
                delete node;
                node = 0;

                // No return: go around from the beginning: text, closing tag, or node.
            }
        }
    }
}
#endif

const char* TiXmlElement::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
    p = SkipWhiteSpace( p, encoding );
    TiXmlDocument* document = GetDocument();

    if ( !p || !*p )
    {
        if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, 0, 0, encoding );
        return 0;
    }

    if ( data )
    {
        data->Stamp( p, encoding );
        location = data->Cursor();
    }

    if ( *p != '<' )
    {
        if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, p, data, encoding );
        return 0;
    }

    p = SkipWhiteSpace( p+1, encoding );

    // Прочитайте имя.
    const char* pErr = p;

    p = ReadName( p, &value, encoding );
    if ( !p || !*p )
    {
        if ( document ) document->SetError( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, encoding );
        return 0;
    }

    TIXML_STRING endTag ("</");
    endTag += value;
    endTag += ">";

    // Проверьте и прочитайте атрибуты. Также ищите пустой
    // тег или закрывающий тег.
    while ( p && *p )
    {
        pErr = p;
        p = SkipWhiteSpace( p, encoding );
        if ( !p || !*p )
        {
            if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding );
            return 0;
        }
        if ( *p == '/' )
        {
            ++p;
            // Пустой тег.
            if ( *p  != '>' )
            {
                if ( document ) document->SetError( TIXML_ERROR_PARSING_EMPTY, p, data, encoding );     
                return 0;
            }
            return (p+1);
        }
        else if ( *p == '>' )
        {
            // Done with attributes (if there were any.)
            // Прочтите значение, которое может включать в себя другие
            // элементы — прочитайте конечный тег и верните.
            ++p;
            p = ReadValue( p, data, encoding );     // Обратите внимание, что это метод Element, и если он произойдет, он установит ошибку.
            if ( !p || !*p )
                return 0;

            // Теперь нам нужно найти конечный тег
            if ( StringEqual( p, endTag.c_str(), false, encoding ) )
            {
                p += endTag.length();
                return p;
            }
            else
            {
                if ( document ) document->SetError( TIXML_ERROR_READING_END_TAG, p, data, encoding );
                return 0;
            }
        }
        else
        {
            // Попробуйте прочитать атрибут:
            TiXmlAttribute* attrib = new TiXmlAttribute();
            if ( !attrib )
            {
                if ( document ) document->SetError( TIXML_ERROR_OUT_OF_MEMORY, pErr, data, encoding );
                return 0;
            }

            attrib->SetDocument( document );
            const char* pErr2 = p;
            p = attrib->Parse( p, data, encoding );

            if ( !p || !*p )
            {
                if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, pErr2, data, encoding );
                delete attrib;
                return 0;
            }

            // Обработайте странный случай двойных атрибутов:
            TiXmlAttribute* node = attributeSet.Find( attrib->NameTStr() );
            if ( node )
            {
                node->SetValue( attrib->Value() );
                delete attrib;
                return 0;
            }

            attributeSet.Add( attrib );
        }
    }
    return p;
}


const char* TiXmlElement::ReadValue( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
    TiXmlDocument* document = GetDocument();

    // Read in text and elements in any order.
    const char* pWithWhiteSpace = p;
    p = SkipWhiteSpace( p, encoding );

    while ( p && *p )
    {
        if ( *p != '<' )
        {
            // Берем то, что у нас есть, делаем текстовый элемент.
            TiXmlText* textNode = new TiXmlText( "" );

            if ( !textNode )
            {
                if ( document ) document->SetError( TIXML_ERROR_OUT_OF_MEMORY, 0, 0, encoding );
                    return 0;
            }

            if ( TiXmlBase::IsWhiteSpaceCondensed() )
            {
                p = textNode->Parse( p, data, encoding );
            }
            else
            {
                // Особый случай: мы хотим сохранить пустое пространство
                // чтобы ведущие пробелы не удалялись.
                p = textNode->Parse( pWithWhiteSpace, data, encoding );
            }

            if ( !textNode->Blank() )
                LinkEndChild( textNode );
            else
                delete textNode;
        } 
        else 
        {
            // Мы нажимаем '<'
            // Мы попали в новый элемент или конечный тег? Это также может быть
            // TiXmlText в стиле «CDATA».
            if ( StringEqual( p, "</", false, encoding ) )
            {
                return p;
            }
            else
            {
                TiXmlNode* node = Identify( p, encoding );
                if ( node )
                {
                    p = node->Parse( p, data, encoding );
                    LinkEndChild( node );
                }               
                else
                {
                    return 0;
                }
            }
        }
        pWithWhiteSpace = p;
        p = SkipWhiteSpace( p, encoding );
    }

    if ( !p )
    {
        if ( document ) document->SetError( TIXML_ERROR_READING_ELEMENT_VALUE, 0, 0, encoding );
    }   
    return p;
}


#ifdef TIXML_USE_STL
void TiXmlUnknown::StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag )
{
    while ( in->good() )
    {
        int c = in->get();  
        if ( c <= 0 )
        {
            TiXmlDocument* document = GetDocument();
            if ( document )
                document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
            return;
        }
        (*tag) += (char) c;

        if ( c == '>' )
        {
            // Все хорошо.
            return;     
        }
    }
}
#endif


const char* TiXmlUnknown::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
    TiXmlDocument* document = GetDocument();
    p = SkipWhiteSpace( p, encoding );

    if ( data )
    {
        data->Stamp( p, encoding );
        location = data->Cursor();
    }
    if ( !p || !*p || *p != '<' )
    {
        if ( document ) document->SetError( TIXML_ERROR_PARSING_UNKNOWN, p, data, encoding );
        return 0;
    }
    ++p;
    value = "";

    while ( p && *p && *p != '>' )
    {
        value += *p;
        ++p;
    }

    if ( !p )
    {
        if ( document ) document->SetError( TIXML_ERROR_PARSING_UNKNOWN, 0, 0, encoding );
    }
    if ( *p == '>' )
        return p+1;
    return p;
}

#ifdef TIXML_USE_STL
void TiXmlComment::StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag )
{
    while ( in->good() )
    {
        int c = in->get();  
        if ( c <= 0 )
        {
            TiXmlDocument* document = GetDocument();
            if ( document )
                document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
            return;
        }

        (*tag) += (char) c;

        if ( c == '>' 
             && tag->at( tag->length() - 2 ) == '-'
             && tag->at( tag->length() - 3 ) == '-' )
        {
            // Все хорошо.
            return;     
        }
    }
}
#endif


const char* TiXmlComment::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
    TiXmlDocument* document = GetDocument();
    value = "";

    p = SkipWhiteSpace( p, encoding );

    if ( data )
    {
        data->Stamp( p, encoding );
        location = data->Cursor();
    }
    const char* startTag = "<!--";
    const char* endTag   = "-->";

    if ( !StringEqual( p, startTag, false, encoding ) )
    {
        document->SetError( TIXML_ERROR_PARSING_COMMENT, p, data, encoding );
        return 0;
    }
    p += strlen( startTag );
    p = ReadText( p, &value, false, endTag, false, encoding );
    return p;
}


const char* TiXmlAttribute::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
    p = SkipWhiteSpace( p, encoding );
    if ( !p || !*p ) return 0;

    int tabsize = 4;
    if ( document )
        tabsize = document->TabSize();

    if ( data )
    {
        data->Stamp( p, encoding );
        location = data->Cursor();
    }
    // Прочтите имя, знак «=» и значение.
    const char* pErr = p;
    p = ReadName( p, &name, encoding );
    if ( !p || !*p )
    {
        if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding );
        return 0;
    }
    p = SkipWhiteSpace( p, encoding );
    if ( !p || !*p || *p != '=' )
    {
        if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding );
        return 0;
    }

    ++p;    // пропустить '='
    p = SkipWhiteSpace( p, encoding );
    if ( !p || !*p )
    {
        if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding );
        return 0;
    }
    
    const char* end;

    if ( *p == '\'' )
    {
        ++p;
        end = "\'";
        p = ReadText( p, &value, false, end, false, encoding );
    }
    else if ( *p == '"' )
    {
        ++p;
        end = "\"";
        p = ReadText( p, &value, false, end, false, encoding );
    }
    else
    {
        // Все значения атрибутов должны быть заключены в одинарные или двойные кавычки.
        // Но это настолько распространенная ошибка, что парсер попытается
        // лучше даже без них.
        value = "";
        while (    p && *p                                      // existence
                && !IsWhiteSpace( *p ) && *p != '\n' && *p != '\r'  // whitespace
                && *p != '/' && *p != '>' )                     // конец дня
        {
            value += *p;
            ++p;
        }
    }
    return p;
}

#ifdef TIXML_USE_STL
void TiXmlText::StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag )
{
    if ( cdata )
    {
        int c = in->get();  
        if ( c <= 0 )
        {
            TiXmlDocument* document = GetDocument();
            if ( document )
                document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
            return;
        }

        (*tag) += (char) c;

        if ( c == '>' 
             && tag->at( tag->length() - 2 ) == ']'
             && tag->at( tag->length() - 3 ) == ']' )
        {
            // Все хорошо.
            return;     
        }
    }
    else
    {
        while ( in->good() )
        {
            int c = in->peek(); 
            if ( c == '<' )
                return;
            if ( c <= 0 )
            {
                TiXmlDocument* document = GetDocument();
                if ( document )
                    document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
                return;
            }

            (*tag) += (char) c;
            in->get();
        }
    }
}
#endif

const char* TiXmlText::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
    value = "";
    TiXmlDocument* document = GetDocument();

    if ( data )
    {
        data->Stamp( p, encoding );
        location = data->Cursor();
    }

    const char* const startTag = "<![CDATA[";
    const char* const endTag   = "]]>";

    if ( cdata || StringEqual( p, startTag, false, encoding ) )
    {
        cdata = true;

        if ( !StringEqual( p, startTag, false, encoding ) )
        {
            document->SetError( TIXML_ERROR_PARSING_CDATA, p, data, encoding );
            return 0;
        }
        p += strlen( startTag );

        // Сохраняйте все пробелы, игнорируйте кодировку и т. д.
        while (    p && *p
                && !StringEqual( p, endTag, false, encoding )
              )
        {
            value += *p;
            ++p;
        }

        TIXML_STRING dummy; 
        p = ReadText( p, &dummy, false, endTag, false, encoding );
        return p;
    }
    else
    {
        bool ignoreWhite = true;

        const char* end = "<";
        p = ReadText( p, &value, ignoreWhite, end, false, encoding );
        if ( p )
            return p-1; // не обрезайте '<'
        return 0;
    }
}

#ifdef TIXML_USE_STL
void TiXmlDeclaration::StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag )
{
    while ( in->good() )
    {
        int c = in->get();
        if ( c <= 0 )
        {
            TiXmlDocument* document = GetDocument();
            if ( document )
                document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
            return;
        }
        (*tag) += (char) c;

        if ( c == '>' )
        {
            // Все хорошо.
            return;
        }
    }
}
#endif

const char* TiXmlDeclaration::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding _encoding )
{
    p = SkipWhiteSpace( p, _encoding );
    // Найдите начало, найдите конец и ищите
    // вещи между ними.
    TiXmlDocument* document = GetDocument();
    if ( !p || !*p || !StringEqual( p, "<?xml", true, _encoding ) )
    {
        if ( document ) document->SetError( TIXML_ERROR_PARSING_DECLARATION, 0, 0, _encoding );
        return 0;
    }
    if ( data )
    {
        data->Stamp( p, _encoding );
        location = data->Cursor();
    }
    p += 5;

    version = "";
    encoding = "";
    standalone = "";

    while ( p && *p )
    {
        if ( *p == '>' )
        {
            ++p;
            return p;
        }

        p = SkipWhiteSpace( p, _encoding );
        if ( StringEqual( p, "version", true, _encoding ) )
        {
            TiXmlAttribute attrib;
            p = attrib.Parse( p, data, _encoding );     
            version = attrib.Value();
        }
        else if ( StringEqual( p, "encoding", true, _encoding ) )
        {
            TiXmlAttribute attrib;
            p = attrib.Parse( p, data, _encoding );     
            encoding = attrib.Value();
        }
        else if ( StringEqual( p, "standalone", true, _encoding ) )
        {
            TiXmlAttribute attrib;
            p = attrib.Parse( p, data, _encoding );     
            standalone = attrib.Value();
        }
        else
        {
            // Перечитайте, что бы это ни было.
            while( p && *p && *p != '>' && !IsWhiteSpace( *p ) )
                ++p;
        }
    }
    return 0;
}

bool TiXmlText::Blank() const
{
    for ( unsigned i=0; i<value.length(); i++ )
        if ( !IsWhiteSpace( value[i] ) )
            return false;
    return true;
}

