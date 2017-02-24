//
// Created by yago on 17/02/22.
//

#include "opencvITKmetric.h"

void opencvITKmetric::activateVerbosity() {
    return;
}

opencvITKmetric::opencvITKmetric(int comparisonType) : comparisonType(comparisonType) {

}



double opencvITKmetric::computeSimilarity(Mat* im1, Mat* im2) {

    const unsigned int Dimension = 2;
    typedef unsigned char                            InputPixelType;
    typedef float                                    RealPixelType;
    typedef unsigned char                            OutputPixelType;
    typedef itk::Image< InputPixelType,  Dimension > InputImageType;
    typedef itk::Image< RealPixelType,   Dimension > RealImageType;
    typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
    typedef itk::OpenCVImageBridge                   BridgeType;
    typedef itk::CastImageFilter< InputImageType, RealImageType >
            CastFilterType;
    typedef itk::CannyEdgeDetectionImageFilter< RealImageType, RealImageType >
            FilterType;
    typedef itk::RescaleIntensityImageFilter< RealImageType, OutputImageType >
            RescaleFilterType;

    InputImageType::Pointer itkFrame1 = itk::OpenCVImageBridge::CVMatToITKImage< InputImageType >( *im1 );
    InputImageType::Pointer itkFrame2 = itk::OpenCVImageBridge::CVMatToITKImage< InputImageType >( *im2 );

    double ret;
    int numberOfBins=256;
    float threshold = 1;

    bool erMMI =false, erRMS=false,erNHMMI=false;

    typedef itk::MeanSquaresImageToImageMetric<InputImageType, InputImageType> MSMetricType;
    typedef itk::LinearInterpolateImageFunction<InputImageType,double > linInterpolatorType;
    typedef itk::AffineTransform< double, 2 > AffineTransformType;
    typedef AffineTransformType::ParametersType parametersType;
    typedef itk::MinimumMaximumImageCalculator< InputImageType > MinimumMaximumImageCalculatorType;

    MSMetricType::Pointer metricMS = MSMetricType::New();
    linInterpolatorType::Pointer interpolator = linInterpolatorType::New();
    AffineTransformType::Pointer transform = AffineTransformType::New();

    // Set identity transform
    transform->SetIdentity();
    parametersType displacement( transform->GetNumberOfParameters() );
    displacement.Fill(  0.0  );

    // minmax thingie calculator
    MinimumMaximumImageCalculatorType::Pointer minmax;
    minmax = MinimumMaximumImageCalculatorType::New();
    minmax->SetImage(itkFrame1);
    minmax->Compute();

    // Finish setting up metric
    metricMS->SetInterpolator( interpolator );
    metricMS->SetTransform( transform );

    metricMS->SetFixedImage(  itkFrame1  );
    metricMS->SetMovingImage( itkFrame2 );
    metricMS->SetFixedImageRegion( itkFrame1->GetBufferedRegion()  );


    metricMS->SetFixedImageSamplesIntensityThreshold(minmax->GetMaximum()*0.05);

    /*if(useAvancedMethods)
    {
        metricMS->SetUseFixedImageSamplesIntensityThreshold(true);
        metricMS->SetFixedImageSamplesIntensityThreshold(threshold);
    }*/

    try
    {
        metricMS->Initialize();
    }
    catch( itk::ExceptionObject & excep )
    {
        std::cerr << "Exception catched !" << std::endl;
        std::cerr << excep << std::endl;
        cout<<"opencvITKmetric::computeSimilarity error computing metric"<<endl;
        throw "opencvITKmetric::computeSimilarity error computing metric";

    }



    ret = metricMS->GetValue( displacement );

    return ret;


  /*FixedImageType::RegionType fixedRegion = fixedImage->GetBufferedRegion();
  MovingImageType::RegionType movingRegion = fixedImage->GetBufferedRegion();

  int numberOfPixelsFixed = fixedRegion.GetNumberOfPixels();
  int numberOfPixelsMoving = movingRegion.GetNumberOfPixels();
  int numberOfPixels;

  if(numberOfPixelsFixed>=numberOfPixelsMoving){
	  numberOfPixels = numberOfPixelsMoving;
  }
  else{
	  numberOfPixels = numberOfPixelsFixed;
  }

  const unsigned int numberOfSamples = static_cast<unsigned int>( numberOfPixels * 100.0 / 100.0 );

    double valueMS;
    double valueMMI;
    double valueNHMMI;

    MetricType0::Pointer metricMS = MetricType0::New();
    MetricType1::Pointer metricMMI = MetricType1::New();
    MetricType2::Pointer metricNHMMI = MetricType2::New();

  //intensity thresholds
  typedef itk::ImageFileReader< ProbabilityImageType > ProbabilityReaderType;


  ProbabilityReaderType::Pointer freader = ProbabilityReaderType::New();
  ProbabilityImageType::Pointer fimage = NULL;

    if (im1.compare("")!=0) {
        freader->SetFileName( im1 );
        // Reading the image.
        try {
            freader->Update();
            fimage = freader->GetOutput();
        }
        catch (itk::ExceptionObject & e) {
            std::cerr << "exception in file reader " << std::endl;
            std::cerr << e.GetDescription() << std::endl;
            std::cerr << e.GetLocation() << std::endl;
        }
    }

  MinimumMaximumImageCalculatorType::Pointer minmax;
  minmax = MinimumMaximumImageCalculatorType::New();
  minmax->SetImage(fimage);
  minmax->Compute();

 switch(metricUsed)
  {
	case 0:
	  // MEAN SQUARE METRIC


	  metricMS->SetInterpolator( interpolator );
	  metricMS->SetTransform( transform );

	  transform->SetIdentity();

	  metricMS->SetFixedImage(  fixedImage  );
	  metricMS->SetMovingImage( movingImage );

	  metricMS->SetFixedImageRegion(  fixedImage->GetBufferedRegion()  );

	  metricMS->SetFixedImageSamplesIntensityThreshold(minmax->GetMaximum()*0.05);

	  if(useAvancedMethods)
	  {
		metricMS->SetUseFixedImageSamplesIntensityThreshold(true);
		metricMS->SetFixedImageSamplesIntensityThreshold(threshold);
	  }

	  try
	    {
	    metricMS->Initialize();
	    }
	  catch( itk::ExceptionObject & excep )
	    {
	    std::cerr << "Exception catched !" << std::endl;
	    std::cerr << excep << std::endl;
	    erRMS=true;
	    }

		if (!erRMS){
			valueMS = metricMS->GetValue( displacement );
		}
		else{
			valueMS = 7777;
		}

	  //std::cout << im1 << " vs " << im2 << " \n";
	  //std::cout << "MS = " << valueMS << std::endl;
	  ret=valueMS;
   	  break;


	case 1:

	  // MATES MUTUAL INFORMATION METRIC

	  metricMMI->SetInterpolator( interpolator );
	  metricMMI->SetTransform( transform );

	  transform->SetIdentity();

	  metricMMI->SetFixedImage(  fixedImage  );
	  metricMMI->SetMovingImage( movingImage );

	  metricMMI->SetFixedImageRegion(  fixedImage->GetBufferedRegion()  );

	  metricMMI->SetNumberOfSpatialSamples(numberOfSamples);
	  metricMMI->SetNumberOfHistogramBins( numberOfBins );

	  metricMMI->SetFixedImageSamplesIntensityThreshold(minmax->GetMaximum()*0.05);

	  if(useAvancedMethods)
	  {
		metricMMI->SetUseFixedImageSamplesIntensityThreshold(true);
		metricMMI->SetFixedImageSamplesIntensityThreshold(threshold);
	  }

		//std::cout <<	metricMMI->GetFixedImageSamplesIntensityThreshold() << std::endl;

	  try
	    {
	    metricMMI->Initialize();
	    }
	  catch( itk::ExceptionObject & excep )
	    {
	    std::cerr << "Exception catched !" << std::endl;
	    std::cerr << excep << std::endl;
	    erMMI=true;
	    }
		if (!erMMI){
			valueMMI = metricMMI->GetValue( displacement );
		}
		else{
			valueMMI = 7777;
		}

	  //std::cout << im1 << " vs " << im2 << " \n";
	  //std::cout << "MMI = " << valueMMI << std::endl;
	 // std::cout << "mesurar distancies entre imatge, cambio el signe de MMI"<< std::endl;
	  ret=-valueMMI;
	break;
*/




    return 0;

}




