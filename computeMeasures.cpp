//
// Created by yago on 16/12/08.
//
#include <opencv2/opencv.hpp>
#include "videoStabilizationQualityMeasures.h"


using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    if(argc < 2) {
        cout << "./computeMeasures [video] [code]" << endl;
        cout << "code: 0=framewise entropy,  1=ITF" << endl;
        return 0;
    }

videoStabilizationQualityMeasures vidStabMeasurer;

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
            cout<<0<<endl;
            exit;
            cout<<vidStabMeasurer.SSIM(argv[1]);
            break;
        default :
            cout << "computeMeasures, wrong measure code" << endl;
            throw("computeMeasures, wrong measure code" );
    }

    // all was good, exit
    return 0;

}