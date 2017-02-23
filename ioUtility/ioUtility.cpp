//
// Created by tokuyama on 17/02/23.
//

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

/**
 * Main method.
 */
int main(int argc, char **argv) {
    // Idea: Allow for some operations, like creating new tag files
    // or outputting data in specific formats

    if (argc == 2 && string(argv[1]) == "--help") {
        cout << "Not implemented" << endl;
        return 0;
    }

    // Check for flag and read if given
    string last_arg = string(argv[argc - 1]);
    if (last_arg.find("-") == 0) {
        if (last_arg.find('X') != string::npos) {
            throw("NOT IMPLEMENTED");
        }
    }
}
