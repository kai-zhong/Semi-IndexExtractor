#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include "../configuration/types.h"
#include "../configuration/config.h"

class EdgeReader
{
    private:
        std::ifstream file;
        std::string filePath;
        bool endOfFile;

    public:
        EdgeReader(const std::string& _filePath = "");
        ~EdgeReader();
        void setFilePath(const std::string& _filePath);
        std::vector<std::pair<VertexID, VertexID>> readNextEdges(const uint& num);
        bool isEndOfFile() const;
};