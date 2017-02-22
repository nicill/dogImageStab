cmake_minimum_required(VERSION 3.3)
project(dogImageStabilization)
find_package( OpenCV REQUIRED )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g ")

#set(SOURCE_FILES main.cpp videostabExample.cpp)
add_executable(computeMeasures computeMeasures.cpp videoStabilizationQualityMeasures.h videoStabilizationQualityMeasures.cpp similarityClusterer/framewiseSimilarityMetric.h similarityClusterer/opencvHistComparer.cpp similarityClusterer/opencvHistComparer.h similarityClusterer/featureComparer.cpp similarityClusterer/featureComparer.h )
target_link_libraries( computeMeasures ${OpenCV_LIBS} )

add_executable(dogImageStabilization simpleExample.cpp )
target_link_libraries( dogImageStabilization ${OpenCV_LIBS} )

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

add_executable(similarityClusterer similarityClusterer/similarityClusterer.cpp similarityClusterer/framewiseSimilarityMetric.h similarityClusterer/opencvHistComparer.cpp similarityClusterer/opencvHistComparer.h similarityClusterer/opencvImageMetric.cpp similarityClusterer/opencvImageMetric.h similarityClusterer/featureComparer.cpp similarityClusterer/featureComparer.h  similarityClusterer/qualityMeasurer.cpp similarityClusterer/qualityMeasurer.h similarityClusterer/FrameInfo.cpp similarityClusterer/ClusterInfo.cpp similarityClusterer/ClusterInfoContainer.cpp similarityClusterer/defaults.h similarityClusterer/defaults.h similarityClusterer/clusterer.cpp similarityClusterer/clusterer.h similarityClusterer/utils.cpp similarityClusterer/classifier.cpp similarityClusterer/classifier.h)
target_link_libraries( similarityClusterer ${OpenCV_LIBS} )

add_executable(similarityTEST similarityTEST.cpp similarityTEST.h)
target_link_libraries( similarityTEST ${OpenCV_LIBS} )

