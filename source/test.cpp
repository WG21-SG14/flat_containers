#include "flat_map.h"
#include "flat_set.h"

#include <iostream>

int main()
{
    std::flat_map<int, int> m;
    std::flat_set<int> s;

    m.insert({1, 2});
    m.insert({7, 8});
    m.insert({3, 4});
    m[5] = 6;

    for (auto v : m)
        std::cout << v.first << ':' << v.second << ' ';

    s.insert(1);
    s.insert(7);
    s.insert(3);
    s.insert(5);

    for (auto v : s)
        std::cout << v << ' ';
}
