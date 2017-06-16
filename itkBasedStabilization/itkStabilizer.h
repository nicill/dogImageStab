//
// Created by yago on 17/03/27.
//

#ifndef DOGIMAGESTABILIZATION_ITKSTABILIZER_H
#define DOGIMAGESTABILIZATION_ITKSTABILIZER_H

#include <iostream>
#include <stdio.h>

#include <opencv2/opencv.hpp>
//#include <opencv2/xfeatures2d/nonfree.hpp>
#include "opencv2/core/core.hpp"
//#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/calib3d/calib3d.hpp"

// ITK includes
#include <itkImage.h>
#include <itkCastImageFilter.h>
//#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkOpenCVImageBridge.h>
#include "itkMeanSquaresImageToImageMetric.h"
//#include <itkMinimumMaximumImageCalculator.h>
#include "itkMattesMutualInformationImageToImageMetric.h"
#include <itkNormalizedCorrelationImageToImageMetric.h>
//#include "itkMutualInformationImageToImageMetric.h"

#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkImageRegistrationMethod.h>
#include <itkResampleImageFilter.h>
#include <itkMeanSquaresImageToImageMetricv4.h>
//#include <itkMattesMutualInformationImageToImageMetricv4.h>
#include <itkCorrelationImageToImageMetricv4.h>

#include <itkRegularStepGradientDescentOptimizerv4.h>
#include <itkConjugateGradientLineSearchOptimizerv4.h>

#include <itkImageRegistrationMethodv4.h>
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>



using namespace std;
using namespace cv;

class itkStabilizer {
public:

    enum type { AFFINESSD = 0, AFFINEMI = 1, AFFINENCC=2, AFFINESSDMR=3,AFFINEMIMR=4,AFFINENCCMR=5}; // Think about other possibilities!
    itkStabilizer(type givenType = AFFINESSD);
    type stabType;

    type getStabType() const;

    void itkStabilize(VideoCapture capture, VideoWriter outputVideo )
    {

        if( stabType==0 || stabType==1 || stabType==2)  //Single resolution
        {
            singleResolutionITKStab(capture,  outputVideo );
        }
       else if (stabType==3|| stabType==4 || stabType==5)  //Multiresolution
        {
               // implement MR function
            multiResolutionITKStab(capture,outputVideo);
        }
        else {
            cout<<"itk stabilizer main, unknown type "<<endl;
            throw ("itk stabilizer main, unknown type ");
        }
    }

private:

    void singleResolutionITKStab(VideoCapture capture, VideoWriter outputVideo );
    void multiResolutionITKStab(VideoCapture capture, VideoWriter outputVideo );

};


// FER UN ESTABILITZADOR AFI AMB LA METRICA QUE HI HAGI A LEXEMPLE, EL RESULTAT DEL DEMONS POT FER MOLT DE RIURE
// POTSER EL MODUL DEL CAMP DE DEFORMACIO DEL DEMONS POT SERVIR PER DETECTAR MOVIMENT???
// FER MAIL A SERGI I A ELOI!


#endif //DOGIMAGESTABILIZATION_ITKSTABILIZER_H
