//
// Created by tokuyama on 17/02/22.
//

#ifndef DOGIMAGESTABILIZATION_UTILS_CPP
#define DOGIMAGESTABILIZATION_UTILS_CPP

#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>

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
     * Combines all given strings into one.
     * Optionally adds a separator between the strings.
     */
    static string combine(vector<string> strings, string sep = "") {
        string result = "";
        for (int i = 0; i < strings.size(); i++) {
            result += strings[i];

            if (i != strings.size() - 1) {
                result += sep;
            }
        }

        return result;
    }

    /**
     * Checks if the first string contains the second.
     */
    static bool stringContains(string s, string c) {
        return s.find(c) != string::npos;
    }

    /**
     * Checks if the first string contains any string in the string vector.
     */
    static bool stringContainsAny(string s, vector<string> c_s) {
        for (string c : c_s) {
            if (stringContains(s, c)) {
                return true;
            }
        }

        return false;
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
    static string getCsvFileName(string videoName, int metricIndex, int subSpecifier, bool extension = true) {
        string name = videoName + "_" + to_string(metricIndex) + "_" + to_string(subSpecifier);

        return extension
            ? name + ".csv"
            : name;
    }

    /**
     * Replacement for basename for string. Extracts the file name (including extension) from a path.
     */
    static string getBasename(string path) {
        // Not implemented for Windows system paths
        assert(path.find("\\") == string::npos);
        size_t lastSlash = path.find_last_of("/");
        if (lastSlash == string::npos) {
            return path;
        } else {
            size_t beginOfName = lastSlash + 1;
            return path.substr(beginOfName, path.size() - beginOfName);
        }
    }

    /**
     * Removes the extension from a string. If none is found, it's not modified.
     * @param s The string. The last extension of the form ".*" is removed.
     */
    static string removeExtension(string s) {
        size_t lastDot = s.find_last_of(".");
        if (lastDot == string::npos) {
            return s;
        } else {
            return s.substr(0, lastDot);
        }
    }
};

#endif // DOGIMAGESTABILIZATION_UTILS_CPP
