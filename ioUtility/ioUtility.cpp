//
// Created by tokuyama on 17/02/23.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include "../similarityClusterer/defaults.h"
#include "../similarityClusterer/utils.cpp"
#include "../similarityClusterer/storageClasses/ClusterInfoContainer.cpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;
using cv::VideoCapture;

// Declarations
void help();
int  main(int argc, char **argv);
void mainCsvMode();
void mainTagMode();

string workingDirectory = "$HOME/";

/**
 * Output help.
 */
void help() {
    cout << "Usage: ./name video.mp4 metricIndex subSpecifier -flag" << endl
         << endl
         << "Flags (always supply exactly one)" << endl
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
    if (!hasFlag || argc != 5) {
        help();
        return 1;
    }

    int metricIndex = atoi(argv[2]);
    int subSpecifier = atoi(argv[3]);

    // TODO Check only for existing file
    VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        cerr << "Could not open the video supplied: " << argv[1] << endl;
        return 1;
    }

    // Check validity of working directory and tag file directory
    if (!utils::canOpenDir(defaults::workingDirectory) || !utils::canOpenDir(defaults::pathToTagFiles)) {
        cerr << "Could not open default directory, please check defaults file." << endl;
        return 1;
    }

    if (last_arg.find('c') != string::npos) {
        mainCsvMode();
    } else if (last_arg.find('t') != string::npos) {
        mainTagMode();
    }

    return 0;
}

void mainCsvMode() {
    cout << "CSV mode activated" << endl
         << "(Using working directory \"" << workingDirectory << "\"" << endl
         << " Reading similarity values from directory \"" << defaults::workingDirectory << "\")" << endl
         << " Reading tag files from directory \"" << defaults::pathToTagFiles << "\")" << endl;

    // 1) Read tag files and create ClusterInfos
    // 2) Read similarity from video (file), build clusters and label
    // 3) Write result to CSV file
    //    Frame, Msec, Similarity, Classification, Stop, Bark
    //    1,     0,    0.321,      1,              1,    1
    //    2,     200,  0.9833,     3,              0,    1
    //    ...

    // TODO implement
    throw("NOT IMPLEMENTED");
}

void mainTagMode() {
    cout << "Tag file mode activated" << endl;
    // TODO implement
    throw("NOT IMPLEMENTED");
}
