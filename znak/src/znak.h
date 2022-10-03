#pragma once

#include <cstddef>
#include <string>
#include <memory>
#include <vector>
#include <ostream>
#include <iostream>
#include <limits>

#include "helpers.h"
#include "matchers.h"
#include "sptr.h"
#include "Parser.h"
#include "ParserBase.h"
#include "ParserMulti.h"
#include "range.h"

namespace znak
{


// matches from min to max of chars for given matchers
// it will try all matchers on every character given
struct MatchRange : ParserBase
{
    static constexpr const size_t MATCH_MAX = std::numeric_limits<size_t>::max();

    MatchRange(const std::string& name, size_t min, size_t max) : ParserBase(name), min(min), max(max) {}

    template <typename TMatcher, typename std::enable_if<std::is_base_of<Matcher, TMatcher>::value, TMatcher>::type* = nullptr>
    MatchRange(const std::string& name, size_t min, size_t max, const TMatcher& ref) : MatchRange(name, min, max) 
    {
        matchers.push_back(sptr<Matcher>::copy(ref));
    }

    template <typename TMatcher, typename std::enable_if<std::is_base_of<Matcher, TMatcher>::value, TMatcher>::type* = nullptr>
    MatchRange(const std::string& name, size_t min, size_t max, TMatcher&& rref) : MatchRange(name, min, max) 
    {
        matchers.push_back(sptr<Matcher>(std::forward<TMatcher>(rref)));
    }

    template<typename TMatcher>
    static MatchRange unlimited(const std::string& name, TMatcher&& value)
    {
        return MatchRange(name, 1, MATCH_MAX, std::forward<TMatcher>(value));
    }

    static MatchRange unlimited(const std::string& name)
    {
        return MatchRange(name, 1, MATCH_MAX);
    }

    template<typename TMatcher>
    static MatchRange once(const std::string& name, TMatcher&& value)
    {
        return MatchRange(name, 1, 1, std::forward<TMatcher&&>(value));
    }

    static MatchRange once(const std::string& name)
    {
        return MatchRange(name, 1, 1);
    }

    #pragma region move_semantics
    MatchRange(const MatchRange&) = delete;
    void operator=(const MatchRange&) = delete;
    MatchRange(MatchRange&& moveHere)
     : ParserBase(std::move(moveHere.name))
     , matchers(std::move(moveHere.matchers))
     , min(moveHere.min)
     , max(moveHere.max)
    {}
    MatchRange& operator=(MatchRange&& moveHere)
    {
        matchers = std::move(moveHere.matchers);
        name = std::move(moveHere.name);
        min = moveHere.min;
        max = moveHere.max;
        return *this;
    }
    #pragma endregion
    // template <typename TMatcherDerived>
    // MatchRange&& add(sptr<TMatcherDerived>&& m) &&
    // {
    //     static_assert(std::is_constructible<TMatcherDerived, Matcher>::value, "Cannot convert given value to Matcher!");

    //     matchers.push_back(m);
    //     return std::move<*this>;
    // }


    // MatchRange& add(sptr<Matcher>&& m)
    // {
    //     //static_assert(std::is_convertible<TMatcherDerived, Matcher>::value, "Cannot convert given value to Matcher!");
    //     matchers.push_back(sptr<Matcher>(std::move(m)));
    //     return *this;
    // }
    // MatchRange& add(sptr<Matcher>& m) = delete;
    //typename std::enable_if<std::is_base_of<Matcher, TFirst>::value, TFirst>::type* = nullptr
    template<typename TFirst, typename... Ts>
    MatchRange&& add(TFirst&& current, Ts&&... remaining) &&
    {
        //std::cout << current.getExpectations() << " expected\n";
        matchers.push_back(sptr<Matcher>(std::forward<TFirst>(current)));
        ((matchers.push_back(sptr<Matcher>(std::forward<Ts...>(remaining)))), ...);
        return std::forward<MatchRange>(*this);
    }

    void reset() override
    {
        ParserBase::reset();
        matched = 0;
    }

    bool complete() const override
    {
        return matched >= min;
    }

    virtual bool shouldContinue(size_t offset, char character) override
    {
        if(state == State::CLEAN && matched != 0)
        {
            throw std::runtime_error("Sequence parser did not reset parser index.");
        }
        if(matched < max)
        {
            for(const Matcher& m : matchers)
            {
                if(m.matches(character)) 
                {
                    matched++;
                    return true;
                }
                    
            }
        }

        return false;
    }
private:
    std::vector<sptr<Matcher>> matchers;
    size_t min;
    size_t max;
    size_t matched = 0;
};

struct Sequence : ParserMulti<Sequence>
{
    Sequence(const std::string& name) : ParserMulti<Sequence>(name) {}

