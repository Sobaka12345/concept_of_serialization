#ifndef TYPE_UTILS_HPP
#define TYPE_UTILS_HPP

#include <variant>
#include <concepts>

template <typename ...Args>
struct TypesPacker
{
    template<typename T>
    constexpr static bool hasType()
    {
        return (std::convertible_to<Args, T> || ...);
    }

    template <template <typename ...> class T>
    struct Apply
    {
        typedef T<Args...> type;
    };
};

template <template <template <typename ...> class> class T>
struct VariantConverter
{
    typedef typename T<std::variant>::type type;
};

template <template <template <typename ...> class> class Packer>
class VariantValue :
    public VariantConverter<Packer>::type
{
    using VariantConverter<Packer>::type::type;
};

#endif //TYPE_UTILS_HPP
