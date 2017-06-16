//
// Created by yago on 17/02/20.
//

#include "featureStabilizer.h"

/**
 * Constructor.
 * @param givenDetectorType Type of detector.
 * @param givenMatcherType Type of matcher.
 * For SIFT, SURF etc. use L2. For ORB, BRIEF, BRISK etc. use HAMMING.
 * If ORB is using WTA_K == 3 or 4 use HAMMING2.
 */
featureStabilizer::featureStabilizer(type givenType, string paramInit) {
    // The parameters to initialize every type are given as argv parameters after argv[3], this should be made nicer
    switch (givenType) {
        case featureStabilizer::SIFT_BFL2:
        {
            /*     static Ptr< SIFT > 	create (int nfeatures=0, int nOctaveLayers=3, double contrastThreshold=0.04, double edgeThreshold=10, double sigma=1.6)
                Parameters:
                   nfeatures – The number of best features to retain. The features are ranked by their scores (measured in SIFT algorithm as the local contrast)
                   nOctaveLayers – The number of layers in each octave. 3 is the value used in D. Lowe paper. The number of octaves is computed automatically from the image resolution.
                   contrastThreshold – The contrast threshold used to filter out weak features in semi-uniform (low-contrast) regions. The larger the threshold, the less features are produced by the detector.
                   edgeThreshold – The threshold used to filter out edge-like features. Note that the its meaning is different from the contrastThreshold, i.e. the larger the edgeThreshold, the less features are filtered out (more features are retained).
                   sigma – The sigma of the Gaussian applied to the input image at the octave #0. If your image is captured with a weak camera with soft lenses, you might want to reduce the number.
            */
            this->detectorType = featureStabilizer::SIFT;
            // SO far for sift we only take the number of features and the sigma
            // Proces paramString
            int nFeatures=0;
            double sigma=1.6;
            int pos1=paramInit.find("nFeatures");
            int pos2=paramInit.find("sigma");
            if(pos1!= string::npos) nFeatures=stoi(paramInit.substr(pos1+9,1));
            if(pos2!= string::npos) sigma=stof(paramInit.substr(pos2+5));
            //std::cout<<"INITIALIZED SIFT WITH PARAMS "<<nFeatures<<" "<<sigma<<std::endl;
            this->featureDetector = xfeatures2d::SIFT::create(nFeatures, 3, 0.04, 10, sigma);
            break;
        }
        case featureStabilizer::SURF_BFL2: {
            /*
             * static Ptr< SURF > 	create (double hessianThreshold=100, int nOctaves=4, int nOctaveLayers=3, bool extended=false, bool upright=false)
             hessianThreshold	Threshold for hessian keypoint detector used in SURF.
             nOctaves	Number of pyramid octaves the keypoint detector will use.
             nOctaveLayers	Number of octave layers within each octave.
             extended	Extended descriptor flag (true - use extended 128-element descriptors; false - use 64-element descriptors).
             upright	Up-right or rotated features flag (true - do not compute orientation of features; false - compute orientation).
             */
            double sigma=100;
            int pos1=paramInit.find("hessianT");
            if(pos1!= string::npos) sigma=stof(paramInit.substr(pos1+8));

            this->detectorType = featureStabilizer::SURF;
            this->featureDetector = xfeatures2d::SURF::create(sigma,4,3,false,false);
            //std::cout<<"INITIALIZED SURF WITH PARAM "<<sigma<<" "<<paramInit.substr(pos1+8)<<std::endl;
            break;
        }
        case featureStabilizer::ORB_BFHAMMING: {
            this->detectorType = featureStabilizer::ORB;
            /*
             *
                nfeatures – The maximum number of features to retain.
                scaleFactor – Pyramid decimation ratio, greater than 1. scaleFactor==2 means the classical pyramid, where each next level has 4x less pixels than the previous, but such a big scale factor will degrade feature matching scores dramatically. On the other hand, too close to 1 scale factor will mean that to cover certain scale range you will need more pyramid levels and so the speed will suffer.
                nlevels – The number of pyramid levels. The smallest level will have linear size equal to input_image_linear_size/pow(scaleFactor, nlevels).
                edgeThreshold – This is size of the border where the features are not detected. It should roughly match the patchSize parameter.
                firstLevel – It should be 0 in the current implementation.
                WTA_K – The number of points that produce each element of the oriented BRIEF descriptor. The default value 2 means the BRIEF where we take a random point pair and compare their brightnesses, so we get 0/1 response. Other possible values are 3 and 4. For example, 3 means that we take 3 random points (of course, those point coordinates are random, but they are generated from the pre-defined seed, so each element of BRIEF descriptor is computed deterministically from the pixel rectangle), find point of maximum brightness and output index of the winner (0, 1 or 2). Such output will occupy 2 bits, and therefore it will need a special variant of Hamming distance, denoted as NORM_HAMMING2 (2 bits per bin). When WTA_K=4, we take 4 random points to compute each bin (that will also occupy 2 bits with possible values 0, 1, 2 or 3).
                scoreType – The default HARRIS_SCORE means that Harris algorithm is used to rank features (the score is written to KeyPoint::score and is used to retain best nfeatures features); FAST_SCORE is alternative value of the parameter that produces slightly less stable keypoints, but it is a little faster to compute.
                patchSize – size of the patch used by the oriented BRIEF descriptor. Of course, on smaller pyramid layers the perceived image area covered by a feature will be larger.
             */

            int nFeatures=500;
            int WTA=2;
            int pos1=paramInit.find("nFeatures");
            int pos2=paramInit.find("*");
            int pos3=paramInit.find("WTA");
            if(pos1!= string::npos) nFeatures=stoi(paramInit.substr(pos1+9,pos2-pos1));
            if(pos3!= string::npos) WTA=stoi(paramInit.substr(pos3+3,1));
           // std::cout<<"INITIALIZED ORB WITH PARAMS "<<nFeatures<<" "<<WTA<<std::endl;
            this->featureDetector = cv::ORB::create(nFeatures, 1.2, 8, 31, 0, WTA, ORB::HARRIS_SCORE, 31);
            //this->featureDetector = cv::ORB::create(500,1.2,8,31,0,2,ORB::HARRIS_SCORE,31);
            break;
        }
        case featureStabilizer::BRISK_BFHAMMING: {
            this->detectorType = featureStabilizer::BRISK;
            /* C++: BRISK::BRISK(int thresh=30, int octaves=3, float patternScale=1.0f)
             Parameters:

            thresh – FAST/AGAST detection threshold score.
            octaves – detection octaves. Use 0 to do single scale.
            patternScale – apply this scale to the pattern used for sampling the neighbourhood of a keypoint.

            */
            int thresh=30;
            double patternScale=1.0f;
            int pos1=paramInit.find("thresh");
            double pos2=paramInit.find("patternScale");
            if(pos1!= string::npos) thresh=stoi(paramInit.substr(pos1+9,1));
            if(pos2!= string::npos) patternScale=stof(paramInit.substr(pos2+12));

            this->featureDetector = cv::BRISK::create(thresh,3, patternScale);
            //std::cout<<"INITIALIZED BRISK WITH PARAMS "<<thresh<<" "<<patternScale<<" "<<paramInit.substr(pos2+12)<<std::endl;

            break;
        }
        default:
            throw("This descriptor hasn't been implemented yet.");
    }

    switch (givenType) {
        case featureStabilizer::SIFT_BFL2:
        case featureStabilizer::SURF_BFL2:
            this->matcherType = featureStabilizer::BF_L2;
            this->descriptorMatcher = BFMatcher::create();
            break;
        case featureStabilizer::BRISK_BFHAMMING:
        case featureStabilizer::ORB_BFHAMMING:
            this->matcherType = featureStabilizer::BF_HAMMING;
            this->descriptorMatcher = BFMatcher::create(BFMatcher::BRUTEFORCE_HAMMING);
    }
}

