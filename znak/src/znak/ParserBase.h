#pragma once

#include "Parser.h"

namespace znak
{

struct ParserBase : Parser
{
    ParserBase(const std::string& name) : name(name) {}

    virtual bool check(size_t offset, char character) override;
 
    // reset the parser to initial state
    virtual void reset() override
    {
        state = State::CLEAN;
        startIndex = length = 0;
    }

    virtual range parsedRange() const override
    {
        range res = {startIndex, startIndex+length, getName()};
        fillInnerRanges(res.nestedValues);
        return res;
    }

    virtual bool valid() const override
    {
        return state == State::DONE || state == State::READING;
    }

    virtual bool empty() const override
    {
        return length == 0;
    }

    virtual std::string getName() const override { return name; }

    protected:
    // If you return true, index will be advanced and parser continues
    // if you return false, complete() decides whether input is kept or discarded/failed
    // If you do not return true, it is assumed that you did not accept the character and it will be reused
    virtual bool shouldContinue(size_t offset, char character) = 0;
    // if you return true, input stops even if you said "yes" to should continue
    virtual bool complete() const = 0;
    // you can fill inner ranges if any
    virtual void fillInnerRanges(std::vector<range>&) const {}

    enum class State: char
    {
        CLEAN,
        READING,
        FAILED,
        DONE
    };
    State state = State::CLEAN;
    size_t startIndex = 0;
    size_t length = 0;
    std::string name;
};

inline bool ParserBase::check(size_t offset, char character)
{
    if(state == State::FAILED || state == State::DONE)
    {
        throw std::runtime_error("Parser fed more data than it accepts!");
    }
    if(state == State::CLEAN)
    {
        state = State::READING;
        startIndex = offset;
    }

    
    bool checkResult = shouldContinue(offset, character);
    if(checkResult) 
    {
        ++length;
        return true;
    }
    else {
        if(complete())
        {
            state = State::DONE;
        }
        else 
        {
            state = State::FAILED;
        }
        
        return false;
    }
};

}

