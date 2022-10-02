#pragma once

#include <string>

namespace znak
{

struct Matcher
{
    virtual bool matches(char character) const = 0;
    // returns a string representing what this matcher expects, only used for error reporting
    virtual std::string getExpectations() const = 0;

    virtual ~Matcher() = default;
};

}