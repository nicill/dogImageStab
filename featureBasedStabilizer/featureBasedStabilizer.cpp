//
// Created by yago on 17/02/16.
//


// NOT MUCH WORKING YET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// based on http://docs.opencv.org/2.4/doc/tutorials/features2d/feature_homography/feature_homography.html


#include <stdio.h>
#include <iostream>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using namespace cv;
using std::vector;

void readme();

/** @function main */
int main( int argc, char** argv )
{
    if( argc != 3 )
    { readme(); return -1; }

    Mat img_object = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
    Mat img_scene = imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE );

    if( !img_object.data || !img_scene.data )   { std::cout<< " --(!) Error reading images " << std::endl; return -1; }

    //-- Step 1: Detect the keypoints and compute descriptors

    // NEW!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Here we should try different descriptor types
    Ptr<Feature2D> detector=xfeatures2d::SURF::create();;

    vector<KeyPoint> keypoints_1, keypoints_2;
    Mat descriptors_1, descriptors_2;
    vector<vector<DMatch>> matchesOfAllKeypoints;

    // Detect keypoints and compute descriptors (feature vectors)
    detector->detectAndCompute(img_object, noArray(), keypoints_1, descriptors_1);
    detector->detectAndCompute(img_scene, noArray(), keypoints_2, descriptors_2);

    // Make sure that keypoints have been found in both images before matching.
    if (keypoints_1.size() == 0 || keypoints_2.size() == 0) {
        //message("No keypoints found!");
        //return vector<vector<DMatch>>();
    }

    //-- Step 3: Matching descriptor vectors
    Ptr<DescriptorMatcher> descriptorMatcher;
    descriptorMatcher = BFMatcher::create();

    // Match descriptors and retrieve the k=2 best results
    descriptorMatcher->knnMatch(descriptors_1, descriptors_2,matchesOfAllKeypoints, 2);

    //Other matching option!!!
    /*FlannBasedMatcher matcher;
    matcher.match( descriptors_1, descriptors_2, matchesOfAllKeypoints,cv::noArray() ); // BEWARE!!!!!!!!!!!!!!! cv::noArray() added!*/

    if (matchesOfAllKeypoints.size() == 0) {
        //  message("No matches found!");
    }


    double max_dist = 0; double min_dist = 100;

    // Maybe not so important
    //-- Quick calculation of max and min distances between keypoints
    //for( int i = 0; i < descriptors_object.rows; i++ )
    //{ double dist = matches[i].distance;
    //    if( dist < min_dist ) min_dist = dist;
    //    if( dist > max_dist ) max_dist = dist;
    //}
    //printf("-- Max dist : %f \n", max_dist );
    //printf("-- Min dist : %f \n", min_dist );
    //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
    //std::vector< DMatch > good_matches;
    //for( int i = 0; i < descriptors_object.rows; i++ )
    //{ if( matches[i].distance < 3*min_dist )
    //    { good_matches.push_back( matches[i]); }
    //}
    //Mat img_matches;
    //drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    //-- Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    //for( int i = 0; i < good_matches.size(); i++ )
    //{
        //-- Get the keypoints from the good matches
        //obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
       // scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
   // }

    // FIND HOMOGRAPHY FROM GOOD MATCHES!!!!!!
    Mat H = findHomography( obj, scene, CV_RANSAC );

    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( img_object.cols, 0 );
    obj_corners[2] = cvPoint( img_object.cols, img_object.rows ); obj_corners[3] = cvPoint( 0, img_object.rows );
    std::vector<Point2f> scene_corners(4);

    perspectiveTransform( obj_corners, scene_corners, H);

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
   // line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
   // line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
   // line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
   // line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );

    //-- Show detected matches
   // imshow( "Good Matches & Object detection", img_matches );

    //waitKey(0);


    // NOW we would still need to store the transformed image I guess



    return 0;
}

/** @function readme */
void readme()
{ std::cout << " Usage: ./SURF_descriptor <img1> <img2>" << std::endl; }
