//
// Created by tokuyama on 17/02/23.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "../similarityClusterer/defaults.h"
#include "../similarityClusterer/clusterer.h"
#include "../similarityClusterer/classifier.h"
#include "../similarityClusterer/similarityFileUtils.cpp"

using std::string;
using std::to_string;
using std::cout;
using std::cerr;
using std::endl;
using std::ofstream;

// Declarations
void help();
int  main(int argc, char **argv);
int  mainCsvMode();
int  mainTagMode();

string workingDirectory = "$HOME";
string ioFileName;
string ioFilePath;
string pathToTagFiles;
double totalFrames;

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

    // Check validity of working directory and tag file directory
    if (!utils::canOpenDir(defaults::workingDirectory) || !utils::canOpenDir(defaults::tagFileDirectory)) {
        cerr << "Could not open default directory, please check defaults file." << endl;
        return 1;
    }

    // Is the video path given valid?
    cv::VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        cerr << "Could not open the video supplied: " << argv[1] << endl;
        return 1;
    }
    totalFrames = capture.get(cv::CAP_PROP_FRAME_COUNT);
    capture.release();
    string videoName = basename(argv[1]);
    pathToTagFiles = utils::combine({ defaults::tagFileDirectory, "/", videoName });

    ioFileName = utils::getCsvFileName(basename(argv[1]), metricIndex, subSpecifier);
    ioFilePath = utils::combine({ defaults::workingDirectory, "/", ioFileName });
    if (!utils::fileExists(ioFilePath)) {
        cout << "No IO file \"" << ioFilePath << "\" found. Please calculate similarity and save to the given file "
             << "before running this program." << endl;
        return 1;
    }

    if (last_arg.find('c') != string::npos) {
        return mainCsvMode();
    } else if (last_arg.find('t') != string::npos) {
        return mainTagMode();
    }
}

int mainCsvMode() {
    cout << "CSV mode activated" << endl
         << "(Using working directory \"" << workingDirectory << "\"" << endl
         << " Reading similarity values from directory \"" << defaults::workingDirectory << "\")" << endl
         << " Reading tag files from directory \"" << pathToTagFiles << "\")" << endl;

    // 1) Read tag files and create ClusterInfos
    vector<ClusterInfoContainer> clustersFromAllFiles =
            similarityFileUtils::readTagFiles(pathToTagFiles, false);

    // 2) Read similarity from video file, build clusters and label
    vector<FrameInfo> frameInfos = similarityFileUtils::readFrameInfosFromCsv(ioFilePath, totalFrames);
    //    Steps: Calculate region average, cluster, classify clusters
    clusterer::calculateRegionAverage(&frameInfos);
    ClusterInfoContainer clusters = clusterer::cluster(clusterer::AVERAGE_REFINED, frameInfos, false);
    classifier::classifyClusters(&clusters);

    // 3) Write result to CSV file
    //    Frame, Msec, Similarity, Classification, Stop, Bark
    //    1,     0,    0.321,      1,              1,    1
    //    2,     200,  0.9833,     3,              0,    1
    //    ...
    ofstream fileStream;
    string filePath = workingDirectory + "/" + ioFileName;
    if(utils::fileExists(ioFilePath)) {
        cerr << "Please make sure the file \"" << filePath << "\" that's supposed to be written to doesn't exist" << endl;
        return 1;
    }

    fileStream.open(workingDirectory + "/" + ioFileName);
    fileStream << "Frame,Msec,Similarity,Classification,Stop,Bark" << endl;


    fileStream.close();

    // TODO implement
    throw("NOT IMPLEMENTED");
}

int mainTagMode() {
    cout << "Tag file mode activated" << endl;
    // TODO implement
    throw("NOT IMPLEMENTED");
}
