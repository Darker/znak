#include <cstddef>
#include <algorithm>

#include "helpers.h"

namespace znak
{

template <char expected>
struct CharacterSimple
{
    constexpr bool matches(char character)
    {
        return character == expected;
    }
};

template<StringLiteral characters>
struct CharacterSet
{
    constexpr bool maches(const char character)
    {
        for(const char one : characters.value)
        {
            if(one == character)
                return true;
        }
        return false;
    }
};

template <class ...Args>
struct Sequence
{
    constexpr bool matches(const char character)
    {
        
    }
}

static_assert(CharacterSet<"abc">().maches('a'));
static_assert(!CharacterSet<"abc">().maches('5'));
}

