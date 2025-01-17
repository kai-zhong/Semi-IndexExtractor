#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <list>
#include <random>

#include "../graph/graph.h"
#include "../graph/vertex.h"
#include "../mbptree/mbpnode.h"
#include "../mbptree/mbptree.h"
#include "../maintainer/coremaintainer.h"
#include "../maintainer/shelltree.h"
#include "../util/common.h"
#include "../configuration/types.h"

class Vertex;
class Graph;
class MbpTree;
class OSTree;
class CoreMaintainer;
class ShellTree;

class semiIndexExtractor
{
    private:
        // std::unordered_map<VertexID, uint> cores;
        CoreMaintainer coremaintainer;
        ShellTree* shellTree;

        std::vector<VertexID> candVertices;
        Graph candGraph;

        std::map<uint, std::list<VertexID>> liIndex;
        std::unordered_map<VertexID, uint> liIndexExists;

        bool answerExists;

        MbpTree* mbptree;
        std::vector<VOEntry> vo;
    public:
        semiIndexExtractor();
        ~semiIndexExtractor();
        
        void buildMbpTree(const Graph& graph, const uint maxcapacity);
        void mbpTreeDigestCompute();
        void mbpTreeAddUpdate(const Vertex& src, const Vertex& dst);
        void mbpTreeDeleteEdgeUpdate(const Vertex& v); // 节点未被删除，但是节点信息发生改变，需要更新节点的摘要
        void mbpTreeDeleteVertexUpdate(const VertexID& vid); // 节点被删除，需要删除mbp树中该节点的摘要

        std::map<VertexID, std::string> serializeGraphInfo(const Graph& graph, const Graph& subgraph, std::vector<VertexID>& subgraphVids);

        void constructVO(const Graph& G, const Graph& subgraph);

        void getRootDigest(unsigned char* _digest);

        void vertify(Graph& subgraph, std::queue<VOEntry>& VO, unsigned char* partdigest);

        size_t calculateVOSize();

        const std::vector<VOEntry>& getVO() const;

        uint getCore(const VertexID& vid) const;

        void insertCoreUpdate(const Graph& graph, const VertexID& src, const VertexID& dst);

        void removeCoreUpdate(const Graph& graph, const VertexID& src, const VertexID& dst);

        void coresDecomposition(const Graph& graph);

        void candidateGeneration(const Graph& graph, const VertexID& queryV, const uint& k);

        void globalExtract(const VertexID& queryV, const uint& k);

        Graph kcoreExtract(const Graph& graph, const VertexID& queryV, const uint& k);

        // Graph kcoreExtractByShell(const Graph& graph, const VertexID& queryV, const uint& k);
        std::chrono::milliseconds kcoreExtractByShell(const Graph& graph, const VertexID& queryV, const uint& k);

        VertexID getLiIndexTop() const;

        void liIndexPop();

        void initLiIndex(const VertexID& queryV);

        void insertToLiIndex(const Graph& graph, const VertexID& vid, std::unordered_map<VertexID, bool>& visted, const uint& k);

        bool isLiIndexEmpty() const;

        const Graph& getCandGraph() const;

        void buildShellTree(const Graph& graph);

        void getTestData(const Graph& graph);

        void printVO() const;

        void printLiIndex() const;

        void printCores() const;
};