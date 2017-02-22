//
// Created by tokuyama on 17/02/22.
//

#include "classifier.h"

vector<FrameInfo> classifier::classifyFrames(vector<FrameInfo> frames) {
    for (int i = 0; i < frames.size(); i++) {
        if (frames[i].similarityToPrevious > 0.6) frames[i].label = highSimLabel;
        else if (frames[i].similarityToPrevious > 0.3) frames[i].label = avgSimLabel;
        else frames[i].label = lowSimLabel;
    }

    return frames;
}

classifier::classification classifier::classifySimilarity(double value) {
    if (value > 0.6) return HIGH;
}
