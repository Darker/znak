#include "sptr.h"
#include <map>
#include <string>
#include <vector>
#include "assertm.h"
 

struct AssertDestruct 
{
    AssertDestruct() {}

    template <typename T>
    struct ReturnRVR
    {
        T value;
        ReturnRVR(T&& val) : value(std::move(val)) {}

        operator T&&() && { return std::move(value); }
        operator T&() = delete;
        T&& get() { return std::move(value); }
    };

    struct Value : znak::nocopy
    {
        static std::string fullName(std::string base, int copy)
        {
            return base + (copy > 0 ? std::string(" copy: ")+std::to_string(copy) : std::string(""));
        }

        Value(AssertDestruct& parent, const std::string& name) : parent(parent), name(name) { parent.reg(fullname()); }
        Value(Value&& other) : parent(other.parent), name(std::move(other.name)), copy(std::move(other.copy)) { }
        Value& operator=(Value&& other) 
        {
            name = std::move(other.name);
            copy = std::move(other.copy);
            parent = other.parent;
            return *this;
        }
        std::string fullname() const
        {
            return name + (copy > 0 ? std::string(" copy: ")+std::to_string(copy) : std::string(""));
        }
        //Value(const Value& other) : parent(other.parent), name(fullName(other.name, other.copy+1)), copy(other.copy+1) { parent.reg(name); }
        Value() = delete;

        AssertDestruct& parent;
        std::string name;
        int copy = 0;
        virtual std::string message() 
        {
            return std::string("Value(\"") + name + "\")";
        }
        virtual ~Value() 
        {
            if(name.size() > 0)
            {
                std::cout << "DELETE" << fullname() << "\n";
                std::string error = name + " deleted twice!";
                assertm(parent.values[fullname()] == false, error.c_str());
                parent.values[fullname()] = true;
                name = "DELETED" + name;
            }
            else
            {
                std::cout << "rvalue destructor ignored...\n";
            }

        }
        // virtual ~Value() &&
        // {
        //     std::cout << "DELETE" << fullname() << "\n";
        // }
    };

    struct ValueSub : Value
    {
        static const char* msg() { return " - subclass ValueSub"; };
        ValueSub(AssertDestruct& parent, const std::string& name) : Value(parent, name) { parent.reg(name + msg()); }
        ValueSub(ValueSub&& other) : Value(std::forward<Value>(other)) { }
        ValueSub() = delete;
        //ValueSub(const ValueSub& other) : Value(other) { parent.reg(name + msg()); }
        int guard = true;
        virtual ~ValueSub() 
        {
            parent.values[name + msg()] = true;
            guard = false;
        }
        virtual std::string message() override { return std::string("ValueSub(\"") + name + "\")"+std::to_string((int)guard); }
    };
    struct ValueNotSub
    {
        bool isBad = true;
    };

    Value* create(const std::string& name) 
    { 
        return new Value(*this, name);
    }

    // Value createByCopy(const std::string& name) 
    // { 
    //     return Value(*this, name);
    // }

    // ReturnRVR<Value> createRvR(const std::string& name) 
    // { 
    //     return ReturnRVR(Value{*this, name});
    // }

    void reg(const std::string& name)
    {
        auto it = values.find(name);
        assertm(it == values.end(), "Cannot create same name twice!");
        values[name] = false;
    }

    std::map<std::string, bool> values;

    ~AssertDestruct() 
    {
        for(auto kv : values)
        {
            auto error = kv.first + " not deleted!";
            assertm(kv.second, error.c_str());
        }

        
    }
};

using TestPtr = znak::sptr<AssertDestruct::Value>;

void acceptPtr(TestPtr value)
{
    std::cout << "Got value: " << value->message() << "\n";
}

void addToVector(TestPtr&& valueRVR, std::vector<TestPtr>& others)
{
    others.push_back(TestPtr(std::move(valueRVR)));
}

struct ReportDelete
{
    ReportDelete() = delete;
    ReportDelete(const std::string& name) : name(name) { std::cout << "create " << name << "\n"; }
    ReportDelete(ReportDelete&& moveHere) : name(std::move(moveHere.name)) {}
    ReportDelete& operator=(ReportDelete&& moveHere)
    {
        name = std::move(moveHere.name);
        return *this;
    }

    ReportDelete(const ReportDelete&) = delete;
    void operator=(const ReportDelete&) = delete;

    std::string name;

    ~ReportDelete()
    {
        std::cout << "delete " << name << "\n";
        name = name + "-deleted";
    }
};

int main(int argc, const char** argv)
{
    // std::vector<ReportDelete> moveTest;
    // moveTest.push_back(ReportDelete("test"));
    using namespace znak;
    using V = AssertDestruct::Value;

    AssertDestruct test;
    {
        AssertDestruct::Value doNotDelete(test, "local var");
        {
            std::vector<TestPtr> ptrs;
            //V testVal(test, "local var");
            ptrs.push_back(test.create("simple ptr"));
            ptrs.push_back(AssertDestruct::Value(test, "rvalref"));
            ptrs.push_back(AssertDestruct::ValueSub(test, "rvalref (subclass)"));
            //ptrs.push_back(test.createByCopy("copy"));
            acceptPtr(AssertDestruct::ValueSub(test, "rvalref to function (subclass)"));
            addToVector(AssertDestruct::ValueSub(test, "rvalref addToVector (subclass)"), ptrs);

            ptrs.push_back(TestPtr::borrow(doNotDelete));
            //ptrs.push_back(AssertDestruct::ValueNotSub());
            //ptrs.push_back(znak::sptr<AssertDestruct::ValueSub>(new AssertDestruct::ValueSub(test, "subclass")));


            for(TestPtr& value : ptrs)
            {
                std::cout << "Still allocated:" << value->message() << "\n";
            }
        }
    }

}