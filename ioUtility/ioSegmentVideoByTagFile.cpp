//
// Created by yago on 17/04/27.
//

#include <iostream>
//#include <opencv2/opencv.hpp>


#include "../similarityClusterer/similarityFileUtils.cpp"


using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::cerr;
using std::ofstream;

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
 * Formats a time in msec as a human readable string, e.g. 73500 -> 1m3s.
 */
string timeString(double msec) {
    double sec = msec / 1000;
    int fullMinutes = (int)floor(sec / 60);
    int roundedRemains = (int)sec - (fullMinutes * 60);
    return utils::combine({ to_string(fullMinutes), "m", to_string(roundedRemains), "s" });
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

int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "./segmentByTags inputVideoFile tagFile outputVideo" << endl;
        return 0;
    }
    string inputVideoFilePath = argv[1];
    string tagFilePath = argv[2];
    string outputVideoFilePath = argv[3];

    // Read segments from segment file - file has to be formatted like a tag file.
    ClusterInfoContainer segments;
    if (!similarityFileUtils::readTagFile(tagFilePath, &segments)) {
        return 1;
    }

    cv::VideoCapture capture(inputVideoFilePath);
    double totalFrames = capture.get(cv::CAP_PROP_FRAME_COUNT);

    if (utils::fileExists(outputVideoFilePath)) {
        errorFileToWriteExists(outputVideoFilePath);
    }

    cv::VideoWriter writer(outputVideoFilePath,0x21,capture.get(CV_CAP_PROP_FPS), cv::Size((int)capture.get(CV_CAP_PROP_FRAME_WIDTH), (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT)));

    if (!writer.isOpened()) {
        cerr << "Couldn't open video writer for \"" << outputVideoFilePath << "\"." << endl;
    }


    for (int i = 0; i < segments.size(); i++) {

        cout<<"segment "<<i<<endl;

        // Iterate through frames and write frame by frame
        for (int frameCounter = (int)capture.get(CV_CAP_PROP_POS_FRAMES); frameCounter <= totalFrames; frameCounter++) {
            Mat currentFrame;
            capture >> currentFrame;

            if (currentFrame.data == nullptr || segments[i].endMsec < capture.get(CV_CAP_PROP_POS_MSEC)) {
                break;
            }

            if (segments[i].containsFrame(FrameInfo(currentFrame, frameCounter, capture.get(CV_CAP_PROP_POS_MSEC)))) {
                writer.write(currentFrame);
            }
        }

        /*cout<<"NOW BUFFER"<<endl;//THIS SHIT IS NOT WORKING
        int bufferFrames=500;
        for (int i=0;i<bufferFrames;i++)
        {
            Mat blackImage(301, 260, CV_8UC3, cvScalar(0, 0, 0));
            writer.write(blackImage);
        }*/

    }

    capture.release();

    writer.release();
    successfulWrite(outputVideoFilePath);



}