    void reset() override
    {
        ParserMulti<Sequence>::reset();
        currentParser = 0;
    }

    bool complete() const override
    {
        if(currentParser == parsers.size())
        {
            return parsers[currentParser-1]->valid();
        }
        return false;
    }

    void fillInnerRanges(std::vector<range>& ranges) const override
    {
        for(const Parser& p : parsers)
        {
            ranges.push_back(p.parsedRange());
        }
    }

    virtual bool shouldContinue(size_t offset, char character) override
    {
        if(state == State::CLEAN && currentParser != 0)
        {
            throw std::runtime_error("Sequence parser did not reset parser index.");
        }
        if(currentParser + 1 > parsers.size())
        {
            throw std::runtime_error("Sequence tried to access more parsers than it has.");
        }

        for(;currentParser<parsers.size();++currentParser)
        {
            Parser& current = parsers[currentParser];
            bool accepted = current.check(offset, character);
            // if valid, we're done with this one
            if(accepted)
            {
                return true;
            }
            else if(current.valid())
            {
                // skipped the character, but the inner data is good
                // nothing happens here, but we reuse the character for the next iteration
            }
            else 
            {
                // if invalid, there is no recovery
                return false;
            }
        }
        return false;
    }

    private:
    size_t currentParser = 0;
};

struct Repeat : ParserBase
{
    static constexpr const size_t MATCH_MAX = std::numeric_limits<size_t>::max();
    Repeat(const std::string& name, size_t min, size_t max, sptr<Parser>&& innerParser)
        : ParserBase(name)
        , min(min)
        , max(max)
        , inner(std::forward<sptr<Parser>>(innerParser))
    {}
    Repeat(const std::string& name, size_t min, size_t max, sptr<Parser>& innerParser) = delete;

    void reset() override
    {
        ParserBase::reset();
        inner->reset();
    }

    bool complete() const override
    {
        return matched >= min;
    }

    virtual bool shouldContinue(size_t offset, char character) override
    {
        if(state == State::CLEAN && matched != 0)
        {
            throw std::runtime_error("Repeat parser did not reset parser index.");
        }
        if(matched == max)
        {
            throw std::runtime_error("Repeat already has matched max num of items");
        }
        if(!inner)
        {
            throw std::runtime_error("Repeat does not have an inner parser assigned!");
        }

        Parser& p = *inner;

        if(!p.check(offset, character))
        {
            // character not accepted, but we can take what was accepted and continue
            if(p.valid())
            {
                ranges.push_back(p.parsedRange());
                ++matched;
                // try reuse the character
                p.reset();
                if(matched < max)
                {
                    return p.check(offset, character);
                }
                // already full, someone else needs to use this character
                else 
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else 
        {
            return true;
        }
    }
private:
    sptr<Parser> inner;
    std::vector<range> ranges;
    size_t min;
    size_t max;
    size_t matched = 0;
};

struct First : ParserMulti<First>
{
    First(const std::string& name) : ParserMulti<First>(name) {}

    void reset() override
    {
        ParserMulti<First>::reset();
        resetUse();
        done = false;
    }

    void fillInnerRanges(std::vector<range>& ranges) const override
    {
        if(!complete())
        {
            throw std::runtime_error("Cannot extract output from a failed parser.");
        }
        ranges.push_back(parsers[chosenOutput]->parsedRange());
    }

    bool complete() const override
    {
        return done;
    }

    virtual bool shouldContinue(size_t offset, char character) override
    {
        bool anyActive = false;
        for(size_t i = 0; i<parsers.size(); ++i)
        {
            if(!use[i])
                continue;

            Parser& p = parsers[i];
            if(!p.check(offset, character))
            {
                if(p.empty() || !p.valid())
                {
                    use[i] = false;
                }
                // parser has characters and is complete
                else
                {
                    done = true;
                    chosenOutput = i;
                    return false;
                }
            }

            anyActive = anyActive || use[i];
        }
        return anyActive;
    }

private:
    size_t chosenOutput = 0;
    bool done = false;
    // if false, parser at that index has failed already and can be skipped next time
    std::vector<bool> use;

    void resetUse()
    {
        use.resize(parsers.size());
        for(size_t i=0; i<parsers.size(); ++i)
        {
            use[i] = true;
        }

    }
};

}

