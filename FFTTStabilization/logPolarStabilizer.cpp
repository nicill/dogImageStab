//
// Created by yago on 17/04/28.
// Original code from: https://github.com/Smorodov/LogPolarFFTTemplateMatcher
//(1) An FFT-based technique for translation, rotation and scale-invariant image registration. BS Reddy, BN Chatterji.IEEE Transactions on Image Processing, 5, 1266-1271, 1996
//(2) An IDL/ENVI implementation of the FFT-based algorithm for automatic image registration. H Xiea, N Hicksa, GR Kellera, H Huangb, V Kreinovich. Computers & Geosciences, 29, 1045-1055, 2003.
// (3) Image Registration Using Adaptive Polar Transform. R Matungka, YF Zheng,RL Ewing. IEEE Transactions on Image Processing, 18(10), 2009.
// That code was based on python version of algorithm:  http://www.lfd.uci.edu/~gohlke/code/imreg.py.html
//

#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using std::vector;
using namespace std;
using namespace cv;

//----------------------------------------------------------
// Recombinate image quaters
//----------------------------------------------------------
void Recomb(Mat &src, Mat &dst)
{
    int cx = src.cols >> 1;
    int cy = src.rows >> 1;
    Mat tmp;
    tmp.create(src.size(), src.type());
    src(Rect(0, 0, cx, cy)).copyTo(tmp(Rect(cx, cy, cx, cy)));
    src(Rect(cx, cy, cx, cy)).copyTo(tmp(Rect(0, 0, cx, cy)));
    src(Rect(cx, 0, cx, cy)).copyTo(tmp(Rect(0, cy, cx, cy)));
    src(Rect(0, cy, cx, cy)).copyTo(tmp(Rect(cx, 0, cx, cy)));
    dst = tmp;
}
//----------------------------------------------------------
// 2D Forward FFT
//----------------------------------------------------------
void ForwardFFT(Mat &Src, Mat *FImg, bool do_recomb = true)
{
    int M = getOptimalDFTSize(Src.rows);
    int N = getOptimalDFTSize(Src.cols);
    Mat padded;
    copyMakeBorder(Src, padded, 0, M - Src.rows, 0, N - Src.cols, BORDER_CONSTANT, Scalar::all(0));
    Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
    Mat complexImg;
    merge(planes, 2, complexImg);
    dft(complexImg, complexImg);
    split(complexImg, planes);
    planes[0] = planes[0](Rect(0, 0, planes[0].cols & -2, planes[0].rows & -2));
    planes[1] = planes[1](Rect(0, 0, planes[1].cols & -2, planes[1].rows & -2));
    if (do_recomb)
    {
        Recomb(planes[0], planes[0]);
        Recomb(planes[1], planes[1]);
    }
    planes[0] /= float(M*N);
    planes[1] /= float(M*N);
    FImg[0] = planes[0].clone();
    FImg[1] = planes[1].clone();
}
//----------------------------------------------------------
// 2D inverse FFT
//----------------------------------------------------------
void InverseFFT(Mat *FImg, Mat &Dst, bool do_recomb = true)
{
    if (do_recomb)
    {
        Recomb(FImg[0], FImg[0]);
        Recomb(FImg[1], FImg[1]);
    }
    Mat complexImg;
    merge(FImg, 2, complexImg);
    idft(complexImg, complexImg);
    split(complexImg, FImg);
    Dst = FImg[0].clone();
}
//-----------------------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------------------
void highpass(Size sz, Mat& dst)
{
    Mat a = Mat(sz.height, 1, CV_32FC1);
    Mat b = Mat(1, sz.width, CV_32FC1);

    float step_y = CV_PI / sz.height;
    float val = -CV_PI*0.5;

    for (int i = 0; i < sz.height; ++i)
    {
        a.at<float>(i) = cos(val);
        val += step_y;
    }

    val = -CV_PI*0.5;
    float step_x = CV_PI / sz.width;
    for (int i = 0; i < sz.width; ++i)
    {
        b.at<float>(i) = cos(val);
        val += step_x;
    }

    Mat tmp = a*b;
    dst = (1.0 - tmp).mul(2.0 - tmp);
}
//-----------------------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------------------
float logpolar(Mat& src, Mat& dst)
{
    float radii = src.cols;
    float angles = src.rows;
    Point2f center(src.cols / 2, src.rows / 2);
    float d = norm(Vec2f(src.cols - center.x, src.rows - center.y));
    float log_base = pow((float)10.0, log10(d) / radii);
    float d_theta = CV_PI / (float)angles;
    float theta = CV_PI / 2.0;
    float radius = 0;
    Mat map_x(src.size(), CV_32FC1);
    Mat map_y(src.size(), CV_32FC1);
    for (int i = 0; i < angles; ++i)
    {
        for (int j = 0; j < radii; ++j)
        {
            radius = pow(log_base, float(j));
            float x = radius * sin(theta) + center.x;
            float y = radius * cos(theta) + center.y;
            map_x.at<float>(i, j) = x;
            map_y.at<float>(i, j) = y;
        }
        theta += d_theta;
    }
    remap(src, dst, map_x, map_y, CV_INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
    return log_base;
}
//-----------------------------------------------------------------------------------------------------
// As input we need equal sized images, with the same aspect ratio,
// scale difference should not exceed 1.8 times.
//-----------------------------------------------------------------------------------------------------
Mat LogPolarFFTTemplateMatch(Mat& im0, Mat& im1, double canny_threshold1, double canny_threshold2)
{
    cout<<"SARIGUELLAINTERIOR!"<<endl;
    // Accept 1 or 3 channel CV_8U, CV_32F or CV_64F images.
    CV_Assert((im0.type() == CV_8UC1) || (im0.type() == CV_8UC3) ||
              (im0.type() == CV_32FC1) || (im0.type() == CV_32FC3) ||
              (im0.type() == CV_64FC1) || (im0.type() == CV_64FC3));

    CV_Assert(im0.rows == im1.rows && im0.cols == im1.cols);

    CV_Assert(im0.channels() == 1 || im0.channels() == 3 || im0.channels() == 4);

    CV_Assert(im1.channels() == 1 || im1.channels() == 3 || im1.channels() == 4);

    cout<<"SARIGUELLAINTERIOR2!"<<endl;

    Mat im0_tmp = im0.clone();
    Mat im1_tmp = im1.clone();
    if (im0.channels() == 3)
    {
        cvtColor(im0, im0, cv::COLOR_BGR2GRAY);
    }

    if (im0.channels() == 4)
    {
        cvtColor(im0, im0, cv::COLOR_BGRA2GRAY);
    }

    if (im1.channels() == 3)
    {
        cvtColor(im1, im1, cv::COLOR_BGR2GRAY);
    }

    if (im1.channels() == 4)
    {
        cvtColor(im1, im1, cv::COLOR_BGRA2GRAY);
    }

    if (im0.type() == CV_32FC1)
    {
        im0.convertTo(im0, CV_8UC1, 1.0 / 255.0);
    }

    if (im1.type() == CV_32FC1)
    {
        im1.convertTo(im1, CV_8UC1, 1.0 / 255.0);
    }

    if (im0.type() == CV_64FC1)
    {
        im0.convertTo(im0, CV_8UC1, 1.0);
    }

    if (im1.type() == CV_64FC1)
    {
        im1.convertTo(im1, CV_8UC1, 1.0);
    }


    Canny(im0, im0, canny_threshold1, canny_threshold2); // you can change this
    Canny(im1, im1, canny_threshold1, canny_threshold2);

    // Ensure both images are of CV_32FC1 type
    im0.convertTo(im0, CV_32FC1, 1.0 / 255.0);
    im1.convertTo(im1, CV_32FC1, 1.0 / 255.0);

    Mat F0[2], F1[2];
    Mat f0, f1;
    ForwardFFT(im0, F0);
    ForwardFFT(im1, F1);
    magnitude(F0[0], F0[1], f0);
    magnitude(F1[0], F1[1], f1);

    // Create filter
    Mat h;
    highpass(f0.size(), h);

    // Apply it in freq domain
    f0 = f0.mul(h);
    f1 = f1.mul(h);

    float log_base;
    Mat f0lp, f1lp;

    log_base = logpolar(f0, f0lp);
    log_base = logpolar(f1, f1lp);

    cout<<"SARIGUELLAINTERIOR3!"<<endl;


    // Find rotation and scale
    Point2d rotation_and_scale = cv::phaseCorrelate(f1lp, f0lp);

    float angle = 180.0 * rotation_and_scale.y / f0lp.rows;
    float scale = pow((float)log_base, rotation_and_scale.x);
    // --------------
    if (scale > 1.8)
    {
        rotation_and_scale = cv::phaseCorrelate(f1lp, f0lp);
        angle = -180.0 * rotation_and_scale.y / f0lp.rows;
        scale = 1.0 / pow(log_base, (float)rotation_and_scale.x);
        if (scale > 1.8)
        {
            cout << "Images are not compatible. Scale change > 1.8" << endl;
            throw ("Images are not compatible. Scale change > 1.8");
        }
    }
    // --------------
    if (angle < -90.0)
    {
        angle += 180.0;
    }
    else if (angle > 90.0)
    {
        angle -= 180.0;
    }
    cout<<"SARIGUELLAINTERIOR4!"<<endl;

    // Now rotate and scale fragment back, then find translation
    Mat rot_mat = getRotationMatrix2D(Point(im1.cols / 2, im1.rows / 2), angle, 1.0 / scale);

    cout<<"SARIGUELLAINTERIOR5!"<<endl;

    // rotate and scale
    Mat im1_rs;
    warpAffine(im1, im1_rs, rot_mat, im1.size());

    cout<<"SARIGUELLAINTERIOR6!"<<endl;

    // find translation
    Point2d tr = cv::phaseCorrelate(im1_rs, im0);

    // compute rotated rectangle parameters
   /* RotatedRect rr;
    rr.center = tr + Point2d(im0.cols / 2, im0.rows / 2);
    rr.angle = -angle;
    rr.size.width = im1.cols / scale;
    rr.size.height = im1.rows / scale;*/

    im0 = im0_tmp.clone();
    im1 = im1_tmp.clone();

    return im1_rs;
}



/** @function main */
int main( int argc, char** argv ) {

    bool verbose=true;
    if (argc != 3) {
        std::cerr << "logPolarStabiliser main::Wrong number of parameters, supplied: " << argc << std::endl;
    }
    // read Video
    VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        std::cerr << "Could not open the video supplied: " << argv[1] << std::endl;
        return 1;
    }
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);

    int ex = static_cast<int>(capture.get(CAP_PROP_FOURCC)); // Get Codec Type- Int form
    Size S = Size((int) capture.get(CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) capture.get(CAP_PROP_FRAME_HEIGHT));
    // Open output video too
    VideoWriter outputVideo;
    outputVideo.open(argv[2], ex, capture.get(CAP_PROP_FPS), S, true);
    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for writting: " << argv[2] << endl;
        return -1;
    }

    // Framewise stabilization
    Mat current, previous;

    // Matrices to store transformations
    Mat H, previousH ;


    // Load first frame
    capture >> previous;
    for (int frameCounter = 2; frameCounter <= totalFrames; frameCounter++) {
cout<<"SARIGUELLA!"<<endl;
        capture >> current;

        if (current.data == NULL) {
            break;
        }

        // Stabilize

        // As input we need equal sized images, with the same aspect ratio,
        // scale difference should not exceed 1.8 times.

        Mat cur2;

        try{
            cur2 = LogPolarFFTTemplateMatch(current, previous,200,100);
        }
        catch(Exception e)
        {
            cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FUCKED logpolarTransf "<<e.what()<<" , doing nothing,  frame "<<frameCounter<<"/"<<totalFrames<<endl;
            current.copyTo(cur2);
        }

        cout<<"SARIGUELLANORMAL"<<endl;

        outputVideo << cur2;

        cout<<"SARIGUELLANORMAL2"<<endl;

        // once everything is finished, just update loop (copy current to previous)
        current.copyTo(previous);

        cout<<"SARIGUELLANORMAL3"<<endl;

        // Output things if necessary
        //if (verbose) {
        // cout << "INFORMATION ON STABILIZATION" << endl;
        //}
    }


}