#include "maintainer/shelltree.h"

ShellTree::ShellTree(){}

ShellTree::~ShellTree()
{
    for(std::unordered_map<uint, std::vector<ShellNode*>>::iterator it = shellNodes.begin(); it!= shellNodes.end(); it++)
    {
        std::vector<ShellNode*> nodes = it->second;
        for(std::vector<ShellNode*>::iterator it2 = nodes.begin(); it2!= nodes.end(); it2++)
        {
            ShellNode* node = *it2;
            if(node != nullptr)
            {
                delete node;
            }
        }
    }
}

void ShellTree::dfs(const Graph& graph, const VertexID& u, ShellNode* component, const uint& componentID, const std::unordered_map<VertexID, uint>& cores, std::unordered_set<VertexID>& visited)
{
    visited.emplace(u);
    component->vertices.emplace_back(u);
    vertexToCC[u] = componentID;

    for(const VertexID& v : graph.getVertexNeighbors(u))
    {
        if(visited.find(v) == visited.end() && cores.at(v) >= component->coreLevel)
        {
            dfs(graph, v, component, componentID, cores, visited);
        }
    }
}

void ShellTree::initBottomNodes(const Graph& graph, const std::unordered_map<VertexID, uint>& cores, const std::vector<VertexID>& bottomSets, const uint& bottomLevel)
{
    std::unordered_set<VertexID> visited;
    uint componentID = 0;

    for(VertexID v : bottomSets)
    {
        if(visited.find(v) == visited.end())
        {
            ShellNode* newNode = new ShellNode(componentID, bottomLevel, nullptr);
            dfs(graph, v, newNode, componentID, cores, visited);
            shellNodes[bottomLevel].emplace_back(newNode);
            componentID++;
        }
    }
}

void ShellTree::buildTree(const Graph& graph, const std::unordered_map<VertexID, uint>& cores)
{
    std::map<uint, std::vector<VertexID>, std::greater<uint>> CSets;
    // 构建每个core对应的集合
    for(const std::pair<VertexID, uint>& p : cores)
    {
        CSets[p.second].emplace_back(p.first);
    }
    initBottomNodes(graph, cores, CSets.begin()->second, CSets.begin()->first);
    for(auto it = std::next(CSets.begin(), 1); it!= CSets.end(); it++)
    {
        uint level = it->first;
        std::vector<VertexID>& S = it->second;

        for(const VertexID& v : S)
        {
            std::unordered_set<ShellNode*> T; // 要记得加上v
            std::unordered_set<VertexID> visited;
            std::unordered_set<ShellNode*> parents; 
            for(const VertexID& neighbor : graph.getVertexNeighbors(v))
            {
                if(visited.find(neighbor) != visited.end())
                {
                    T.insert(shellNodes[level][vertexToCC[neighbor]]);
                    parents.insert(shellNodes[level][vertexToCC[neighbor]]);
                }
                else if(vertexToCC.find(neighbor) != vertexToCC.end())
                {
                    ShellNode* neighborCC = shellNodes[cores.at(neighbor)][vertexToCC[neighbor]];
                    while(neighborCC->parent != nullptr)
                    {
                        neighborCC = neighborCC->parent;
                    }
                    T.insert(neighborCC);
                    if(neighborCC->coreLevel == level)
                    {
                        parents.insert(neighborCC);
                    }
                }
            }
            visited.emplace(v);

            if(parents.size() == 1) // 找到唯一的父节点,只需要将v加入到父节点的集合中即可
            {
                ShellNode* parent = *parents.begin();
                parent->vertices.emplace_back(v);
                vertexToCC[v] = parent->id;
                for(ShellNode* t : T)
                {
                    if(t->id != parent->id)
                    {
                        if(t->parent != nullptr)
                        {
                            t->parent->children.erase(t);
                        }
                        t->parent = parent;
                        parent->children.insert(t);
                    }
                }
            }
            else // 找到多个父节点或无父节点,需要新建一个节点作为这些父节点的父节点
            {
                ShellNode* newNode = new ShellNode(shellNodes[level].size(), level, nullptr);
                newNode->vertices.emplace_back(v);
                vertexToCC[v] = newNode->id;
                shellNodes[level].emplace_back(newNode);
                for(ShellNode* t : T)
                {
                    if(t->parent != nullptr)
                    {
                        t->parent->children.erase(t);
                    }
                    t->parent = newNode;
                    newNode->children.insert(t);
                }
            }
        }
    }
}

void ShellTree::traverseShellNode(ShellNode* node, std::unordered_set<VertexID>& vertices)
{
    for(const VertexID& v : node->vertices)
    {
        vertices.emplace(v);
    }

    for(ShellNode* child : node->children)
    {
        traverseShellNode(child, vertices);
    }
}

std::chrono::milliseconds ShellTree::query(const Graph& graph, const VertexID& queryV, const uint& queryVCore, const uint& k)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::unordered_set<VertexID> vertexset;
    uint CCID = vertexToCC.at(queryV);
    ShellNode* queryCC = shellNodes.at(queryVCore).at(CCID);

    if(queryVCore < k)
    {
        std::cerr << "No satisfied " << k << "-core graph for Vertex "<< queryV << std::endl;
        // throw std::runtime_error("No satisfied k-core graph for Vertex");
        auto midEnd = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(midEnd - start);
        std::cout << "query time: " << duration.count() << "ms" << std::endl;
        return duration;
    }

    ShellNode* curNode = queryCC;
    while(curNode->coreLevel >= k && curNode->parent != nullptr)
    {
        curNode = curNode->parent;
    }

    traverseShellNode(curNode, vertexset);

    std::cout << "vertexset size: " << vertexset.size() << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "query time: " << duration.count() << "ms" << std::endl;

    return duration;

    // Graph kcoreGraph;
    // // for(const VertexID& v : vertexset)
    // // {
    // //     for(const VertexID& neighbor : graph.getVertexNeighbors(v))
    // //     {
    // //         if(vertexset.find(neighbor) != vertexset.end())
    // //         {
    // //             kcoreGraph.addEdge(v, neighbor, false, false);
    // //         }
    // //     }
    // // }
    // // kcoreGraph.computeVertexDigest();
    // return kcoreGraph;
}

// Graph ShellTree::query(const Graph& graph, const VertexID& queryV, const uint& queryVCore, const uint& k)
// {
//     auto start = std::chrono::high_resolution_clock::now();
//     std::unordered_set<VertexID> vertexset;
//     uint CCID = vertexToCC.at(queryV);
//     ShellNode* queryCC = shellNodes.at(queryVCore).at(CCID);

//     if(queryVCore < k)
//     {
//         std::cerr << "No satisfied " << k << "-core graph for Vertex "<< queryV << std::endl;
//         throw std::runtime_error("No satisfied k-core graph for Vertex");
//     }

//     ShellNode* curNode = queryCC;
//     while(curNode->coreLevel >= k && curNode->parent != nullptr)
//     {
//         curNode = curNode->parent;
//     }

//     traverseShellNode(curNode, vertexset);

//     auto end = std::chrono::high_resolution_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//     std::cout << "query time: " << duration.count() << "ms" << std::endl;

//     Graph kcoreGraph;
//     // for(const VertexID& v : vertexset)
//     // {
//     //     for(const VertexID& neighbor : graph.getVertexNeighbors(v))
//     //     {
//     //         if(vertexset.find(neighbor) != vertexset.end())
//     //         {
//     //             kcoreGraph.addEdge(v, neighbor, false, false);
//     //         }
//     //     }
//     // }
//     // kcoreGraph.computeVertexDigest();
//     return kcoreGraph;
// }