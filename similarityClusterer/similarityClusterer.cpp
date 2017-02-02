//
// Created by yago on 17/01/20.
//

#include <opencv2/opencv.hpp>
#include "framewiseSimilarityMetric.h"
#include "opencvHistComparer.h"
#include "opencvImageMetric.h"
#include "featureComparer.h"

using namespace std;
using namespace cv;

struct FrameInfo
{
    Mat frame;
    double frameNum;
    double similarityToPrevious;
    double averageSimilarity;

    FrameInfo() {}
    FrameInfo(Mat _frame,
              double _frameNum,
              double _similarityToPrevious = -1,
              double _averageSimilarity = -1) {
        _frame.copyTo(this->frame);
        this->frameNum = _frameNum;
        this->similarityToPrevious = _similarityToPrevious;
        this->averageSimilarity = _averageSimilarity;
    }
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
    else if (argc == 4 && string(argv[3]) == "-v") {
        verbose = true;
        cout << "Verbose output activated." << endl;
    }
    else if (argc == 4 && string(argv[3]) == "-l") {
        computerReadable = true;
    }
    else if (argc == 4 && ((atoi(argv[2]) == 1) || (atoi(argv[2]) == 3))) {
        // Valid arguments; Better solution necessary
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

    Mat current, previous;
    framewiseSimilarityMetric *comparer;
    int metricType = atoi(argv[2]);

    switch (metricType) {
        case 1  :
            comparer = new opencvHistComparer(atoi(argv[3]));
            if(verbose)
            {
                cout << "Computing histogram measures with value " << argv[3] << endl;
                //http://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=comparehist#comparehist
                cout << "Options - 0: Correlation 1: Chi-square 2: Intersection 3: Bhattacharyya" << endl;
            }
            break;
        case 2 :
            comparer = new featureComparer(featureComparer::SIFT, featureComparer::BF_L2);
            break;
        case 3:
            comparer = new opencvImageMetric(atoi(argv[3]));
            if(verbose)
            {
                cout << "Computing image metrics with value "<<argv[3] << endl;
                //http://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=comparehist#comparehist
                cout<<"Options 0: PSNR 1: SSIM "<<endl;
            }
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
    vector<FrameInfo> frameInfos;

    // Load first frame
    capture >> previous;
    frameInfos.push_back(FrameInfo(previous, 0));
    for (;;) {
        // TODO temp
        if (frameCounter > 100) break;

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
        frameInfos.push_back(FrameInfo(current, frameCounter, currentSimilarity));

        current.copyTo(previous);
        frameCounter++;
    }

    // Clustering
    for (FrameInfo info : frameInfos) {
        cout << info.frameNum << endl;
    }

    delete comparer;
    return 0;
}
