#include "semiIndexExtractor/semiIndexExtractor.h"

semiIndexExtractor::semiIndexExtractor()
{
    answerExists = false;
    mbptree = nullptr;
}

semiIndexExtractor::~semiIndexExtractor()
{
    if(mbptree != nullptr)
    {
        delete mbptree;
    }
}

void semiIndexExtractor::buildMbpTree(const Graph& graph, const uint maxcapacity)
{
    if(mbptree != nullptr)
    {
        delete mbptree;
        mbptree = nullptr;
    }
    mbptree = new MbpTree(maxcapacity);
    for(const std::pair<uint, Vertex>& nodepair : graph.getNodes())
    {
        VertexID vid = nodepair.first;
        mbptree->setVertexDigest(vid, graph.getVertexDigest(vid));
    }
    mbptree->getRoot()->digestCompute();
}

std::map<VertexID, std::string> semiIndexExtractor::serializeGraphInfo(const Graph& graph, const Graph& subgraph, std::vector<VertexID>& subgraphVids)
{
    std::map<VertexID, std::string> serializedInfo;
    std::ostringstream oss;

    for(const std::pair<uint, Vertex>& nodepair : subgraph.getNodes())
    {
        unsigned char splitter = '/';
        oss = std::ostringstream();
        VertexID vid = nodepair.first;
        const Vertex& subgraphNode = nodepair.second;
        const Vertex& graphNode = graph.getVertex(vid);
        const std::vector<VertexID>& subgraphNeighbors = subgraphNode.getNeighbors();
        const std::vector<VertexID>& graphNeighbors = graphNode.getNeighbors();

        subgraphVids.emplace_back(vid);

        oss << vid;
        for(const VertexID& subNeighbor : subgraphNeighbors)
        {
            oss << splitter << subNeighbor;
        }
        oss << '|' << vid;
        for(const VertexID& graphNeighbor : graphNeighbors)
        {
            oss << splitter << graphNeighbor;
        }

        serializedInfo[vid] = oss.str();
    }
    return serializedInfo;
}

void semiIndexExtractor::constructVO(const Graph& G, const Graph& subgraph)
{
    std::vector<VertexID> subgraphVids;
    subgraphVids.reserve(subgraph.getVertexNum());
    std::map<VertexID, std::string> serializedInfo = serializeGraphInfo(G, subgraph, subgraphVids);
    std::cout << "Serialized graph information has been constructed. " << std::endl;
    mbptree->constructVO(vo, subgraphVids, serializedInfo);
    std::cout << "VO has been constructed." << std::endl;
}

void semiIndexExtractor::getRootDigest(unsigned char* _digest)
{
    if(mbptree != nullptr)
    {
        mbptree->getRoot()->getDigest(_digest);
    }
    else
    {
        std::cerr << "MbpTree is not built." << std::endl;
        throw std::out_of_range("MbpTree is not built.");
    }
}

