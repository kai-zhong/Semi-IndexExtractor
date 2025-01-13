#include "util/edgeReader.h"

EdgeReader::EdgeReader(const std::string& _filePath)
{
    if(!_filePath.empty())
    {
        setFilePath(_filePath);
    }
}

EdgeReader::~EdgeReader()
{
    if(file.is_open())
    {
        file.close();
    }
}

void EdgeReader::setFilePath(const std::string& _filePath)
{
    filePath = _filePath;
    endOfFile =false;
    file.open(filePath);
    if(!file.is_open())
    {
        std::cerr << "Error: cannot open file " << filePath << std::endl;
        throw std::runtime_error("Error: cannot open file " + filePath);
    }
}

std::vector<std::pair<VertexID, VertexID>> EdgeReader::readNextEdges(const uint& num)
{
    if(endOfFile || num == 0)
    {
        std::cout << "File end reached or no more edges to read" << std::endl;
        return {};
    }

    std::vector<std::pair<VertexID, VertexID>> edges;
    std::string line;
    uint linesRead = 0;

    while(std::getline(file, line) && linesRead < num)
    {
        std::istringstream iss(line);
        VertexID src, dst;
        if(!(iss >> src >> dst))
        {
            std::cerr<<"Error: Invalid input format: "<<line<<std::endl;
            throw std::runtime_error("Invalid input format in file " + filePath);
        }
        edges.emplace_back(src, dst);
        linesRead++;
    }

    if(file.eof())
    {
        endOfFile = true;
    }

    return edges;
}

bool EdgeReader::isEndOfFile() const
{
    return endOfFile;
}