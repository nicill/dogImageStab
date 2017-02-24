//
// Created by yago on 16/12/08.
//
#include <opencv2/opencv.hpp>
#include "videoStabilizationQualityMeasures.h"
#include "similarityClusterer/framewiseSimilarityMetric.h"
#include "similarityClusterer/opencvHistComparer.h"
#include "similarityClusterer/featureComparer.h"
#include "similarityClusterer/opencvITKmetric.h"


using namespace std;
using namespace cv;


double computeOtherMetrics(String video, framewiseSimilarityMetric* comparer)
{
    // Open the video capture.
    VideoCapture capture(video);

    if (!capture.isOpened()) {
        const string couldNotOpen = "computeOtherMetrics - Could not open video ";
        cout << couldNotOpen << video << endl;
        throw(couldNotOpen +video);
    }

    Mat current, previous;

    // Framewise metric computation
    int frameCounter = 1;
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);

    double average=0;
    // Load first frame
    capture >> previous;
    for (;;) {
        capture >> current;

        if (current.data == NULL) {
            break;
        }

        frameCounter++;

        double currentSimilarity = comparer->computeSimilarity(&previous, &current);

        // accumulate
        average=average+currentSimilarity;
        //cout<<"Frame "<<frameCounter<<"/"<<totalFrames<<" returned similarity "<<current<<" current average "<<average<<endl;

        current.copyTo(previous);
    }

    return average/totalFrames;
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        cout << "./computeMeasures [video] [code]" << endl;
        cout << "code: 0=framewise entropy,  1=ITF" << endl;
        return 0;
    }


videoStabilizationQualityMeasures vidStabMeasurer;
framewiseSimilarityMetric* metric;

    switch(atoi(argv[2])) {
        case 0 :
            //cout << "Computing framewise entropy for "<<argv[1]<< endl;
            cout<<vidStabMeasurer.framewiseEntropy(argv[1]);
            break;
        case 1 :
            //cout << "Computing ITF for "<<argv[1]<< endl;
            cout<<vidStabMeasurer.ITF(argv[1]);
            break;
        case 2 :
            //cout << "Computing black Pixel percentage for "<<argv[1]<< endl;
            cout<<vidStabMeasurer.blackPixelPercent(argv[1]);
            break;
        case 3 :
            //cout << "Computing SSIM for "<<argv[1]<< endl;
                //http://docs.opencv.org/2.4/doc/tutorials/highgui/video-input-psnr-ssim/video-input-psnr-ssim.html
            //“Z. Wang, A. C. Bovik, H. R. Sheikh and E. P. Simoncelli, “Image quality assessment: From error visibility to structural similarity,” IEEE Transactions on Image Processing, vol. 13, no. 4, pp. 600-612, Apr. 2004.”
            cout<<vidStabMeasurer.SSIM(argv[1]);
            break;
        case 4: //histogram 0
            metric=new opencvHistComparer(0);
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        case 5: //histogram 1
            metric=new opencvHistComparer(1);
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        case 6: //histogram 2
            metric=new opencvHistComparer(2);
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        case 7: //histogram 3
            metric=new opencvHistComparer(3);
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        case 8: //feature 0 SIFT
            metric=new featureComparer((featureComparer::type)0);
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        case 9: //feature 1 SURF
            metric=new featureComparer((featureComparer::type)1);
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        case 10: //feature 2 ORB
            metric=new featureComparer((featureComparer::type)2);
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        case 11: //feature 3 BRISK
            metric=new featureComparer((featureComparer::type)3);
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        case 12: //ITK 1 MS
            metric=new opencvITKmetric();
            cout<<computeOtherMetrics(argv[1],metric);
            break;
        default :
            cout << "computeMeasures, wrong measure code" << endl;
            throw("computeMeasures, wrong measure code" );
    }

    // all was good, exit
    return 0;

}