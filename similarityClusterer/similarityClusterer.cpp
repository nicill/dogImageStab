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

int main(int argc, char **argv) {
    bool verbose = true;

    // Only valid case of one argument being given.
    if (argc == 2 && string(argv[1]) == "--help") {
        cout << "Usage:" << endl
             << "./computeMeasures [video] [metricIndex]" << endl
             << endl
             << "Arguments:" << endl
             << "-v: Enable verbose logging." << endl;
        return 0;
    }
    // Only two valid cases of three arguments being given.
    else if (argc == 4 && string(argv[3]) == "-v") {
        verbose = true;
        cout << "Verbose logging activated." << endl;
    }
    else if (argc == 4 && ( (atoi(argv[2]) == 1)||(atoi(argv[2]) == 3)) ) {
        if(verbose)
        {
           if((atoi(argv[2]) == 1))
           {
               cout << "Computing histogram measures with value "<<argv[3] << endl;
            //http://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=comparehist#comparehist
            cout<<"Options 0: Correlation 1:Chi-square 2:intersection 3:Bhattacharyya "<<endl;
           }
           else if((atoi(argv[2]) == 3))
           {
                cout << "Computing image metrics with value "<<argv[3] << endl;
                //http://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=comparehist#comparehist
                cout<<"Options 0: PSNR 1: SSIM "<<endl;
            }

        }
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
            comparer = new opencvHistComparer(atoi(argv[3]));
            break;
        case 2 :
            comparer = new featureComparer(featureComparer::SIFT, featureComparer::BF_L2);
            break;
        case 3:
            comparer = new opencvImageMetric(atoi(argv[3]));
            break;
        default : // Optional
            cout << "Invalid metric index provided: " << metricType << endl;
            return 0;
    }

    if (verbose) comparer->activateVerbosity();

    // Load first frame
    capture >> previous;

    // Framewise metric computation
    int frameCounter = 2;
    for (;;) {
        capture >> current;

        if (current.data == NULL) {
            break;
        }

        double currentSimilarity = comparer->computeSimilarity(&previous, &current);

        if (verbose) {
            cout << "Frame #" << frameCounter
                 << " has similarity " << currentSimilarity
                 << " to the last frame" << endl;
        }

        current.copyTo(previous);
        frameCounter++;
    }

    return 0;
}
