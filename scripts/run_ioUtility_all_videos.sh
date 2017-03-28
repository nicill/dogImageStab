#!/bin/bash

programdir="/home/tokuyama/dog/dogImageStab/cmake-build-debug/ioUtility"
videodir="/home/tokuyama/dog/videosToStabilise"

if [[ $1 == "" ]];
then
	echo "Please provide a flag"
	exit 1
fi

for file in "20140126.mp4" "20140720middledog.mp4" "20140720smalldog.mp4" "20150801.mp4" "20150802.mp4" "20150927.mp4" "20151114.mp4"
do
	for metric in "1 0" "1 1" "1 2" "1 3" "2 2" "2 1" "2 0" "3 0" "3 1"
	do
		echo "Running ioUtility with "$file
		$programdir $videodir"/"$file $metric $1
	done
done

