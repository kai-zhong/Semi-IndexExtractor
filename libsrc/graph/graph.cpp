#include "graph/graph.h"
#include "graph/vertex.h"

Graph::Graph()
{
    vertex_num = 0;
    edge_num = 0;
}

Graph::~Graph(){}

VertexID Graph::getMinDegreeVertexID()
{
    VertexID minVid = 0;
    bool found = false;

    if(vertex_num == 0)
    {
        std::cerr << "Error: Graph is empty!" << std::endl;
        throw std::runtime_error("Graph is empty!");
    }

    for(std::map<uint, std::list<VertexID>>::iterator it = invertedIndex.begin(); it != invertedIndex.end();)
    {
        uint degree = it->first;
        std::list<VertexID>& vidList = it->second;

        while(!vidList.empty())
        {
            VertexID vid = vidList.front();
            if(!hasVertex(vid) || nodes.at(vid).getDegree() != degree) // 该链表节点失效
            {
                vidList.pop_front();
            }
            else
            {
                found = true;
                minVid = vid;
                // vidList.pop_front(); // 可以考虑在这里删除，但是如果是最后一个元素要考虑删除list。但是返回后可能还会进行某些操作，所以暂时保留，因为后面再搜索到也会直接删除该链表节点
                break;
            }
        }
        if(found)
        {
            break;
        }
        else
        {
            if(vidList.empty())
            {
                it = invertedIndex.erase(it);
            }
        }
    }

    return minVid;
}

uint Graph::getMinDegreeWithtraversal()
{
    uint minDegree = std::numeric_limits<uint>::max();
    for(const std::pair<uint, Vertex>& nodepair : nodes)
    {
        uint degree = nodepair.second.getDegree();
        if(degree < minDegree)
        {
            minDegree = degree;
        }
    }
    return minDegree;
}

uint Graph::getVertexNum() const
{
    return vertex_num;
}

uint Graph::getEdgeNum() const
{
    return edge_num;
}

uint Graph::getVertexDegree(const VertexID& vid) const
{
    if(!hasVertex(vid))
    {
        std::cerr << "Error: Vertex " << vid << " does not exist!" << std::endl;
        throw std::runtime_error("Vertex " + std::to_string(vid) + " does not exist!");
    }

    return nodes.at(vid).getDegree();
}

Vertex Graph::getVertex(const VertexID& vid) const
{
    if(!hasVertex(vid))
    {
        std::cerr << "Error: Vertex " << vid << " does not exist!" << std::endl;
        throw std::runtime_error("Vertex " + std::to_string(vid) + " does not exist!");
    }
    return nodes.at(vid);
}

const std::vector<VertexID>& Graph::getVertexNeighbors(const VertexID& vid) const
{
    if(!hasVertex(vid))
    {
        std::cerr << "Error: Vertex " << vid << " does not exist!" << std::endl;
        throw std::runtime_error("Vertex " + std::to_string(vid) + " does not exist!");
    }
    return nodes.at(vid).getNeighbors();
}

std::array<unsigned char, SHA256_DIGEST_LENGTH> Graph::getVertexDigest(const VertexID& vid) const
{
    if(!hasVertex(vid))
    {
        std::cerr << "Error: Vertex " << vid << " does not exist!" << std::endl;
        throw std::runtime_error("Vertex " + std::to_string(vid) + " does not exist!");
    }
    return nodes.at(vid).getDigest();
}

const std::map<uint, Vertex>& Graph::getNodes() const
{
    return nodes;
}

bool Graph::hasVertex(const VertexID& vid) const
{
    return nodes.find(vid) != nodes.end();
}

void Graph::loadGraphfromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if(!file.is_open())
    {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        throw std::runtime_error("Cannot open file " + filename);
    }

    std::string line;
    while(std::getline(file, line))
    {
        std::istringstream iss(line);
        VertexID src, dst;
        
        if(!(iss >> src >> dst))
        {
            std::cerr<<"Error: Invalid input forma: "<<line<<std::endl;
            throw std::runtime_error("Invalid input format in file " + filename);
        }

        addEdge(src, dst, false, false);
    }
    file.close();

    std::cout << "Graph : Graph has been loaded from file " << filename << std::endl;

    buildInvertedIndex();
    std::cout << "Graph : Inverted Index built." << std::endl;
    
    computeVertexDigest();
    std::cout << "Graph : Vertexs Digest has been Computed." << std::endl;

    std::cout << "Graph : Graph build complete."<< std::endl;
    std::cout << std::endl;
}

void Graph::writeGraphtoFile(const std::string& filename)
{

}

void Graph::addVertex(const VertexID& vid, bool updateIndex, bool computeVDigest)
{
    if(!hasVertex(vid))
    {
        nodes[vid] = Vertex(vid);
        if(computeVDigest == true)
        {
            nodes[vid].digestCompute();
        }
        ++vertex_num;

        if(updateIndex)
        {
            updateInvertedIndexAAV(vid);
        }
    }
}

