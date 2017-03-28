#!/bin/bash

programdir="/home/tokuyama/dog/dogImageStab/cmake-build-release/similarityClusterer"
videodir="/home/tokuyama/dog/videosToStabilise"

for file in "20140126.mp4" "20140720middledog.mp4" "20140720smalldog.mp4" "20150801.mp4" "20150802.mp4" "20150927.mp4" "20151114.mp4"
do
	for metric in "1 0" "1 1" "1 2" "1 3" "2 2" "2 1" "2 0" "3 0" "3 1"
	do
		echo "Analysing "$file" with metric "$metric
		$programdir $videodir"/"$file $metric "-tfd"
	done
done

