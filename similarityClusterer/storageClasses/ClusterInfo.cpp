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
     * Consructor with only label.
     */
    ClusterInfo(string _label) {
        this->label = _label;
    }

    /**
     * Constructor with only label, begin and end time.
     */
    ClusterInfo(string _label,
                double _beginMsec,
                double _endMsec) {
            this->label = _label;
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
            : ClusterInfo(defaultLabel(_frames), _frames) {}

    /**
     * Nested constructor with label and one frame.
     */
    ClusterInfo(string _label, FrameInfo _frame)
            : ClusterInfo(_label, utils::package(_frame)) {}

    /**
     * Constructor.
     */
    ClusterInfo(string _label, vector<FrameInfo> _frames) {
        this->label = _label;
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
     * Creates a ClusterInfo with the overlap with the given cluster.
     * @param other The ClusterInfo to compare this to.
     * @return A ClusterInfo with begin and end (msec) of the overlap, an empty ClusterInfo if there is no overlap.
     */
    ClusterInfo getOverlap(ClusterInfo other) {
        // Logic: If an overlap exists (neither is the begin after the end nor the end before the begin)
        //        it is between the bigger begin and the smaller end value.
        double biggerBegin =
                ( this->beginMsec > other.beginMsec)
                ? this->beginMsec
                : other.beginMsec;
        double smallerEnd =
                ( this->endMsec < other.endMsec)
                ? this->endMsec
                : other.endMsec;

        if (biggerBegin > smallerEnd) {
            return ClusterInfo("No overlap");
        } else {
            return ClusterInfo("Overlap of " + this->label + " and " + other.label, biggerBegin, smallerEnd);
        }
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

    /**
     * Sorts by begin time and if those are equal, by end time.
     */
    bool operator < (const ClusterInfo& second) const {
        if (this->beginMsec < second.beginMsec) {
            return true;
        } else if (this->beginMsec == second.beginMsec) {
            return this->endMsec < second.endMsec;
        } else {
            return false;
        }
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
     * Creates a default label for the given cluster.
     */
    string defaultLabel(vector<FrameInfo> frames) {
        return "Cluster from " + std::to_string(frames.front().msec) + " msec"
               + " to " + std::to_string(frames.back().msec) + " msec";
    }
};

#endif // DOGIMAGESTABILIZATION_CLUSTERINFO_CPP
