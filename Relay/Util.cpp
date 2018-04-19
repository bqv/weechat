#include <iterator>
#include <sstream>

#include "Util.h"

std::string Util::join(const std::vector<std::string>& elements, const char * const separator)
{
    switch (elements.size())
    {
    case 0:
        return "";
    case 1:
        return elements[0];
    default:
        std::ostringstream os;
        std::copy(elements.begin(), elements.end() - 1, std::ostream_iterator<std::string>(os, separator));
        os << *elements.rbegin();
        return os.str();
    }
}
