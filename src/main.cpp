#include <iostream>

#include "./util/common.h"
#include "./util/cmdline.h"
#include "./util/edgeReader.h"
#include "./semiIndexExtractor/semiIndexExtractor.h"

const int addBatchNum = 10000;
const int delBatchNum = 10000;
const int queryNum = 10;

int main(int argc, char* argv[])
{
    cmdOptions options = parseCmdLineArgs(argc, argv);

    std::ofstream dataFile(options.experimentFilePath);
    if(!dataFile.is_open())
    {
        std::cerr << "Failed to open file: " << options.experimentFilePath << std::endl;
        throw std::runtime_error("Failed to open file");
    }

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
    // extractor.buildShellTree(graph);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Core Decomposition Time taken: " << duration.count() << " ms" << std::endl << std::endl;

    size_t cnt = 0;
    auto maxAddDuration = std::chrono::milliseconds(0);
    auto minAddDuration = std::chrono::milliseconds(1000000000);
    auto totalAddDuration = std::chrono::milliseconds(0);
    while(!addEdgeReader.isEndOfFile())
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
        // extractor.buildShellTree(graph); // 可以增删结束后统一计算
        extractor.mbpTreeDigestCompute();
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Add [" << addEdges.size() << "] Edges Time taken: " << duration.count() << " ms" << std::endl << std::endl;
        maxAddDuration = std::max(maxAddDuration, duration);
        minAddDuration = std::min(minAddDuration, duration);
        totalAddDuration += duration;
    }
    std::cout << "Add Edges Max Time taken: " << maxAddDuration.count() << " ms" << std::endl;
    std::cout << "Add Edges Min Time taken: " << minAddDuration.count() << " ms" << std::endl;
    std::cout << "Add Edges Total Time taken: " << totalAddDuration.count() << " ms" << std::endl;
    std::cout << "Add Edges Avg Time taken: " << totalAddDuration.count() / cnt << " ms" << std::endl << std::endl;

    dataFile << "Add " << addBatchNum << "*" << cnt << " Edges Data: " << std::endl;
    dataFile << "   Max Time taken: " << maxAddDuration.count() << " ms" << std::endl;
    dataFile << "   Min Time taken: " << minAddDuration.count() << " ms" << std::endl;
    dataFile << "   Total Time taken: " << totalAddDuration.count() << " ms" << std::endl;
    dataFile << "   Avg Time taken: " << totalAddDuration.count() / cnt << " ms" << std::endl << std::endl;

    // 开始query
    extractor.coresDecomposition(graph);
    start = std::chrono::high_resolution_clock::now();
    extractor.buildShellTree(graph);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Shell Tree building Time taken: " << duration.count() << " ms" << std::endl << std::endl;
    dataFile << "Shell Tree building Time taken: " << duration.count() << " ms" << std::endl << std::endl;

    for(const std::pair<uint, std::vector<VertexID>>& p : options.queryMap)
    {
        uint queryK = p.first;
        std::vector<VertexID> querys = p.second;

        auto kcoreExtractTotalTime = std::chrono::milliseconds(0);
        auto kcoreExtractMaxTime = std::chrono::milliseconds(0);
        auto kcoreExtractMinTime = std::chrono::milliseconds(1000000000);

        auto vertifyTotalTime = std::chrono::milliseconds(0);
        auto vertifyMaxTime = std::chrono::milliseconds(0);
        auto vertifyMinTime = std::chrono::milliseconds(1000000000);

        uint resultMaxVNum = 0;
        uint resultMinVNum = 1000000000;
        double resultAvgVNum = 0;

        double resultMaxVOSize = 0;
        double resultMinVOSize = std::numeric_limits<double>::max();
        double resultAvgVOSize = 0;

        std::cout << "Query " << queryK << " : " << std::endl;
        uint num = 0;
        for(VertexID q : querys)
        {
            if(num >= queryNum)
            {
                break;
            }
            std::cout << "Query K = " << queryK << ", number = " << ++num << std::endl;
            // k-core子图提取
            Graph kcoreGraph = extractor.kcoreExtract(graph, q, queryK);
            duration = extractor.kcoreExtractByShell(graph, q, queryK);
            std::cout << "Computation Time taken: " << duration.count() << " ms" << std::endl << std::endl;
            kcoreExtractMaxTime = std::max(kcoreExtractMaxTime, duration);
            kcoreExtractMinTime = std::min(kcoreExtractMinTime, duration);
            kcoreExtractTotalTime += duration;
            // VO验证
            Graph resultGraph;
            unsigned char vertifyDigest[SHA256_DIGEST_LENGTH];
            std::queue<VOEntry> queueVO = convertVectorToQueue(extractor.getVO());
            start = std::chrono::high_resolution_clock::now();
            extractor.vertify(resultGraph, queueVO, vertifyDigest);
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Vertify Time taken: " << duration.count() << " ms" << std::endl << std::endl;
            vertifyMaxTime = std::max(vertifyMaxTime, duration);
            vertifyMinTime = std::min(vertifyMinTime, duration);
            vertifyTotalTime += duration;
            // VO大小计算
            size_t voSize = extractor.calculateVOSize();
            std::cout << "Total size of vo:" << std::endl;
            std::cout << "  Bytes: " << voSize << " B" << std::endl;
            std::cout << "  Kilobytes: " << voSize / 1024.0 << " KB" << std::endl;
            std::cout << "  Megabytes: " << voSize / (1024.0 * 1024.0) << " MB" << std::endl;
            std::cout << std::endl;
            resultMaxVOSize = std::max(resultMaxVOSize, (double)voSize);
            resultMinVOSize = std::min(resultMinVOSize, (double)voSize);
            resultAvgVOSize += (double)voSize / (double)queryNum;

            resultMaxVNum = std::max(resultMaxVNum, kcoreGraph.getVertexNum());
            resultMinVNum = std::min(resultMinVNum, kcoreGraph.getVertexNum());
            resultAvgVNum += kcoreGraph.getVertexNum() / (double)queryNum;
            std::cout << "Result Graph Vertex Num : " << kcoreGraph.getVertexNum() << std::endl;
            std::cout << std::endl;
        }
        std::cout << "Query k = " << queryK << " : " << std::endl;
        std::cout << "  Kcore Extract Max Time taken: " << kcoreExtractMaxTime.count() << " ms" << std::endl;
        std::cout << "  Kcore Extract Min Time taken: " << kcoreExtractMinTime.count() << " ms" << std::endl;
        std::cout << "  Kcore Extract Avg Time taken: " << kcoreExtractTotalTime.count() / queryNum << " ms" << std::endl << std::endl;
        std::cout << "  Vertify Max Time taken: " << vertifyMaxTime.count() << " ms" << std::endl;
        std::cout << "  Vertify Min Time taken: " << vertifyMinTime.count() << " ms" << std::endl;
        std::cout << "  Vertify Avg Time taken: " << vertifyTotalTime.count() / queryNum << " ms" << std::endl << std::endl;
        std::cout << "  VO Max Size: " << resultMaxVOSize / (1024.0 * 1024.0) << " MB" << std::endl;
        std::cout << "  VO Min Size: " << resultMinVOSize / (1024.0 * 1024.0) << " MB" << std::endl;
        std::cout << "  VO Avg Size: " << resultAvgVOSize / (1024.0 * 1024.0) << " MB" << std::endl << std::endl;
        std::cout << "  Result Graph Max Vertex Num : " << resultMaxVNum << std::endl;
        std::cout << "  Result Graph Min Vertex Num : " << resultMinVNum << std::endl;
        std::cout << "  Result Graph Avg Vertex Num : " << resultAvgVNum << std::endl;
        std::cout << std::endl;

        dataFile << "Query " << queryK << " Data: " << std::endl;
        dataFile << "  Kcore Extract Max Time taken: " << kcoreExtractMaxTime.count() << " ms" << std::endl;
        dataFile << "  Kcore Extract Min Time taken: " << kcoreExtractMinTime.count() << " ms" << std::endl;
        dataFile << "  Kcore Extract Avg Time taken: " << kcoreExtractTotalTime.count() / queryNum << " ms" << std::endl << std::endl;
        dataFile << "  Vertify Max Time taken: " << vertifyMaxTime.count() << " ms" << std::endl;
        dataFile << "  Vertify Min Time taken: " << vertifyMinTime.count() << " ms" << std::endl;
        dataFile << "  Vertify Avg Time taken: " << vertifyTotalTime.count() / queryNum << " ms" << std::endl << std::endl;
        dataFile << "  VO Max Size: " << resultMaxVOSize << "B | " << resultMaxVOSize / 1024.0 << "KB | " << resultMaxVOSize / (1024.0 * 1024.0) << " MB" << std::endl;
        dataFile << "  VO Min Size: " << resultMinVOSize << "B | " << resultMinVOSize / 1024.0 << "KB | " << resultMinVOSize / (1024.0 * 1024.0) << " MB" << std::endl;
        dataFile << "  VO Avg Size: " << resultAvgVOSize << "B | " << resultAvgVOSize / 1024.0 << "KB | " << resultAvgVOSize / (1024.0 * 1024.0) << " MB" << std::endl << std::endl;
        dataFile << "  Result Graph Max Vertex Num : " << resultMaxVNum << std::endl;
        dataFile << "  Result Graph Min Vertex Num : " << resultMinVNum << std::endl;
        dataFile << "  Result Graph Avg Vertex Num : " << resultAvgVNum << std::endl;
        dataFile << std::endl;
    }


    cnt = 0;
    auto maxDelDuration = std::chrono::milliseconds(0);
    auto minDelDuration = std::chrono::milliseconds(1000000000);
    auto totalDelDuration = std::chrono::milliseconds(0);
    while(!addEdgeReader.isEndOfFile() || !delEdgeReader.isEndOfFile())
    {
        std::cout << ++cnt << "th round of delete edges : " << std::endl;
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
        maxDelDuration = std::max(maxDelDuration, duration);
        minDelDuration = std::min(minDelDuration, duration);
        totalDelDuration += duration;
    }
    std::cout << "Delete Edges Max Time taken: " << maxDelDuration.count() << " ms" << std::endl;
    std::cout << "Delete Edges Min Time taken: " << minDelDuration.count() << " ms" << std::endl;
    std::cout << "Delete Edges Total Time taken: " << totalDelDuration.count() << " ms" << std::endl;
    std::cout << "Delete Edges Avg Time taken: " << totalDelDuration.count() / cnt << " ms" << std::endl << std::endl;

    dataFile << "Delete " << delBatchNum << "*" << cnt << " Edges Data: " << std::endl;
    dataFile << "   Max Time taken: " << maxDelDuration.count() << " ms" << std::endl;
    dataFile << "   Min Time taken: " << minDelDuration.count() << " ms" << std::endl;
    dataFile << "   Total Time taken: " << totalDelDuration.count() << " ms" << std::endl;
    dataFile << "   Avg Time taken: " << totalDelDuration.count() / cnt << " ms" << std::endl << std::endl;

    dataFile.close();
    return 0;
}





