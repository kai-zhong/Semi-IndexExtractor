#include "graph/vertex.h"

Vertex::Vertex() : id(0), degree(0) 
{
    memset(digest, 0, SHA256_DIGEST_LENGTH);
}

Vertex::Vertex(VertexID vid) : id(vid), degree(0) 
{
    memset(digest, 0, SHA256_DIGEST_LENGTH);
}

Vertex::Vertex(const Vertex& other)
{
    id = other.id;
    degree = other.degree;
    neighbors = other.neighbors;
    std::memcpy(digest, other.digest, SHA256_DIGEST_LENGTH);
}

Vertex::~Vertex() {}

VertexID Vertex::getVid() const
{
    return id;
}

uint Vertex::getDegree() const
{
    return degree;
}

std::array<unsigned char, SHA256_DIGEST_LENGTH> Vertex::getDigest() const
{
    std::array<unsigned char, SHA256_DIGEST_LENGTH> result;
    std::copy(digest, digest + SHA256_DIGEST_LENGTH, result.begin());
    return result;
}

const std::vector<VertexID>& Vertex::getNeighbors() const
{
    return neighbors;
}

bool Vertex::hasNeighbor(VertexID neighbor_vid) const
{
    return std::binary_search(neighbors.begin(), neighbors.end(), neighbor_vid);
}

void Vertex::addNeighbor(VertexID neighbor_vid)
{
    auto it = std::lower_bound(neighbors.begin(), neighbors.end(), neighbor_vid);
    
    // 如果 neighbor_vid 已存在，则不插入
    if(it != neighbors.end() && *it == neighbor_vid)
    {
        return ;
    }

    neighbors.insert(it, neighbor_vid);
    degree++;
}

void Vertex::removeNeighbor(VertexID neighbor_vid)
{
    auto it = std::lower_bound(neighbors.begin(), neighbors.end(), neighbor_vid);
    if(it != neighbors.end())
    {
        neighbors.erase(it);
        degree--;
    }
}

void Vertex::digestCompute()
{
    std::ostringstream oss;
    unsigned char splitter = '/';
    oss << id;
    for (auto neighbor : neighbors)
    {
        oss << splitter << neighbor;
    }
    std::string vertexDataStr = oss.str();

    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (unsigned char*)vertexDataStr.c_str(), vertexDataStr.size());
    SHA256_Final(digest, &ctx);
}

bool Vertex::operator==(const Vertex& other) const
{
    return id == other.id;
}

bool Vertex::operator>(const Vertex& other) const
{
    return id > other.id;
}

bool Vertex::operator<(const Vertex& other) const
{
    return id < other.id;
}

void Vertex::printInfo() const
{
    std::cout << "Vertex ID: " << id << std::endl;
    std::cout << "Degree: " << degree << std::endl;
    std::cout << "Neighbors: ";
    for (auto neighbor : neighbors)
    {
        std::cout << neighbor << " ";
    }
    std::cout << std::endl;
    std::cout << "Digest: ";
    printDigest();
    std::cout << PRINT_SEPARATOR << std::endl;
}

void Vertex::printNeighbors() const
{
    std::cout << "[";
    for (auto neighbor : neighbors)
    {
        std::cout << neighbor;
        if(neighbor!= neighbors.back())
        {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

void Vertex::printDigest() const
{
    std::cout << id << ": ";
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        printf("%02x", digest[i]);
    }
    std::cout << std::endl;
}