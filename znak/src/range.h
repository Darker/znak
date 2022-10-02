#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <ostream>
#include <string_view>

namespace znak
{

struct range
{
    size_t start;
    size_t end;
    std::string name;
    //std::vector<std::string> names;
    std::vector<range> nestedValues;

    void print(std::ostream& str, const std::string_view& originalString, int indent = 0) const
    {
        const std::string_view value = (start < end ? originalString.substr(start, end - start) : "[EMPTY]");

        str << std::string(indent, ' ') << name << " = " << value
         << "  [" << start << ", " << end << ")"
         << std::endl;

        for(const range& r : nestedValues)
        {
            r.print(str, originalString, indent + 2);
        }
    }
};

}
