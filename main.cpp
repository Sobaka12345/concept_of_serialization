#include <iostream>
#include "json.hpp"

int main()
{
    JSONRoot::create()
        ->key("biba")->beginObject()
        ->key("ass")->beginObject()
        ->key("as")->beginObject()
        ->key("sas")->beginObject()
        ->key("ass")->beginObject()
        ->key("ass")->beginObject();

    return 0;
}
