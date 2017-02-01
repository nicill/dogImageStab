//
// Created by yago on 17/01/20.
//

#include <opencv2/opencv.hpp>
#include "framewiseSimilarityMetric.h"
#include "opencvHistComparer.h"
#include "featureComparer.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv) {
    bool verbose = false;

    // Only valid case of one argument being given.
    if (argc == 2 && string(argv[1]) == "--help") {
        cout << "Usage:" << endl
             << "./computeMeasures [video] [metricIndex]" << endl
             << endl
             << "Arguments:" << endl
             << "-v: Enable verbose logging." << endl;
        return 0;
    }
    // Only valid case of three arguments being given.
    else if (argc == 4 && string(argv[3]) == "-v") {
        verbose = true;
        cout << "Verbose logging activated." << endl;
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
    int frameCounter = 0;

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

    // Load first frame
    capture >> previous;

    // Framewise metric computation
    for (;;) {
        capture >> current;
        frameCounter++;

        if (current.data == NULL) {
            break;
        }

        double currentSimilarity = comparer->computeSimilarity(&previous, &current);

        if (verbose) cout << "Frame: #" << frameCounter << " has similarity " << currentSimilarity << endl;

        current.copyTo(previous);
    }

    delete comparer;
    return 0;
}
