#include "flat_map.h"
#include "flat_set.h"

int main()
{
    std::flat_map<int, int> m;
    std::flat_set<int> s;

    // m.insert({1, 2});
    // m.insert({7, 8});
    // m.insert({3, 4});

    s.insert(1);
    s.insert(7);
    s.insert(3);
}
