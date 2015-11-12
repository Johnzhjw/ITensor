//
// Distributed under the ITensor Library License, Version 1.2
//    (See accompanying LICENSE file.)
//
#ifndef __ITENSOR_STDX
#define __ITENSOR_STDX

#include <array>
#include <vector>
#include <algorithm>

namespace stdx {

template<typename T>
using remove_const_t = typename std::remove_const<T>::type;

template<typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template<typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template<typename T>
using add_pointer_t = typename std::add_pointer<T>::type;

template<typename T>
using decay_t = typename std::decay<T>::type;

template<bool B, class T = void>
using enable_if_t = typename std::enable_if<B,T>::type;

template<bool B, typename T1, typename T2>
using conditional_t = typename std::conditional<B,T1,T2>::type;

template<bool B, typename T1, typename T2>
using conditional_t = typename std::conditional<B,T1,T2>::type;

template<class T>
using result_of_t = typename std::result_of<T>::type;

template<typename... Conds>
struct all
    {
    bool static constexpr value = true;
    constexpr operator bool() const noexcept { return value; }
    };
template<typename Cond, typename... Conds>
struct all<Cond,Conds...> : all<Conds...>
    {
    bool static constexpr value = Cond::value && all<Conds...>::value;
    constexpr operator bool() const noexcept { return value; }
    };

template<typename... Conditions>
using require = enable_if_t<all<Conditions...>::value>;

template<typename... Conditions>
using require_not = enable_if_t<not all<Conditions...>::value>;

//
//Useful for disabling candidate template functions
//where if Expressions.. fail to compile (fail template
//substitution), then a lower-precedence overload
//will be selected.
//
template<typename ReturnValue, typename... Expressions>
using if_compiles_return = ReturnValue;

//Helper type for making static_assert always fail,
//but only if a given template is instantiated
//(unlike std::false_type, this type depends on
//the typename T so will not be evaluated until
//the template is instantiated)
template<typename T, typename... Rest>
struct false_regardless_of : public std::false_type
    {
    using ignored_type = T;
    };

//
//Dummy argument types to simplify
//template overload precedence.
//Type "select_overload" auto converts to
//choice<1>, then choice<2>, etc.
//in decreasing order of precendence.
//(credit to R. Martinho Fernandes)
//
//Usage:
// funcImpl(..., choice<2>) { }
// funcImpl(..., choice<1>) { }
// func(...) { funcImpl(...,select_overload{}); }
//
template<unsigned I>
struct choice : choice<I+1> { constexpr choice(){} };

template<>
struct choice<10> { constexpr choice(){} };

struct select_overload : choice<1> { constexpr select_overload(){} };



//
// Traits
//

template<typename T>
struct isRvalue
    {
    bool static constexpr value = std::is_rvalue_reference<T&&>::value;
    constexpr operator bool() const noexcept { return value; }
    };

template<typename C>
auto
isIterImpl(choice<2>, C const&)
    -> std::false_type
    { return std::false_type{}; }
template<typename C>
auto
isIterImpl(choice<1>, C const& t)
    -> if_compiles_return<std::true_type,
       decltype(t.begin()), //check if t.begin() compiles
       decltype(t.end())>   //check if t.end() compiles
    { return std::true_type{}; }
template<typename C>
using isIterable = decltype(isIterImpl(select_overload{},std::declval<C>()));

template<typename C>
auto
isIndexableImpl(choice<2>, C const&)
    -> std::false_type
    { return std::false_type{}; }
template<typename C>
auto
isIndexableImpl(choice<1>, C const& t)
    -> if_compiles_return<std::true_type,
       decltype(t[0])> //check if t[0] compiles
    { return std::true_type{}; }
template<typename C>
using isIndexable = decltype(isIndexableImpl(select_overload{},std::declval<C>()));

template<typename Val, typename C>
void
containerOfImpl(choice<3>, C const&) { }
template<typename Val, typename C>
auto
containerOfImpl(choice<2>, C const& c) -> if_compiles_return<Val,decltype(c[0])>
    { return c[0]; }
template<typename Val, typename C>
auto
containerOfImpl(choice<1>, C const& c) -> if_compiles_return<Val,decltype(c.begin())> 
    { return *(c.begin()); }
template<typename Val, typename C>
using containerOf = conditional_t<std::is_same<stdx::decay_t<Val>,
                                               decltype(containerOfImpl<Val,C>(select_overload{},std::declval<C>()))>::value,
                                        std::true_type,
                                        std::false_type>;


template<typename... VArgs>
auto
make_array(VArgs&&... vargs)
    -> std::array<typename std::common_type<VArgs...>::type,sizeof...(VArgs)>
    {
    return {{ std::forward<VArgs>(vargs)... }};
    }

template<typename T>
std::vector<T>
reserve_vector(typename std::vector<T>::size_type size)
    {
    std::vector<T> v;
    v.reserve(size);
    return v;
    }

template<typename Container,
         typename F>
void
generate(Container&& C,
     F&& f)
    {
    std::generate(C.begin(),C.end(),std::forward<F>(f));
    }

template<typename Container,
         typename T>
void
fill(Container&& C,
     T&& val)
    {
    std::fill(C.begin(),C.end(),std::forward<T>(val));
    }

template<typename Container,
         typename T>
auto
count(Container&& C,
     T const& val) -> decltype(std::count(C.begin(),C.end(),val))
    {
    return std::count(C.begin(),C.end(),val);
    }

template<typename Container,
         typename T>
auto
find(Container&& C,
     T const& val) -> decltype(C.begin())
    {
    return std::find(C.begin(),C.end(),val);
    }

template<typename Container,
         class UnaryCmpFunc>
auto
find_if(Container&& C,
        UnaryCmpFunc&& f) -> decltype(C.begin())
    {
    return std::find_if(C.begin(),C.end(),std::forward<UnaryCmpFunc>(f));
    }

template<typename Container,
         class UnaryFunc>
void
for_each(Container&& C,
         UnaryFunc&& f)
    {
    std::for_each(std::begin(C),std::end(C),std::forward<UnaryFunc>(f));
    }

template<typename Container,
         class CmpFun>
void
sort(Container && C,
     CmpFun && f)
    {
    std::sort(std::begin(C),std::end(C),std::forward<CmpFun>(f));
    }

} //namespace stdx

#endif
