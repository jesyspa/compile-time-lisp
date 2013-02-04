#include "ctlisp.hpp"
#include <cassert>
#include <iostream>

auto one = compile<nil, constant<1>>::result();
auto two = compile<nil, list<plus, constant<1>, constant<1>>>::result();
auto id = compile<list<struct a>, var<struct a>>::result();
auto inc = compile<list<struct a>, list<plus, constant<1>, var<struct a>>>::result();

int main() {
    assert(one() == 1);
    assert(two() == 2);
    assert(id(3) == 3);
    assert(inc(3) == 4);
}