// int main(int argc, char* argv[])
// {
//     cmdOptions options = parseCmdLineArgs(argc, argv);

//     Graph graph;
//     semiIndexExtractor extractor;
//     EdgeReader addEdgeReader(options.addFilename);
//     EdgeReader delEdgeReader(options.deleteFilename);

//     // 图加载
//     graph.loadGraphfromFile(options.filename);
//     std::cout << "Graph Vertex Num : " << graph.getVertexNum() << std::endl;

//     // MbpTree构建
//     auto start = std::chrono::high_resolution_clock::now();
//     extractor.buildMbpTree(graph, options.maxcapacity);
//     auto end = std::chrono::high_resolution_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//     std::cout << "Tree building Time taken: " << duration.count() << " ms" << std::endl << std::endl;

//     // core decomposition
//     start = std::chrono::high_resolution_clock::now();
//     extractor.coresDecomposition(graph);
//     // extractor.buildShellTree(graph);
//     end = std::chrono::high_resolution_clock::now();
//     duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//     std::cout << "Core Decomposition Time taken: " << duration.count() << " ms" << std::endl << std::endl;

//     size_t cnt = 0;
//     while(!addEdgeReader.isEndOfFile() || !delEdgeReader.isEndOfFile())
//     {
//         std::cout << ++cnt << "th round of updating edges : " << std::endl;
//         std::vector<std::pair<VertexID, VertexID>> addEdges = addEdgeReader.readNextEdges(addBatchNum);
//         start = std::chrono::high_resolution_clock::now();
//         for(auto edge : addEdges)
//         {
//             VertexID srcVid = edge.first;
//             VertexID dstVid = edge.second;
//             graph.addEdge(srcVid, dstVid, true, true);
//             extractor.mbpTreeAddUpdate(graph.getVertex(srcVid), graph.getVertex(dstVid));
//             extractor.insertCoreUpdate(graph, srcVid, dstVid);
//         }
//         // extractor.buildShellTree(graph); // 可以增删结束后统一计算
//         extractor.mbpTreeDigestCompute();
//         end = std::chrono::high_resolution_clock::now();
//         duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//         std::cout << "Add [" << addEdges.size() << "] Edges Time taken: " << duration.count() << " ms" << std::endl << std::endl;

