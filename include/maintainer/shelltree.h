#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <chrono>

#include "../configuration/types.h"
#include "../configuration/config.h"
#include "../graph/vertex.h"
#include "../graph/graph.h"

class Vertex;
class Graph;

struct ShellNode
{
    uint id;
    uint coreLevel;
    std::vector<VertexID> vertices;
    ShellNode* parent;
    std::unordered_set<ShellNode*> children;
    ShellNode(uint _id, uint _coreLevel, ShellNode* _parent = nullptr) : id(_id), coreLevel(_coreLevel), parent(_parent) {}
};

class ShellTree
{
    private:
        std::unordered_map<uint, std::vector<ShellNode*>> shellNodes;
        std::unordered_map<VertexID, uint> vertexToCC;

    public:
        ShellTree();
        ~ShellTree();

        void dfs(const Graph& graph, const VertexID& u, ShellNode* component, const uint& componentID, const std::unordered_map<VertexID, uint>& cores, std::unordered_set<VertexID>& visited);
        void initBottomNodes(const Graph& graph, const std::unordered_map<VertexID, uint>& cores, const std::vector<VertexID>& bottomSets, const uint& bottomLevel);
        void buildTree(const Graph& graph, const std::unordered_map<VertexID, uint>& cores);

        void traverseShellNode(ShellNode* node, std::unordered_set<VertexID>& vertices);
        // Graph query(const Graph& graph, const VertexID& queryV, const uint& queryVCore, const uint& k);
        std::chrono::milliseconds query(const Graph& graph, const VertexID& queryV, const uint& queryVCore, const uint& k);
};