/**
 * Destructor.
 */
featureStabilizer::~featureStabilizer() {
    featureDetector.release();
    descriptorMatcher.release();
}

/**
 * Interface function. Compute similarity between two given frames.
 * @param im1 Frame #1
 * @param im2 Subsequent frame #2
 * @return Similarity score [0,1]
 */
/*double featureStabilizer::computeSimilarity(Mat* im1, Mat* im2) {
    this->processedComparisons++;

    vector<vector<DMatch>> matchesOfAllKeypoints = this->getMatches(im1, im2);
    if (matchesOfAllKeypoints.size() == 0) return 0;

    vector<DMatch> goodMatches;
    for (vector<DMatch> matchesAtKeypoint : matchesOfAllKeypoints) {
        // Match list might not contain any matches
        if (matchesAtKeypoint.size() == 0) {
            message("Skipping keypoint with 0 matches.");
            continue;
        }

        // Matches are sorted from best to worst.
        if (this->hasGoodMatch(matchesAtKeypoint)) {
            goodMatches.push_back(matchesAtKeypoint[0]);
        }
    }

    return (double)goodMatches.size() / matchesOfAllKeypoints.size();
}*/

/**
 * Interface function. Activates output.
 */
void featureStabilizer::activateVerbosity() {
    this->verbose = true;
}

