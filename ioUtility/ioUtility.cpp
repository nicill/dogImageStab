//
// Created by tokuyama on 17/02/23.
//

#include <iostream>
#include "../similarityClusterer/defaults.h"

using std::cout;
using std::endl;
using std::string;

// Declarations
void help();
void mainCsvMode();
void mainTagMode();

/**
 * Output help.
 */
void help() {
    cout << "Usage: ./name video.mp4 [-flag]" << endl
         << endl
         << "Flags (only one at a time)" << endl
         << "-c: Create csv file from data" << endl
         << "-t: Read from / write to tag files" << endl;
}

/**
 * Main method. Allow for some operations, like creating new tag files
 * or outputting data in specific formats
 */
int main(int argc, char **argv) {
    if (argc == 2 && string(argv[1]) == "--help") {
        help();
        return 0;
    }

    // Check for flag
    string last_arg = string(argv[argc - 1]);
    bool hasFlag = last_arg.find("-") == 0;

    // Check validity of arguments
    if ((hasFlag && argc != 3) || !hasFlag && argc != 2) {
        help();
        return 1;
    }

    if (hasFlag) {
        if (last_arg.find('c') != string::npos) {
            mainCsvMode();
        } else if (last_arg.find('t') != string::npos) {
            mainTagMode();
        }
    }

    // TODO check validity of given video

    // TODO
    cout << "IMPLEMENTATION MISSING" << endl;
    return 1;
}

void mainCsvMode() {
    string workingDirectory = "$HOME/";
    cout << "CSV mode activated" << endl
         << "(Using working directory \"" << workingDirectory << "\"" << endl
         << " Reading similarity values from directory \"" << defaults::workingDirectory << "\")" << endl
         << " Reading tag files from directory \"" << defaults::pathToTagFiles << "\")" << endl;

    // 1) Read tag files and create ClusterInfos
    // 2) Read similarity from video (file), build clusters and label
    // 3) Write result to CSV file
    //    Frame, Msec, Similarity, Classification, Stop, Bark
    //    1      0     0.321       1               1     1
    //    2      200   0.9833      3               0     1
    //    ...

    // TODO implement
    throw("NOT IMPLEMENTED");
}

void mainTagMode() {
    cout << "Tag file mode activated" << endl;
    // TODO implement
    throw("NOT IMPLEMENTED");
}
