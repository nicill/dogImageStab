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
bool getTagFilesByNames(string, vector<string>, vector<string>, ClusterInfoContainer*, ClusterInfoContainer*);
void announceMode(string mode, vector<string> infos);
void successfulWrite(string filePath);
void errorFileToWriteExists(string filePath);

string workingDirectory = utils::combine({ getenv("HOME"), "/dog/results" });
string videoName_noExt;
string ioFileName_noExt;
string ioFilePath;
string pathToTagFiles;
double totalFrames;
// Messages
string workingDirectoryMessage;
string readingTagFilesFromMessage;

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

    // Is the video path given valid?
    cv::VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        cerr << "Could not open the video supplied: " << argv[1] << endl;
        return 1;
    }
    totalFrames = capture.get(cv::CAP_PROP_FRAME_COUNT);
    capture.release();
    string videoName_ext = basename(argv[1]);
    videoName_noExt = utils::removeExtension(videoName_ext);

    // Check validity of working directory and tag file directory
    pathToTagFiles = utils::combine({ defaults::tagFileDirectory, "/", videoName_noExt });
    if (!utils::canOpenDir(defaults::workingDirectory) || !utils::canOpenDir(pathToTagFiles)) {
        cerr << "Could not open directory, please check these directories:" << endl
             << defaults::workingDirectory << endl
             << pathToTagFiles << endl;
        return 1;
    }

    // Check existence of IO file.
    string ioFileName_ext = utils::getCsvFileName(videoName_ext, metricIndex, subSpecifier);
    ioFilePath = utils::combine({ defaults::workingDirectory, "/", ioFileName_ext });
    if (!utils::fileExists(ioFilePath)) {
        cout << "No IO file \"" << ioFilePath << "\" found. Please calculate similarity and save to the given file "
             << "before running this program." << endl;
        return 1;
    }

    // Set global variables
    ioFileName_noExt = utils::removeExtension(ioFileName_ext);
    workingDirectoryMessage    = "Using working directory \"" + workingDirectory + "\"";
    readingTagFilesFromMessage = "Reading tag files from directory \"" + pathToTagFiles + "\"";

    // Main logic.
    if (last_arg.find('c') != string::npos) {
        return mainCsvMode();
    } else if (last_arg.find('t') != string::npos) {
        return mainTagMode();
    }
}

/**
 * Main method for CSV mode.
 */
int mainCsvMode() {
    announceMode("CSV mode",
                 { workingDirectoryMessage,
                   utils::combine({ "Reading similarity values from directory \"", defaults::workingDirectory, "\"" }),
                   readingTagFilesFromMessage });

    // 1) Read tag files and make sure we have tag files for bark and stop.
    ClusterInfoContainer barkFileClusters;
    ClusterInfoContainer stopFileClusters;
    bool success = getTagFilesByNames(pathToTagFiles,
                                      {"bark", "Bark"}, {"stop", "Stop"},
                                      &barkFileClusters, &stopFileClusters);

    if (!success) {
        return 1;
    }

    // 2) Read similarity from analysis file, build clusters and label
    vector<FrameInfo> frames;
    if (!similarityFileUtils::readFrameInfosFromCsv(ioFilePath, totalFrames, &frames)) {
        return 1;
    }
    //    Steps: Calculate region average, cluster, classify clusters
    clusterer::calculateRegionAverage(&frames);
    ClusterInfoContainer clusters = clusterer::cluster(clusterer::AVERAGE_REFINED, frames, false);
    classifier::classifyClusters(&clusters);

    vector<FrameInfo>::iterator framesIterator = frames.begin();
    for (const auto &cluster : clusters.clusterInfos) {
        for (const auto &frame : cluster.frames) {
            assert((*framesIterator).frameNo == frame.frameNo);

            if (cluster.label == classifier::lowSimLabel) {
                (*framesIterator).label = "1";
            } else if (cluster.label == classifier::mediumSimLabel) {
                (*framesIterator).label = "2";
            } else if (cluster.label == classifier::highSimLabel) {
                (*framesIterator).label = "3";
            } else {
                throw ("Label is unknown");
            }

            framesIterator++;
        }
    }

    // 3) Write result to CSV file
    //    Frame, Msec, Similarity, Avg. similarity, Classification, Stop, Bark
    //    1,     0,    0.321,      0.6663333211     1,              1,    1
    //    2,     200,  0.9833,     0.8877472988     3,              0,    1
    //    ...
    ofstream fileStream;
    string filePath = workingDirectory + "/" + ioFileName_noExt + "_extended.csv";
    if (utils::fileExists(filePath)) {
        errorFileToWriteExists(filePath);
        return 1;
    }

    fileStream.open(filePath);
    fileStream << "Frame,Msec,Similarity,Avg. similarity,Classification,Stop,Bark" << endl;
    fileStream.close();

    // Iterate through all FrameInfo objects and save info plus stop and bark field
    vector<ClusterInfo>::iterator barkFileClustersIterator = barkFileClusters.clusterInfos.begin();
    vector<ClusterInfo>::iterator stopFileClustersIterator = stopFileClusters.clusterInfos.begin();
    for (FrameInfo frame : frames) {
        while (frame.msec > (*barkFileClustersIterator).endMsec
               && barkFileClustersIterator != barkFileClusters.clusterInfos.end()) {
            barkFileClustersIterator++;
        }
        while (frame.msec > (*stopFileClustersIterator).endMsec
               && stopFileClustersIterator != stopFileClusters.clusterInfos.end()) {
            stopFileClustersIterator++;
        }

        bool stop = (*stopFileClustersIterator).containsFrame(frame);
        bool bark = (*barkFileClustersIterator).containsFrame(frame);

        similarityFileUtils::appendToCsv(filePath, frame.frameNo, frame.msec, frame.similarityToPrevious,
                                         frame.averageSimilarity, frame.label, stop, bark);
    }

    successfulWrite(filePath);
    return 0;
}

