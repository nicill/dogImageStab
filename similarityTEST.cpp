//
// Created by tokuyama on 17/01/27.
//

#include "similarityTEST.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>

using namespace std;
using namespace cv;

// This video stablisation smooths the global trajectory using a sliding average window

const int SMOOTHING_RADIUS = 30; // In frames. The larger the more stable the video, but less reactive to sudden panning
const int HORIZONTAL_BORDER_CROP = 20; // In pixels. Crops the border to reduce the black borders from stabilisation being too noticeable.

// 1. Get previous to current frame transformation (dx, dy, da) for all frames
// 2. Accumulate the transformations to get the image trajectory
// 3. Smooth out the trajectory using an averaging window
// 4. Generate new set of previous to current transform, such that the trajectory ends up being the same as the smoothed trajectory
// 5. Apply the new transformation to the video

struct TransformParam
{
    TransformParam() {}
    TransformParam(double _dx, double _dy, double _da) {
        dx = _dx;
        dy = _dy;
        da = _da;
    }

    double dx;
    double dy;
    double da; // angle
};

struct Trajectory
{
    Trajectory() {}
    Trajectory(double _x, double _y, double _a) {
        x = _x;
        y = _y;
        a = _a;
    }

    double x;
    double y;
    double a; // angle
};

int main(int argc, char **argv)
{
    if(argc < 2) {
        cout << "./VideoStab [video.avi]" << endl;
        return 0;
    }

    // For further analysis
    ofstream out_transform("prev_to_cur_transformation.txt");
    ofstream out_trajectory("trajectory.txt");
    ofstream out_smoothed_trajectory("smoothed_trajectory.txt");
    ofstream out_new_transform("new_prev_to_cur_transformation.txt");

    VideoCapture capture(argv[1]);
    assert(capture.isOpened());

    Mat current, current_grey;
    Mat previous, previous_grey;

    capture >> previous;
    cvtColor(previous, previous_grey, COLOR_BGR2GRAY);

    // Step 1 - Get previous to current frame transformation (dx, dy, da) for all frames
    vector <TransformParam> prev_to_cur_transform; // previous to current

    int k=1;
    int max_frames = capture.get(CV_CAP_PROP_FRAME_COUNT);
    Mat last_T;

    // DEBUG
    string outp = "";

    while(true) {
        capture >> current;

        if(current.data == NULL) {
            break;
        }

        cvtColor(current, current_grey, COLOR_BGR2GRAY);

        // vector from previous to current
        vector <Point2f> prev_corner, cur_corner;
        vector <Point2f> prev_corner2, cur_corner2;
        vector <uchar> status;
        vector <float> err;

        goodFeaturesToTrack(previous_grey, prev_corner, 200, 0.01, 30);
        try
        {
            calcOpticalFlowPyrLK(previous_grey, current_grey, prev_corner, cur_corner, status, err);
        }
        catch (Exception e)
        {
            cout << outp << endl;
            cout << "Exception caught at calcOpticalFlowPyrLK, skipping! " << e.what() << endl;
            k++;
            break;
        }

        // weed out bad matches
        for(size_t i=0; i < status.size(); i++) {
            if(status[i]) {
                prev_corner2.push_back(prev_corner[i]);
                cur_corner2.push_back(cur_corner[i]);
            }
        }

        // translation + rotation only
        Mat T;
        try
        {
            T = estimateRigidTransform(prev_corner2, cur_corner2, false); // false = rigid transform, no scaling/shearing

            // in rare cases no transform is found. We'll just use the last known good transform.
            if(T.data == NULL) {
                last_T.copyTo(T);
            }
        }
        catch (Exception e)
        {
            cout << outp << endl;
            cout << "Exception caught at estimateRigidTransform, skipping! " << e.what() << endl;
            last_T.copyTo(T);
        }

        T.copyTo(last_T);

        // decompose T
        double dx = T.at<double>(0,2);
        double dy = T.at<double>(1,2);
        double da = atan2(T.at<double>(1,0), T.at<double>(0,0));

        prev_to_cur_transform.push_back(TransformParam(dx, dy, da));

        out_transform << k << " " << dx << " " << dy << " " << da << endl;

        current.copyTo(previous);
        current_grey.copyTo(previous_grey);

        //cout << "Frame: " << k << "/" << max_frames << " - good optical flow: " << prev_corner2.size() << endl;

        outp = "Past frame: " + to_string(k) + "/" + to_string(max_frames) + " - good optical flow: " + to_string(prev_corner2.size());

        k++;
    }

    // Step 2 - Accumulate the transformations to get the image trajectory

    // Accumulated frame to frame transform
    double a = 0;
    double x = 0;
    double y = 0;

    vector <Trajectory> trajectory; // trajectory at all frames

    for(size_t i=0; i < prev_to_cur_transform.size(); i++) {
        x += prev_to_cur_transform[i].dx;
        y += prev_to_cur_transform[i].dy;
        a += prev_to_cur_transform[i].da;

        trajectory.push_back(Trajectory(x,y,a));

        out_trajectory << (i+1) << " " << x << " " << y << " " << a << endl;

        // Print the trajectory aka "difference"
        cout << i+1 << " | Trajectory: " << x << ", " << y << ", " << a << "; Sum: " << abs(x)+abs(y)+abs(a) << endl;
    }

    return 0;
}