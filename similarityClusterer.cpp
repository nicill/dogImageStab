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
    if (argc == 1 && argv[0] == "--help") {
        cout << "Usage:" << endl
             << "./computeMeasures [video] [metricIndex]" << endl
             << endl
             << "Arguments:" << endl
             << "-v / --verbose: Enable verbose logging." << endl;
        return 0;
    }
    else if (argc < 2) {
        cout << "./computeMeasures [video] [metricIndex]" << endl;
        return 0;
    }

    // TODO read argument
    bool verbose=true;

    // make a function that returns a vector, with, for every time a label depending on the threshold of similarity

    // Open capture
    VideoCapture capture(argv[1]);

    if (!capture.isOpened()) {
        const string couldNotOpen = "similarityclusterer::main - Could not open video ";
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
            cout << "Invalid metric index provided:" << metricType << endl;
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

        double currentSimilarity = metric->computeSimilarity(&current,&previous);

        if (verbose) cout << "Frame: " << frameCounter << "# has similarity " << currentSimilarity << endl;

        current.copyTo(previous);
    }
}
