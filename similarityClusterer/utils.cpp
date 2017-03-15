//
// Created by tokuyama on 17/02/22.
//

#ifndef DOGIMAGESTABILIZATION_UTILS_CPP
#define DOGIMAGESTABILIZATION_UTILS_CPP

#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

using std::string;
using std::to_string;
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
     * Checks if a given directory can be opened.
     */
    static bool canOpenDir(string path) {
        DIR *directory = opendir(path.c_str());
        if (directory == nullptr || readdir(directory) == nullptr) {
            return false;
        }

        closedir(directory);
        return true;
    }

    /**
     * Checks if a file exists at the given path.
     */
    static bool fileExists(string filePath) {
        struct stat result;
        int fileNotFound = stat(filePath.c_str(), &result);
        return fileNotFound == 0; // 0 means false
    }

    /**
     * Builds the csv file name (so that all methods use the same naming scheme).
     */
    static string getCsvFileName(string videoName, int metricIndex, int subSpecifier) {
        return videoName + "_" + to_string(metricIndex) + "_" + to_string(subSpecifier) + ".csv";
    }
};

#endif // DOGIMAGESTABILIZATION_UTILS_CPP
