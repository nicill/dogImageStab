//
// Created by tokuyama on 17/02/23.
//

#include <iostream>

using std::cout;
using std::endl;
using std::string;

/**
 * Main method. Allow for some operations, like creating new tag files
 * or outputting data in specific formats
 */
int main(int argc, char **argv) {
    bool csvMode = false;
    bool tagMode = false;

    if (argc == 2 && string(argv[1]) == "--help") {
        cout << "Flags (only one at a time)" << endl
             << "-c: Create csv file from data" << endl
             << "-t: Read from / write to tag files" << endl;
        return 0;
    }

    // Check for flag and read if given
    string last_arg = string(argv[argc - 1]);
    if (last_arg.find("-") == 0) {
        if (last_arg.find('c') != string::npos) {
            csvMode = true;
            cout << "CSV mode activated" << endl;
        } else if (last_arg.find('t') != string::npos) {
            tagMode = true;
            cout << "Tag file mode activated" << endl;
        }
    }

    cout << "IMPLEMENTATION MISSING" << endl;
    return 1;
}
