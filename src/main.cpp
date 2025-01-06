#include <iostream>

#include "./util/common.h"
#include "./semiIndexExtractor/semiIndexExtractor.h"

int main(int argc, char* argv[])
{
    cmdOptions options = parseCmdLineArgs(argc, argv);

    Graph graph;
    semiIndexExtractor extractor;
    
    graph.loadGraphfromFile(options.filename);
    extractor.buildMbpTree(graph, options.maxcapacity);
    // std::cout << "Graph Vertex Num : " << graph.getVertexNum() << std::endl;

    extractor.coresDecomposition(graph);
    // extractor.printCores();
    extractor.kcoreExtract(graph, options.query, options.k);

    Graph resultGraph;
    unsigned char vertifyDigest[SHA256_DIGEST_LENGTH];
    std::queue<VOEntry> queueVO = convertVectorToQueue(extractor.getVO());
    extractor.vertify(resultGraph, queueVO, vertifyDigest);
    extractor.calculateVOSize();

    return 0;
}