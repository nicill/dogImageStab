//
// Created by yago on 17/01/20.
//

#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <iomanip>
#include "framewiseSimilarityMetric.h"
#include "opencvHistComparer.h"
#include "featureComparer/featureComparer.h"


using namespace std;
using namespace cv;

int main(int argc, char **argv) {
    if (argc < 2) {
       // cout << "./computeMeasures [video] [code]" << endl;
       // cout << "code: 0=framewise entropy,  1=ITF" << endl;
       // return 0;
    }

    // make a function that returns a vector, with, for every time a label depending on the threshold of similarity


    bool verbose=true;
    int frameNum = 0;          // Frame counter

    VideoCapture capt(argv[1]); //open the same

    if (!capt.isOpened()){
        cout  << "similarityclusterer::main Could not open video " << argv[1] << endl;
        throw(" similarityclusterer::main Could not open video ");
    }

    Size refS = Size((int) capt.get(CV_CAP_PROP_FRAME_WIDTH),(int) capt.get(CV_CAP_PROP_FRAME_HEIGHT));

    Mat curr,prev;
    capt >> prev;

    // Declare similarity metric
    framewiseSimilarityMetric *metric;

    // Get the code of the type that we want from arg[2] and initialize pointer
    // 1: Histogram comparison
    int metrictype=atoi(argv[2]);


    switch(metrictype){
        case 1  :
            metric = new opencvHistComparer();
            break;
        case 2 :
            metric = new featureComparer(featureComparer::SIFT, featureComparer::BF);
            break;
        case 3 :
            metric = new featureComparer(featureComparer::SURF, featureComparer::BF);
            break;
        case 4 :
            metric = new featureComparer(featureComparer::ORB, featureComparer::BF);
            break;
        default : // Optional
           cout<<"Wrong metric code in similarity clusterer "<<metrictype<<endl;
            throw("Wrong metric code in similarity clusterer");
    }

    for(;;) //framewise metric computation
    {
        capt >> curr;

        if(curr.data == NULL) {
            break;
        }

        double currentSimilarity=metric->computeSimilarity(&curr,&prev);

        if(verbose) cout << "Frame: " << frameNum << "# has similarity "<<currentSimilarity<<endl;

        //update prev
        prev=curr;
        frameNum++;
    }



}