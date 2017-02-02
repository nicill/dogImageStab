//
// Created by yago on 17/01/20.
//

#include <opencv2/opencv.hpp>
#include "framewiseSimilarityMetric.h"
#include "opencvHistComparer.h"
#include "featureComparer.h"

using namespace std;
using namespace cv;

struct FrameInfo
{
    FrameInfo(Mat _frame,
              double _similarityToPrevious = -1,
              double _averageSimilarity = -1) {
        _frame.copyTo(this->frame);
        this->similarityToPrevious = _similarityToPrevious;
        this->averageSimilarity = _averageSimilarity;
    }

    double similarityToPrevious;
    double averageSimilarity;
    Mat frame;
};

int main(int argc, char **argv) {
    bool verbose = false;
    bool computerReadable = false;

    // Only valid case of one argument being given.
    if (argc == 2 && string(argv[1]) == "--help") {
        cout << "Usage:" << endl
             << "./computeMeasures [video] [metricIndex]" << endl
             << endl
             << "Arguments:" << endl
             << "-v: Enable verbose output." << endl
             << "-l: Computer readable output." << endl;
        return 0;
    }
    // Only valid cases of three arguments being given.
    else if (argc == 4 && string(argv[3]) == "-v") {
        verbose = true;
        cout << "Verbose output activated." << endl;
    }
    else if (argc == 4 && string(argv[3]) == "-l") {
        computerReadable = true;
    }
    else if (argc != 3) {
        cout << "./computeMeasures [video] [metricIndex] [flags]" << endl;
        return 0;
    }

    // Open the video capture.
    VideoCapture capture(argv[1]);

    if (!capture.isOpened()) {
        const string couldNotOpen = "similarityClusterer::main - Could not open video ";
        cout << couldNotOpen << argv[1] << endl;
        throw(couldNotOpen + argv[1]);
    }

    Size refS = Size(
            (int) capture.get(CV_CAP_PROP_FRAME_WIDTH),
            (int) capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    Mat current, previous;
    framewiseSimilarityMetric *comparer;
    int metricType = atoi(argv[2]);

    switch (metricType) {
        case 1  :
            comparer = new opencvHistComparer();
            break;
        case 2 :
            comparer = new featureComparer(featureComparer::SIFT, featureComparer::BF_L2);
            break;
        default : // Optional
            cout << "Invalid metric index provided: " << metricType << endl;
            return 0;
    }

    if (verbose) comparer->activateVerbosity();
    else if (computerReadable) {
        cout << "Frame no.,Milliseconds,Similarity to last frame" << endl;
    }

    // Framewise metric computation
    int frameCounter = 2;
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
    vector<FrameInfo> frameInfos = vector<FrameInfo>(totalFrames);

    // Load first frame
    capture >> previous;
    frameInfos[0] = FrameInfo(previous);
    for (;;) {
        capture >> current;

        if (current.data == NULL) {
            break;
        }

        double currentSimilarity = comparer->computeSimilarity(&previous, &current);

        if (verbose) {
            cout << "Similarity " << currentSimilarity
                 << " for frame #" << frameCounter << "/" << totalFrames
                 << " at " << capture.get(CAP_PROP_POS_MSEC) << " msec"
                 << " compared to the last frame" << endl;
        }
        else if (computerReadable) {
            string separator = ",";
            cout << frameCounter << separator
                 << capture.get(CAP_PROP_POS_MSEC) << separator
                 << currentSimilarity << endl;
        }

        // frameCounter is 1-based
        frameInfos[frameCounter-1] = FrameInfo(current, currentSimilarity);

        current.copyTo(previous);
        frameCounter++;
    }

    return 0;
}
