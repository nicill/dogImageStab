//
// Created by tokuyama on 17/03/15.
//

#ifndef DOGIMAGESTABILIZATION_TAGFILEUTILS_CPP
#define DOGIMAGESTABILIZATION_TAGFILEUTILS_CPP

#include <string>
#include <vector>
#include <dirent.h>
#include "storageClasses/ClusterInfoContainer.cpp"

using std::string;
using std::vector;
using std::stod;
using std::ifstream;
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::setprecision;
using std::ios;

struct similarityFileUtils {
    /**
     * Reads in the given tag files and creates ClusterInfoContainers to hold the info during run time.
     * @param pathToTagFileDirectory A directory containing tag files. Must be a valid directory.
     * @param verbose Activate verbosity to cout.
     * @return A vector of ClusterInfoContainer objects, one for each file in the directory.
     */
    static vector<ClusterInfoContainer> readTagFiles(string pathToTagFileDirectory, bool verbose) {
        vector<ClusterInfoContainer> tagFilesClusterings;

        DIR *tagFileDir = opendir(pathToTagFileDirectory.c_str());
        struct dirent *fileEntity;
        while ((fileEntity = readdir(tagFileDir)) != nullptr) {
            // Necessary for comparison with string literals.
            string fileName(fileEntity->d_name);
            if (fileName == "." || fileName == "..") {
                if (verbose) cout << "Skipping \"" << fileName << "\"." << endl;
                continue;
            }
            if (verbose) cout << "Reading file \"" << fileName << "\"." << endl;

            ClusterInfoContainer clustersFromFile;
            if (!readTagFile(pathToTagFileDirectory + "/" + fileName, &clustersFromFile)) {
                cerr << "Skipping file..." << endl;
                continue;
            }

            tagFilesClusterings.push_back(clustersFromFile);
        }

        if (tagFilesClusterings.size() == 0) {
            cerr << "Couldn't open any file." << endl;
        }

        closedir(tagFileDir);
        return tagFilesClusterings;
    }

    /**
     * Tries to read the file at the given path as a tag file and interpret its clusterings.
     * @param pathToTagFile Path to the tag file to read.
     * @param clusters The ClusterInfoContainer to fill.
     * @return False if unsuccessful. Will output error message to cerr.
     */
    static bool readTagFile(string pathToTagFile, ClusterInfoContainer* clusters) {
        string fileName = utils::getBasename(pathToTagFile);
        *clusters = ClusterInfoContainer(fileName);

        ifstream tagFile;
        tagFile.open(pathToTagFile);
        if (!tagFile.is_open()) {
            cerr << "Couldn't open file \"" << pathToTagFile << "\" as tag file." << endl;
            return false;
        }

        string parseErrorMsg = "The tag file \"" + pathToTagFile + "\" seems to be in a wrong format. Please verify its contents.";

        string line;
        getLineSafe(tagFile, line);
        if (line != "start,stop") {
            cerr << parseErrorMsg << endl;
            return false;
        }

        vector<string> split;
        while (readLine(tagFile, &split)) {
            // Split should contain: Start (msec), end (msec)
            if(split.size() != 3) {
                cerr << parseErrorMsg << endl;
                return false;
            }

            double beginMsec = 1000 * stod(split[0].c_str());
            double endMsec = 1000 * stod(split[1].c_str());

            (*clusters).add(ClusterInfo(fileName, beginMsec, endMsec));
        }

        tagFile.close();

        if ((*clusters).size() == 0) {
            cerr << "Didn't find any cluster entries in tag file." << endl;
            return false;
        }
        return true;
    }

