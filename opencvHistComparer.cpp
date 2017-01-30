//
// Created by yago on 17/01/23.
//

#include "opencvHistComparer.h"

opencvHistComparer::opencvHistComparer(int i) { comparisonType=i;}

double opencvHistComparer::computeSimilarity(Mat* im1, Mat* im2) {

    // Original code:
    //http://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_comparison/histogram_comparison.html

    Mat hsv1,hsv2;

    /// Convert to HSV
    cvtColor( *im1, hsv1 , COLOR_BGR2HSV );
    cvtColor( *im2, hsv2 , COLOR_BGR2HSV );

    /// Using 50 bins for hue and 60 for saturation
    int h_bins = 50; int s_bins = 60;
    int histSize[] = { h_bins, s_bins };

    // hue varies from 0 to 179, saturation from 0 to 255
    float h_ranges[] = { 0, 180 };
    float s_ranges[] = { 0, 256 };

    const float* ranges[] = { h_ranges, s_ranges };

    // Use the o-th and 1-st channels
    int channels[] = { 0, 1 };

    /// Histograms
    Mat hist1;
    Mat hist2;

    /// Calculate the histograms for the HSV images
    calcHist( &hsv1, 1, channels, Mat(), hist1, 2, histSize, ranges, true, false );
    normalize( hist1, hist1, 0, 1, NORM_MINMAX, -1, Mat() );

    calcHist( &hsv2, 1, channels, Mat(), hist2, 2, histSize, ranges, true, false );
    normalize( hist2, hist2, 0, 1, NORM_MINMAX, -1, Mat() );

    /// Apply the histogram comparison methods
    double histComparison = compareHist( hist1, hist2, comparisonType);

    return histComparison;
}

