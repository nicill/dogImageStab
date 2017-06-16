//
// Created by yago on 17/03/27.
//


#include "itkStabilizer.h"

itkStabilizer::itkStabilizer(type givenType) {

    this->stabType=givenType;

    // maybe also do dedicated things?
/*    switch (givenType) {
        case itkStabilizer::AFFINESSD:
            //this->detectorType = featureStabilizer::SIFT;
            //this->featureDetector = xfeatures2d::SIFT::create();
            break;
        case itkStabilizer::AFFINEMI:
            //this->detectorType = featureStabilizer::SURF;
            //this->featureDetector = xfeatures2d::SURF::create();
            break;
        default:
            throw("itkStabilizer::This descriptor hasn't been implemented yet.");
    }*/

}

itkStabilizer::type itkStabilizer::getStabType() const {
    return stabType;
}


// Class functions
void itkStabilizer::singleResolutionITKStab(VideoCapture capture, VideoWriter outputVideo )
{
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
    int ex = static_cast<int>(capture.get(CAP_PROP_FOURCC)); // Get Codec Type- Int form
    Size S = Size((int) capture.get(CAP_PROP_FRAME_WIDTH), (int) capture.get(CAP_PROP_FRAME_HEIGHT)); // Acquire input size

    bool verbose=false;

    //ITK declarations
    const unsigned int Dimension = 2;
    typedef unsigned char InputPixelType;
    //typedef float RealPixelType;
   // typedef unsigned char OutputPixelType;
    typedef itk::Image<InputPixelType, Dimension> InputImageType;
    //typedef itk::Image<RealPixelType, Dimension> RealInputImageType;
    //typedef itk::Image<OutputPixelType, Dimension> OutputInputImageType;
    typedef itk::OpenCVImageBridge BridgeType;

    //typedef itk::CastImageFilter<InputImageType, RealInputImageType> CastFilterType;
   // typedef itk::CannyEdgeDetectionImageFilter<RealInputImageType, RealInputImageType> FilterType;
   // typedef itk::RescaleIntensityImageFilter<RealInputImageType, OutputInputImageType> RescaleFilterType;

    // AFFINE REGISTRATION!!!!!!!!!! ITK/Examples/Registration/ImageRegistrationMethodAffine, exemples a : https://itk.org/Wiki/ITK/Examples#Image_Registration
    //  transform maps fixed into moving (transform "pulls")
    typedef itk::AffineTransform< double, Dimension > TransformType;

    // deffine all metric types
    typedef itk::MeanSquaresImageToImageMetric< InputImageType,  InputImageType >    MetricTypeSSD; // SSD metric
    //typedef itk::MattesMutualInformationImageToImageMetric<InputImageType,InputImageType >    MetricTypeMI; //MattesMI metric
    typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<InputImageType,InputImageType >    MetricTypeMI;
    typedef itk::NormalizedCorrelationImageToImageMetric<InputImageType,InputImageType> MetricTypeNCC; // Normalised correlation metric

    // other declarations
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef itk::LinearInterpolateImageFunction< InputImageType, double >    InterpolatorType;
    typedef itk::ImageRegistrationMethod< InputImageType, InputImageType >    RegistrationType;
    typedef RegistrationType::ParametersType ParametersType;

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

        // Open everything in itk
        InputImageType::Pointer fixedImage = itk::OpenCVImageBridge::CVMatToITKImage<InputImageType>(current);
        InputImageType::Pointer movingImage = itk::OpenCVImageBridge::CVMatToITKImage<InputImageType>(previous);

        // initializations
        //MetricType::Pointer         metric        = MetricType::New();
        TransformType::Pointer      transform     = TransformType::New();
        OptimizerType::Pointer      optimizer     = OptimizerType::New();
        InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
        RegistrationType::Pointer   registration  = RegistrationType::New();

        int numIterations=500;

        // Each component is now connected to the instance of the registration method.

        if(this->getStabType()==0) //SSD metric
        {
            MetricTypeSSD::Pointer         metricSSD        = MetricTypeSSD::New();
            registration->SetMetric(        metricSSD        );
        }
        else if (this->getStabType()==1)  //NormalisedMI metric
        {
            MetricTypeMI::Pointer         metricMI        = MetricTypeMI::New();
            MetricTypeMI::HistogramType::SizeType histogramSize;
            int numberOfHistogramBins =32;
            histogramSize.SetSize(2);
            histogramSize[0] = numberOfHistogramBins;
            histogramSize[1] = numberOfHistogramBins;
            metricMI->SetHistogramSize( histogramSize );

            //metricMI->SetNumberOfHistogramBins( 512 ); // Mattes MI PARAMETERS!!!!
            //metricMI->SetNumberOfSpatialSamples( 10000 );
            registration->SetMetric(        metricMI        );
        }
        else if (this->getStabType()==2)  //Normalised Cross correlation
        {
            MetricTypeNCC::Pointer         metricNCC        = MetricTypeNCC::New();
            registration->SetMetric(        metricNCC        );
        }
        else {
            cout<<"itk stabilizer main, unknown type "<<endl;
            throw ("itk stabilizer main, unknown type ");
        }

        registration->SetOptimizer(     optimizer     );
        registration->SetTransform(     transform     );
        registration->SetInterpolator(  interpolator  );

        // Set the registration inputs
        registration->SetFixedImage(fixedImage);
        registration->SetMovingImage(movingImage);
        registration->SetFixedImageRegion(fixedImage->GetLargestPossibleRegion() );

        //  Initialize the transform
        ParametersType initialParameters( transform->GetNumberOfParameters() );

        // Initial transform Id
        initialParameters[0] = 1.0;  // R(0,0)
        initialParameters[1] = 0.0;  // R(0,1)
        initialParameters[2] = 0.0;  // R(1,0)
        initialParameters[3] = 1.0;  // R(1,1)
        initialParameters[4] = 0.0;        // translation vector
        initialParameters[5] = 0.0;
        registration->SetInitialTransformParameters( initialParameters );

        // Optimizer params
        optimizer->SetMaximumStepLength( .1 ); // If this is set too high, you will get a
        //"itk::ERROR: MeanSquaresImageToImageMetric(0xa27ce70): Too many samples map outside moving image buffer: 1818 / 10000" error
        optimizer->SetMinimumStepLength( 0.01 );
        optimizer->SetNumberOfIterations( numIterations );

        bool registrationFailure=false;

        try
        {
            registration->Update();
        }
        catch( itk::ExceptionObject & err )
        {
            std::cerr << "ExceptionObject caught !" << std::endl;
            std::cerr << err << std::endl;

            // in this case, use previous transform parameters
            registrationFailure=true;
            // here we could count the number of failures
        }


        ParametersType finalParameters; // retrieve the final transform, depending on what we had success or not
        if(!registrationFailure) finalParameters = registration->GetLastTransformParameters();
        else  finalParameters=initialParameters;

        if (verbose) std::cout << "Final parameters: " << finalParameters << std::endl;

        const double bestValue = optimizer->GetValue();

        // Print out results
        //
        if (verbose) {
            std::cout << "Result = " << std::endl;
            std::cout << " Metric value  = " << bestValue << std::endl;
        }

        // Resample and write to file, maybe for demons?
        /* typedef itk::ResampleImageFilter< InputImageType, InputImageType >    ResampleFilterType;
         ResampleFilterType::Pointer resampler = ResampleFilterType::New();
         resampler->SetInput( movingImage);

         resampler->SetTransform( registration->GetOutput()->Get() );
         resampler->SetSize( fixedImage->GetLargestPossibleRegion().GetSize() );
         resampler->SetOutputOrigin(  fixedImage->GetOrigin() );
         resampler->SetOutputSpacing( fixedImage->GetSpacing() );
         resampler->SetOutputDirection( fixedImage->GetDirection() );
         resampler->SetDefaultPixelValue( 100 );

         typedef unsigned char OutputPixelType;
         typedef itk::Image< OutputPixelType, Dimension > OutputInputImageType;
         typedef itk::CastImageFilter< InputImageType, InputImageType > CastFilterType;

         CastFilterType::Pointer  caster =  CastFilterType::New();
         caster->SetInput( resampler->GetOutput() );
         caster->Update();*/

        //transform with the final registration parameters
        Mat T(2,3,CV_64F);

        T.at<double>(0,0) = finalParameters[0];
        T.at<double>(0,1) = finalParameters[1];
        T.at<double>(1,0) = finalParameters[2];
        T.at<double>(1,1) = finalParameters[3];

        T.at<double>(0,2) = finalParameters[4];
        T.at<double>(1,2) = finalParameters[5];

        Mat cur2;

        warpAffine(current, cur2, T, current.size());
        outputVideo << cur2;

        // THIS IMAGE INTO THE NEXT frame in opencv and not itk
/*        Mat cur2 = itk::OpenCVImageBridge::ITKImageToCVMat(caster->GetOutput()); //does this work?
        outputVideo << cur2;

        char str[256];
        sprintf(str, "./%08d.jpg", frameCounter);
        imwrite(str, cur2);*/


        // once everything is finished, just update loop (copy current to previous)
        current.copyTo(previous);
    }



}