//         // extractor.coresDecomposition(graph);

//         std::vector<std::pair<VertexID, VertexID>> delEdges = delEdgeReader.readNextEdges(delBatchNum);
//         start = std::chrono::high_resolution_clock::now();
//         for(auto edge : delEdges)
//         {
//             VertexID srcVid = edge.first;
//             VertexID dstVid = edge.second;
//             graph.removeEdge(srcVid, dstVid, true, true);
//             bool srcExists = graph.hasVertex(srcVid);
//             bool dstExists = graph.hasVertex(dstVid);
//             if(!srcExists)
//             {
//                 extractor.mbpTreeDeleteVertexUpdate(srcVid);
//             }
//             else
//             {
//                 extractor.mbpTreeDeleteEdgeUpdate(graph.getVertex(srcVid));
//             }
//             if(!dstExists)
//             {
//                 extractor.mbpTreeDeleteVertexUpdate(dstVid);
//             }
//             else
//             {
//                 extractor.mbpTreeDeleteEdgeUpdate(graph.getVertex(dstVid));
//             }
//             extractor.removeCoreUpdate(graph, srcVid, dstVid);
//         }
//         extractor.mbpTreeDigestCompute();
//         // extractor.buildShellTree(graph); // 可以增删结束后统一计算
//         end = std::chrono::high_resolution_clock::now();
//         duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//         std::cout << "Delete [" << delEdges.size() << "] Edges Time taken: " << duration.count() << " ms" << std::endl << std::endl;

