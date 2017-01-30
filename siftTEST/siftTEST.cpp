//
// Created by tokuyama on 17/01/30.
//

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    if(argc != 2) {
        cout << "Usage: ./siftTEST vid.avi" << endl;
        return 0;
    }

    VideoCapture capture(argv[1]);
    assert(capture.isOpened());

    Mat current, current_grey;
    Mat previous, previous_grey;

    capture >> previous;
    cvtColor(previous, previous_grey, COLOR_BGR2GRAY);

    int k=1;
    int max_frames = capture.get(CV_CAP_PROP_FRAME_COUNT);

    while(true) {
        capture >> current;

        if(current.data == NULL) {
            break;
        }

        cvtColor(current, current_grey, COLOR_BGR2GRAY);

        // Calculate SIFT



    return 0;
}
