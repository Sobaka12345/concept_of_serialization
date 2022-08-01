#include <iostream>
#include "json.hpp"
#include <array>

int main()
{
    //JSONKey<int> ass; ass.addValue(JSONValue{std::monostate()});
    JSONKey<int>::create(nullptr);
    return 0;
}
