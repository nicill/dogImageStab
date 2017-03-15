//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_CLUSTERINFO_CPP
#define DOGIMAGESTABILIZATION_CLUSTERINFO_CPP

#include <string>
#include "../utils.cpp"
#include "FrameInfo.cpp"

using std::string;
using std::vector;

struct ClusterInfo {
    string label = "INVALID";
    double beginMsec = -1;
    double endMsec = -1;
    double length = -1;

    bool hasFrames = false;
    vector<FrameInfo> frames;
    double beginFrameNo = -1;
    double endFrameNo = -1;
    double averageSimilarity = -1;

    /**
     * Empty constructor.
     */
    ClusterInfo() {};

    /**
     * Constructor with only name, begin and end time.
     */
    ClusterInfo(string _name,
                double _beginMsec,
                double _endMsec) {
            this->label = _name;
            this->beginMsec = _beginMsec;
            this->endMsec = _endMsec;
            this->length = this->endMsec - this->beginMsec;
            assert(this->length >= 0);
    }

    /**
     * Nested constructor with only one frame.
     */
    ClusterInfo(FrameInfo _frame)
            : ClusterInfo(utils::package(_frame)) {}

    /**
     * Nested constructor with only a frame vector.
     */
    ClusterInfo(vector<FrameInfo> _frames)
            : ClusterInfo(defaultName(_frames), _frames) {}

    /**
     * Nested constructor with name and one frame.
     */
    ClusterInfo(string _name, FrameInfo _frame)
            : ClusterInfo(_name, utils::package(_frame)) {}

    /**
     * Constructor.
     */
    ClusterInfo(string _name, vector<FrameInfo> _frames) {
        this->label = _name;
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

    /**
     * Checks if the given frame is within the cluster boundaries.
     */
    bool containsFrame(FrameInfo frame) {
        return frame.msec >= this->beginMsec
               && frame.msec <= this->endMsec
               && (!this->hasFrames || (frame.frameNo >= this->beginMsec
                                        && frame.frameNo <= this->endFrameNo));
    }

    /**
     * Equals method which compares begin and end time as well as begin and end frame number.
     * @param compare The ClusterInfo to compare this to.
     */
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

private:
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
    string defaultName(vector<FrameInfo> frames) {
        return "Cluster from " + std::to_string(frames.front().msec) + " msec"
               + " to " + std::to_string(frames.back().msec) + " msec";
    }
};

#endif // DOGIMAGESTABILIZATION_CLUSTERINFO_CPP