void semiIndexExtractor::vertify(Graph& subgraph, std::queue<VOEntry>& VO, unsigned char* partDigest)
{
    unsigned char vertifyDigest[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    while(!VO.empty())
    {
        VOEntry entry = VO.front();
        VO.pop();
        if(entry.type == VOEntry::NODEDATA)
        {
            if(entry.nodeData != nullptr)
            {
                // 构建子图
                std::string nodeDataStr(entry.nodeData);
                std::pair<std::string, std::string> infoPair = splitStringtoTwoParts(nodeDataStr, "|");
                std::string subgraphNodeDataStr = infoPair.first;
                std::string graphNodeDataStr = infoPair.second;
                std::vector<VertexID> subvertexInfo;
                splitString(subgraphNodeDataStr, "/", subvertexInfo);
                VertexID vid = subvertexInfo[0];
                for(size_t i = 1; i < subvertexInfo.size(); i++)
                {
                    subgraph.addEdge(vid, subvertexInfo[i], false, false);
                }

                // 计算摘要
                unsigned char subgraphVertexDigest[SHA256_DIGEST_LENGTH];
                SHA256_CTX minictx;
                SHA256_Init(&minictx);
                SHA256_Update(&minictx, (unsigned char*)graphNodeDataStr.c_str(), graphNodeDataStr.size());
                SHA256_Final(subgraphVertexDigest, &minictx);

                // std::cout << "Vertex " << vid  << ": ";
                // std::cout << graphNodeDataStr << " -> ";
                // digestPrint(subgraphVertexDigest);

                SHA256_Update(&ctx, subgraphVertexDigest, SHA256_DIGEST_LENGTH);
            }
        }
        else if(entry.type == VOEntry::DIGEST)
        {
            // std::cout << "VOENTRY_DIGEST: ";
            // digestPrint(entry.digest);
            SHA256_Update(&ctx, entry.digest, SHA256_DIGEST_LENGTH);
        }
        else if(entry.type == VOEntry::SPECIAL)
        {
            if(entry.specialChar == '[')
            {
                unsigned char innerDigest[SHA256_DIGEST_LENGTH];
                vertify(subgraph, VO, innerDigest);
                if(VO.empty())
                {
                    memcpy(vertifyDigest, innerDigest, SHA256_DIGEST_LENGTH);
                    break;
                }
                else
                {
                    SHA256_Update(&ctx, innerDigest, SHA256_DIGEST_LENGTH);
                }
            }
            else if(entry.specialChar == ']')
            {
                SHA256_Final(partDigest, &ctx);
                return ;
            }
        }
        else
        {
            std::cerr << "Unknown VOEntry type: " << entry.type << std::endl;
            throw std::runtime_error("Unknown VOEntry type.");
        }
    }
    std::cout << "Vertify digest: ";
    digestPrint(vertifyDigest);
    unsigned char rootDigest[SHA256_DIGEST_LENGTH];
    getRootDigest(rootDigest);
    std::cout << "Root digest: ";
    digestPrint(rootDigest);
    std::cout << std::endl;
}

void semiIndexExtractor::calculateVOSize()
{
    // size_t totalSize = sizeof(vo); // vector 内部结构的占用
    size_t totalSize = 0; // 仅计算 VOEntry 占用的大小
    for(const auto& entry : vo)
    {
        totalSize += sizeof(entry); // 每个 VOEntry 的占用
        if(entry.type == VOEntry::NODEDATA && entry.nodeData != nullptr)
        {
            totalSize += std::strlen(entry.nodeData) + 1; // 动态分配的 nodeData 大小
        }
    }
    std::cout << "Total size of vo:" << std::endl;
    std::cout << "  Bytes: " << totalSize << " B" << std::endl;
    std::cout << "  Kilobytes: " << totalSize / 1024.0 << " KB" << std::endl;
    std::cout << "  Megabytes: " << totalSize / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << std::endl;
}

const std::vector<VOEntry>& semiIndexExtractor::getVO() const
{
    return vo;
}

const uint& semiIndexExtractor::getCore(const VertexID& vid) const
{
    auto it = cores.find(vid);
    if (it != cores.end()) 
    {
        // 如果找到，则返回对应的核心值
        return it->second;
    } 
    else 
    {
        // 如果未找到，抛出异常或处理错误
        std::cerr << "VertexID not found in cores." << std::endl;
        throw std::out_of_range("VertexID not found in cores.");
    }
}

void semiIndexExtractor::coresDecomposition(const Graph& graph)
{
    uint vertexNum = graph.getVertexNum();
    std::unordered_map<VertexID, VertexID> global2local;
    std::vector<VertexID> local2global = graph.convertToLocalID();
    std::vector<uint> degree(vertexNum);
    std::vector<uint> bin(vertexNum, 0);
    std::vector<uint> pos(vertexNum);
    std::vector<VertexID> vert(vertexNum);
    uint maxDegree = 0;

    global2local.reserve(vertexNum);
    for(size_t localID = 0; localID < vertexNum; localID++)
    {
        global2local[local2global[localID]] = localID;
    }

    // 第一步：计算每个节点的度数并找出最大度数 md
    for(size_t localID = 0; localID < vertexNum; localID++)
    {
        degree[localID] = graph.getVertexDegree(local2global[localID]);
        maxDegree = std::max(maxDegree, degree[localID]);
    }

    // 第二步：初始化 bin 数组，用于统计每个度数的节点数量
    for(size_t d = 0; d <= maxDegree; d++)
    {
        bin[d] = 0;
    }
    for(size_t localID = 0; localID < vertexNum; localID++)
    {
        ++bin[degree[localID]];
    }

    // 第三步：将 bin 转换为前缀和，表示每个度数的起始索引
    size_t start = 0;
    for(size_t d = 0; d <= maxDegree; d++)
    {
        size_t num = bin[d];
        bin[d] = start;
        start += num;
    }
    
    // 第四步：初始化 pos 和 vert 数组
    for(size_t localID = 0; localID < vertexNum; localID++)
    {
        pos[localID] = bin[degree[localID]];
        vert[pos[localID]] = localID;
        ++bin[degree[localID]];
    }

    // 第五步：恢复 bin 数组的偏移量（向后移一位）
    for(size_t d = maxDegree; d > 0; d--)
    {
        bin[d] = bin[d-1];
    }
    bin[0] = 0;

    // 第六步：核心分解算法
    for(size_t i = 0;i < vertexNum; i++)
    {
        VertexID localV = vert[i];
        
        for(const VertexID& u : graph.getVertexNeighbors(local2global[localV]))
        {
            VertexID localU = global2local[u];
            if(degree[localU] > degree[localV])
            {
                uint degU = degree[localU]; // 节点 u 的度数
                uint posU = pos[localU]; // 节点 u 的起始索引
                uint posW = bin[degU]; // 当前度数 degU 的起始索引
                uint w = vert[posW]; // 度数 degU 的第一个节点

                // 如果u和w不相同,则交换位置
                if(localU != w)
                {
                    pos[localU] = posW;
                    vert[posU] = w;
                    pos[w] = posU;
                    vert[posW] = localU;
                }

                ++bin[degU]; // 更新当前度数的起始索引
                --degree[localU]; // 节点 u 的度数减一
            }
        }
    }

    if(!cores.empty())
    {
        cores.clear();
    }

    for(size_t localID = 0; localID < vertexNum; localID++)
    {
        cores[local2global[localID]] = degree[localID];
    }
}

VertexID semiIndexExtractor::getLiIndexTop() const
{
    std::map<uint, std::list<VertexID>>::const_reverse_iterator it = liIndex.rbegin();
    if(it == liIndex.rend())
    {
        std::cerr << "LiIndex is empty." << std::endl;
        throw std::out_of_range("LiIndex is empty.");
    }
    return it->second.front();
}

void semiIndexExtractor::liIndexPop()
{
    std::map<uint, std::list<VertexID>>::reverse_iterator it = liIndex.rbegin();
    if(it == liIndex.rend())
    {
        std::cerr << "LiIndex is empty." << std::endl;
        throw std::out_of_range("LiIndex is empty.");
    }
    liIndexExists.erase(it->second.front());
    it->second.pop_front();
    if(it->second.empty())
    {
        liIndex.erase(it->first);
    }
}

void semiIndexExtractor::initLiIndex(const VertexID& queryV)
{
    liIndex.clear();
    liIndex[0].push_back(queryV);
    liIndexExists[queryV] = 0;
}

void semiIndexExtractor::insertToLiIndex(const Graph& graph, const VertexID& vid, std::unordered_map<VertexID, bool>& visited, const uint& k)
{
    std::vector<VertexID> neighbors;
    for(const VertexID& neighbor : graph.getVertexNeighbors(vid))
    {
        if(!visited[neighbor] && cores[neighbor] >= k)
        {
            neighbors.emplace_back(neighbor);
        }
    }

    if(neighbors.empty())
    {
        return;
    }

    for(const VertexID& neighbor : neighbors)
    {
        if(liIndexExists.find(neighbor) == liIndexExists.end()) // 未加入LiIndex
        {
            liIndex[1].push_back(neighbor);
            liIndexExists[neighbor] = 1;
        }
        else // 已加入LiIndex
        {
            uint li = liIndexExists[neighbor];
            liIndex[li].remove(neighbor);
            if(liIndex[li].empty())
            {
                liIndex.erase(li);
            }
            liIndex[li + 1].push_back(neighbor);
            liIndexExists[neighbor] = li + 1;
        }
    }
}

bool semiIndexExtractor::isLiIndexEmpty() const
{
    return liIndex.empty();
}

void semiIndexExtractor::candidateGeneration(const Graph& graph, const VertexID& queryV, const uint& k)
{
    if(!candVertices.empty())
    {
        candVertices.clear();
    }
    std::unordered_map<VertexID, bool> visited;

    for(const std::pair<VertexID, Vertex>& nodePair : graph.getNodes())
    {
        visited[nodePair.first] = false;
    }

    initLiIndex(queryV);

    while(!isLiIndexEmpty())
    {
        VertexID currentV = getLiIndexTop();
        visited[currentV] = true;
        liIndexPop();

        candGraph.addVertex(currentV, true, false);
        for(const VertexID& neighbor : graph.getVertexNeighbors(currentV))
        {
            if(visited[neighbor] == true)
            {
                candGraph.addEdge(currentV, neighbor, true, false);
            }
        }
        candVertices.emplace_back(currentV);

        // uint minDegree = std::numeric_limits<uint>::max();
        // for(const VertexID& v : candVertices)
        // {
        //     minDegree = std::min(minDegree, candGraph.getVertexDegree(v));
        // }
        // std::cout << "MinDegree : " << minDegree << std::endl;
        
        uint minDegree = candGraph.getVertexDegree(candGraph.getMinDegreeVertexID());
        if(minDegree >= k)
        {
            std::cout << "Answer exists." << std::endl;
            answerExists = true;
            break;
        }

        // 批量更新LiIndex
        insertToLiIndex(graph, currentV, visited, k);
        // std::cout << "CurrentV : " << currentV << ": "<<std::endl;
        // printLiIndex();
    }
}

void semiIndexExtractor::globalExtract(const VertexID& queryV, const uint& k)
{
    VertexID minDegreeV;
    uint deg;
    bool satisfyKcore = false;
    Graph kcoreG;
    // candGraph.printGraphInfoSimple();
    while(true)
    {
        minDegreeV = candGraph.getMinDegreeVertexID();
        deg = candGraph.getVertexDegree(minDegreeV);
        if(deg < k)
        {
            if(minDegreeV == queryV)
            {
                break;
            }
            candGraph.removeVertex(minDegreeV, true, false);
        }
        else
        {
            satisfyKcore = true;
            break;
        }
    }
    if(!satisfyKcore)
    {
        std::cout << "The graph does not satisfy k-core property." << std::endl;
    }
}

Graph semiIndexExtractor::kcoreExtract(const Graph& graph, const VertexID& queryV, const uint& k)
{
    if(!answerExists)
    {
        answerExists = false;
    }

    candidateGeneration(graph, queryV, k);

    if(!answerExists)
    {
        candGraph.buildInvertedIndex();
        globalExtract(queryV, k);
    }

    constructVO(graph, candGraph);
    return candGraph;
}

const Graph& semiIndexExtractor::getCandGraph() const
{
    return candGraph;
}

void semiIndexExtractor::printVO() const
{
    for(size_t i = 0; i < vo.size(); i++)
    {
        vo[i].printVOEntry();
        if(i < vo.size() - 1 && vo[i].type != VOEntry::SPECIAL && vo[i+1].type != VOEntry::SPECIAL)
        {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
}

void semiIndexExtractor::printLiIndex() const
{
    if(liIndex.empty())
    {
        std::cout << "LiIndex is empty." << std::endl;
        return;
    }
    for (const auto& pair : liIndex)
    {
        std::cout << "Key: " << pair.first << ", Values: ";
        for (const auto& value : pair.second)
        {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void semiIndexExtractor::printCores() const
{
    for(const auto& c : cores)
    {
        std::cout << "Vertex " << c.first << " k-core-max : " << c.second << std::endl;
    }
    std::cout << std::endl;
}