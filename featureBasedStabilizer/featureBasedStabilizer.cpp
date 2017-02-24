//
// Created by yago on 17/02/16.
//

// based on http://docs.opencv.org/2.4/doc/tutorials/features2d/feature_homography/feature_homography.html

#include <stdio.h>
#include <iostream>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "featureStabilizer.h"

using namespace cv;
using namespace std;
using std::vector;

void readme();

/** @function main */
int main( int argc, char** argv ) {

    bool verbose=true;
    if (argc != 4) {
        std::cerr << "featureBasedStabilizer main::Wrong number of parameters, supplied: " << argc << std::endl;
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


    // Stabilizer type treatment
    featureStabilizer *stabilizer;
    stabilizer=new featureStabilizer((featureStabilizer::type)atoi(argv[3]));


    // Framewise stabilization
    Mat current, previous;

    // Matrices to store transformations
    Mat H, previousH ;


    // Load first frame
    capture >> previous;
    for (int frameCounter = 2; frameCounter <= totalFrames; frameCounter++) {

        capture >> current;

        if (current.data == NULL) {
            break;
        }

        // Stabilize
        //Step 1,2,3 compute detectors, keypoints and their matches
        vector<KeyPoint> keypoints_1, keypoints_2;
        vector<vector<DMatch>> matchesOfAllKeypoints = stabilizer->getMatches(&current, &previous,&keypoints_1,&keypoints_2);

       //cout<<"feature stabilizer, keypoint sizes "<<keypoints_1.size()<<" "<<keypoints_2.size()<<endl;

        // Step 4, find points with good matches to compute transformation
        vector<DMatch> goodMatches;
        for (vector<DMatch> matchesAtKeypoint : matchesOfAllKeypoints) {
            // Match list might not contain any matches
            if (matchesAtKeypoint.size() == 0) {
               cout<<"Skipping keypoint with 0 matches."<<endl;
            }

            // Matches are sorted from best to worst.
            if (stabilizer->hasGoodMatch(matchesAtKeypoint)) {
                goodMatches.push_back(matchesAtKeypoint[0]);
            }
        }

       //cout<<"feature stabilizer, goodmatches size "<<goodMatches.size()<<endl;

        //-- Get the keypoints from the good matches
        std::vector<Point2f> obj;
        std::vector<Point2f> scene;

        for( int i = 0; i < goodMatches.size(); i++ )
        {
            obj.push_back( keypoints_1[ goodMatches[i].queryIdx ].pt );
            scene.push_back( keypoints_2[ goodMatches[i].trainIdx ].pt );
        }

       //if(frameCounter%100==0) cout<<"feature stabilizer, matches with good keypoint sizes "<<obj.size()<<" "<<scene.size()<< " frame "<<frameCounter<<"/"<<totalFrames<<endl;

        // Step 5, compute deformation from point couples
        try {
            H  = findHomography( obj, scene, CV_RANSAC );
            //H = estimateRigidTransform(obj, scene, false);
            previousH=H;
        }
        catch (Exception e)
        {
            cout<<"****************************************************************** FUCKED KEYPOINT SELECTION "<<e.what()<< ", doing nothing "<<obj.size()<<" "<<scene.size()<<" frame "<<frameCounter<<"/"<<totalFrames<<endl;

            // If it is not possible to find any match, just take previous matrix
            H=previousH;
        }

        // Other possible computations possible!!!!!

        // Step 6 warp video
        Mat cur2;
        try{
            warpPerspective(current, cur2, H, current.size());
        }
        catch(Exception e)
        {
            cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FUCKED warping "<<e.what()<<" , doing nothing "<<obj.size()<<" "<<scene.size()<<" frame "<<frameCounter<<"/"<<totalFrames<<endl;
            current.copyTo(cur2);
        }
        //warpAffine(current, cur2, H, current.size());

        //cur2 = cur2(Range(vert_border, cur2.rows-vert_border), Range(HORIZONTAL_BORDER_CROP, cur2.cols-HORIZONTAL_BORDER_CROP));
        // save the output video
        outputVideo << cur2;

        // once everything is finished, just update loop (copy current to previous)
        current.copyTo(previous);

        // Output things if necessary
        //if (verbose) {
           // cout << "INFORMATION ON STABILIZATION" << endl;
        //}
    }


}


/*


// Drawing matches and shit

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


//step 4: Find transformation from matches
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
*/

    // NOW we would still need to store the transformed image I guess, possibly
    // Step 5 - Apply the new transformation to the video
    /*cap.set(CV_CAP_PROP_POS_FRAMES, 0);
    Mat T(2,3,CV_64F);

    //int vert_border = HORIZONTAL_BORDER_CROP * prev.rows / prev.cols; // get the aspect ratio correct
    int vert_border = prev.rows / prev.cols; // get the aspect ratio correct

    //prepare outpout video
    int ex = static_cast<int>(cap.get(CAP_PROP_FOURCC)); // Get Codec Type- Int form
    Size S = Size((int) cap.get(CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) cap.get(CAP_PROP_FRAME_HEIGHT));

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

        T.at<double>(0,0) = cos(new_prev_to_cur_transform[k].da);
        T.at<double>(0,1) = -sin(new_prev_to_cur_transform[k].da);
        T.at<double>(1,0) = sin(new_prev_to_cur_transform[k].da);
        T.at<double>(1,1) = cos(new_prev_to_cur_transform[k].da);

        T.at<double>(0,2) = new_prev_to_cur_transform[k].dx;
        T.at<double>(1,2) = new_prev_to_cur_transform[k].dy;

        Mat cur2;

        warpAffine(cur, cur2, T, cur.size());

        cur2 = cur2(Range(vert_border, cur2.rows-vert_border), Range(HORIZONTAL_BORDER_CROP, cur2.cols-HORIZONTAL_BORDER_CROP));

        // Resize cur2 back to cur size, for better side by side comparison
        resize(cur2, cur2, cur.size());

           // save the output video and the comparison side by side
        outputVideo << cur2;
        //char str[256];
        //sprintf(str, "images/%08d.jpg", k);
        //imwrite(str, canvas);

        k++;

    }
    */
