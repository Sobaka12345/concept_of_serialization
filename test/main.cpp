#include <json.hpp>

#include <fstream>
#include <limits>

int main()
{
    std::ios::sync_with_stdio(false);

    auto root = JSONRoot::create();
    auto array = root->key("მასივი")->beginArray();
    for (int i = 0; i < 100; ++i)
    {
        array->null()
             ->integer(i)
             ->boolean(i % 2 == 0)
             ->string(std::to_string(i))
             ->real(1234.5678 * i, std::numeric_limits<long double>::digits10);
    }

    auto subObject = root->key("kekObject")->beginObject();
    for (int i = 0; i < 100; ++i)
    {
        subObject->key(std::to_string(i))
                 ->boolean(true);
    }

    auto nestedObjects = root->key("1")->beginObject()
            ->key("2")->beginObject()
            ->key("3")->beginObject()
            ->key("4")->beginObject()
            ->key("5")->beginObject();

    std::fstream output("test.json", std::ios_base::out);
    output << root;

    return 0;
}
