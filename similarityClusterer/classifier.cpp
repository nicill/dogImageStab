//
// Created by tokuyama on 17/02/22.
//

#include "classifier.h"

vector<FrameInfo> classifier::classifyFrames(vector<FrameInfo> frames) {
    for (int i = 0; i < frames.size(); i++) {
        switch (classifySimilarity(frames[i].similarityToPrevious)) {
            case HIGH:
                frames[i].label = highSimLabel;
                break;
            case MEDIUM:
                frames[i].label = mediumSimLabel;
                break;
            case LOW:
                frames[i].label = lowSimLabel;
                break;
            default:
                throw("Classification not implemented!");
        }
    }

    return frames;
}

classification classifier::classifySimilarity(double value) {
    if (value > 0.6) return HIGH;
    else if (value > 0.3) return MEDIUM;
    else return LOW;
}
