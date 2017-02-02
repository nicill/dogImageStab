//
// Created by yago on 16/12/08.
//

#include "videoStabilizationQualityMeasures.h"

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

    //First, create a mask with black pixels, we do not want to count them
    cv::Mat mask = cv::Mat::zeros(seq.size(), CV_8UC1);
    mask.setTo(255, seq > 0);

    Mat hist;

    /// Compute the histograms:
    try{ calcHist( &seq, 1, 0, mask, hist, 1, &histSize, &histRange, uniform, accumulate );}
    catch (exception e) {
        cout<<" wtf "<<e.what()<<endl;
        exit(0);
    }

    return hist;
}

double getPSNRColor(const Mat& I1, const Mat& I2)
{
     Mat s1;
    absdiff(I1, I2, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2

    Scalar s = sum(s1);        // sum elements per channel

    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

    if( sse <= 1e-10) // for small values return zero
        return 0;
    else
    {
        double mse  = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }
}

double getPSNR(const Mat& I1, const Mat& I2) // from grayscale images, does not take into account pixels that are black in the two images
{
    int nRows = I1.rows;
    int nCols = I1.cols;

    int i,j;
    double sse=0;
    int counter=0;
    for( i = 0; i < nRows; ++i)
    {
        for ( j = 0; j < nCols; ++j)
        {
            if( (I1.at<uchar>(i,j)!=0)||(I1.at<uchar>(i,j)!=0))
            {
                // In this case, compute absolute difference
                double currentDif=fabs(I1.at<uchar>(i,j)-I2.at<uchar>(i,j));
                currentDif=currentDif*currentDif;
                sse=sse+currentDif;
                counter++;
            }

        }
    }

    if( sse <= 1e-10) // for small values return zero
        return 0;
    else
    {
        double mse  = sse / (double)(counter);
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }
}


// S L O W
Scalar getMSSIM( const Mat& i1, const Mat& i2) {
    const double C1 = 6.5025, C2 = 58.5225;
    //***************************** INITS **********************************
    int d = CV_32F;

    Mat I1, I2;
    i1.convertTo(I1, d);            // cannot calculate on one byte large values
    i2.convertTo(I2, d);

    cout<<"value at 3,3"<<(double)I1.at<uchar>(3,3)<<endl;
    cout<<"other value at 3,3"<<(double)I2.at<uchar>(3,3)<<endl;

    Mat I2_2 = I2.mul(I2);        // I2^2
    Mat I1_2 = I1.mul(I1);        // I1^2
    Mat I1_I2 = I1.mul(I2);        // I1 * I2

    //*************************** END INITS **********************************

    Mat mu1, mu2;                   // PRELIMINARY COMPUTING
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);

    Mat mu1_2 = mu1.mul(mu1);
    Mat mu2_2 = mu2.mul(mu2);
    Mat mu1_mu2 = mu1.mul(mu2);

    Mat sigma1_2, sigma2_2, sigma12;

    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;

    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;

    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    ///////////////////////////////// FORMULA ////////////////////////////////
    Mat t1, t2, t3;

    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);                 // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);                 // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

    Mat ssim_map;
    divide(t3, t1, ssim_map);        // ssim_map =  t3./t1;

    Scalar mssim = mean(ssim_map);   // mssim = average of ssim map
    //cout<<endl<<"getMSSIM::GOING TO RETURN STH WITH VALUES "<<mssim.val[0]<<" "<<mssim.val[1]<<" "<<mssim.val[2]<<endl;
    return mssim;

}


