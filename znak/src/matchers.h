#pragma once

#include "Matcher.h"

#include <string>

namespace znak
{

template <char expected>
struct CharacterSimple final : Matcher
{
    virtual bool matches(char character) const override
    {
        return character == expected;
    }

    virtual ~CharacterSimple() = default;
    virtual std::string getExpectations() const { return std::string(1, expected); }
};
template <char expected>
struct NotCharacter final : Matcher
{
    virtual bool matches(char character) const override
    {
        return character != expected;
    }
    virtual ~NotCharacter() = default;
    virtual std::string getExpectations() const { return "not:  "+std::string(1, expected); }
};


struct Anything final : Matcher
{
    virtual bool matches(char character) const override
    {
        return true;
    }
    virtual ~Anything() = default;
    virtual std::string getExpectations() const { return "Accepted anything and you still couldn't do that..."; }
};

struct CharacterSet : Matcher
{
    CharacterSet(const std::string expectedChars) : expected(expectedChars) {}
    CharacterSet(const CharacterSet& a, const CharacterSet& b) : expected(a.expected+b.expected) {}

    // You are responsible for that pointer, just to be clear
    CharacterSet* copy() const
    {
        return new CharacterSet(*this);
    }

    virtual bool matches(char character) const override
    {
        for(const auto& one : expected)
        {
            if(one == character)
                return true;
        }
        return false;
    }
    std::string expected;

    std::string getExpectations() const { return expected; }

    virtual ~CharacterSet() = default;
};

struct ExceptCharacters final : CharacterSet
{
    ExceptCharacters(const std::string unwantedChars) : CharacterSet(unwantedChars) {}
    ExceptCharacters(const CharacterSet& a, const CharacterSet& b) : CharacterSet(a, b) {}
    ExceptCharacters(const CharacterSet& a) : CharacterSet(a) {}

    virtual bool matches(char character) const override
    {
        return !CharacterSet::matches(character);
    }

    virtual ~ExceptCharacters() = default;
};

namespace chars
{
    static const CharacterSet LOWERCASE("abcdefghijklmnopqrstuvwxyz");
    static const CharacterSet UPPERCASE("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    static const CharacterSet LETTERS(LOWERCASE, UPPERCASE);
    static const CharacterSet NUMBERS("1234567890");
    static const CharacterSet OPERATORS("+-*/");
    static const CharacterSet WHITESPACE(" \t\r\n\v\f");
    static const CharacterSet ALPHANUMERIC(LETTERS, NUMBERS);
    static const CharacterSet WORD(ALPHANUMERIC, CharacterSet("_"));

    static const ExceptCharacters NON_WHITESPACE(WHITESPACE);
}
}