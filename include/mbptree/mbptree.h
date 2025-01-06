#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <tuple>
#include <string>

#include "mbpnode.h"
#include "../configuration/types.h"
#include "../configuration/config.h"

class MbpNode;

class MbpTree
{
    private:
        MbpNode* root;
        uint maxCapacity;
        uint minCapacity;
        uint depth;

        void deleteTree(MbpNode* node); // 用于析构函数，递归删除子树的辅助函数

    public:
        MbpTree(uint _maxCapaciy = 4);
        ~MbpTree();

        MbpNode* getRoot() const; // 获取根节点

        MbpNode* findLeaf(uint key) const; // 查找包含key的叶子节点

        std::array<unsigned char, SHA256_DIGEST_LENGTH> getVertexDigest(const VertexID& vid) const; // 根据关键字获取对应顶点的摘要
        // uint get(uint key) const; // 获取key对应的值
        void setVertexDigest(const VertexID& vid, const std::array<unsigned char, SHA256_DIGEST_LENGTH>& _digest); // 设置关键字对应的顶点的摘要
        // void set(uint key, uint value); // 设置key对应的值

        void insert(std::tuple<uint, MbpNode*, MbpNode*> result); 

        void removefromLeaf(uint key, MbpNode* node); // 从叶子节点删除key
        void removefromInternal(uint key, MbpNode* node); // 从内部节点删除key

        void borrowKeyfromRightLeaf(MbpNode* node, MbpNode* next);
        void borrowKeyfromLeftLeaf(MbpNode* node, MbpNode* prev);
        void mergeNodewithRightLeaf(MbpNode* node, MbpNode* next);
        void mergeNodewithLeftLeaf(MbpNode* node, MbpNode* prev);

        void borrowKeyfromRightInternal(int posinParent, MbpNode* node, MbpNode* next);
        void borrowKeyfromLeftInternal(int posinParent, MbpNode* node, MbpNode* prev);
        void mergeNodewithRightInternal(int posinParent, MbpNode* node, MbpNode* next);
        void mergeNodewithLeftInternal(int posinParent, MbpNode* node, MbpNode* prev);

        void remove(uint key, MbpNode* node = nullptr);

        void constructVO(std::vector<VOEntry>& vo, std::vector<VertexID> subgraphVids, const std::map<VertexID, std::string>& serializedVertexInfo);

        void printMbpTreeInfo(MbpNode* node = nullptr, std::string _prefix = "", bool _last = true);
};