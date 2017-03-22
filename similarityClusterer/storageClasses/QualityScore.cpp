//
// Created by tokuyama on 17/03/22.
//

#ifndef DOGIMAGESTABILIZATION_QUALITYSCORE_CPP
#define DOGIMAGESTABILIZATION_QUALITYSCORE_CPP

#include <string>

using std::string;

struct QualityScore {
    string tagFileName = "INVALID";
    double qualityScore = -1;
    double precision = -1;
    double recall = -1;

    QualityScore() {}
    QualityScore(string _tagFileName, double _qualityScore, double _precision, double _recall) {
        this->tagFileName = _tagFileName;
        this->qualityScore = _qualityScore;
        this->precision = _precision;
        this->recall = _recall;
    }
};

#endif // DOGIMAGESTABILIZATION_QUALITYSCORE_CPP