/**
 * Main method for tag mode.
 */
int mainTagMode() {
    announceMode("Tag file mode",
                 { workingDirectoryMessage,
                   readingTagFilesFromMessage });

    // 1. Load tag files to be combined and make sure we have tag files for bark and stop.
    ClusterInfoContainer barkFileClusters;
    ClusterInfoContainer stopFileClusters;
    bool success = getTagFilesByNames(pathToTagFiles,
                                      {"bark", "Bark"}, {"stop", "Stop"},
                                      &barkFileClusters, &stopFileClusters);

    if (!success) {
        return 1;
    }
    // 2. Iterate through clusters and combine with AND
    ClusterInfoContainer overlaps = ClusterInfoContainer("Overlaps of bark and stop");
    vector<ClusterInfo>::iterator barkFileClustersIterator = barkFileClusters.clusterInfos.begin();
    vector<ClusterInfo>::iterator stopFileClustersIterator = stopFileClusters.clusterInfos.begin();
    // TODO Solve without code duplication (see qualityMeasurer.getClusterOverlapMsec())
    while (barkFileClustersIterator != barkFileClusters.clusterInfos.end()
           && stopFileClustersIterator != stopFileClusters.clusterInfos.end()) {
        if ((*barkFileClustersIterator).beginMsec >= (*stopFileClustersIterator).endMsec) {
            stopFileClustersIterator++;
            continue;
        }

        if ((*stopFileClustersIterator).beginMsec >= (*barkFileClustersIterator).endMsec) {
            barkFileClustersIterator++;
            continue;
        }

        ClusterInfo overlap = (*barkFileClustersIterator).getOverlap((*stopFileClustersIterator));
        assert(overlap.beginMsec != -1 && overlap.endMsec != -1);
        overlaps.add(overlap);

        // Iterate the cluster that has been matched to its end.
        if (overlap.endMsec == (*stopFileClustersIterator).endMsec) {
            stopFileClustersIterator++;
        } else {
            barkFileClustersIterator++;
        }
    }

    // 3. Save as new tag file
    ofstream fileStream;
    string filePath = workingDirectory + "/bark+stop.csv";
    if (utils::fileExists(filePath)) {
        errorFileToWriteExists(filePath);
        return 1;
    }

    fileStream.open(filePath);
    fileStream << "start,stop" << utils::tagFileEol;
    for (const auto &cluster : overlaps.clusterInfos) {
        fileStream << cluster.beginMsec << "," << cluster.endMsec << utils::tagFileEol;
    }
    fileStream.close();

    successfulWrite(filePath);
    return 0;
}

/**
 * Checks the given list for two ClusterInfoContainer objects with one of the respective, given names.
 */
bool getTagFilesByNames(string pathToTagFiles,
                        vector<string> names_1, vector<string> names_2,
                        ClusterInfoContainer *fileClusters_1, ClusterInfoContainer *fileClusters_2) {
    vector<ClusterInfoContainer> allTagFileClusters = similarityFileUtils::readTagFiles(pathToTagFiles, false);

    int hasBoth = 0;
    for (ClusterInfoContainer tagFile : allTagFileClusters) {
        if (utils::stringContainsAny(tagFile.name, names_1)) {
            hasBoth |= 1;
            (*fileClusters_1) = tagFile;
        } else if (utils::stringContainsAny(tagFile.name, names_2)) {
            hasBoth |= 2;
            (*fileClusters_2) = tagFile;
        }
    }

    if ((hasBoth & 3) != 3) {
        cerr << "Couldn't find tag files for "
             << utils::combine(names_1, ", ") << " and/or "
             << utils::combine(names_2, ", ") << ". Please make sure both exist and are named correctly."
             << endl;
        return false;
    }

    return true;
}

/**
 * Internal use. Announce mode and infos.
 */
void announceMode(string mode, vector<string> infos) {
    cout << mode << " activated" << endl;

    for (const auto &info : infos) {
        cout << "- " << info << endl;
    }

    cout << endl;
}

/**
 * Internal use. Announce successful write to file.
 */
void successfulWrite(string filePath) {
    cout << endl << "File \"" << filePath << "\" has been written successfully." << endl;
}

/**
 * Internal use. Announce error that file to write to exists.
 */
void errorFileToWriteExists(string filePath) {
    cerr << "Please make sure the file \"" << filePath << "\" that's supposed to be written to doesn't exist" << endl;
}
