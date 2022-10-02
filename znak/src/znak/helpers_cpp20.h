#include <cstddef>
#include <algorithm>

namespace znak
{

/**
 * Literal class type that wraps a constant expression string.
 *
 * Uses implicit conversion to allow templates to *seemingly* accept constant strings.
 */
template<size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]):value() {
        for(size_t i=0; i<N; ++i)
        {
            value[i] = str[i];
        }
    }
    
    char value[N];
};

}