//
// Created by yago on 16/11/24.
//

#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    if(argc < 3) {
        cout << "./showTwoTogether [video1] [video2] (skipFirst)" << endl;
        return 0;
    }

    VideoCapture cap1(argv[1]);
    assert(cap1.isOpened());
    VideoCapture cap2(argv[2]);
    assert(cap2.isOpened());

    //prepare output video
    int ex = static_cast<int>(cap1.get(CAP_PROP_FOURCC)); // Get Codec Type- Int form
    Size S = Size( 2*(((int) cap1.get(CAP_PROP_FRAME_WIDTH)))+10,(int) cap1.get(CAP_PROP_FRAME_HEIGHT));
    //Size S = Size( 336,1206);

   VideoWriter outputVideo;
    outputVideo.open(argv[3], ex, cap1.get(CAP_PROP_FPS), S, true);
    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for writting: " << argv[3] << endl;
        return -1;
    }

    // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height<< " of nr#: " << cap1.get(CAP_PROP_FRAME_COUNT)<<" "<< cap1.get(CAP_PROP_FPS) << endl;
    cout << "Input codec type: " << EXT << endl;

    Mat cur,cur2;

    cout<<"Checking shit "<<argv[4]<<"   "<<argc<<endl;

    // Read one frame of the original video so we are showing the same frames
    if((argc>4) && atoi( argv[4])==1)
    {
        cout<<"Skipping the first frame because you told me to  "<<atoi( argv[4])<<endl;
        cap1 >> cur;
    }

    int  k=0;
    int max_frames = cap1.get(CV_CAP_PROP_FRAME_COUNT);

    while(k < max_frames-1) { // don't process the very last frame, no valid transform

        //cout << "RAMBUTAN " <<k<< endl;

        cap1 >> cur;
        cap2 >> cur2;

        resize(cur2, cur2, Size(cur.cols, cur.rows));

        if(cur.data == NULL) {
            break;
        }


         // Now draw the original and stablised side by side for coolness
        Mat canvas = Mat::zeros(cur.rows, 2*cur.cols+10, cur.type());

        //if(k%1000==0)
        //{
        //    cout<<"iteration "<<k<<"                                     "<< canvas.rows<<" "<<canvas.cols<<endl;
        // }


        //cout << "Canvas size: " << canvas.rows<<" "<<canvas.cols<<" at iteration "<<k<<" rames "<<max_frames-1<<" "<<outputVideo.get(CAP_PROP_FRAME_COUNT) <<endl;
       // cout << "other sizes: " << cur.rows<<" "<<cur.cols<<" "<<cur2.rows<<" "<<cur2.cols<<endl;

        try{cur.copyTo(canvas(Range::all(), Range(0, cur.cols))); }
        catch(exception w) {
            cout<<" merda "<<w.what()<<endl;
            exit(0);
        }

        try{ cur2.copyTo(canvas(Range::all(), Range(cur.cols+10, cur.cols*2+10)));}
            catch(exception w) {
                cout<<" merda2 "<<w.what()<<endl;
                exit(0);
            }


        // If too big to fit on the screen, then scale it down by 2, hopefully it'll fit :)
        if(canvas.cols > 1920) {
            resize(canvas, canvas, Size(canvas.cols/2, canvas.rows/2));
        }

        //imshow("Showing videos side by side ", canvas);
        // save the output video and the comparison side by side
        outputVideo << canvas;
        //waitKey(20);

        k++;

    }

    return 0;
}