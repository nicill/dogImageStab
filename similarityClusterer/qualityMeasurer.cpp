//
// Created by tokuyama on 17/02/03.
//

#include "qualityMeasurer.h"

/**
 * Scores the quality of the clustering when compared to tag files.
 * @param pathToTagFileDirectory A directory containing tag files. Must be a valid directory.
 * @param clusters The clustering to be evalued.
 * @return A quality score in [0,1].
 */
double qualityMeasurer::scoreQuality(string pathToTagFileDirectory, vector<vector<FrameInfo>> clusters) {
    // TODO implement
    // throw("NOT IMPLEMENTED");

    DIR *tagFileDir = opendir(pathToTagFileDirectory.c_str());
    struct dirent *fileEntity;
    while ((fileEntity = readdir(tagFileDir)) != nullptr) {
        cout << fileEntity->d_name << endl;
        // TODO use value
        readTagFile(pathToTagFileDirectory + "/" + fileEntity->d_name);
    }
}

/**
 * Tries to read the file at the given path as a tag file and interpret its clusterings.
 * @param pathToTagFile The path, including the file name.
 * @return A vector of clusterings.
 * @throw Throws if unsuccessful, for example because the file isn't correcly formatted.
 */
vector<ClusterInfo> qualityMeasurer::readTagFile(string pathToTagFile) {
    ifstream tagFile;
    tagFile.open(pathToTagFile);
    if (!tagFile.is_open()) throw("(readTagFile) Couldn't open file.");;

    string line;
    regex pattern = regex("(\\w|\\/)*\\s+\\d+\\s+\\d+\\s+\\d+");
    while (getline(tagFile, line)) {
        if (!regex_match(line, pattern)) throw("(readTagFile) Couldn't match pattern.");

        vector<string> split = splitLine(line);
    }

    tagFile.close();
    // return wat
}

vector<string> qualityMeasurer::splitLine(string inputString) {
    vector<string> splitString;
    bool building = false;
    int baseIndex = 0;
    for (int i = 0; i < inputString.length(); i++) {
        if (&inputString[i] == " ") {
            if (building) {
                splitString.push_back(inputString.substr((size_t)baseIndex, (size_t)1 + i - baseIndex));
                building = false;
            }

            baseIndex = i + 1;
            continue;
        }

        building = true;
    }

    return splitString;
}
