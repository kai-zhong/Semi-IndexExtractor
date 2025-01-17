#include "util/common.h"


cmdOptions parseCmdLineArgs(int argc, char* argv[])
{
    bool verbose = false;

    cmdline::parser parser;
    parser.add<std::string>("filename", 'f', "Graph data file path", true);
    parser.add<std::string>("addFilename", 'a', "Graph data file path for adding edges", false);
    parser.add<std::string>("deleteFilename", 'd', "Graph data file path for deleting edges", false);
    parser.add<std::string>("experimentFilePath", 'e', "record experiment result file path", false);
    parser.add<VertexID>("query", 'q', "Query vertex ID", false);
    parser.add<uint>("k", 'k', "kmax of k-core subgraph", false);
    parser.add<uint>("khop", 'h', "k-hop neighborhood of query vertex", false, 6);
    parser.add<uint>("maxcapacity", 'c', "Maximum capacity of the Mbptree", false, 16);
    parser.add<std::string>("queryFile", 'Q', "File containing query information", false);


    parser.parse_check(argc, argv);

    cmdOptions options;
    options.filename = parser.get<std::string>("filename");
    options.addFilename = parser.get<std::string>("addFilename");
    options.deleteFilename = parser.get<std::string>("deleteFilename");
    options.experimentFilePath = parser.get<std::string>("experimentFilePath");
    options.query = parser.get<VertexID>("query");
    options.k = parser.get<uint>("k");
    options.khop = parser.get<uint>("khop");
    options.maxcapacity = parser.get<uint>("maxcapacity");
    if(parser.exist("queryFile"))
    {
        std::ifstream inFile(parser.get<std::string>("queryFile"));
        if(!inFile.is_open())
        {
            std::cerr << "Error: cannot open query file: " << parser.get<std::string>("queryFile") << std::endl;
            throw std::runtime_error("Error: cannot open query file");
        }

        std::string line;
        while(std::getline(inFile, line))
        {
            std::istringstream iss(line);
            uint k;
            iss >> k;
            VertexID query;
            std::vector<VertexID> querys;
            while(iss >> query)
            {
                querys.emplace_back(query);
            }
            options.queryMap[k] = querys;
        }
        inFile.close();
    }


    if(verbose)
    {
        std::cout << "file path: "<< options.filename << std::endl;
        std::cout << "add file path: "<< options.addFilename << std::endl;
        std::cout << "delete file path: "<< options.deleteFilename << std::endl;
        std::cout << "query vertex: "<< options.query << std::endl;
        std::cout << "k: " << options.k << std::endl;
        std::cout << "khop: " << options.khop << std::endl;
        std::cout << "max capacity: " << options.maxcapacity << std::endl;
        std::cout << PRINT_SEPARATOR << std::endl;
    }

    return options;
}

void digestPrint(const unsigned char* digest)
{
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        printf("%02x", digest[i]);
    }
    std::cout << std::endl;
}

std::pair<std::string, std::string> splitStringtoTwoParts(const std::string& str, const std::string& delimiter)
{
    size_t pos = str.find(delimiter);
    if(pos != std::string::npos)
    {
        return std::make_pair(str.substr(0, pos), str.substr(pos + delimiter.length()));
    }
    return std::make_pair(str, "");
}

void splitString(const std::string& str, const std::string& delimiter, std::vector<VertexID>& result)
{
    size_t start = 0;
    size_t end = str.find(delimiter);

    while(end != std::string::npos)
    {
        result.emplace_back(std::stoul(str.substr(start, end - start)));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    result.emplace_back(std::stoul(str.substr(start)));
}

std::queue<VOEntry> convertVectorToQueue(const std::vector<VOEntry>& VO)
{
    std::queue<VOEntry> queue;
    for(auto& entry : VO)
    {
        queue.push(entry);
    }
    return queue;
}

