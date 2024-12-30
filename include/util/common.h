#pragma once

#include <string>
#include <chrono>
#include <openssl/sha.h>
#include <iostream>
#include <iomanip>
#include <queue>
#include <sstream>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include "../util/cmdline.h"
#include "../configuration/types.h"
#include "../configuration/config.h"

struct cmdOptions
{
    std::string filename;
    VertexID query;
    uint k;
    uint khop;
    uint maxcapacity;
};

struct VOEntry
{
    enum DataType {NODEDATA, DIGEST, SPECIAL};
    DataType type;
    union
    {
        char* nodeData; // 以节点v的id打头，后面跟着v在子图中的邻居节点，然后以'|'分割，然后跟着v在原图中的邻居节点（除去在子图中的邻居节点）
        unsigned char digest[SHA256_DIGEST_LENGTH];
        char specialChar;
    };

    VOEntry(const std::string& serializedVertexInfo):type(NODEDATA)
    {
        nodeData = new char[serializedVertexInfo.length() + 1];
        std::strcpy(nodeData, serializedVertexInfo.c_str());
    }

    VOEntry(const unsigned char* _digest, size_t length) : type(DIGEST)
    {
        if(length!= SHA256_DIGEST_LENGTH)
        {
            throw std::runtime_error("Invalid digest length!");
        }
        std::memcpy(digest, _digest, SHA256_DIGEST_LENGTH);
    }

    VOEntry(char _specialChar) : type(SPECIAL)
    {
        specialChar = _specialChar;
    }

    VOEntry(const VOEntry& other) : type(other.type)
    {
        switch(type)
        {
            case NODEDATA:
                if(other.nodeData!= nullptr)
                {
                    size_t length = std::strlen(other.nodeData);
                    nodeData = new char[length + 1];
                    std::strcpy(nodeData, other.nodeData);
                    break;
                }
                else
                {
                    nodeData = nullptr;
                }
                break;
            case DIGEST:
                std::memcpy(digest, other.digest, SHA256_DIGEST_LENGTH);
                break;
            case SPECIAL:
                specialChar = other.specialChar;
                break;
            default:
                throw std::runtime_error("Invalid VOEntry type!");
        }
    }

    ~VOEntry()
    {
        if(type == NODEDATA && nodeData!= nullptr)
        {
            delete[] nodeData;
        }
    }

    void printVOEntry() const
    {
        switch(type)
        {
            case NODEDATA:
                if(nodeData!= nullptr)
                {
                    std::cout << nodeData;
                }
                break;
            case DIGEST:
                for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) 
                {
                    printf("%02x", digest[i]);
                }
                break;
            case SPECIAL:
                std::cout << specialChar;
                break;
            default:
                throw std::runtime_error("Invalid VOEntry type!");
        }
    }
};

template<typename T>
uint countCommonElements(const std::vector<T>& vec1, const std::vector<T>& vec2)
{
    std::unordered_set<T> set1(vec1.begin(), vec1.end());

    size_t commonCount = 0;
    for(const auto& element : vec2)
    {
        if(set1.find(element) != set1.end())
        {
            commonCount++;
        }
    }
    return commonCount;
}

cmdOptions parseCmdLineArgs(int argc, char* argv[]);

void digestPrint(const unsigned char* digest);

std::pair<std::string, std::string> splitStringtoTwoParts(const std::string& str, const std::string& delimiter);

void splitString(const std::string& str, const std::string& delimiter, std::vector<VertexID>& result);

std::queue<VOEntry> convertVectorToQueue(const std::vector<VOEntry>& VO);