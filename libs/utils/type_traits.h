#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

namespace utils
{

  template<typename t>
  struct remove_reference
  {
    using type = t;
  };

  template<typename t>
  struct remove_reference<t&>
  {
    using type = t;
  };

  template<typename t>
  struct remove_reference<t&&>
  {
    using type = t;  
  };

  template<typename t>
  using remove_reference_t = typename remove_reference<t>::type;

  template<typename t> constexpr
  decltype(auto) move(t&& val)
  {
    return static_cast<remove_reference_t<t>>(val);
  }
  
}

#endif // TYPE_TRAITS_H
