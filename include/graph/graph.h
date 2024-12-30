#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <queue>
#include <string>
#include <climits>
#include <fstream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <map>
#include <array>
#include "../configuration/types.h"
#include "../configuration/config.h"
#include "vertex.h"

class Vertex;

class Graph 
{
    private:
        uint vertex_num;
        uint edge_num; // 因为是无向图，所以一条边只算一次，边（0，1）和边（1，0）不会重复计算，只添加一次
        std::map<VertexID, Vertex> nodes;
        std::map<uint, std::list<VertexID>> invertedIndex;

    public:
        Graph();
        ~Graph();

        VertexID getMinDegreeVertexID();

        uint getMinDegreeWithtraversal();

        uint getVertexNum() const;
        uint getEdgeNum() const;
        uint getVertexDegree(const VertexID& vid) const;
        
        Vertex getVertex(const VertexID& vid) const;
        const std::vector<VertexID>& getVertexNeighbors(const VertexID& vid) const;
        std::array<unsigned char, SHA256_DIGEST_LENGTH> getVertexDigest(const VertexID& vid) const;

        const std::map<uint, Vertex>& getNodes() const;

        bool hasVertex(const VertexID& vid) const;

        void loadGraphfromFile(const std::string& filename);
        void writeGraphtoFile(const std::string& filename);

        void addVertex(const VertexID& vid, bool updateIndex, bool computeVDigest);
        void removeVertex(const VertexID& vid, bool updateIndex, bool computeVDigest);
        void addEdge(const VertexID& src, const VertexID& dst, bool updateIndex, bool computeVDigest);
        void removeEdge(const VertexID& src, const VertexID& dst, bool updateIndex, bool computeVDigest);

        void buildInvertedIndex();
        void updateInvertedIndexADV(const VertexID& vid); // 删除节点后更新倒排索引
        void updateInvertedIndexAAV(const VertexID& vid); // 增加节点后更新倒排索引
        void updateInvertedIndexAUE(const VertexID& src, const VertexID& dst); // 更新边（包括删除和增加）后更新倒排索引

        void computeVertexDigest();

        std::vector<VertexID> convertToLocalID() const;

        void printGraphInfo(int verboseNodeNum = -1) const;
        void printGraphInfoSimple(int verboseNodeNum = -1) const;
};