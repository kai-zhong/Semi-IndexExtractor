#include "coremaintainer/coremaintainer.h"

CoreMaintainer::CoreMaintainer()
{}

CoreMaintainer::CoreMaintainer(const Graph& graph)
{

}

uint CoreMaintainer::getCore(const VertexID& vid) const
{
    auto it = cores.find(vid);
    if(it == cores.end())
    {
        std::cerr << "Vertex " << vid << " not found in cores" << std::endl;
        throw std::runtime_error("Vertex not found in cores");
    }
    return it->second;
}

void CoreMaintainer::insertToOrderk(const std::vector<VertexID>& vert, const std::vector<VertexID> local2global, uint startPos, uint endPos, uint k)
{
    if(startPos >= vert.size() || endPos > vert.size() || startPos > endPos)
    {
        std::cerr << "Invalid input" << std::endl;
        throw std::runtime_error("Invalid input");
    }

    if(startPos == endPos)
    {
        return ;
    }

    std::unordered_map<VertexID, uint>& _degPlus = degPlus;
    std::vector<VertexID> orderVert(vert.begin() + startPos, vert.begin() + endPos); // 不包括索引为 endPos 的元素

    /* k-order序保持 */
    for(uint i = 0; i < orderVert.size(); i++)
    {
        ostrees[k].insertBack(local2global[orderVert[i]]);
        orderkV[k].push_back(local2global[orderVert[i]]);
    }

    // /* 小deg+优先 */
    // for(uint i = 0; i < orderVert.size(); i++)
    // {
    //     ostrees[k].insertBack(local2global[orderVert[i]]);
    // }

    // std::sort(orderVert.begin(), orderVert.end(), [&_degPlus, &local2global](VertexID a, VertexID b){return _degPlus.at(local2global[a]) < _degPlus.at(local2global[b]);});

    // for(uint i = 0; i < orderVert.size(); i++)
    // {
    //     orderkV[k].push_back(local2global[orderVert[i]]);
    // }
}

void CoreMaintainer::initmcd(const Graph& graph, const std::vector<VertexID> local2global)
{
    uint vertexNum = graph.getVertexNum();
    for(VertexID localID = 0; localID < vertexNum; localID++)
    {
        VertexID vid = local2global[localID];
        uint vCore = cores.at(vid);
        mcd[vid] = 0;
        for(const VertexID& neighbor : graph.getVertexNeighbors(vid))
        {
            if(cores.at(neighbor) >= vCore)
            {
                mcd[vid]++;
            }
        }
    }
}

void CoreMaintainer::coresDecomp(const Graph& graph)
{
    uint vertexNum = graph.getVertexNum();
    std::unordered_map<VertexID, VertexID> global2local;
    std::vector<VertexID> local2global = graph.convertToLocalID();
    std::vector<uint> degree(vertexNum);
    std::vector<uint> degreeP(vertexNum);
    std::vector<uint> bin(vertexNum, 0);
    std::vector<uint> pos(vertexNum);
    std::vector<VertexID> vert(vertexNum);
    uint maxDegree = 0;

    if(!cores.empty())
    {
        cores.clear();
    }

    global2local.reserve(vertexNum);
    for(size_t localID = 0; localID < vertexNum; localID++)
    {
        global2local[local2global[localID]] = localID;
    }

    // 第一步：计算每个节点的度数并找出最大度数 md
    for(size_t localID = 0; localID < vertexNum; localID++)
    {
        degree[localID] = graph.getVertexDegree(local2global[localID]);
        degreeP[localID] = degree[localID];
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

    uint startPos = 0;
    uint lastK = 1;
    // 第六步：核心分解算法
    for(size_t i = 0;i < vertexNum; i++)
    {
        VertexID localV = vert[i];
        cores[local2global[localV]] = degree[localV];
        degPlus[local2global[localV]] = degreeP[localV];
        if(degree[localV] != lastK)
        {
            insertToOrderk(vert, local2global, startPos, i, lastK);
            startPos = i;
            lastK = degree[localV];
        }
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
            if(degree[localU] >= degree[localV])
            {
                --degreeP[localU];
            }
        }
    }

    if(startPos < vertexNum)
    {
        insertToOrderk(vert, local2global, startPos, vertexNum, lastK);
    }

    initmcd(graph, local2global);
}

bool CoreMaintainer::comparekorder(const uint& a, const uint& b, const uint& k)
{
    OSTree& ost = ostrees.at(k);
    return ost.getRank(a) < ost.getRank(b);
}

void CoreMaintainer::addVertex(const VertexID& vid)
{
    cores[vid] = 1;
    degPlus[vid] = 1;
    mcd[vid] = 1;
    ostrees.at(1).insertFront(vid);
    orderkV.at(1).emplace_front(vid);
}