/*double videoStabilizationQualityMeasures::framewiseEntropy(string videoPath)
{
    bool verbose=false;

    VideoCapture cap1(videoPath);
    assert(cap1.isOpened());

    Mat cur,prev;

    int  k=1;
    int max_frames = cap1.get(CV_CAP_PROP_FRAME_COUNT);

    int histSize = 256;//number of bins

    double entropy0=0;
    double entropy1=0;
    double entropy2=0;

    while(k < max_frames) {
        cap1 >> cur;

        if(cur.data == NULL) {
            break;
        }

        //if(k==3) imwrite("./papaapa.jpg",cur);
        Mat ycrcb[3];
        split(cur, ycrcb);

        Mat hist0 = myEntropy(ycrcb[0], histSize);
        Mat hist1 = myEntropy(ycrcb[1], histSize);
        Mat hist2 = myEntropy(ycrcb[2], histSize);

        double ent=entropy(hist0,Size(cur.cols, cur.rows), histSize);
        if(ent!=0) {
            entropy0 = entropy0 + ent;
            ent = entropy(hist1, Size(cur.cols, cur.rows), histSize);
            if (ent != 0) {
                entropy1 = entropy1 + ent;
                ent = entropy(hist2, Size(cur.cols, cur.rows), histSize);
                if (ent != 0) {
                    entropy2 = entropy2 + ent;
                    k++;
                }
            }
        }
    }

    entropy0=entropy0/k;
    entropy1=entropy1/k;
    entropy2=entropy2/k;

    if(verbose) {
        cout << "average entropy for channel 0 : " << entropy0 << endl;
        cout << "average entropy for channel 1 : " << entropy1 << endl;
        cout << "average entropy for channel 2 : " << entropy2 << endl;
        cout << "average average entropy : " << (entropy0 + entropy1 + entropy2) / 3. << endl;
    }

    return ((entropy0 + entropy1 + entropy2) / 3.);
}*/

// entropy, grayscale version
double videoStabilizationQualityMeasures::framewiseEntropy(string videoPath)
{
    bool verbose=false;

    VideoCapture cap1(videoPath);
    assert(cap1.isOpened());

    Mat cur,prev;

    int  k=1;
    int max_frames = cap1.get(CV_CAP_PROP_FRAME_COUNT);

    int histSize = 256;//number of bins

    double entropyValue=0;

    while(k < max_frames) {
        cap1 >> cur;

        if(cur.data == NULL) {
            break;
        }

        //if(k==3) imwrite("./papaapa.jpg",cur);
        Mat grayImage;
        cvtColor(cur, grayImage, CV_RGB2GRAY);

        Mat hist0 = myEntropy(grayImage, histSize);

        double ent=entropy(hist0,Size(grayImage.cols, grayImage.rows), histSize);
        if(ent!=0) {
            entropyValue = entropyValue + ent;
             k++;
        }
    }

    entropyValue=entropyValue/k;

    if(verbose) {
        cout << "average entropy in grayscale : " << entropyValue << endl;
    }

    return (entropyValue);
}

double videoStabilizationQualityMeasures::ITF(string video) {

    stringstream conv;
    bool verbose=false;

    int psnrTriggerValue, delay;

    char c;
    int frameNum = 0;          // Frame counter

    VideoCapture capt(video); //open the same

    if (!capt.isOpened()){
        cout  << "videoStabilizationQualityMeasures::ITF(string video) Could not open video " << video << endl;
        throw("        cout  << \"videoStabilizationQualityMeasures::ITF(string video) Could not open video \" << current << endl;");
    }

    Size refS = Size((int) capt.get(CV_CAP_PROP_FRAME_WIDTH),(int) capt.get(CV_CAP_PROP_FRAME_HEIGHT));

    const char* WIN_RF = "Image to show";

    // Windows
    namedWindow(WIN_RF, CV_WINDOW_AUTOSIZE);
    cvMoveWindow(WIN_RF, 400       , 0);         //750,  2 (bernat =0)

    if(verbose) {
        cout << "Reference frame resolution: Width=" << refS.width << "  Height=" << refS.height << " of nr#: " <<
        capt.get(CV_CAP_PROP_FRAME_COUNT) << endl;
        cout << "PSNR trigger value " << setiosflags(ios::fixed) << setprecision(3) << psnrTriggerValue << endl;
    }

    Mat curr,prev;
    capt >> prev;

    Mat grayImagePrev;
    cvtColor(prev, grayImagePrev, CV_RGB2GRAY);

    double psnrV;
    double ITF=0; //to accumulata psnrv values
    //Scalar mssimV;

    for(;;) //Show the image captured in the window and repeat
    {
        capt >> curr;

        if(curr.data == NULL) {
            break;
        }

//       if(diffSizes) resize(frameUnderTest, frameUnderTest, Size(frameReference.cols, frameReference.rows));

        if(verbose) cout << "Frame: " << frameNum << "# ";


        // First, transform to grayscale
        Mat grayImageCurr;
        cvtColor(curr, grayImageCurr, CV_RGB2GRAY);

        ///////////////////////////////// PSNR ////////////////////////////////////////////////////
        psnrV = getPSNR(grayImageCurr,grayImagePrev);
        //cout << setiosflags(ios::fixed) << setprecision(3) << psnrV << "dB";

        // accumulat for ITF computations
        if(psnrV!=0)
        {
            ITF=ITF+psnrV;
            ++frameNum;
        }

        //update prev
        //grayImagePrev=grayImageCurr;
        grayImageCurr.copyTo(grayImagePrev);

        //////////////////////////////////// MSSIM /////////////////////////////////////////////////
       /* if (psnrV < psnrTriggerValue && psnrV)
        {
            mssimV = getMSSIM(curr, prev);

            if(verbose)  cout << " MSSIM: " << " R " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[2] * 100 << "%" << " G " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[1] * 100 << "%" << " B " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[0] * 100 << "%";
        }*/

        if(verbose) cout << endl;

        ////////////////////////////////// Show Image /////////////////////////////////////////////
        if(verbose) {
            imshow(WIN_RF, curr);
            c = (char) cvWaitKey(delay);
            if (c == 27) break;
        }
    }

    return (ITF/frameNum);
}

