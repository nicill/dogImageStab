//
// Created by yago on 17/05/16.
//

#include "testVideoGenerator.h"



void testVideoGenerator::generateImageRotation(string imagePath,double rotAngle,string outputVideoPath)
{

    int totalFrames=500;
    double fps=25;

    /// Load the image
    Mat src = imread( imagePath, 1 );

    // make the size up!!!
    int ex = static_cast<int>(CV_FOURCC('M', 'J', 'P', 'G')); // Get Codec Type- Int form
    Size S = Size(2*src.size().width,2*src.size().height);    // Acquire input size

    //double transx=2*src.size().width;
    //double transy=2*src.size().width;

    double transx=(S.width/2)-(src.size().width/2);
    double transy=(S.height/2)-(src.size().height/2) ;

    Point2f srcTri[3];
    Point2f dstTri[3];
    srcTri[0] = Point2f( 0,0 );
    srcTri[1] = Point2f( 1, 0 );
    srcTri[2] = Point2f( 0, 1 );

    dstTri[0] = Point2f( transx,transy  );
    dstTri[1] = Point2f( 1+transx,transy );
    dstTri[2] = Point2f( transx, 1+transy );

    /// Get the Affine Transform
    Mat warp_mat = getAffineTransform( srcTri, dstTri );

    // Open output video too
    VideoWriter outputVideo;
    outputVideo.open(outputVideoPath, ex, fps, S, true);
    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for writting: " << outputVideoPath << endl;
        exit(-1);
    }

    double angle=rotAngle;

    for (int frameCounter = 2; frameCounter <= totalFrames; frameCounter++) {

        // generate cur as a rotated copy of the original image


        /// Compute a rotation matrix with respect to the center of the image
        Point center = Point( S.width/2, S.height/2 );
        angle = angle+rotAngle;

        /// Get the rotation matrix with the specifications above
        Mat rot_mat = getRotationMatrix2D( center, angle, 1 );

        Mat translated;

        Mat rotated;

        /// Rotate the warped image
        warpAffine( src, translated, warp_mat, S );
        warpAffine( translated, rotated, rot_mat, S );

        outputVideo << rotated;

    }
}





int main(int argc, char** argv) {

    // receive an image and an angle increment and generate a 100 frame video with rotations around the center
    bool verbose=true;

    if (argc != 4) {
        std::cerr << "generateTest input main::Possible Wrong number of parameters, supplied: " << argc << std::endl;
    }

    testVideoGenerator testVideoGen= testVideoGenerator();
    testVideoGen.generateImageRotation(argv[1],atof(argv[2]),argv[3]);

    //testVideoGenerator


}