#include <iostream>

#include "./util/common.h"
#include "./util/cmdline.h"
#include "./util/edgeReader.h"
#include "./semiIndexExtractor/semiIndexExtractor.h"

const int addBatchNum = 10000;
const int delBatchNum = 5000;

int main(int argc, char* argv[])
{
    cmdOptions options = parseCmdLineArgs(argc, argv);

    Graph graph;
    semiIndexExtractor extractor;
    EdgeReader addEdgeReader(options.addFilename);
    EdgeReader delEdgeReader(options.deleteFilename);

    // 图加载
    graph.loadGraphfromFile(options.filename);
    std::cout << "Graph Vertex Num : " << graph.getVertexNum() << std::endl;

    // MbpTree构建
    auto start = std::chrono::high_resolution_clock::now();
    extractor.buildMbpTree(graph, options.maxcapacity);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Tree building Time taken: " << duration.count() << " ms" << std::endl << std::endl;

    // core decomposition
    start = std::chrono::high_resolution_clock::now();
    extractor.coresDecomposition(graph);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Core Decomposition Time taken: " << duration.count() << " ms" << std::endl << std::endl;

    size_t cnt = 0;
    while(!addEdgeReader.isEndOfFile() || !delEdgeReader.isEndOfFile())
    {
        std::cout << ++cnt << "th round of updating edges : " << std::endl;
        std::vector<std::pair<VertexID, VertexID>> addEdges = addEdgeReader.readNextEdges(addBatchNum);
        start = std::chrono::high_resolution_clock::now();
        for(auto edge : addEdges)
        {
            VertexID srcVid = edge.first;
            VertexID dstVid = edge.second;
            graph.addEdge(srcVid, dstVid, true, true);
            extractor.mbpTreeAddUpdate(graph.getVertex(srcVid), graph.getVertex(dstVid));
            extractor.insertCoreUpdate(graph, srcVid, dstVid);
        }
        extractor.mbpTreeDigestCompute();
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Add [" << addEdges.size() << "] Edges Time taken: " << duration.count() << " ms" << std::endl << std::endl;

        std::vector<std::pair<VertexID, VertexID>> delEdges = delEdgeReader.readNextEdges(delBatchNum);
        start = std::chrono::high_resolution_clock::now();
        for(auto edge : delEdges)
        {
            VertexID srcVid = edge.first;
            VertexID dstVid = edge.second;
            graph.removeEdge(srcVid, dstVid, true, true);
            bool srcExists = graph.hasVertex(srcVid);
            bool dstExists = graph.hasVertex(dstVid);
            if(!srcExists)
            {
                extractor.mbpTreeDeleteVertexUpdate(srcVid);
            }
            else
            {
                extractor.mbpTreeDeleteEdgeUpdate(graph.getVertex(srcVid));
            }
            if(!dstExists)
            {
                extractor.mbpTreeDeleteVertexUpdate(dstVid);
            }
            else
            {
                extractor.mbpTreeDeleteEdgeUpdate(graph.getVertex(dstVid));
            }
            extractor.removeCoreUpdate(graph, srcVid, dstVid);
        }
        extractor.mbpTreeDigestCompute();
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Delete [" << delEdges.size() << "] Edges Time taken: " << duration.count() << " ms" << std::endl << std::endl;

        // k-core子图提取
        start = std::chrono::high_resolution_clock::now();
        Graph kcoreGraph = extractor.kcoreExtract(graph, options.query, options.k);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Computation Time taken: " << duration.count() << " ms" << std::endl << std::endl;

        // VO验证
        Graph resultGraph;
        unsigned char vertifyDigest[SHA256_DIGEST_LENGTH];
        std::queue<VOEntry> queueVO = convertVectorToQueue(extractor.getVO());
        start = std::chrono::high_resolution_clock::now();
        extractor.vertify(resultGraph, queueVO, vertifyDigest);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Vertify Time taken: " << duration.count() << " ms" << std::endl << std::endl;

        // VO大小计算
        extractor.calculateVOSize();
        std::cout << "Result Graph Vertex Num : " << kcoreGraph.getVertexNum() << std::endl;
        std::cout << std::endl;
    }
    return 0;
}