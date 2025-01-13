#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <tuple>
#include <array>
#include "../configuration/types.h"
#include "../configuration/config.h"
#include "../util/common.h"

class MbpNode
{
    private:
        MbpNode* parent;
        MbpNode* next;
        MbpNode* prev;

        unsigned char digest[SHA256_DIGEST_LENGTH]; // 该节点的摘要

        bool isDigestComputed; // 是否已经计算了摘要,或者需要重新计算
        bool isLeaf;

    public:
        std::vector<uint> keys;
        // std::vector<uint> values;
        std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>> vertexDigests;
        std::vector<MbpNode*> children;


        MbpNode(MbpNode* _parent = nullptr, MbpNode* _prev = nullptr, MbpNode* _next = nullptr, bool _isLeaf = false);
        ~MbpNode();

        uint indexofChild(const uint& key) const; // 根据关键字查找其对应的子节点的索引
        uint indexofKey(const uint& key) const; // 根据关键字查找其在节点中的索引
        bool hasKey(const uint& key) const; // 是否存在关键字

        MbpNode* getParent() const; // 获取父节点
        MbpNode* getPrev() const; // 获取前一个节点
        MbpNode* getNext() const; // 获取后一个节点
        MbpNode* getChild(const uint& key) const; // 获取关键字对应的子节点

        void setParent(MbpNode* _parent); // 设置父节点
        void setPrev(MbpNode* _prev); // 设置前一个节点
        void setNext(MbpNode* _next); // 设置后一个节点

        bool isLeafNode() const; // 是否为叶子节点

        void setChild(const uint& key, const std::vector<MbpNode*>& value); // 设置关键字对应的子节点

        std::tuple<uint, MbpNode*, MbpNode*> splitInternal(); // 分裂中间节点，返回中间关键字和分裂后的两个节点
        std::tuple<uint, MbpNode*, MbpNode*> splitLeaf(); // 分裂叶子节点，返回中间关键字和分裂后的两个节点

        // uint get(const uint& key) const; // 根据关键字获取对应的值（仅叶子节点）
        std::array<unsigned char, SHA256_DIGEST_LENGTH> getVertexDigest(const VertexID& vid) const; // 根据关键字获取对应顶点的摘要（仅叶子节点）

        // void set(const uint& key, const uint& value); // 设置关键字及其对应的值（仅叶子节点）
        void setVertexDigest(const VertexID& vid, const std::array<unsigned char, SHA256_DIGEST_LENGTH>& _digest); // 设置关键字对应的顶点的摘要（仅叶子节点）

        void getDigest(unsigned char* _digest); // 获取节点摘要

        void setFalseDigestComputed();

        void digestCompute();

        void constructVO(std::vector<VOEntry>& vo, std::vector<VertexID>& subgraphVids, const std::map<VertexID, std::string>& serializedVertexInfo);

        void printNodeInfo();
};