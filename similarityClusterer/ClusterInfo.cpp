//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_CLUSTERINFO_CPP
#define DOGIMAGESTABILIZATION_CLUSTERINFO_CPP

#include <string>
#include "FrameInfo.cpp"

struct ClusterInfo {
    std::string name = "INVALID";
    double beginMsec = -1;
    double endMsec = -1;
    double length = -1;

    bool hasFrames = false;
    std::vector<FrameInfo> frames;
    double beginFrameNo = -1;
    double endFrameNo = -1;
    double averageSimilarity = -1;

    ClusterInfo() {};
    ClusterInfo(std::string _name,
                double _beginMsec,
                double _endMsec) {
            this->name = _name;
            this->beginMsec = _beginMsec;
            this->endMsec = _endMsec;
            this->length = this->endMsec - this->beginMsec;
            assert(this->length >= 0);
    }
    ClusterInfo(std::vector<FrameInfo> _frames)
            : ClusterInfo(defaultName(_frames), _frames) {}
    ClusterInfo(std::string _name,
                std::vector<FrameInfo> _frames) {
        this->name = _name;
        this->beginMsec = _frames.front().msec;
        this->endMsec = _frames.back().msec;

        this->hasFrames = true;
        this->frames = _frames;
        this->beginFrameNo = _frames.front().frameNo;
        this->endFrameNo = _frames.back().frameNo;
        assert(this->endFrameNo - this->beginFrameNo + 1 == this->frames.size());

        updateValues();
        assert(this->length >= 0);
    }

    /**
     * Add a frame to the back of this cluster. Only possible if the cluster contains frames.
     * @param frame The frame to add.
     */
    void addFrameAtBack(FrameInfo frame) {
        assert(this->hasFrames
               && frame.msec > this->beginMsec
               && frame.frameNo == this->endFrameNo + 1);

        this->frames.push_back(frame);
        this->endMsec = frame.msec;
        this->endFrameNo = frame.frameNo;
        updateValues();
    }

    bool equals(ClusterInfo compare) {
        bool equal (this->hasFrames == compare.hasFrames
                    && this->beginMsec == compare.beginMsec
                    && this->endMsec == compare.endMsec);

        if (this->hasFrames) {
            equal &= (this->beginFrameNo == compare.beginFrameNo
                      && this->endFrameNo == compare.endFrameNo);
        }

        return equal;
    }

    /**
     * Updates length and average similarity fields of this object.
     */
    void updateValues() {
        this->length = this->endMsec - this->beginMsec;
        // Average similarity
        double summedUpAverages = 0;
        for (FrameInfo frameInfo : this->frames) {
            summedUpAverages += frameInfo.averageSimilarity;
        }
        this->averageSimilarity = summedUpAverages / this->frames.size();
    }

    /**
     * Creates a default name for the given cluster.
     */
    std::string defaultName(std::vector<FrameInfo> frames) {
        return "Cluster from " + std::to_string(frames.front().msec) + " msec"
               + " to " + std::to_string(frames.back().msec) + " msec";
    }
};

#endif // DOGIMAGESTABILIZATION_CLUSTERINFO_CPP