//         extractor.coresDecomposition(graph);
//         // extractor.buildShellTree(graph);
//         // k-core子图提取
//         start = std::chrono::high_resolution_clock::now();
//         Graph kcoreGraph = extractor.kcoreExtract(graph, options.query, options.k);
//         // Graph kcoreGraph = extractor.kcoreExtractByShell(graph, options.query, options.k);
//         end = std::chrono::high_resolution_clock::now();
//         duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//         std::cout << "Computation Time taken: " << duration.count() << " ms" << std::endl << std::endl;

//         // VO验证
//         Graph resultGraph;
//         unsigned char vertifyDigest[SHA256_DIGEST_LENGTH];
//         std::queue<VOEntry> queueVO = convertVectorToQueue(extractor.getVO());
//         start = std::chrono::high_resolution_clock::now();
//         extractor.vertify(resultGraph, queueVO, vertifyDigest);
//         end = std::chrono::high_resolution_clock::now();
//         duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//         std::cout << "Vertify Time taken: " << duration.count() << " ms" << std::endl << std::endl;

//         // VO大小计算
//         extractor.calculateVOSize();
//         std::cout << "Result Graph Vertex Num : " << kcoreGraph.getVertexNum() << std::endl;
//         std::cout << std::endl;
//     }
//     return 0;
// }