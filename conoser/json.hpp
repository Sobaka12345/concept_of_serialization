#ifndef JSON_HPP
#define JSON_HPP

#include "type_utils.hpp"

#include <memory>
#include <string>
#include <vector>
#include <iomanip>
#include <unordered_map>

class IJSON;
using JSONPtr = std::shared_ptr<IJSON>;

template <typename T>
class JSONObject;

template <typename T>
class JSONArray;

class IJSON
{
public:
    virtual void print(std::ostream& stream) const = 0;
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

class JSONReal
    : public IJSON
{
public:
    JSONReal(long double val, int precision)
        : m_precision(precision)
        , m_value(val)
    {}

    virtual void print(std::ostream& stream) const
    {
        stream << std::setprecision(m_precision) << m_value;
    }

private:
    int m_precision;
    long double m_value;
};

using JSONTypes = TypesPacker<int64_t, JSONReal, bool, std::string, JSONPtr, std::monostate>;

class JSONValue
    : public VariantValue<JSONTypes::Apply>
    , public IJSON
{
public:
    using VariantValue::VariantValue;
    virtual void print(std::ostream& stream) const
    {
        std::visit([&stream] (const auto& var) {
            if constexpr (std::is_same_v<std::decay_t<decltype(var)>, int64_t>) {
                stream << var;
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(var)>, bool>) {
                stream << std::boolalpha << var;
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(var)>, JSONReal>) {
                var.print(stream);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(var)>, std::string>) {
                stream << std::quoted(var);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(var)>, std::monostate>) {
                stream << "null";
            }
            else
            {
                var->print(stream);
            }
        }, *this);
    }
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
        return static_cast<T*>(this);
    }

    auto returnValue()
    {
        return type()->reset();
    }

public:
    auto boolean(bool val)
    {
        type()->addValue(val);
        return returnValue();
    }

    auto integer(int64_t val)
    {
        type()->addValue(val);
        return returnValue();
    }

    auto real(long double val, int precision = 6)
    {
        type()->addValue(JSONReal(val, precision));
        return returnValue();
    }

    auto string(const std::string& val)
    {
        type()->addValue(val);
        return returnValue();
    }

    auto null()
    {
        type()->addValue(std::monostate());
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
    using AddersType = JSONAdders<JSONKey<T>>;
    using ParentPtr = std::shared_ptr<T>;
    using JSONKeyPtr = std::shared_ptr<JSONKey<T>>;
public:
    static JSONKeyPtr create(ParentPtr parent)
    {
        return JSONKeyPtr(new JSONKey(parent));
    }

    virtual void print(std::ostream& stream) const override
    {
        m_value.print(stream);
    }

    std::shared_ptr<JSONArray<T>> beginArray();
    std::shared_ptr<JSONObject<T>> beginObject();

protected:
    JSONKey(ParentPtr parent)
        : BaseType(parent)
    {}

private:
    friend AddersType;

    std::shared_ptr<T> reset()
    {
        return BaseType::m_parent.lock();
    }

    void addValue(JSONValue value)
    {
        m_value = value;
    }

private:
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

    friend std::ostream& operator<<(std::ostream& stream, JSONObjectPtr ptr)
    {
        ptr->print(stream);
        return stream;
    }

    virtual void print(std::ostream& stream) const
    {
        const auto printKeyVal = [&stream](auto iter) {
            stream << std::quoted(iter->first) << ':';
            iter->second.print(stream);
        };

        stream << '{';
        if(!m_fields.empty())
        {
            auto iter = m_fields.begin();
            printKeyVal(m_fields.begin());
            while(++iter != m_fields.end())
            {
                stream << ',';
                printKeyVal(iter);
            }
        }
        stream << '}';
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
        return BaseType::m_parent.lock();
    }

private:
    JSONObject(ParentPtr parent)
        : BaseType(parent)
    {}

private:
    std::unordered_map<std::string, JSONValue> m_fields;
};

template <typename T>
class JSONArray
        : public JSONBase<JSONArray, T>
        , public JSONAdders<JSONArray<T>>
{
    using BaseType = JSONBase<JSONArray, T>;
    using BaseType::shared_from_this;
    using AddersType = JSONAdders<JSONArray<T>>;
    using ParentPtr = std::shared_ptr<T>;
    using JSONArrayPtr = std::shared_ptr<JSONArray<T>>;
public:
    static JSONArrayPtr create(ParentPtr parent)
    {
        return JSONArrayPtr(new JSONArray(parent));
    }

    friend std::ostream& operator<<(std::ostream& stream, JSONArrayPtr ptr)
    {
        ptr->print(stream);
        return stream;
    }

    virtual void print(std::ostream& stream) const
    {
        stream << '[';
        for(int i = 0; i < m_values.size() - 1; ++i)
        {
            m_values[i].print(stream);
            stream << ",";
        }
        if (!m_values.empty())
        {
            m_values.back().print(stream);
        }
        stream << ']';
    }

    ParentPtr endArray()
    {
        return BaseType::m_parent.lock();
    }

protected:
    JSONArray(ParentPtr parent)
        : BaseType(parent)
    {}

private:
    friend AddersType;

    JSONArrayPtr reset()
    {
        return shared_from_this();
    }

    void addValue(JSONValue val)
    {
        m_values.push_back(val);
    }

private:
    std::vector<JSONValue> m_values;
};

template <typename T>
std::shared_ptr<JSONArray<T>> JSONKey<T>::beginArray()
{
    auto ptr = JSONArray<T>::create(BaseType::m_parent.lock());
    m_value = ptr;
    return ptr;
}

template <typename T>
std::shared_ptr<JSONObject<T>> JSONKey<T>::beginObject()
{
    auto ptr = JSONObject<T>::create(BaseType::m_parent.lock());
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
