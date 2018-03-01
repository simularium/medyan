#ifndef MEDYAN_READ_SNAPSHOT_HPP
#define MEDYAN_READ_SNAPSHOT_HPP

#include <string>

class SnapshotReader {
private:
    std::string _snapshotFilepath;
    std::string _vmdFilepath;

public:
    SnapshotReader(const std::string& snapshotFilepath, const std::string& vmdFilepath):
        _snapshotFilepath(snapshotFilepath), _vmdFilepath(vmdFilepath) {}

    void readAndConvertToVmd();
};



#endif