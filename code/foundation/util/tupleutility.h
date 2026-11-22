#pragma once
#include <tuple>
namespace Util
{
template <typename... T>
using Tuple = std::tuple<T...>;
template <typename... T>
constexpr Tuple<T...>
MakeTuple(const T&... args)
{
    return std::make_tuple(args...);
}
template <int INDEX, typename... T>
constexpr const std::tuple_element_t<INDEX, std::tuple<T...>>&
Get(const Tuple<T...>& tuple)
{
    return std::get<INDEX>(tuple);
}
template <typename A, typename B>
using Pair = std::pair<A, B>;
template <typename A, typename B>
constexpr Pair<A, B>
MakePair(const A& a, const B& b)
{
    return std::make_pair(a, b);
}
template <int INDEX, typename A, typename B>
constexpr const std::tuple_element_t<INDEX, std::tuple<A, B>>&
Get(const Pair<A, B>& pair)
{
    static_assert(INDEX == 0 || INDEX == 1, "Индекс должен быть либо 0, либо 1.");
    return std::get<INDEX>(pair);
}
template <typename C>
struct get_template_type;
template <template <typename> class C, typename T>
struct get_template_type<C<T>>
{
    using type = T;
};
template <template <typename, int> class C, typename T, int I>
struct get_template_type<C<T, I>>
{
    using type = T;
};
template <template <int, typename> class C, int I, typename T>
struct get_template_type<C<I, T>>
{
    using type = T;
};
template <template <typename> class C, typename T>
struct get_template_type<const C<T>&>
{
    using type = T;
};
template <template <int, typename> class C, int I, typename T>
struct get_template_type<C<I, T>&>
{
    using type = T;
};
template <template <typename, int> class C, typename T, int I>
struct get_template_type<C<T, I>&>
{
    using type = T;
};
template <typename C>
using get_template_type_t = typename get_template_type<C>::type;
template<class...Ts, std::size_t...Is> void
alloc_for_each_in_tuple(std::tuple<Ts...>& tuple, std::index_sequence<Is...>)
{
    (std::get<Is>(tuple).Append(typename get_template_type<Ts>::type()), ...);
}
template<class...Ts> void
alloc_for_each_in_tuple(std::tuple<Ts...>& tuple)
{
    alloc_for_each_in_tuple(tuple, std::make_index_sequence<sizeof...(Ts)>());
}
template<class...Ts, std::size_t...Is> void
clear_for_each_in_tuple(std::tuple<Ts...>& tuple, std::index_sequence<Is...>)
{
    (std::get<Is>(tuple).Clear(), ...);
}
template<class...Ts> void
clear_for_each_in_tuple(std::tuple<Ts...>& tuple)
{
    clear_for_each_in_tuple(tuple, std::make_index_sequence<sizeof...(Ts)>());
}
template <class...Ts, std::size_t...Is> void
move_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t to, uint32_t from, std::index_sequence<Is...>)
{
    ((std::get<Is>(tuple)[to] = std::get<Is>(tuple)[from]), ...);
}
template <class...Ts> void
move_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t to, uint32_t from)
{
    move_for_each_in_tuple(tuple, to, from, std::make_index_sequence<sizeof...(Ts)>());
}
template <class...Ts, std::size_t...Is> void
erase_index_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t i, std::index_sequence<Is...>)
{
    (std::get<Is>(tuple).EraseIndex(i), ...);
}
template <class...Ts> void
erase_index_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t i)
{
    erase_index_for_each_in_tuple(tuple, i, std::make_index_sequence<sizeof...(Ts)>());
}
template <class...Ts, std::size_t...Is> void
erase_index_swap_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t i, std::index_sequence<Is...>)
{
    (std::get<Is>(tuple).EraseIndexSwap(i), ...);
}
template <class...Ts> void
erase_index_swap_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t i)
{
    erase_index_swap_for_each_in_tuple(tuple, i, std::make_index_sequence<sizeof...(Ts)>());
}
template <class...Ts, std::size_t...Is> void
erase_range_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t start, uint32_t end, std::index_sequence<Is...>)
{
    (std::get<Is>(tuple).EraseRange(start, end), ...);
}
template <class...Ts> void
erase_range_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t start, uint32_t end)
{
    erase_range_for_each_in_tuple(tuple, start, end, std::make_index_sequence<sizeof...(Ts)>());
}
template <class...Ts, std::size_t...Is, class...TYPES> void
set_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t i, std::index_sequence<Is...>, TYPES const& ... values)
{
    ((std::get<Is>(tuple)[i] = values), ...);
}
template <class...Ts, class...TYPES> void
set_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t i, TYPES const& ... values)
{
    set_for_each_in_tuple(tuple, i, std::make_index_sequence<sizeof...(Ts)>(), values...);
}
template <class...Ts, std::size_t...Is> void
reserve_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t size, std::index_sequence<Is...>)
{
    (std::get<Is>(tuple).Reserve(size), ...);
}
template <class...Ts> void
reserve_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t size)
{
    reserve_for_each_in_tuple(tuple, size, std::make_index_sequence<sizeof...(Ts)>());
}
template <class...Ts, std::size_t...Is> void
set_size_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t size, std::index_sequence<Is...>)
{
    (std::get<Is>(tuple).SetSize(size), ...);
}
template <class...Ts> void
set_size_for_each_in_tuple(std::tuple<Ts...>& tuple, uint32_t size)
{
    set_size_for_each_in_tuple(tuple, size, std::make_index_sequence<sizeof...(Ts)>());
}
template <int MEMBER, class ... TYPES>
using tuple_array_t = get_template_type_t<std::tuple_element_t<MEMBER, std::tuple<Util::Array<TYPES>...>>>;
}