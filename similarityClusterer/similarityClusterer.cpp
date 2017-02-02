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
    double msec;
    double similarityToPrevious;
    double averageSimilarity;

    FrameInfo() {}
    FrameInfo(Mat _frame,
              double _frameNum,
              double _msec,
              double _similarityToPrevious = -1,
              double _averageSimilarity = -1) {
        _frame.copyTo(this->frame);
        this->frameNum = _frameNum;
        this->msec = _msec;
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
        // TODO Currently, as far as I understand, the combination "verbose" and this case can never happen.
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

    // Framewise metric computation
    int frameCounter = 1;
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
    vector<FrameInfo> frameInfos;

    // Load first frame
    capture >> previous;
    frameInfos.push_back(FrameInfo(previous, 1, 0));
    for (;;) {
        capture >> current;

        if (current.data == NULL) {
            break;
        }

        frameCounter++;

        double currentSimilarity = comparer->computeSimilarity(&previous, &current);
        frameInfos.push_back(FrameInfo(current, frameCounter, capture.get(CAP_PROP_POS_MSEC), currentSimilarity));

        current.copyTo(previous);

        if (verbose) {
            cout << "Similarity " << currentSimilarity
                 << " for frame #" << frameCounter << "/" << totalFrames
                 << " at " << capture.get(CAP_PROP_POS_MSEC) << " msec"
                 << " compared to the last frame" << endl;
        }
    }

    if (computerReadable) {
        cout << "Frame no.,Milliseconds,Similarity to last frame,Average similarity in region" << endl;
    }

    // Clustering - get average similarity for region
    int maxIndex = frameInfos.size() - 1;
    for (int i = 0; i < maxIndex; i++) {
        // calculate the average for each frame (5 back, 5 front)
        int start = 0;
        int end = maxIndex;

        if (i >= 5) start = i - 5;
        if (i <= maxIndex - 5) end = i + 5;

        int summedUpSimilarities = 0;
        for (int j = start; j <= end; j++) {
            summedUpSimilarities += frameInfos[j].similarityToPrevious;
        }
        frameInfos[i].averageSimilarity = summedUpSimilarities / (1 + end - start);

        if (computerReadable) {
            string separator = ",";
            cout << frameInfos[i].frameNum << separator
                 << frameInfos[i].msec << separator
                 << frameInfos[i].similarityToPrevious << separator
                 << frameInfos[i].averageSimilarity << endl;
        }
    }

    delete comparer;
    return 0;
}