void CoreMaintainer::orderInsert(const Graph& graph, const VertexID src, const VertexID dst) // 要考虑节点被新添加的情况
{
    static int cnt = 0;
    static std::chrono::milliseconds prepareTime(0);
    static std::chrono::milliseconds coreMaintenanceTime(0);
    static std::chrono::milliseconds endTime(0);

    if(cores.find(src) == cores.end() && cores.find(dst) == cores.end()) // 节点 src 和 dst 是新加入的节点
    {
        addVertex(src);
        addVertex(dst);
        return ;
    }
    if(cores.find(src) == cores.end()) // 节点 src 是新加入的节点
    {
        addVertex(src);

        if(cores.at(dst) == 1)
        {
            ++mcd[dst];
        }
        return ;
    }
    if(cores.find(dst) == cores.end()) // 节点 dst 是新加入的节点
    {
        addVertex(dst);

        if(cores.at(src) == 1)
        {
            ++mcd[src];
        }
        return ;
    }
    /* 准备阶段 */
    auto start = std::chrono::high_resolution_clock::now();
    VertexID u = src; // k-order更小的节点
    VertexID v = dst; // k-order更大的节点
    uint srcCore = cores.at(src);
    uint dstCore = cores.at(dst);
    uint K = std::min(cores.at(src), cores.at(dst));
    OSTree& ost = ostrees.at(K);
    if(dstCore < srcCore || ( dstCore == srcCore && ost.getRank(src) > ost.getRank(dst)))
    {
        u = dst;
        v = src;
    }
    ++degPlus[u];
    auto end = std::chrono::high_resolution_clock::now();
    prepareTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    

    /* 核心维护阶段 */
    start = std::chrono::high_resolution_clock::now();
    if(degPlus[u] <= K)
    {
        return ;
    }

    std::list<VertexID>& orderK = orderkV.at(K); // 只表示处理顺序，节点顺序并不保持k-order
    std::list<VertexID> _orderK;
    std::vector<VertexID> Vc; // 候选集(从Vc中删除只需要在inVc中删除即可，inVc中存在节点号才表示数据有效)，Vc的顺序保持k-order
    std::unordered_map<VertexID, uint> inVc; // 候选集中是否包含节点,uint保存了节点的k-order位置

    std::unordered_map<VertexID, uint> degStar;

    static std::chrono::milliseconds mainCodeTime(0);
    auto start1 = std::chrono::high_resolution_clock::now();
    size_t orderkVSize = orderK.size();
    degStar.reserve(orderkVSize);
    for (const VertexID& v : orderK) 
    {
        degStar.emplace(v, 0);
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    mainCodeTime += std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);

    for(std::list<VertexID>::iterator it = orderK.begin(); it!= orderK.end();)
    {
        VertexID curV = *it;

        if(degStar[curV] + degPlus[curV] > K)
        {
            uint curVRank = ost.getRank(curV);
            inVc[curV] = curVRank;
            Vc.emplace_back(curV);

            for(const VertexID& neighbor : graph.getVertexNeighbors(curV))
            {
                if(cores.at(neighbor) == K && curVRank < ost.getRank(neighbor))
                {
                    degStar[neighbor]++;
                }
            }

            it = orderK.erase(it);
        }
        else if(degStar[curV] == 0)
        {
            while(it != orderK.end())
            {
                VertexID _vid = *it;
                if(degStar[_vid] > 0 || degPlus[_vid] > K)
                {
                    break;
                }
                _orderK.emplace_back(_vid);
                it = orderK.erase(it);
            }
            if(orderK.empty() || it == orderK.end())
            {
                break;
            }
        }
        else
        {
            _orderK.emplace_back(curV);
            degPlus[curV] += degStar[curV];
            degStar[curV] = 0;
            removeCandidates(graph, degStar, inVc, _orderK, curV, K); // 插入会_orderK中要使用顺序插入函数，保持k-order
            it = orderK.erase(it);
        }
    }


    end = std::chrono::high_resolution_clock::now();
    coreMaintenanceTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    /* 结束阶段 */
    start = std::chrono::high_resolution_clock::now();
    std::vector<VertexID> VStar;
    for(const VertexID& w : Vc)
    {
        if(inVc.find(w) == inVc.end())
        {
            continue;
        }
        ++cores[w];
        VStar.emplace_back(w);
    }
    for(int i = VStar.size() - 1; i >= 0; i--)
    {
        ostrees[K].erase(VStar[i]);
        orderkV[K+1].emplace_front(VStar[i]);
        ostrees[K+1].insertFront(VStar[i]);
    }
    orderkV[K] = _orderK;
    updatemcdInsert(graph, VStar, K);
    end = std::chrono::high_resolution_clock::now();
    endTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // if(++cnt % 1000 == 0)
    // {
    //     std::cout << cnt << "th round time: " << std::endl;
    //     std::cout << "Prepare time: " << prepareTime.count() << "ms" << std::endl;
    //     std::cout << "main code time: " << mainCodeTime.count() << "ms" << std::endl;
    //     std::cout << "Core maintenance time: " << coreMaintenanceTime.count() << "ms" << std::endl;
    //     std::cout << "End time: " << endTime.count() << "ms" << std::endl;
    //     std::cout << "Total time: " << (prepareTime + coreMaintenanceTime + endTime).count() << "ms" << std::endl;
    //     prepareTime = std::chrono::milliseconds(0);
    //     mainCodeTime = std::chrono::milliseconds(0);
    //     coreMaintenanceTime = std::chrono::milliseconds(0);
    //     endTime = std::chrono::milliseconds(0);
    // }
}

