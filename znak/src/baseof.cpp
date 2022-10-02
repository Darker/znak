#include <memory>
#include <vector>
#include <iostream>
#include "sptr.h"

struct Base
{
    Base(double v) : value(new double(v)) {}
    int i = 0;
    std::unique_ptr<double> value = nullptr;

    #pragma region no_copy
    Base(const Base&) = delete;
    Base& operator=(const Base&) = delete;
    #pragma endregion no_copy

    #pragma region move_semantics
    Base(Base&& moveHere) 
     : value(std::move(moveHere.value))
     , i(moveHere.i)
    {}
    Base& operator=(Base&& moveHere)
    {
        value = std::move(moveHere.value);
        i = moveHere.i;
        return *this;
    }
    #pragma endregion

    virtual ~Base() = default;

    virtual void print(std::ostream& out) const
    {
        out << "value: " << *value << "i: " << i << "\n"; 
    }

};

struct Derived : Base
{
    Derived(double v, int my) : Base(v), check(my), checkPtr(new int(my)) {}

    int check = 6;
    std::unique_ptr<int> checkPtr;

    virtual void print(std::ostream& out) const override
    {
        out << "check: " << check << " ptr:" << *checkPtr << " [BASE]: ";
        Base::print(out); 
    }
};



int main()
{
    Derived val = Derived(0.1, 3);
    std::vector<znak::sptr<Base>> testMove;

    testMove.push_back(Derived(1.4, 12));
    testMove.push_back(Base(6.0));
    testMove.reserve(200);
    //std::cout << "Values after move: " << *(testMove[0].value) << ", " << testMove[0].check << "\n";

    for(const Base& d : testMove)
    {
        d.print(std::cout);
    }
}


