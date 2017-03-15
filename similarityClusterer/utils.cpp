//
// Created by tokuyama on 17/02/22.
//

#ifndef DOGIMAGESTABILIZATION_UTILS_CPP
#define DOGIMAGESTABILIZATION_UTILS_CPP

#include <string>
#include <vector>
#include <dirent.h>

using std::string;
using std::vector;

struct utils {
    /**
     * Packages an item in a list of that item's type.
     */
    template <typename T>
    static vector<T> package(T item) {
        return { item };
    }

    /**
     * Checks to see if a given directory can be opened.
     * @param path The directory to check.
     * @return True for success.
     */
    static bool canOpenDir(string path) {
        DIR *directory = opendir(path.c_str());
        if (directory == nullptr || readdir(directory) == nullptr) {
            return false;
        }

        closedir(directory);
        return true;
    }
};

#endif // DOGIMAGESTABILIZATION_UTILS_CPP