void CoreMaintainer::removeCandidates(const Graph& graph, std::unordered_map<VertexID, uint>& degStar, std::unordered_map<VertexID, uint>& inVc, std::list<VertexID>& _orderK, VertexID w, uint K)
{
    OSTree& ost = ostrees.at(K);
    uint wRank = ost.getRank(w);
    std::queue<VertexID> Q;
    std::unordered_set<VertexID> visited;
    for(const VertexID& neighbor : graph.getVertexNeighbors(w))
    {
        if(inVc.find(neighbor) != inVc.end())
        {
            --degPlus[neighbor];
            if(degPlus[neighbor] + degStar[neighbor] <= K)
            {
                Q.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }

    while(!Q.empty())
    {
        VertexID curV = Q.front();
        Q.pop();
        degPlus[curV] += degStar[curV];
        degStar[curV] = 0;

        uint curVRank = inVc[curV];
        inVc.erase(curV);
        std::list<VertexID>::iterator it = std::lower_bound(_orderK.begin(), _orderK.end(), curV, [&K, this](const uint& a, const uint& b){return comparekorder(a, b, K);});
        _orderK.insert(it, curV);

        for(const VertexID& neighbor : graph.getVertexNeighbors(curV))
        {
            if(cores.at(neighbor) == K)
            {
                uint neighborRank = ost.getRank(neighbor);
                if(wRank < neighborRank)
                {
                    --degStar[neighbor];
                }
                else if(curVRank < neighborRank && inVc.find(neighbor) != inVc.end())
                {
                    --degStar[neighbor];
                    if(degStar[neighbor] + degPlus[neighbor] <= K && visited.find(neighbor) == visited.end())
                    {
                        Q.push(neighbor);
                        visited.insert(neighbor);
                    }
                }
                else if(inVc.find(neighbor) != inVc.end())
                {
                    --degPlus[neighbor];
                    if(degStar[neighbor] + degPlus[neighbor] <= K && visited.find(neighbor) == visited.end())
                    {
                        Q.push(neighbor);
                        visited.insert(neighbor);
                    }
                }
            }
        }
    }
}

void CoreMaintainer::updatemcdInsert(const Graph& graph, const std::vector<VertexID>& VStar, uint K)
{
    for(const VertexID& w : VStar)
    {
        mcd[w] = 0;
        for(const VertexID& neighbor : graph.getVertexNeighbors(w))
        {
            if(cores.at(neighbor) == K + 1)
            {
                mcd[w]++;
                mcd[neighbor]++;
            }
        }
    }
}

void CoreMaintainer::removeVertex(const VertexID& vid)
{
    cores.erase(vid);
    degPlus.erase(vid);
    mcd.erase(vid);
    ostrees.at(1).erase(vid);
    for(std::list<VertexID>::iterator it = orderkV.at(1).begin(); it != orderkV.at(1).end();)
    {
        if(*it == vid)
        {
            it = orderkV.at(1).erase(it);
        }
    }
}

void CoreMaintainer::orderRemove(const Graph& graph, const VertexID src, const VertexID dst)
{
    // 考虑节点被删除的情况
    if(graph.hasVertex(src) == false && graph.hasVertex(dst) == false)
    {
        removeVertex(src);
        removeVertex(dst);
        return ;
    }
    if(graph.hasVertex(src) == false)
    {
        if(cores.at(dst) == 1)
        {
            --mcd[dst];
            if(ostrees.at(1).getRank(dst) < ostrees.at(1).getRank(src))
            {
                --degPlus[dst];
            }
        }
        removeVertex(src);
        return ;
    }
    if(graph.hasVertex(dst) == false)
    {
        if(cores.at(src) == 1)
        {
            --mcd[src];
            if(ostrees.at(1).getRank(src) < ostrees.at(1).getRank(dst))
            {
                --degPlus[src];
            }
        }
        removeVertex(dst);
        return ;
    }
    uint K = std::min(cores.at(src), cores.at(dst));
    OSTree& ost = ostrees.at(K);
    if(cores.at(src) <= cores.at(dst))
    {
        --mcd[src];
    }
    if(cores.at(dst) <= cores.at(src))
    {
        --mcd[dst];
    }
    std::unordered_map<VertexID, uint> inVStar;
    std::vector<VertexID> VStar = traverseVStarFind(graph, inVStar, src, dst, K); // 要负责删除OK中的VStar节点

    updatemcdRemove(graph, VStar, inVStar, K);
    for(const VertexID& w : VStar)
    {
        degPlus[w] = 0;
        for(const VertexID& neighbor : graph.getVertexNeighbors(w))
        {
            if(cores.at(neighbor) == K)
            {
                uint neighborRank;
                if(inVStar.find(neighbor) != inVStar.end())
                {
                    neighborRank = inVStar.at(neighbor);
                }
                else
                {
                    neighborRank = ost.getRank(neighbor);
                }
                if(neighborRank < inVStar.at(w))
                {
                    --degPlus[neighbor];
                }   
            }
            if(cores.at(neighbor) >= K || inVStar.find(neighbor) != inVStar.end())
            {
                ++degPlus[w];
            }
        }
        inVStar.erase(w);
        orderkV[K-1].emplace_back(w);
        ostrees[K-1].insertBack(w);
    }
}

std::vector<VertexID> CoreMaintainer::traverseVStarFind(const Graph& graph, std::unordered_map<VertexID, uint>& inVStar, const VertexID& src, const VertexID& dst, uint K)
{
    // 要负责删除OK中的VStar节点
    std::vector<VertexID> VStar;
    std::queue<VertexID> Q;
    std::unordered_map<VertexID, uint> cd = mcd;
    std::unordered_set<VertexID> removed;
    if(cores.at(src) == K)
    {
        Q.push(src);
    }
    if(cores.at(dst) == K)
    {
        Q.push(dst);
    }

    while(!Q.empty())
    {
        VertexID w = Q.front();
        Q.pop();
        if(cores.at(w) == K && cd[w] < K)
        {
            VStar.emplace_back(w);
            inVStar.insert(std::make_pair(w, ostrees.at(K).getRank(w)));
            removed.insert(w);
            --cores[w];
            for(const VertexID& z : graph.getVertexNeighbors(w))
            {
                if(removed.find(z) != removed.end())
                {
                    continue;
                }
                if(cores.at(z) == K)
                {
                    --cd[z];
                    if(cd[z] < K)
                    {
                        Q.push(z);
                    }
                }
            }
        }
    }
    // 删除Ok中的VStar节点
    std::list<VertexID>& orderK = orderkV.at(K);
    for(std::list<VertexID>::iterator it = orderK.begin(); it != orderK.end();)
    {
        if(inVStar.find(*it) != inVStar.end())
        {
            VertexID _vid = *it;
            it = orderK.erase(it);
            ostrees.at(K).erase(_vid);
        }
        else
        {
            ++it;
        }
    }
    return VStar;
}

void CoreMaintainer::updatemcdRemove(const Graph& graph, const std::vector<VertexID>& VStar, const std::unordered_map<VertexID, uint>& inVStar, uint K)
{
    for(const VertexID& w : VStar)
    {
        mcd[w] = 0;
        for(const VertexID& neighbor : graph.getVertexNeighbors(w))
        {
            if(inVStar.find(neighbor) != inVStar.end())
            {
                mcd[w]++;
            }
            else
            {
                if(cores.at(neighbor) == K)
                {
                    mcd[w]++;
                    mcd[neighbor]--;
                }
                else if(cores.at(neighbor) == K - 1)
                {
                    mcd[w]++;
                }
            }
        }
    }
}

void CoreMaintainer::printOrderk(uint k) const
{
    if(orderkV.find(k) == orderkV.end())
    {
        std::cout << "No orderk for k = " << k << std::endl;
        return ;
    }
    std::cout << "Orderk for k = " << k << ": ";
    for(const VertexID& vid : orderkV.at(k))
    {
        std::cout << vid << " ";
    }
    std::cout << std::endl;
}

void CoreMaintainer::printOSTree(uint k) const
{
    if(ostrees.find(k) == ostrees.end())
    {
        std::cout << "No OSTree for k = " << k << std::endl;
        return ;
    }    
    ostrees.at(k).display();
}

void CoreMaintainer::printCores() const
{
    for(const auto& core : cores)
    {
        std::cout << "Core of vertex " << core.first << " is " << core.second << std::endl;
    }
    std::cout << std::endl;
}