// Class functions
void itkStabilizer::multiResolutionITKStab(VideoCapture capture, VideoWriter outputVideo )
{
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
    int ex = static_cast<int>(capture.get(CAP_PROP_FOURCC)); // Get Codec Type- Int form
    Size S = Size((int) capture.get(CAP_PROP_FRAME_WIDTH), (int) capture.get(CAP_PROP_FRAME_HEIGHT)); // Acquire input size

    bool verbose=false;

    //ITK declarations
    const unsigned int Dimension = 2;
    //typedef unsigned char InputPixelType;
    typedef float RealPixelType;
    //typedef unsigned char OutputPixelType;
    typedef itk::Image<RealPixelType, Dimension> InputImageType;
    //typedef itk::Image<RealPixelType, Dimension> RealInputImageType;
    //typedef itk::Image<OutputPixelType, Dimension> OutputInputImageType;
    typedef itk::OpenCVImageBridge BridgeType;

    //typedef itk::CastImageFilter<InputImageType, RealInputImageType> CastFilterType;
   // typedef itk::CannyEdgeDetectionImageFilter<RealInputImageType, RealInputImageType> FilterType;
   // typedef itk::RescaleIntensityImageFilter<RealInputImageType, OutputInputImageType> RescaleFilterType;

    // AFFINE REGISTRATION!!!!!!!!!! ITK/Examples/Registration/ImageRegistrationMethodAffine, exemples a : https://itk.org/Wiki/ITK/Examples#Image_Registration
    //  transform maps fixed into moving (transform "pulls")
    typedef itk::AffineTransform< double, Dimension > TransformType;

    // deffine all metric types
    typedef itk::MeanSquaresImageToImageMetricv4< InputImageType,  InputImageType >    MetricTypeSSD; // SSD metric
    //typedef itk::MattesMutualInformationImageToImageMetricv4<InputImageType,InputImageType >    MetricTypeMI; //MattesMI metric
    typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<InputImageType,InputImageType >    MetricTypeMI;
    typedef itk::CorrelationImageToImageMetricv4<InputImageType,InputImageType> MetricTypeNCC; // Normalised correlation metric

    // other declarations
    //typedef itk::RegularStepGradientDescentOptimizerv4<double>  OptimizerType;
    typedef itk::ConjugateGradientLineSearchOptimizerv4Template< double >         AOptimizerType;
    typedef itk::LinearInterpolateImageFunction< InputImageType, double >  InterpolatorType;

    typedef itk::ImageRegistrationMethodv4< InputImageType,InputImageType,  TransformType >   RegistrationType;
    typedef itk::ConjugateGradientLineSearchOptimizerv4Template<double >  OptimizerType;
    typedef OptimizerType::ParametersType ParametersType;

    typedef InputImageType::SpacingType    SpacingType;
    typedef InputImageType::PointType      OriginType;
    typedef InputImageType::RegionType     RegionType;
    typedef InputImageType::SizeType       SizeType;

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

        // Open everything in itk
        InputImageType::Pointer fixedImage = itk::OpenCVImageBridge::CVMatToITKImage<InputImageType>(current);
        InputImageType::Pointer movingImage = itk::OpenCVImageBridge::CVMatToITKImage<InputImageType>(previous);


        TransformType::Pointer      transform     = TransformType::New();
        OptimizerType::Pointer      optimizer     = OptimizerType::New();

       // MetricType::Pointer         metric        = MetricType::New();

        RegistrationType::Pointer   registration  = RegistrationType::New();
        registration->SetOptimizer( optimizer );
        //registration->SetMetric( metric  );

        int numIterations=500;

        // Each component is now connected to the instance of the registration method.
        MetricTypeSSD::Pointer         metricSSD        = MetricTypeSSD::New();
        //MetricTypeMI::Pointer         metricMI        = MetricTypeMI::New();
        MetricTypeMI::Pointer           metricMI        = MetricTypeMI::New();

        MetricTypeNCC::Pointer         metricNCC        = MetricTypeNCC::New();

        if(this->getStabType()==3) //SSD metric
        {
            registration->SetMetric(        metricSSD        );
        }
        else if (this->getStabType()==4)  //Normalised MI metric
        {
           // this is really not supported
            cout<<"FUCK MULTIRES MI NOT SUPPORTED"<<endl;
            throw("NOT SUPPORTED");
            //metricMI->SetNumberOfHistogramBins( 512 ); // Mattes MI PARAMETERS!!!!
           // metricMI->SetNumberOfSpatialSamples( 10000 );
           // registration->SetMetric(        metricMI        );
        }
        else if (this->getStabType()==5)  //Normalised Cross correlation
        {
            registration->SetMetric(        metricNCC        );
        }
        else {
            cout<<"itk stabilizer main, unknown type "<<endl;
            throw ("itk stabilizer main, unknown type ");
        }

        registration->SetOptimizer(     optimizer     );

        //  Initialize the transform
        ParametersType initialParameters( transform->GetNumberOfParameters() );


        const SpacingType fixedSpacing = fixedImage->GetSpacing();
        const OriginType  fixedOrigin  = fixedImage->GetOrigin();
        const RegionType  fixedRegion  = fixedImage->GetLargestPossibleRegion();
        const SizeType    fixedSize    = fixedRegion.GetSize();

        TransformType::InputPointType centerFixed;
        centerFixed[0] = fixedOrigin[0] + fixedSpacing[0] * fixedSize[0] / 2.0;
        centerFixed[1] = fixedOrigin[1] + fixedSpacing[1] * fixedSize[1] / 2.0;
        const unsigned int numberOfFixedParameters = transform->GetFixedParameters().Size();
        TransformType::ParametersType fixedParameters( numberOfFixedParameters );
        for (unsigned int i = 0; i < numberOfFixedParameters; ++i)
        {
            fixedParameters[i] = centerFixed[i];
        }
        transform->SetFixedParameters( fixedParameters );

       // registration->SetInitialTransform( transform );
       // registration->InPlaceOn();

        optimizer->SetNumberOfIterations( 125 );

        typedef itk::RegistrationParameterScalesFromPhysicalShift< MetricTypeSSD> ScalesEstimatorTypeSSD;
        typedef itk::RegistrationParameterScalesFromPhysicalShift< MetricTypeMI> ScalesEstimatorTypeMI;
        typedef itk::RegistrationParameterScalesFromPhysicalShift< MetricTypeNCC> ScalesEstimatorTypeNCC;

        ScalesEstimatorTypeSSD::Pointer scalesEstimatorSSD =  ScalesEstimatorTypeSSD::New();
       // ScalesEstimatorTypeMI::Pointer scalesEstimatorMI =  ScalesEstimatorTypeMI::New();
        ScalesEstimatorTypeNCC::Pointer scalesEstimatorNCC =  ScalesEstimatorTypeNCC::New();

        if(this->getStabType()==3) //SSD metric
        {
            scalesEstimatorSSD->SetMetric( metricSSD );
            scalesEstimatorSSD->SetTransformForward( true );
            optimizer->SetScalesEstimator( scalesEstimatorSSD );

        }
        else if (this->getStabType()==4)  //metric
        {
            //scalesEstimatorMI->SetMetric( metricMI );
            //scalesEstimatorMI->SetTransformForward( true );
            //optimizer->SetScalesEstimator( scalesEstimatorMI );

        }
        else if (this->getStabType()==5)  //Normalised Cross correlation
        {
            scalesEstimatorNCC->SetMetric( metricNCC );
            scalesEstimatorNCC->SetTransformForward( true );
            optimizer->SetScalesEstimator( scalesEstimatorNCC );

        }
        else {
            cout<<"itk stabilizer main, unknown type "<<endl;
            throw ("itk stabilizer main, unknown type ");
        }

        optimizer->SetDoEstimateLearningRateOnce( true );
        optimizer->SetDoEstimateLearningRateAtEachIteration( false );
        optimizer->SetLowerLimit( 0 );
        optimizer->SetUpperLimit( 2 );
        optimizer->SetEpsilon( 0.2 );
        optimizer->SetNumberOfIterations( 200 );
        optimizer->SetMinimumConvergenceValue( 1e-6 );
        optimizer->SetConvergenceWindowSize( 5 );

        // Set the registration inputs
        registration->SetFixedImage(fixedImage);
        registration->SetMovingImage(movingImage);


        // MULTIRESOLUTION PART
        const unsigned int numberOfLevels = 3;
        RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
        shrinkFactorsPerLevel.SetSize( 3 );
        shrinkFactorsPerLevel[0] = 3;
        shrinkFactorsPerLevel[1] = 2;
        shrinkFactorsPerLevel[2] = 1;
        RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
        smoothingSigmasPerLevel.SetSize( 3 );
        smoothingSigmasPerLevel[0] = 0;
        smoothingSigmasPerLevel[1] = 0;
        smoothingSigmasPerLevel[2] = 0;
        registration->SetNumberOfLevels ( numberOfLevels );
        registration->SetShrinkFactorsPerLevel( shrinkFactorsPerLevel );
        registration->SetSmoothingSigmasPerLevel( smoothingSigmasPerLevel );


        bool registrationFailure=false;

        try
        {
            registration->Update();
        }
        catch( itk::ExceptionObject & err )
        {
            std::cerr << "ExceptionObject caught !" << std::endl;
            std::cerr << err << std::endl;

            // in this case, use previous transform parameters
            registrationFailure=true;
            // here we could count the number of failures
        }

        ParametersType finalParameters; // retrieve the final transform, depending on what we had success or not
        if(!registrationFailure) finalParameters = transform->GetParameters();
        else  finalParameters=initialParameters;

        if (verbose) std::cout << "Final parameters: " << finalParameters << std::endl;

        const double bestValue = optimizer->GetValue();

        // Print out results
        //
        if (verbose) {
            std::cout << "Result = " << std::endl;
            std::cout << " Metric value  = " << bestValue << std::endl;
        }

        //transform with the final registration parameters
        Mat T(2,3,CV_64F);

        T.at<double>(0,0) = finalParameters[0];
        T.at<double>(0,1) = finalParameters[1];
        T.at<double>(1,0) = finalParameters[2];
        T.at<double>(1,1) = finalParameters[3];

        T.at<double>(0,2) = finalParameters[4];
        T.at<double>(1,2) = finalParameters[5];

        Mat cur2;

        warpAffine(current, cur2, T, current.size());
        outputVideo << cur2;

        // once everything is finished, just update loop (copy current to previous)
        current.copyTo(previous);
    }



}




// *************************************** END OF CLASS FUNCTION IMPLEMENTATION



// also main function to generate executable
/** @function main */
int main( int argc, char** argv ) {

    // Parameters, 1 input file, 2 output file,
    if (argc != 4) {
        std::cerr << "itkStabilizer main::Wrong number of parameters, supplied: " << argc << std::endl;
    }
    // read Video
    VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        std::cerr << "Could not open the video supplied: " << argv[1] << std::endl;
        return 1;
    }
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
    int ex = static_cast<int>(capture.get(CAP_PROP_FOURCC)); // Get Codec Type- Int form
    Size S = Size((int) capture.get(CAP_PROP_FRAME_WIDTH), (int) capture.get(CAP_PROP_FRAME_HEIGHT)); // Acquire input size

    // Open output video too
    VideoWriter outputVideo;
    outputVideo.open(argv[2], ex, capture.get(CAP_PROP_FPS), S, true);
    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for writting: " << argv[2] << endl;
        return -1;
    }

    // Stabilizer type treatment
    itkStabilizer *stabilizer;
    stabilizer=new itkStabilizer((itkStabilizer::type)atoi(argv[3]));

    stabilizer->itkStabilize(capture,outputVideo);


}


