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
    auto start = std::chrono::high_resolution_clock::now();
    extractor.kcoreExtract(graph, options.query, options.k);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl << std::endl;

    Graph resultGraph;
    unsigned char vertifyDigest[SHA256_DIGEST_LENGTH];
    std::queue<VOEntry> queueVO = convertVectorToQueue(extractor.getVO());
    extractor.vertify(resultGraph, queueVO, vertifyDigest);
    extractor.calculateVOSize();

    std::cout << "Result Graph Vertex Num : " << resultGraph.getVertexNum() << std::endl;

    return 0;
}