#include <iostream>
#include "json.hpp"
#include <array>


class IBase
{
public:
    virtual ~IBase(){}
};

template<typename T>
class Base : public std::enable_shared_from_this<T>
        , public IBase
{
public:
};

class Descendant :
        public Base<Descendant>
{
public:
    auto GETD()
    {
        return shared_from_this();
    }
    auto HOPA()
    {

    }
};


int main()
{
    //JSONKey<int> ass; ass.addValue(JSONValue{std::monostate()});
    //JSONKey<int>::create(nullptr);

    auto kek = std::make_shared<Descendant>();
    std::shared_ptr<IBase> lol = kek->GETD();
    auto test = static_cast<Descendant*>(lol.get())->GETD();
    std::shared_ptr<Base<Descendant>> b = kek->shared_from_this();

    return 0;
}