void Graph::removeVertex(const VertexID& vid, bool updateIndex, bool computeVDigest)
{
    if(hasVertex(vid))
    {
        if(updateIndex)
        {
            updateInvertedIndexADV(vid);
        }

        std::vector<VertexID> neighbors = nodes.at(vid).getNeighbors();
        for(const VertexID& neighbor : neighbors)
        {
            nodes.at(neighbor).removeNeighbor(vid);
            if(computeVDigest == true)
            {
                nodes.at(neighbor).digestCompute();
            }
            --edge_num;
        }

        nodes.erase(vid);
        --vertex_num;
    }
}

void Graph::addEdge(const VertexID& src, const VertexID& dst, bool updateIndex, bool computeVDigest)
{
    // 会检查src和dst是否存在，不存在则会自动添加
    addVertex(src, false, computeVDigest);
    addVertex(dst, false, computeVDigest);
    
    if(!nodes.at(src).hasNeighbor(dst))
    {
        nodes.at(src).addNeighbor(dst);
        nodes.at(dst).addNeighbor(src);
        if(computeVDigest == true)
        {
            nodes.at(src).digestCompute();
            nodes.at(dst).digestCompute();
        }
        ++edge_num;

        if(updateIndex)
        {
            updateInvertedIndexAUE(src, dst);
        }
    }
}

void Graph::removeEdge(const VertexID& src, const VertexID& dst, bool updateIndex, bool computeVDigest)
{
    if(hasVertex(src) && hasVertex(dst))
    {
        if(nodes.at(src).hasNeighbor(dst))
        {
            nodes.at(src).removeNeighbor(dst);
            nodes.at(dst).removeNeighbor(src);
            if(computeVDigest == true)
            {
                nodes.at(src).digestCompute();
                nodes.at(dst).digestCompute();
            }
            --edge_num;
        }

        if(updateIndex)
        {
            updateInvertedIndexAUE(src, dst);
        }
    }
}

void Graph::buildInvertedIndex()
{
    invertedIndex.clear();
    for(const std::pair<uint, Vertex>& nodepair : nodes)
    {
        uint degree = nodepair.second.getDegree();
        invertedIndex[degree].push_back(nodepair.second.getVid());
    }
}

void Graph::updateInvertedIndexADV(const VertexID& vid)
{
    Vertex node = nodes.at(vid);
    uint degree = node.getDegree();
    std::vector<VertexID> neighbors = node.getNeighbors();
    for(const VertexID& neighbor : neighbors)
    {
        uint neighborDegree = nodes.at(neighbor).getDegree();
        invertedIndex[neighborDegree-1].push_back(neighbor);
    }
}

void Graph::updateInvertedIndexAAV(const VertexID& vid)
{
    uint degree = nodes.at(vid).getDegree();

    invertedIndex[degree].push_back(vid);
}

void Graph::updateInvertedIndexAUE(const VertexID& src, const VertexID& dst)
{
    uint srcDegree = nodes.at(src).getDegree();
    uint dstDegree = nodes.at(dst).getDegree();

    invertedIndex[srcDegree].push_back(src);
    invertedIndex[dstDegree].push_back(dst);
}

void Graph::computeVertexDigest()
{
    for(auto& node : nodes)
    {
        node.second.digestCompute();
    }
}

std::vector<VertexID> Graph::convertToLocalID() const
{
    std::vector<VertexID> local2global(vertex_num);

    VertexID localIndex = 0;

    for(const std::pair<VertexID, Vertex>& nodePair : nodes)
    {
        local2global[localIndex] = nodePair.first;
        localIndex++;
    }
    return local2global;
}

void Graph::printGraphInfo(int verboseNodeNum) const
{
    std::cout << "Graph Information:" << std::endl;
    std::cout << PRINT_SEPARATOR << std::endl;
    std::cout << "Number of vertices: " << vertex_num << std::endl;
    std::cout << "Number of edges: " << edge_num << std::endl;

    std::cout << "Nodes Information:" << std::endl;
    int count = 0; // 计数器
    int verboseNum = verboseNodeNum;
    if(verboseNodeNum == -1)
    {
        verboseNum = vertex_num;
    }
    for (const std::pair<VertexID, Vertex>& node : nodes) 
    {
        if (count >= verboseNum)
        { 
            break;
        }
        node.second.printInfo(); // 调用 Vertex 类的 printInfo() 打印节点信息
        count++;
    }
    std::cout<<PRINT_SEPARATOR<<std::endl;

    std::cout << "Inverted Index:" << std::endl;
    for (const auto& [degree, vertexList] : invertedIndex) 
    {
        std::cout << "Degree " << degree << ": ";
        for (const auto& vertex : vertexList) 
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
    }
}

void Graph::printGraphInfoSimple(int verboseNodeNum) const
{
    int count = 0; // 计数器
    int verboseNum = verboseNodeNum;
    if(verboseNodeNum == -1)
    {
        verboseNum = vertex_num;
    }

    std::cout << "Graph Information:" << std::endl;
    std::cout << "Number of vertices: " << vertex_num << std::endl;
    std::cout << "Number of edges: " << edge_num << std::endl;
    std::cout << "Nodes Information:" << std::endl;
    for (const std::pair<VertexID, Vertex>& node : nodes) 
    {
        if (count >= verboseNum) 
        {
            break;
        }
        std::cout << node.second.getVid() << ": ";
        node.second.printNeighbors();
        node.second.printDigest();
        count++;
    }
}