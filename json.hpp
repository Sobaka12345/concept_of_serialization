#ifndef JSON_HPP
#define JSON_HPP

#include "type_utils.hpp"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class JSONArray;
class JSONObject;

using JSONArrayPtr = std::shared_ptr<JSONArray>;
using JSONObjectPtr = std::shared_ptr<JSONObject>;

using JSONTypes = TypesPacker<int64_t, double, std::string, JSONObjectPtr, JSONArrayPtr, std::monostate>;

class JSONValue : public VariantValue<JSONTypes::Apply>
{
public:
    using VariantValue::VariantValue;
};

template <typename T>
concept IsJSONType = (
   JSONTypes().hasType<T>()
);

template <typename T>
concept IsJSONAdders = requires(T o)
{
    o.addValue(JSONValue{std::monostate()});
    o.returnValue();
};

template <IsJSONAdders T>
class JSONAdders
{
    auto& type()
    {
        return *static_cast<T*>(this);
    }

    auto& returnValue()
    {
        return type().returnValue();
    }

public:
    auto& integer(int64_t val)
    {
        type().addValue(val);
        return returnValue();
    }

    auto& real(double val)
    {
        type().addValue(val);
        return returnValue();
    }

    auto& string(const std::string& val)
    {
        type().addValue(val);
        return returnValue();
    }

    auto& null()
    {
        type().addValue(std::monostate());
        return returnValue();
    }
};

template <typename T>
class JSONKey : public JSONAdders<JSONKey<T>>
{
public:
    JSONKey() {}

    void addValue(JSONValue value)
    {

    }

    int returnValue()
    {
        return 0;
    }
};

class JSONObject : private std::enable_shared_from_this<JSONObject>
{
public:
    static JSONObjectPtr create(JSONObjectPtr parent = nullptr)
    {
        return JSONObjectPtr(new JSONObject(parent));
    }

    JSONObjectPtr ptr()
    {
        return shared_from_this();
    }

    //template<typename T>
    //key

    JSONObjectPtr beginObject(const std::string& key)
    {
        auto [item, ok] = m_fields.emplace(
            std::pair<std::string, JSONObjectPtr>{key, create()});

        if (!ok) throw std::runtime_error("Failed to begin object!");

        return std::get<JSONObjectPtr>(item->second)->ptr();
    }

    JSONObjectPtr endObject()
    {
        return m_parent.lock()->ptr();
    }

private:
    JSONObject(JSONObjectPtr parent)
        : m_parent(parent)
    {}

private:
    std::weak_ptr<JSONObject> m_parent;
    std::unordered_map<std::string, JSONValue> m_fields;
};

class JSONArray
{
public:
    JSONArray() = default;
    JSONArray& addValue(JSONValue val)
    {
        return *this;
    }

private:
    std::vector<JSONValue> m_values;
};


#endif //JSON_HPP
