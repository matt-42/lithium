#pragma once

#include <utility>
#include <vector>

namespace iod {

  template <typename E>
  struct Exp {};

  template <typename E>
  struct array_subscriptable;

  template <typename E>
  struct callable;

  template <typename E>
  struct assignable;

  template <typename E>
  struct array_subscriptable;


  template <typename M, typename... A>
  struct function_call_exp :
    public array_subscriptable<function_call_exp<M, A...>>,
    public callable<function_call_exp<M, A...>>,
    public assignable<function_call_exp<M, A...>>,
    public Exp<function_call_exp<M, A...>>
  {
    using assignable<function_call_exp<M, A...>>::operator=;

    function_call_exp(const M& m, A&&... a)
      : method(m), args(std::forward<A>(a)...) {}

    M method;
    std::tuple<A...> args;
  };

  template <typename O, typename M>
  struct array_subscript_exp :
    public array_subscriptable<array_subscript_exp<O, M>>,
    public callable<array_subscript_exp<O, M>>,
    public assignable<array_subscript_exp<O, M>>,
    public Exp<array_subscript_exp<O, M>>
  {
    using assignable<array_subscript_exp<O, M>>::operator=;

    array_subscript_exp(const O& o, const M& m) : object(o), member(m) {}
    
    O object;
    M member;
  };

  template <typename L, typename R>
  struct assign_exp : public Exp<assign_exp<L, R>>
  {
    typedef L left_t;
    typedef R right_t;

    template <typename U, typename V>
    assign_exp(U&& l, V&& r) : left(std::forward<U>(l)), right(std::forward<V>(r)) {}
    //assign_exp(U&& l, V&& r) : left(l), right(r) {}
    // assign_exp(const L& l, R&& r) : left(l), right(std::forward<R>(r)) {}

    L left;
    R right;
  };
  
  template <typename E>
  struct array_subscriptable
  {
  public:
    // Member accessor
    template <typename S>
    constexpr auto operator[](S&& s) const
    {
      return array_subscript_exp<E, S>(*static_cast<const E*>(this), std::forward<S>(s));
    }

  };

  template <typename E>
  struct callable
  {
  public:
    // Direct call.
    template <typename... A>
    constexpr auto operator()(A&&... args) const
    {
      return function_call_exp<E, A...>(*static_cast<const E*>(this),
                                        std::forward<A>(args)...);
    }

  };
 
  template <typename E>
  struct assignable
  {
  public:

    template <typename L>
    auto operator=(L&& l) const
    {
      return assign_exp<E, L>(static_cast<const E&>(*this), std::forward<L>(l));
    }

    template <typename L>
    auto operator=(L&& l)
    {
      return assign_exp<E, L>(static_cast<E&>(*this), std::forward<L>(l));
    }
    
    template <typename T>
    auto operator=(const std::initializer_list<T>& l) const
    {
      return assign_exp<E, std::vector<T>>(static_cast<const E&>(*this), std::vector<T>(l));
    }

  };

#define iod_query_declare_binary_op(OP, NAME)                           \
  template <typename A, typename B>                                     \
  struct NAME##_exp :                                                   \
   public assignable<NAME##_exp<A, B>>,                                 \
  public Exp<NAME##_exp<A, B>>                                          \
  {                                                                     \
    using assignable<NAME##_exp<A, B>>::operator=; \
    NAME##_exp()  {}                                                    \
    NAME##_exp(A&& a, B&& b) : lhs(std::forward<A>(a)), rhs(std::forward<B>(b)) {} \
    typedef A lhs_type;                                                 \
    typedef B rhs_type;                                                 \
    lhs_type lhs;                                                       \
    rhs_type rhs;                                                       \
  };                                                                    \
  template <typename A, typename B>                                     \
  inline                                                                \
  std::enable_if_t<std::is_base_of<Exp<A>, A>::value or \
                   std::is_base_of<Exp<B>, B>::value,\
                   NAME##_exp<A, B >>                                                    \
  operator OP (const A& b, const B& a)                                \
  { return NAME##_exp<std::decay_t<A>, std::decay_t<B>>{b, a}; }

  iod_query_declare_binary_op(+, plus);
  iod_query_declare_binary_op(-, minus);
  iod_query_declare_binary_op(*, mult);
  iod_query_declare_binary_op(/, div);
  iod_query_declare_binary_op(<<, shiftl);
  iod_query_declare_binary_op(>>, shiftr);
  iod_query_declare_binary_op(<, inf);
  iod_query_declare_binary_op(<=, inf_eq);
  iod_query_declare_binary_op(>, sup);
  iod_query_declare_binary_op(>=, sup_eq);
  iod_query_declare_binary_op(==, eq);
  iod_query_declare_binary_op(!=, neq);
  iod_query_declare_binary_op(&, logical_and);
  iod_query_declare_binary_op(^, logical_xor);
  iod_query_declare_binary_op(|, logical_or);
  iod_query_declare_binary_op(&&, and);
  iod_query_declare_binary_op(||, or);

}
