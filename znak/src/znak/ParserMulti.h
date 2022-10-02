#pragma once
#include "ParserBase.h"
#include "sptr.h"

namespace znak
{

template<typename T>
class rref_capture
{
  sptr<T>* ptr;
public:
  rref_capture(sptr<T>&& x) : ptr(&x) {}
  operator sptr<T>&& () const { return std::move(*ptr); } // restitute rvalue ref
};

template <typename Derived>
struct ParserMulti : ParserBase
{
    ParserMulti(const std::string& name) : ParserBase(name) {}

    #pragma region move_semantics
    ParserMulti(const ParserMulti&) = delete;
    void operator=(const ParserMulti&) = delete;
    ParserMulti(ParserMulti&& moveHere)
     : ParserBase(std::move(moveHere.name))
     , parsers(std::move(moveHere.parsers))
    {}
    ParserMulti& operator=(ParserMulti&& moveHere)
    {
        name = std::move(moveHere.name);
        parsers = std::move(moveHere.parsers);
        return *this;
    }
    #pragma endregion

    template<typename... Ts>
    void doAdd(Ts&&... remaining)
    {
        ((parsers.push_back(sptr<Parser>(std::forward<Ts>(remaining)))), ...);
    }

    template<typename... Ts>
    Derived&& add(Ts&&... remaining) &&
    {
        doAdd(std::forward<Ts>(remaining)...);
        return std::forward<Derived>(static_cast<Derived&>(*this));
    }

    template<typename... Ts>
    Derived& add(Ts&&... remaining) &
    {
        doAdd(std::forward<Ts>(remaining)...);
        return static_cast<Derived&>(*this);
    }

    void reset() override
    {
        ParserBase::reset();
        for(Parser& p : parsers)
        {
            p.reset();
        }
    }

    void fillInnerRanges(std::vector<range>& ranges) const override
    {
        for(const Parser& p : parsers)
        {
            ranges.push_back(p.parsedRange());
        }
    }
protected:
    std::vector<sptr<Parser>> parsers;

};

}