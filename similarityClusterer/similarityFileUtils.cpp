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

        ClusterInfoContainer clustersFromFile =
                ClusterInfoContainer(basename(const_cast<char*>(pathToTagFile.c_str())));
        string line;
        while (getline(tagFile, line)) {
            // Check correctness with RegEx?
            vector<string> split = splitLine(line);

            // Split should contain: Name, start (msec), end (msec), duration (msec)
            assert(split.size() == 4);
            double beginMsec = stod(split[1].c_str());
            double endMsec = stod(split[2].c_str());
            assert(stod(split[3].c_str()) == (endMsec - beginMsec));

            clustersFromFile.clusterInfos.push_back(ClusterInfo(split[0], beginMsec, endMsec));
        }

        tagFile.close();
        return clustersFromFile;
    }

    /**
     * Splits the given line at whitespace/tab/carriage return characters.
     * @param inputString The string to split.
     * @return List of substrings. Empty strings are discarded.
     */
    static vector<string> splitLine(string inputString) {
        vector<string> splitString;
        bool building = false;
        int baseIndex = 0;

        for (int i = 0; i < inputString.length(); i++) {
            char current = inputString[i];
            if (current == ' ' || current == '\t' || current == '\r') {
                if (building) {
                    splitString.push_back(inputString.substr((size_t)baseIndex, (size_t)i - baseIndex));
                    building = false;
                }

                baseIndex = i + 1;
                continue;
            }

            building = true;
        }

        return splitString;
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
        if (!getline(csvStream, line)) return vector<FrameInfo>();
        while (getline(csvStream, line)) {
            vector<char> charLine(line.c_str(), line.c_str() + line.size() + 1u);

            double frameNo = stod(strtok(&charLine[0], ","));
            double msec = stod(strtok(nullptr, ","));
            double similarity = stod(strtok(nullptr, ","));
            assert(nullptr == strtok(nullptr, ","));

            frameInfos.push_back(FrameInfo(Mat(), frameNo, msec, similarity));
        }

        csvStream.close();

        assert(frameInfos.size() == supposedNumberOfFrames);
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
                            bool stop, bool bark) {
        vector<double> elements = { frameNo, msec, similarity, averageSimilarity, (double)stop, (double)bark };
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

        ioFileStream.close();
    }
};

#endif // DOGIMAGESTABILIZATION_TAGFILEUTILS_CPP
