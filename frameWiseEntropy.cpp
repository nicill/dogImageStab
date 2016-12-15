//
// Created by yago on 16/11/25.
//


#include <opencv2/opencv.hpp>

#include <cassert>

using namespace std;
using namespace cv;

// functions from http://stackoverflow.com/questions/24930134/entropy-for-a-gray-image-in-opencv
float entropy(Mat seq, Size size, int index)
{
    int cnt = 0;
    float entr = 0;
    float total_size = size.height * size.width; //total size of all symbols in an image

    for(int i=0;i<index;i++)
    {
        float sym_occur = seq.at<float>(0, i); //the number of times a sybmol has occured
        if(sym_occur>0) //log of zero goes to infinity
        {
            cnt++;
            entr += (sym_occur/total_size)*(log2(total_size/sym_occur));
        }
    }
   // cout<<"cnt: "<<cnt<<endl;
    return entr;

}

// myEntropy calculates relative occurrence of different symbols within given input sequence using histogram
Mat myEntropy(Mat seq, int histSize)
{

    float range[] = { 0, 256 } ;
    const float* histRange = { range };

    bool uniform = true; bool accumulate = false;

    Mat hist;

    /// Compute the histograms:
   try{ calcHist( &seq, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );}
    catch (exception e) {
        cout<<" wtf "<<e.what()<<endl;
        exit(0);
    }

    return hist;
}


int main(int argc, char **argv)
{
    if(argc < 2) {
        cout << "./frameWiseEntropy [video]" << endl;
        return 0;
    }

    VideoCapture cap1(argv[1]);
    assert(cap1.isOpened());

    Mat cur,prev;

    int  k=0;
    int max_frames = cap1.get(CV_CAP_PROP_FRAME_COUNT);

    int histSize = 256;//number of bins

    double entropy0=0;
    double entropy1=0;
    double entropy2=0;

    while(k < max_frames) {
        cap1 >> cur;

        //resize(cur2, cur2, Size(cur.cols, cur.rows));

        if(cur.data == NULL) {
            break;
        }

        //if(k==3) imwrite("./papaapa.jpg",cur);
        Mat ycrcb[3];
        split(cur, ycrcb);

        Mat hist0 = myEntropy(ycrcb[0], histSize);
        Mat hist1 = myEntropy(ycrcb[1], histSize);
        Mat hist2 = myEntropy(ycrcb[2], histSize);

        entropy0=entropy0 + entropy(hist0,Size(cur.cols, cur.rows), histSize);
        entropy1=entropy1 + entropy(hist1,Size(cur.cols, cur.rows), histSize);
        entropy2=entropy2 + entropy(hist2,Size(cur.cols, cur.rows), histSize);

        k++;

    }

    entropy0=entropy0/max_frames;
    entropy1=entropy1/max_frames;
    entropy2=entropy2/max_frames;

    cout<<"average entropy for channel 0 : "<<entropy0<<endl;
    cout<<"average entropy for channel 1 : "<<entropy1<<endl;
    cout<<"average entropy for channel 2 : "<<entropy2<<endl;
    cout<<"average average entropy : "<<(entropy0+entropy1+entropy2)/3.<<endl;



    return 0;
}