double videoStabilizationQualityMeasures::blackPixelPercent(string videoPath) {
    bool verbose=false;

    VideoCapture cap1(videoPath);
    assert(cap1.isOpened());

    Mat cur,prev;

    int  k=1;
    int max_frames = cap1.get(CV_CAP_PROP_FRAME_COUNT);

    double blackPixelPercent=0;

    while(k < max_frames) {
        cap1 >> cur;

        if(cur.data == NULL) {
            break;
        }

        // first, change to grayscale
        Mat grayImage;
        cvtColor(cur, grayImage, CV_RGB2GRAY);

        int TotalNumberOfPixels = grayImage.rows * grayImage.cols;
        int ZeroPixels = TotalNumberOfPixels - countNonZero(grayImage);
        double currentBlackPixelPercent=100*((double)ZeroPixels)/((double)TotalNumberOfPixels);
        blackPixelPercent=blackPixelPercent+currentBlackPixelPercent;
        if(verbose) {cout<<"The number of pixels that are zero is "<<ZeroPixels<<endl;}

         k++;
    }

    blackPixelPercent=blackPixelPercent/k;

    if(verbose) {
        cout << "Black pixel percentage : " << blackPixelPercent << endl;
    }

    return (blackPixelPercent);


}

double videoStabilizationQualityMeasures::SSIM(string video) {
    bool verbose=true;

    int psnrTriggerValue, delay;

    char c;
    double frameNum = 0;          // Frame counter

    VideoCapture capt(video); //open the same

    if (!capt.isOpened()){
        cout  << "videoStabilizationQualityMeasures::SSIM(string video) Could not open video " << video << endl;
        throw("        cout  << \"videoStabilizationQualityMeasures::ITF(string video) Could not open video \" << current << endl;");
    }

    Size refS = Size((int) capt.get(CV_CAP_PROP_FRAME_WIDTH),(int) capt.get(CV_CAP_PROP_FRAME_HEIGHT));

    const char* WIN_RF = "Image to show";

    // Windows
   // namedWindow(WIN_RF, CV_WINDOW_AUTOSIZE);
   // cvMoveWindow(WIN_RF, 400       , 0);         //750,  2 (bernat =0)

    if(verbose) {
        cout << "Reference frame resolution: Width=" << refS.width << "  Height=" << refS.height << " of nr#: " <<
        capt.get(CV_CAP_PROP_FRAME_COUNT) << endl;
    }

    Mat curr;
    Mat prev;
    capt >> prev;

    Scalar mssimV;
    double accumulate=0;// accumulate averages of everything

    for(;;) //Show the image captured in the window and repeat
    {
        capt >> curr;

        if(curr.data == NULL) {
            break;
        }

//       if(diffSizes) resize(frameUnderTest, frameUnderTest, Size(frameReference.cols, frameReference.rows));

        if(verbose) cout << "Frame: " << frameNum << "# ";

        //////////////////////////////////// MSSIM /////////////////////////////////////////////////
        mssimV = getMSSIM(curr, prev);

        // accumulate MSSIM
        accumulate=accumulate+((mssimV.val)[0]+(mssimV.val)[1]+(mssimV.val)[2] )/3.;

        //update prev and frame count
        frameNum++;

        if(verbose)  cout << " MSSIM: " << " R " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[2] * 100. << "%" << " G " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[1] * 100.<< "%" << " B " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[0] * 100. << "%"<<endl;

        ////////////////////////////////// Show Image /////////////////////////////////////////////
        if(verbose) {
           // imshow(WIN_RF, curr);
           // c = (char) cvWaitKey(delay);
           // if (c == 27) break;
        }
        //prev=curr;
        curr.copyTo(prev);
    }

    if(verbose) cout<<" SSIM going to return "<<accumulate/frameNum<<endl;
    return ( accumulate/frameNum);
}

