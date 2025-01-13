#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <chrono>

#include "../configuration/types.h"
#include "../configuration/config.h"
#include "../graph/vertex.h"
#include "../graph/graph.h"
#include "../ostree/ostree.h"

class Vertex;
class Graph;
class OSTree;

struct MinHeapCmp
{
    bool operator()(const std::pair<uint, VertexID>& a, const std::pair<uint, VertexID>& b) const
    {
        return a.first > b.first;
    }
};

class CoreMaintainer
{
    private:
        std::unordered_map<VertexID, uint> cores;
        std::unordered_map<VertexID, uint> mcd;
        std::unordered_map<VertexID, uint> degPlus;
        std::unordered_map<uint, OSTree> ostrees;
        // std::unordered_map<uint, std::list<VertexID>> orderkV;

        uint version = 0;
    public:
        CoreMaintainer();
        CoreMaintainer(const Graph& graph);

        uint getCore(const VertexID& vid) const;

        void insertToOrderk(const std::vector<VertexID>& vert, const std::vector<VertexID> local2global, uint startPos, uint endPos, uint k);
        void initmcd(const Graph& graph, const std::vector<VertexID> local2global);
        void coresDecomp(const Graph& graph);

        bool comparekorder(const uint& a, const uint& b, const uint& k);

        void orderInsert(const Graph& graph, const VertexID src, const VertexID dst);
        void addVertex(const VertexID& vid);
        void removeCandidates(const Graph& graph, std::unordered_map<VertexID, uint>& degStar, std::unordered_map<VertexID, uint>& inVc, std::unordered_set<VertexID>& inHeap, VertexID w, uint K);
        void updatemcdInsert(const Graph& graph, const std::vector<VertexID>& VStar, uint K); // 插入和删除更新操作不同
        
        void orderRemove(const Graph& graph, const VertexID src, const VertexID dst);
        void removeVertex(const VertexID& vid);
        void traverseVStarFind(const Graph& graph, std::vector<VertexID>& VStar, std::unordered_map<VertexID, uint>& inVStar, const VertexID& src, const VertexID& dst, uint K);
        void updatemcdRemove(const Graph& graph, const std::vector<VertexID>& VStar, const std::unordered_map<VertexID, uint>& inVStar, uint K);

        // void printOrderk(uint k) const;
        void printOSTree(uint k) const;
        void printCores() const;
};