/**
 * Computes matches between the two given images.
 * @param img1 The query image.
 * @param img2 The training image.
 * @return A list of the best two matches for each determined keypoint.
 * Elements might contain no matches!
 */
vector<vector<DMatch>> featureStabilizer::getMatches(Mat* img1, Mat* img2, vector<KeyPoint> * keypoints_1, vector<KeyPoint> * keypoints_2) {
   // vector<KeyPoint> keypoints_1, keypoints_2;
    bool verbose=true;
    Mat descriptors_1, descriptors_2;
    vector<vector<DMatch>> matchesOfAllKeypoints;

    // Detect keypoints and compute descriptors (feature vectors)
    clock_t t;
    t = clock();
    this->featureDetector->detectAndCompute(*img1, noArray(), *keypoints_1, descriptors_1);
    t = clock() - t;
    if(verbose) cout<<"                           Current frame detection and description took in seconds: "<<((float)t)/CLOCKS_PER_SEC<<" and in clicks "<<t<<endl;
    t = clock();
    this->featureDetector->detectAndCompute(*img2, noArray(), *keypoints_2, descriptors_2);
    t = clock() - t;
    if(verbose) cout<<"                           Previous frame detection and description took in seconds: "<<((float)t)/CLOCKS_PER_SEC<<" and in clicks "<<t<<endl;

    // Make sure that keypoints have been found in both images before matching.
    if (keypoints_1->size() == 0 || keypoints_2->size() == 0) {
        message("No keypoints found!");
        return vector<vector<DMatch>>();
    }

    // Match descriptors and retrieve the k=2 best results
    t = clock();
    this->descriptorMatcher->knnMatch(descriptors_1,descriptors_2, matchesOfAllKeypoints, 2);
    t = clock() - t;
    if(verbose) cout<<"                           Key-point matching took: "<<((float)t)/CLOCKS_PER_SEC<<" and in clicks "<<t<<endl;

    //Other matching option!!!
    //FlannBasedMatcher matcher;
    //matcher.match( descriptors_1, descriptors_2, matchesOfAllKeypoints,cv::noArray() ); // BEWARE!!!!!!!!!!!!!!! cv::noArray() added!

    if (matchesOfAllKeypoints.size() == 0) {
        message("featureStabilizer::getMatches No matches found!");
    }

    return matchesOfAllKeypoints;
}

/**
 * Checks the given list of matches if the best one can be considered "good".
 * Uses ratio test proposed by Lowe.
 * @param possibleMatches All possible matches for a keypoint.
 * Must be of size 2!
 */
bool featureStabilizer::hasGoodMatch(vector<DMatch> possibleMatches) {
    // Alternatively use crosscheck? http://docs.opencv.org/3.0-beta/doc/py_tutorials/py_feature2d/py_matcher/py_matcher.html

    // Filter matches based on metric proposed by Lowe (2004), p. 104.
    assert(possibleMatches.size() <= 2);

    if (possibleMatches.size() != 2) return false;
    else return possibleMatches[0].distance < 0.8 * possibleMatches[1].distance;
}







/**
 * Handles showing output messages based on the verbosity setting.
 * @param text The text to display on cout.
 */
void featureStabilizer::message(string text) {
    if (!this->verbose) {
        return;
    }

    std::cout << "[featureStabilizer] " << text << std::endl;
}
