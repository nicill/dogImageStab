//
// Created by yago on 17/02/02.
//

#include "opencvImageMetric.h"


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



opencvImageMetric::opencvImageMetric(int i) { comparisonType=i;}

double opencvImageMetric::computeSimilarity(Mat* im1, Mat* im2) {
   if(comparisonType==0) return getPSNR(*im1,*im2);
    else if (comparisonType==1){
       Scalar mssimV =getMSSIM(*im1,*im2);
       return ((mssimV.val)[0]+(mssimV.val)[1]+(mssimV.val)[2] )/3. ;
   }
    else
   {
       std::cout<<"WRONG METRIC TYPE AT double opencvImageMetric::computeSimilarity(Mat* im1, Mat* im2) "<<std::endl;
       throw "WRONG METRIC TYPE AT double opencvImageMetric::computeSimilarity(Mat* im1, Mat* im2) ";
   }
}

// TODO implement!
void opencvImageMetric::activateVerbosity() {
    std::cerr << "Verbosity not yet implemented in opencvImageMetric.cpp" << std::endl;
    return;
}