    /**
     * Reads in the given csv file to retrieve the saved frame infos and verifies that the number of entries corresponds
     * to the given frame number.
     * @param filePath Path to the CSV file.
     * @param supposedNumberOfFrames The number of frames in the video.
     * @param frames Empty vector to save the results to.
     * @return False if unsuccessful. Will output error message to cerr.
     */
    static bool readFrameInfosFromCsv(string filePath, double supposedNumberOfFrames, vector<FrameInfo>* frames) {
        ifstream csvStream;
        csvStream.open(filePath);
        assert(csvStream.is_open());

        *frames = vector<FrameInfo>();
        // Read in header line.
        string s;
        if (!getLineSafe(csvStream, s)) return false;
        vector<string> split;
        while (readLine(csvStream, &split)) {
            if(split.size() != 3) {
                cerr << "Invalid file format in file \"" << filePath << "\". Please verify contents." << endl;
                return false;
            }

            double frameNo = stod(split[0]);
            double msec = stod(split[1]);
            double similarity = stod(split[2]);

            (*frames).push_back(FrameInfo(Mat(), frameNo, msec, similarity));
        }

        csvStream.close();

        if ((*frames).size() != supposedNumberOfFrames) {
            cerr << "Tag file \"" << filePath << "\" has the wrong number of entries." << endl
                 << "Expected: " << supposedNumberOfFrames << ", actual: " << (*frames).size() << endl
                 << "Please verify the file integrity and recalculate, if necessary." << endl;
            return false;
        }
        return true;
    }

    /**
     * Reads the next line of the given CSV file.
     * @param fileStream The CSV file's file stream.
     * @param split The vector to save the split line into.
     * @param separator (optional) Separator character.
     * @return True if a line was read, false for EOF.
     */
    static bool readLine(ifstream& fileStream, vector<string>* split, char separator = ',') {
        *split = vector<string>();
        string line;
        if (getLineSafe(fileStream, line)) {
            size_t nextSepPos;
            while (utils::findToken(line, separator, &nextSepPos)) {
                split->push_back(line.substr(0, nextSepPos));
                size_t nextStart = nextSepPos + 1;
                line = line.substr(nextStart, line.size() - nextStart);
            }

            // Add last value (after the last separator)
            split->push_back(line);
            return true;
        } else {
            return false;
        }
    }

    /**
     * Appends the given values to the given csv file (must be writable) with highest precision.
     * @param filePath File path of the csv file.
     */
    static void appendToCsv(string filePath, double frameNo, double msec, double similarity) {
        vector<double> elements = { frameNo, msec, similarity };
        appendToCsv(filePath, elements);
    }

    /**
     * Appends the given values to the given csv file (must be writable) with highest precision.
     * @param filePath File path of the csv file.
     */
    static void appendToCsv(string filePath, double frameNo, double msec, double similarity, double averageSimilarity,
                            string classification, bool stop, bool bark) {
        vector<double> elements = { frameNo, msec, similarity, averageSimilarity,
                                    stod(classification), (double)stop, (double)bark };
        appendToCsv(filePath, elements);
    }

private:
    static void appendToCsv(string filePath, vector<double> elements) {
        char sep = ',';
        ofstream ioFileStream;
        // Setting the precision is not strictly necessary, but useful to preserve exact similarity values.
        ioFileStream << setprecision(100);
        ioFileStream.open(filePath, ios::app); // append


        for (int i = 0; i < elements.size(); i++) {
            ioFileStream << elements[i];

            if (i != elements.size() - 1) {
                ioFileStream << sep;
            }
        }

        ioFileStream << endl;
        ioFileStream.close();
    }

    /**
     * Replaces getLine. Works independently of used EOL character sequence.
     * @param fileStream The stream to read from.
     * @param line The string to save to.
     * @return False for EOF.
     */
    static bool getLineSafe(ifstream& fileStream, string& line) {
        for(;;) {
            int c = fileStream.get();
            switch (c) {
                case '\n':
                    return true;
                case '\r':
                    if (fileStream.peek() == '\n') {
                        fileStream.get();
                    }
                    return true;
                case EOF:
                    if (line.empty()) {
                        fileStream.setstate(std::ios::eofbit);
                    }
                    return false;
                default:
                    line += (char)c;
            }
        }
    }
};

#endif // DOGIMAGESTABILIZATION_TAGFILEUTILS_CPP
