//
// Created by yago on 17/07/25.
//


#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>

using namespace std;
using namespace cv;

/*struct TransformParam
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
*/


int main(int argc, char **argv)
{
    if(argc < 3) {
        cout << "./farnebackStabilization [video.avi] outputVideo motioncode" << endl;
        return 0;
    }

    //const int SMOOTHING_RADIUS = 30; // In frames. The larger the more stable the video, but less reactive to sudden panning
    //int SMOOTHING_RADIUS = atoi(argv[2]); // In frames. The larger the more stable the video, but less reactive to sudden panning
    bool motionType=false;
    int motionCode=atoi(argv[3]);
    if(motionCode==1) motionType=true;

    bool verbose=false;

    VideoCapture cap(argv[1]);
    assert(cap.isOpened());

    Mat cur, cur_grey;
    Mat prev, prev_grey;

    cap >> prev;
    cvtColor(prev, prev_grey, COLOR_BGR2GRAY);

    int k=1;
    int max_frames = cap.get(CV_CAP_PROP_FRAME_COUNT);
    Mat last_T;

    vector<bool> fucked (max_frames);
    fucked[0]=false;

    while(true) {
        cap >> cur;

        if(cur.data == NULL) {
            break;
        }

        // turn frame to greyscale
        cvtColor(cur, cur_grey, COLOR_BGR2GRAY);

        // vector from prev to cur
        vector <Point2f> prev_corner, cur_corner;

        Mat flow;
        try{ calcOpticalFlowFarneback(prev_grey, cur_grey, flow, 0.5, 3, 15, 3, 5, 1.2, 0);} //github.com/opencv/opencv/blob/master/samples/cpp/fback.cpp
        catch(Exception e){cout<<"Exception caught when computing Farneback optical flow, moving on! "<<e.what()<<endl;}

        for (int y = 0; y < prev.rows; y += 5) {
            for (int x = 0; x < prev.cols; x += 5)
            {
                // get the flow from y, x position * 10 for better visibility
                const Point2f flowatxy = flow.at<Point2f>(y, x) * 10;

//cout<<"sariguella "<<endl;
                // draw line at flow direction
                line(prev, Point(x, y), Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)), Scalar(255,0,0));
                // draw initial point
                circle(prev, Point(x, y), 1, Scalar(0, 0, 0), -1);

                imshow("prew", prev);

            }

        }

        if(waitKey(30)>=0)
            break;


        // translation + rotation only
        Mat T;
       // try{T = estimateRigidTransform(prev_corner2, cur_corner2, motionType);} // motionType false = rigid transform, no scaling/shearing
       // catch(Exception e){cout<<"Exception caught when computing rigid transform, moving on! "<<e.what()<<endl;}

        // in rare cases no transform is found. We'll just use the last known good transform.
        if(T.data == NULL) {
            last_T.copyTo(T);
            fucked[k]=true;

            //NOT REALLYWORKING!!!!, we will just make up an identity transform
            /*Mat identity;
            try{
                identity=estimateRigidTransform(cur_corner2, cur_corner2, motionType);
                identity.copyTo(T);
            }
            catch(Exception e)
            {
                cout<<"Exception caught when making up identity transform, moving on! "<<e.what()<<endl;
                last_T.copyTo(T);
            }*/
        } else {fucked[k]= false;}

        T.copyTo(last_T);

        // decompose T
        double dx = T.at<double>(0,2);
        double dy = T.at<double>(1,2);
        double da = atan2(T.at<double>(1,0), T.at<double>(0,0));

       // prev_to_cur_transform.push_back(TransformParam(dx, dy, da));

     //   out_transform << k << " " << dx << " " << dy << " " << da << endl;

        cur.copyTo(prev);
        cur_grey.copyTo(prev_grey);

//        if(verbose) cout << "Frame: " << k << "/" << max_frames << " - good optical flow: " << prev_corner2.size() << endl;
        k++;


    }




    // Step 5 - Apply the new transformation to the video
    cap.set(CV_CAP_PROP_POS_FRAMES, 0);
    Mat T(2,3,CV_64F);

    //int vert_border = HORIZONTAL_BORDER_CROP * prev.rows / prev.cols; // get the aspect ratio correct
    int vert_border = prev.rows / prev.cols; // get the aspect ratio correct

    //prepare output video
    int ex = static_cast<int>(cap.get(CAP_PROP_FOURCC)); // Get Codec Type- Int form
    Size S = Size((int) cap.get(CAP_PROP_FRAME_WIDTH), (int) cap.get(CAP_PROP_FRAME_HEIGHT));   // Acquire input size

    VideoWriter outputVideo;
    outputVideo.open(argv[2], ex, cap.get(CAP_PROP_FPS), S, true);
    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for writting: " << argv[2] << endl;
        return -1;
    }

    // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};

    if(verbose) {
        cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height << " of nr#: " <<
             cap.get(CAP_PROP_FRAME_COUNT) << endl;
        cout << "Input codec type: " << EXT << endl;
    }

    k=0;
    while(k < max_frames-1) { // don't process the very last frame, no valid transform
        cap >> cur;

        if(cur.data == NULL) {
            break;
        }



        Mat cur2;

        warpAffine(cur, cur2, T, cur.size());

        //cur2 = cur2(Range(vert_border, cur2.rows-vert_border), Range(HORIZONTAL_BORDER_CROP, cur2.cols-HORIZONTAL_BORDER_CROP));

        // Resize cur2 back to cur size, for better side by side comparison
        resize(cur2, cur2, cur.size());

        // Now draw the original and stablised side by side for coolness
        Mat canvas = Mat::zeros(cur.rows, cur.cols*2+10, cur.type());

        cur.copyTo(canvas(Range::all(), Range(0, cur2.cols)));
        cur2.copyTo(canvas(Range::all(), Range(cur2.cols+10, cur2.cols*2+10)));

        // If too big to fit on the screen, then scale it down by 2, hopefully it'll fit :)
        if(verbose) {
            if (canvas.cols > 1920) {
                resize(canvas, canvas, Size(canvas.cols / 2, canvas.rows / 2));
            }

            imshow("before and after", canvas);
            waitKey(20);
        }

        // save the output video and the comparison side by side
        if(!fucked[k]) outputVideo << cur2;
        else outputVideo << cur;

        //char str[256];
        //sprintf(str, "images/%08d.jpg", k);
        //imwrite(str, canvas);

        k++;

    }

    return 0;
}