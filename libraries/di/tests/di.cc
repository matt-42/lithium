
#include <iod/di/di.hh>
#include <cassert>

struct A{};
struct A1{};
struct A2{};
struct A3{};
struct A4{};
struct A5{};
struct A6{};
struct A7{};
struct A8{};
struct A9{};
struct A10{};
struct A11{};
struct A12{};
struct A13{};
struct A14{};
struct A15{};
struct A16{};
struct A17{};
struct A18{};
struct A19{};

int main()
{

  {
    auto fun =  [] (int x) { return x; };  
    assert(iod::di_call(fun, 2) == 2);
  }

  {
    auto fun =  [] (int x, float y) { return x + y; };  
    assert(iod::di_call(fun, 2, 3.4f) == 5.4f);
    assert(iod::di_call(fun, 3.4f, 2) == 5.4f);
  }

  {
    auto fun =  [] (A, A1, A2, A3, A4) { };
    iod::di_call(fun, A{}, A1{}, A2{}, A3{}, A4{});
  }


  {
    auto fun =  [] (A, A1, A2, A3, A4, A5, A6, A7, A8, A9) { };
    iod::di_call(fun, A8{}, A9{}, A{}, A1{}, A5{}, A6{}, A7{}, A2{}, A3{}, A4{});
  }


  {
    auto fun =  [] (A, A1, A2, A3, A4, A5, A6, A7, A8, A9,
                    A10, A11, A12, A13, A14, A15, A16, A17, A18, A19) { };
    iod::di_call(fun, A{}, A1{}, A2{}, A3{}, A4{}, A5{}, A6{}, A7{}, A8{}, A9{},
                 A10{}, A11{}, A12{}, A13{}, A14{}, A15{}, A16{}, A17{}, A18{}, A19{});
  }



  {
    auto fun =  [] (A, A1, A2, A3, A4, A5, A6, A7, A8, A9,
                    A10, A11, A12, A13, A14, A15, A16, A17, A18, A19) { };
    iod::di_call(fun, A{}, A1{}, A2{}, A3{}, A4{}, A5{}, A6{}, A17{}, A7{}, A8{}, A9{},
                 A10{}, A11{}, A12{}, A13{}, A14{}, A15{}, A16{}, A18{}, A19{});
  }


  {
    auto fun =  [] (A, A1, A2, A3, A4, A5, A6, A7, A8, A9,
                    A10, A11, A12, A13, A14, A15, A16, A17, A18, A19) { };
    iod::di_call(fun, A{}, A1{}, A2{}, A3{}, A4{}, A5{}, A6{}, A17{}, A7{}, A8{}, A9{},
                 A10{}, A11{}, A12{}, A14{}, A13{}, A15{}, A16{}, A18{}, A19{});
  }

  {
    auto fun =  [] (A, A1, A2, A3, A4, A5, A6, A7, A8, A9,
                    A10, A11, A12, A13, A14, A15, A16, A17, A18, A19) { };
    iod::di_call(fun, A{}, A1{}, A2{}, A13{}, A4{}, A5{}, A6{}, A17{}, A7{}, A8{}, A9{},
                 A10{}, A11{}, A12{}, A3{}, A14{}, A15{}, A16{}, A18{}, A19{});
  }

  {
    auto fun =  [] (A, A1, A2, A3, A4, A5, A6, A7, A8, A9,
                    A10, A11, A12, A13, A14, A15, A16, A17, A18, A19) { };
    iod::di_call(fun, A{}, A1{}, A2{}, A4{}, A3{}, A5{}, A6{}, A17{}, A7{}, A8{}, A9{},
                 A10{}, A11{}, A12{}, A13{}, A14{}, A15{}, A16{}, A18{}, A19{});
  }
  

}