vector<int> *videoStabilizationQualityMeasures::ITFRegularThresholder(string video, int numThresholds) {
    vector<int> *returnValue = new vector<int>();

    stringstream conv;
    bool verbose=false;

    int psnrTriggerValue, delay;

    char c;
    int frameNum = 0;          // Frame counter

    VideoCapture capt(video); //open the same

    if (!capt.isOpened()){
        cout  << "videoStabilizationQualityMeasures::ITF(string video) Could not open video " << video << endl;
        throw("        cout  << \"videoStabilizationQualityMeasures::ITF(string video) Could not open video \" << current << endl;");
    }

    Size refS = Size((int) capt.get(CV_CAP_PROP_FRAME_WIDTH),(int) capt.get(CV_CAP_PROP_FRAME_HEIGHT));

    const char* WIN_RF = "Image to show";

    // Windows
    namedWindow(WIN_RF, CV_WINDOW_AUTOSIZE);
    cvMoveWindow(WIN_RF, 400       , 0);         //750,  2 (bernat =0)

    if(verbose) {
        cout << "Reference frame resolution: Width=" << refS.width << "  Height=" << refS.height << " of nr#: " <<
        capt.get(CV_CAP_PROP_FRAME_COUNT) << endl;
        cout << "PSNR trigger value " << setiosflags(ios::fixed) << setprecision(3) << psnrTriggerValue << endl;
    }

    Mat curr,prev;
    capt >> prev;

    Mat grayImagePrev;
    cvtColor(prev, grayImagePrev, CV_RGB2GRAY);

    double psnrV;
    double ITF=0; //to accumulata psnrv values
    //Scalar mssimV;

    for(;;) //Show the image captured in the window and repeat
    {
        capt >> curr;

        if(curr.data == NULL) {
            break;
        }

//       if(diffSizes) resize(frameUnderTest, frameUnderTest, Size(frameReference.cols, frameReference.rows));

        if(verbose) cout << "Frame: " << frameNum << "# ";


        // First, transform to grayscale
        Mat grayImageCurr;
        cvtColor(curr, grayImageCurr, CV_RGB2GRAY);

        ///////////////////////////////// PSNR ////////////////////////////////////////////////////
        psnrV = getPSNR(grayImageCurr,grayImagePrev);
        //cout << setiosflags(ios::fixed) << setprecision(3) << psnrV << "dB";

        // accumulat for ITF computations
        if(psnrV!=0)
        {
            ITF=ITF+psnrV;
            ++frameNum;
        }

        //update prev
        //grayImagePrev=grayImageCurr;
        grayImageCurr.copyTo(grayImagePrev);
        //////////////////////////////////// MSSIM /////////////////////////////////////////////////
        /* if (psnrV < psnrTriggerValue && psnrV)
         {
             mssimV = getMSSIM(curr, prev);

             if(verbose)  cout << " MSSIM: " << " R " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[2] * 100 << "%" << " G " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[1] * 100 << "%" << " B " << setiosflags(ios::fixed) << setprecision(2) << mssimV.val[0] * 100 << "%";
         }*/

        if(verbose) cout << endl;

        ////////////////////////////////// Show Image /////////////////////////////////////////////
        if(verbose) {
            imshow(WIN_RF, curr);
            c = (char) cvWaitKey(delay);
            if (c == 27) break;
        }
    }

    return returnValue;
}
