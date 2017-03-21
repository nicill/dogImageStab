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

            ClusterInfoContainer clustersFromFile = readTagFile(pathToTagFileDirectory + "/" + fileName);

            if (clustersFromFile.clusterInfos.size() == 0) {
                cerr << "Couldn't open file " << fileName << " as tag file. Skipping..." << endl;
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
     * @param pathToTagFile The path, including the file name.
     * @return A vector of clusterings if successful, otherwise an empty vector.
     */
    static ClusterInfoContainer readTagFile(string pathToTagFile) {
        ifstream tagFile;
        tagFile.open(pathToTagFile);
        if (!tagFile.is_open()) return ClusterInfoContainer();

        string fileName = utils::getBasename(pathToTagFile);
        ClusterInfoContainer clustersFromFile = ClusterInfoContainer(fileName);

        string line;
        getLineSafe(tagFile, line);
        if (line != "start,stop") {
            cerr << "The CSV file \"" << pathToTagFile << "\" seems to be in a wrong format." << endl;
            throw("Wrong CSV file format");
        }

        vector<string> split;
        while (readLine(tagFile, &split)) {
            // Split should contain: Start (msec), end (msec)
            assert(split.size() == 2);
            double beginMsec = stod(split[0].c_str());
            double endMsec = stod(split[1].c_str());

            clustersFromFile.clusterInfos.push_back(ClusterInfo(fileName, beginMsec, endMsec));
        }

        tagFile.close();
        return clustersFromFile;
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
     * Reads in the given csv file to retrieve the saved frame infos and verifies that the number of entries corresponds
     * to the given frame number.
     * @param csvStream The file stream. Needs to be open.
     * @return An empty vector if unsuccessful.
     */
    static vector<FrameInfo> readFrameInfosFromCsv(string filePath, double supposedNumberOfFrames) {
        ifstream csvStream;
        csvStream.open(filePath);
        assert(csvStream.is_open());

        vector<FrameInfo> frameInfos;
        string line;
        // Read in header line.
        if (!getLineSafe(csvStream, line)) return vector<FrameInfo>();
        while (getLineSafe(csvStream, line)) {
            vector<char> charLine(line.c_str(), line.c_str() + line.size() + 1u);

            double frameNo = stod(strtok(&charLine[0], ","));
            double msec = stod(strtok(nullptr, ","));
            double similarity = stod(strtok(nullptr, ","));
            assert(nullptr == strtok(nullptr, ","));

            frameInfos.push_back(FrameInfo(Mat(), frameNo, msec, similarity));
        }

        csvStream.close();

        if (frameInfos.size() != supposedNumberOfFrames) {
            cerr << "Tag file \"" << filePath << "\" has the wrong number of entries." << endl
                 << "Expected: " << supposedNumberOfFrames << ", actual: " << frameInfos.size() << endl
                 << "Please verify the file integrity and recalculate, if necessary." << endl;
            throw("Invalid number of entries in CSV file.");
        }
        return frameInfos;
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
