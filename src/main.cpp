#include <iostream>

#include "./avlTree/avlTree.h"
#include "./util/common.h"
#include "./semiIndexExtractor/semiIndexExtractor.h"

int main(int argc, char* argv[])
{
    cmdOptions options = parseCmdLineArgs(argc, argv);

    Graph graph;
    semiIndexExtractor extractor;
    
    graph.loadGraphfromFile(options.filename);
    std::cout << "Graph Vertex Num : " << graph.getVertexNum() << std::endl;

    extractor.coresDecomposition(graph);
    extractor.printCores();

    return 0;
}