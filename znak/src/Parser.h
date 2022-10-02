#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "range.h"

namespace znak
{
struct Parser
{
    // return true if requirements of this part were met once the parser fails a check
    virtual bool valid() const = 0;
    // return true if after processing this character, this parser is still expecting more
    // if returns false, you must reuse the character, but the parser may be valid and have a value
    virtual bool check(size_t offset, char character) = 0;
    // reset the parser to initial state
    virtual void reset() = 0;
    // return the range that satisfied this parser, only valid on complete
    virtual range parsedRange() const = 0;
    // get the name for this parser
    virtual std::string getName() const = 0;
    // true means no values are currently represented (ie. since last reset, check was not called or returned false)
    virtual bool empty() const = 0;

    // // Parser may contain sub-parsers, this is how you access them
    // virtual const std::vector<
};
}