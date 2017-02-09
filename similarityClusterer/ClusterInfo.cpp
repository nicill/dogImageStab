//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_CLUSTERINFO_CPP
#define DOGIMAGESTABILIZATION_CLUSTERINFO_CPP

#include <string>
#include <assert.h>

struct ClusterInfo {
    std::string name = "INVALID";
    double beginMsec = -1;
    double endMsec = -1;
    double length = -1;

    ClusterInfo() {};
    ClusterInfo(std::string _name, double _beginMsec, double _endMsec) {
        this->name = _name;
        this->beginMsec = _beginMsec;
        this->endMsec = _endMsec;
        this->length = _endMsec - _beginMsec;
        assert(this->length >= 0);
    }
};

#endif // DOGIMAGESTABILIZATION_CLUSTERINFO_CPP
