#include "semiIndexExtractor/semiIndexExtractor.h"

semiIndexExtractor::semiIndexExtractor()
{
}

semiIndexExtractor::~semiIndexExtractor()
{
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

    for(size_t localID = 0; localID < vertexNum; localID++)
    {
        cores[local2global[localID]] = degree[localID];
    }
}

void semiIndexExtractor::printCores() const
{
    for(const auto& c : cores)
    {
        std::cout << "Vertex " << c.first << " k-core-max : " << c.second << std::endl;
    }
    std::cout << std::endl;
}