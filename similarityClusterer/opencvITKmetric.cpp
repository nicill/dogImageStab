//
// Created by yago on 17/02/22.
//

#include "opencvITKmetric.h"

void opencvITKmetric::activateVerbosity() {
    return;
}

opencvITKmetric::opencvITKmetric(int comparisonType) : comparisonType(comparisonType) { }



double opencvITKmetric::computeSimilarity(Mat* im1, Mat* im2) {

    const unsigned int Dimension = 2;
    typedef unsigned char InputPixelType;
    typedef float RealPixelType;
    typedef unsigned char OutputPixelType;
    typedef itk::Image<InputPixelType, Dimension> InputImageType;
    typedef itk::Image<RealPixelType, Dimension> RealImageType;
    typedef itk::Image<OutputPixelType, Dimension> OutputImageType;
    typedef itk::OpenCVImageBridge BridgeType;

    typedef itk::CastImageFilter<InputImageType, RealImageType> CastFilterType;
    typedef itk::CannyEdgeDetectionImageFilter<RealImageType, RealImageType> FilterType;
    typedef itk::RescaleIntensityImageFilter<RealImageType, OutputImageType> RescaleFilterType;

    InputImageType::Pointer itkFrame1 = itk::OpenCVImageBridge::CVMatToITKImage<InputImageType>(*im1);
    InputImageType::Pointer itkFrame2 = itk::OpenCVImageBridge::CVMatToITKImage<InputImageType>(*im2);

    double ret;
    int numberOfBins = 256;
    unsigned int numberOfSamples = 10000;

    float threshold = 1;

    bool erMMI = false, erRMS = false, erNHMMI = false;

    typedef itk::MeanSquaresImageToImageMetric<InputImageType, InputImageType> MSMetricType;
    typedef itk::MattesMutualInformationImageToImageMetric<InputImageType, InputImageType> MattesMIMetricType;

    typedef itk::LinearInterpolateImageFunction<InputImageType, double> linInterpolatorType;
    typedef itk::AffineTransform<double, 2> AffineTransformType;
    typedef AffineTransformType::ParametersType parametersType;
    typedef itk::MinimumMaximumImageCalculator<InputImageType> MinimumMaximumImageCalculatorType;

    MSMetricType::Pointer metricMS = MSMetricType::New();
    MattesMIMetricType::Pointer metricMMI = MattesMIMetricType::New();

    // minmax thingie calculator
    MinimumMaximumImageCalculatorType::Pointer minmax;
    minmax = MinimumMaximumImageCalculatorType::New();
    minmax->SetImage(itkFrame1);
    minmax->Compute();

    linInterpolatorType::Pointer interpolator = linInterpolatorType::New();
    AffineTransformType::Pointer transform = AffineTransformType::New();

    // Set identity transform
    transform->SetIdentity();
    parametersType displacement(transform->GetNumberOfParameters());
    displacement.Fill(0.0);

    double valueMMI = 7777;

    switch (comparisonType) {
        case 0: // MS metric



            // Finish setting up metric
            metricMS->SetInterpolator(interpolator);
            metricMS->SetTransform(transform);

            metricMS->SetFixedImage(itkFrame1);
            metricMS->SetMovingImage(itkFrame2);
            metricMS->SetFixedImageRegion(itkFrame1->GetBufferedRegion());


            metricMS->SetFixedImageSamplesIntensityThreshold(minmax->GetMaximum() * 0.05);

            /*if(useAvancedMethods)
            {
                metricMS->SetUseFixedImageSamplesIntensityThreshold(true);
                metricMS->SetFixedImageSamplesIntensityThreshold(threshold);
            }*/

            try {
                metricMS->Initialize();
            }
            catch (itk::ExceptionObject &excep) {
                std::cerr << "Exception catched !" << std::endl;
                std::cerr << excep << std::endl;
                cout << "opencvITKmetric::computeSimilarity error computing metric" << endl;
                throw "opencvITKmetric::computeSimilarity error computing metric";

            }

            ret = metricMS->GetValue(displacement);
            break;


        case 1: // Mattes mutual information
            // MATES MUTUAL INFORMATION METRIC


            metricMMI->SetInterpolator(interpolator);
            metricMMI->SetTransform(transform);

            transform->SetIdentity();

            metricMMI->SetFixedImage(itkFrame1);
            metricMMI->SetMovingImage(itkFrame2);

            metricMMI->SetFixedImageRegion(itkFrame1->GetBufferedRegion());

            metricMMI->SetNumberOfSpatialSamples(numberOfSamples);
            metricMMI->SetNumberOfHistogramBins(numberOfBins);

            metricMMI->SetFixedImageSamplesIntensityThreshold(minmax->GetMaximum() * 0.05);

            /* if(useAvancedMethods)
             {
                 metricMMI->SetUseFixedImageSamplesIntensityThreshold(true);
                 metricMMI->SetFixedImageSamplesIntensityThreshold(threshold);
             }*/

            //std::cout <<	metricMMI->GetFixedImageSamplesIntensityThreshold() << std::endl;

            try {
                metricMMI->Initialize();
            }
            catch (itk::ExceptionObject &excep) {
                std::cerr << "Exception catched !" << std::endl;
                std::cerr << excep << std::endl;
                erMMI = true;
            }
            if (!erMMI) {
                valueMMI = metricMMI->GetValue(displacement);
            }

            //std::cout << im1 << " vs " << im2 << " \n";
            //std::cout << "MMI = " << valueMMI << std::endl;
            // std::cout << "mesurar distancies entre imatge, cambio el signe de MMI"<< std::endl;
            ret = -valueMMI;
            break;
        default:
            cout << "opencvITKmetric::computeSimilarity wrong type of metric " << endl;
            exit(-1);
    }

    return ret;
}
