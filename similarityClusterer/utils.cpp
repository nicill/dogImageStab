//
// Created by tokuyama on 17/02/22.
//

#ifndef DOGIMAGESTABILIZATION_UTILS_CPP
#define DOGIMAGESTABILIZATION_UTILS_CPP

#include <vector>

using std::vector;

struct utils {
    template <typename T>
    static vector<T> package(T item) {
        return { item };
    }
};

#endif // DOGIMAGESTABILIZATION_UTILS_CPP
