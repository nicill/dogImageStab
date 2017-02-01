//
// Created by yago on 17/01/20.
//

#include <opencv2/opencv.hpp>
#include "framewiseSimilarityMetric.h"
#include "opencvHistComparer.h"
#include "featureComparer/featureComparer.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv) {
    bool verbose = false;

    // Only valid case of one argument being given.
    if (argc == 1 && argv[0] == "--help") {
        cout << "Usage:" << endl
             << "./computeMeasures [video] [metricIndex]" << endl
             << endl
             << "Arguments:" << endl
             << "-v: Enable verbose logging." << endl;
        return 0;
    }
    // Only valid case of three arguments being given.
    else if (argc == 3 && argv[0] == "-v") {
        verbose = true;
        cout << "Verbose logging activated." << endl;
    }
    else if (argc != 2) {
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
    framewiseSimilarityMetric *metric;
    int metricType = atoi(argv[2]);
    int frameCounter = 0;

    switch (metricType) {
        case 1  :
            metric = new opencvHistComparer();
            break;
        case 2 :
            metric = new featureComparer(featureComparer::SIFT, featureComparer::BF_L2);
            break;
        case 3 :
            metric = new featureComparer(featureComparer::SURF, featureComparer::BF_L2);
            break;
        case 4 :
            metric = new featureComparer(featureComparer::ORB, featureComparer::BF_L2);
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

        double currentSimilarity = metric->computeSimilarity(&previous, &current);

        if (verbose) cout << "Frame: #" << frameCounter << " has similarity " << currentSimilarity << endl;

        current.copyTo(previous);
    }
}
