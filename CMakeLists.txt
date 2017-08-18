cmake_minimum_required(VERSION 3.3)
SET(ITK_DIR /home/yago/soft/InsightToolkit-4.11.0/BINARY)
project(dogImageStabilization)
find_package( OpenCV REQUIRED )
find_package( ITK REQUIRED )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g ")

include(${ITK_USE_FILE})


#set(SOURCE_FILES main.cpp videostabExample.cpp)
add_executable(computeMeasures computeMeasures.cpp videoStabilizationQualityMeasures.h videoStabilizationQualityMeasures.cpp similarityClusterer/opencvITKmetric.cpp similarityClusterer/opencvITKmetric.h similarityClusterer/framewiseSimilarityMetric.h similarityClusterer/opencvHistComparer.cpp similarityClusterer/opencvHistComparer.h similarityClusterer/featureComparer.cpp similarityClusterer/featureComparer.h )
target_link_libraries( computeMeasures ${OpenCV_LIBS} ${ITK_LIBRARIES} )

add_executable(dogImageStabilization simpleExample.cpp )
target_link_libraries( dogImageStabilization ${OpenCV_LIBS} )

add_executable(farnebackStabilization simpleFarneback.cpp )
target_link_libraries( farnebackStabilization ${OpenCV_LIBS} )


add_executable(testVideoGenerator testVideoGenerator/testVideoGenerator.cpp )
target_link_libraries( testVideoGenerator ${OpenCV_LIBS} )


add_executable(videoStabExample videostabExample.cpp)
target_link_libraries( videoStabExample ${OpenCV_LIBS} )

add_executable(featureBasedStabilizer featureBasedStabilizer/featureBasedStabilizer.cpp featureBasedStabilizer/featureStabilizer.cpp featureBasedStabilizer/featureStabilizer.h)
target_link_libraries( featureBasedStabilizer ${OpenCV_LIBS} )

add_executable(showTwoTogether showTwoTogether.cpp)
target_link_libraries( showTwoTogether ${OpenCV_LIBS} )

#add_executable(frameWiseEntropy frameWiseEntropy.cpp)
#target_link_libraries( frameWiseEntropy ${OpenCV_LIBS} )

#add_executable(twoImageMeasures twoImageMeasures.cpp)
#target_link_libraries( twoImageMeasures ${OpenCV_LIBS} )

add_executable(brno brno.cpp Stabilizer.h StabilizerGCBPM.cpp StabilizerGCBPM.h Exceptions.h Parameters.cpp Parameters.h xgetopt.cpp xgetopt.h StabilizerSVD.cpp StabilizerSVD.h)
target_link_libraries( brno ${OpenCV_LIBS} )

add_executable(logPolarStab FFTTStabilization/logPolarStabilizer.cpp)
target_link_libraries( logPolarStab ${OpenCV_LIBS} )

add_executable(segmentByTags ioUtility/ioSegmentVideoByTagFile.cpp similarityClusterer/similarityFileUtils.cpp)
target_link_libraries(segmentByTags  ${OpenCV_LIBS} )

add_executable(similarityClusterer
        similarityClusterer/similarityClusterer.cpp
        similarityClusterer/framewiseSimilarityMetric.h
        similarityClusterer/opencvHistComparer.cpp similarityClusterer/opencvHistComparer.h
        similarityClusterer/opencvImageMetric.cpp similarityClusterer/opencvImageMetric.h
        similarityClusterer/featureComparer.cpp similarityClusterer/featureComparer.h
        similarityClusterer/qualityMeasurer.cpp similarityClusterer/qualityMeasurer.h
        similarityClusterer/clusterer.cpp similarityClusterer/clusterer.h
        similarityClusterer/classifier.cpp similarityClusterer/classifier.h
        similarityClusterer/storageClasses/FrameInfo.cpp
        similarityClusterer/storageClasses/ClusterInfo.cpp
        similarityClusterer/storageClasses/ClusterInfoContainer.cpp
        similarityClusterer/storageClasses/QualityScore.cpp
        similarityClusterer/defaults.h
        similarityClusterer/similarityFileUtils.cpp
        similarityClusterer/utils.cpp
        similarityClusterer/opencvITKmetric.cpp similarityClusterer/opencvITKmetric.h)
target_link_libraries( similarityClusterer ${OpenCV_LIBS} ${ITK_LIBRARIES} )

add_executable(ioUtility
        ioUtility/ioUtility.cpp
        similarityClusterer/clusterer.cpp similarityClusterer/clusterer.h
        similarityClusterer/classifier.cpp similarityClusterer/classifier.h
        similarityClusterer/storageClasses/FrameInfo.cpp
        similarityClusterer/storageClasses/ClusterInfo.cpp
        similarityClusterer/storageClasses/ClusterInfoContainer.cpp
        similarityClusterer/defaults.h
        similarityClusterer/similarityFileUtils.cpp)
target_link_libraries( ioUtility ${OpenCV_LIBS} )

add_executable(similarityTEST similarityTEST.cpp similarityTEST.h)
target_link_libraries( similarityTEST ${OpenCV_LIBS} )

add_executable(itkStabilizer itkBasedStabilization/itkStabilizer.cpp itkBasedStabilization/itkStabilizer.h)
target_link_libraries( itkStabilizer ${OpenCV_LIBS} ${ITK_LIBRARIES})