#ifndef JSON_HPP
#define JSON_HPP

#include "type_utils.hpp"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class JSONBase;
using JSONBasePtr = std::shared_ptr<JSONBase>;

template <typename T>
class JSONObject;

class JSONBase
    : private std::enable_shared_from_this<JSONBase>
{
public:
    JSONBase(JSONBasePtr parent)
        : m_parent(parent)
    {

    }

    JSONBasePtr ptr()
    {
        return shared_from_this();
    }

    virtual ~JSONBase() {}

protected:
    std::weak_ptr<JSONBase> m_parent;
};

using JSONTypes = TypesPacker<int64_t, double, std::string, JSONBasePtr, std::monostate>;

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
concept IsJSONAdders = requires(T o) {
    o.addValue(JSONValue{std::monostate()});
    o.reset();
};

template <typename T>
class JSONAdders
{
    auto& type()
    {
        static_assert(IsJSONAdders<T>);
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
class JSONKey
    : public JSONBase
    , public JSONAdders<JSONKey<T>>
    , public std::enable_shared_from_this<JSONKey<T>>
{
    using JSONKeyPtr = std::shared_ptr<JSONKey<T>>;
    using std::enable_shared_from_this<JSONKey<T>>::shared_from_this;
public:
    static JSONKeyPtr create(JSONBasePtr parent)
    {
        return (new JSONKey<T>(parent))->shared_from_this();
    }

    void addValue(JSONValue value)
    {

    }

    std::shared_ptr<JSONObject<T>> beginObject()
    {

    }

    std::shared_ptr<T> reset()
    {
        static_cast<T*>(m_parent.lock().get())->enable_shared_from_this();
    }

private:
    JSONKey(JSONBasePtr parent)
        : JSONBase(parent)
    {}
};

template <typename T>
class JSONObject
    : public JSONBase
    , public std::enable_shared_from_this<JSONObject<T>>
{
    using JSONObjectPtr = std::shared_ptr<JSONObject>;
    using std::enable_shared_from_this<JSONObject<T>>::shared_from_this;
public:
    static JSONObjectPtr create(JSONBasePtr parent = nullptr)
    {
        return (new JSONObject<T>(parent))->shared_from_this();
    }

//    JSONObjectPtr beginObject(const std::string& key)
//    {
//        auto [item, ok] = m_fields.emplace(
//            std::pair<std::string, JSONObjectPtr>{key, create()});

//        if (!ok) throw std::runtime_error("Failed to begin object!");

//        return std::get<JSONObjectPtr>(item->second)->ptr();
//    }

    T& endObject()
    {
        return *static_cast<T*>(m_parent.lock()->ptr());
    }

private:
    JSONObject(JSONBasePtr parent)
        : JSONBase(parent)
    {}

private:
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
