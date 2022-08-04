#ifndef JSON_HPP
#define JSON_HPP

#include "type_utils.hpp"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class IJSON;
using JSONPtr = std::shared_ptr<IJSON>;

template <typename T>
class JSONObject;

class IJSON
{
public:
    virtual ~IJSON() {}
};

template <template <typename> class T, typename ParentType>
class JSONBase
    : public std::enable_shared_from_this<T<ParentType>>
    , public IJSON
{
protected:
    JSONBase(std::shared_ptr<ParentType> parent)
        : m_parent(parent)
    {
    }

protected:
    std::weak_ptr<ParentType> m_parent;
};

using JSONTypes = TypesPacker<int64_t, double, std::string, JSONPtr, std::monostate>;

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
    auto type()
    {
        static_assert(IsJSONAdders<T>);
        return *static_cast<T*>(this);
    }

    auto returnValue()
    {
        return type().reset();
    }

public:
    auto integer(int64_t val)
    {
        type().addValue(val);
        return returnValue();
    }

    auto real(double val)
    {
        type().addValue(val);
        return returnValue();
    }

    auto string(const std::string& val)
    {
        type().addValue(val);
        return returnValue();
    }

    auto null()
    {
        type().addValue(std::monostate());
        return returnValue();
    }
};

template <typename T>
class JSONKey
    : public JSONBase<JSONKey, T>
    , public JSONAdders<JSONKey<T>>
{
    using BaseType = JSONBase<JSONKey, T>;
    using BaseType::shared_from_this;
    using ParentPtr = std::shared_ptr<T>;
    using JSONKeyPtr = std::shared_ptr<JSONKey<T>>;
public:
    static JSONKeyPtr create(ParentPtr parent)
    {
        return JSONKeyPtr(new JSONKey(parent));
    }

    void addValue(JSONValue value)
    {
        m_value = value;
    }

    std::shared_ptr<JSONObject<JSONKey<T>>> beginObject();

    std::shared_ptr<T> reset()
    {
        return BaseType::m_parent.lock();
    }

private:
    JSONKey(ParentPtr parent)
        : BaseType(parent)
    {}

private:
    std::string m_key;
    JSONValue m_value;
};

template <typename T>
class JSONObject
    : public JSONBase<JSONObject, T>
{
    using BaseType = JSONBase<JSONObject, T>;
    using BaseType::shared_from_this;
    using ParentPtr = std::shared_ptr<T>;
    using JSONObjectPtr = std::shared_ptr<JSONObject<T>>;
public:
    static JSONObjectPtr create(ParentPtr parent)
    {
        return JSONObjectPtr(new JSONObject(parent));
    }

    std::shared_ptr<JSONKey<JSONObject<T>>> key(const std::string& key)
    {
        auto obj = JSONKey<JSONObject<T>>::create(shared_from_this());
        auto [_, ok] = m_fields.emplace(key, obj);
        if (!ok)
        {
            throw std::runtime_error("Cannot create key!");
        }
        return obj;
    }

    std::shared_ptr<T> endObject()
    {
        return BaseType::m_parent.lock()->ptr();
    }

private:
    JSONObject(ParentPtr parent)
        : BaseType(parent)
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

// JSONKey
template <typename T>
std::shared_ptr<JSONObject<JSONKey<T>>> JSONKey<T>::beginObject()
{
    auto ptr = JSONObject<JSONKey<T>>::create(shared_from_this());
    m_value = ptr;
    return ptr;
}

class JSONRoot
{
    using RootObject = JSONObject<JSONRoot>;
    using RootObjectPtr = std::shared_ptr<RootObject>;
public:
    static RootObjectPtr create()
    {
        return RootObject::create(std::shared_ptr<JSONRoot>());
    }

private:
    JSONRoot(){};
    JSONRoot(std::shared_ptr<JSONRoot>){};
};

#endif //JSON_HPP
