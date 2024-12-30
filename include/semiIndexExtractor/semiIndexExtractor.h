#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>

#include "../graph/graph.h"
#include "../graph/vertex.h"
#include "../util/common.h"
#include "../configuration/types.h"

class Vertex;
class Graph;

class semiIndexExtractor
{
    private:
        std::unordered_map<VertexID, uint> cores;
    public:
        semiIndexExtractor();
        ~semiIndexExtractor();
        
        const uint& getCore(const VertexID& vid) const;

        void coresDecomposition(const Graph& graph);

        void printCores() const;
};