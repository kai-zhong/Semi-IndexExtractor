#pragma once

#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <memory>
#include <chrono>
#include <vector>

#include "../configuration/types.h"
#include "../configuration/config.h"

struct TreeNode
{
    VertexID vid;
    uint size; // 以当前节点为根的子树大小
    uint height; // 以当前节点为根的子树高度
    std::shared_ptr<TreeNode> left;
    std::shared_ptr<TreeNode> right;
    std::shared_ptr<TreeNode> parent;

    TreeNode(VertexID _vid, std::shared_ptr<TreeNode> _parent = nullptr) : vid(_vid), size(1), height(1), left(nullptr), right(nullptr), parent(_parent) {}
};

class OSTree
{
    private:
        std::shared_ptr<TreeNode> root;
        std::unordered_map<VertexID, std::shared_ptr<TreeNode>> node_map;

    private:
        uint getHeight(std::shared_ptr<TreeNode> node) const;
        int getBalanceFactor(std::shared_ptr<TreeNode> node) const;
        uint getSize(std::shared_ptr<TreeNode> node) const;
        void updateSize(std::shared_ptr<TreeNode> node);
        std::shared_ptr<TreeNode> rightRotate(std::shared_ptr<TreeNode> y);
        std::shared_ptr<TreeNode> leftRotate(std::shared_ptr<TreeNode> x);
        std::shared_ptr<TreeNode> balance(std::shared_ptr<TreeNode> node, bool isFront);
        std::shared_ptr<TreeNode> insertFront(std::shared_ptr<TreeNode> node, VertexID vid, std::shared_ptr<TreeNode> parent);
        std::shared_ptr<TreeNode> insertBack(std::shared_ptr<TreeNode> node, VertexID vid, std::shared_ptr<TreeNode> parent);
        std::shared_ptr<TreeNode> minValueNode(std::shared_ptr<TreeNode> node);
        std::shared_ptr<TreeNode> erase(std::shared_ptr<TreeNode> node, uint vRank);
        std::shared_ptr<TreeNode> findNode(std::shared_ptr<TreeNode> node, uint rank);
        uint rank(std::shared_ptr<TreeNode> node) const;
        void getVertexes(std::shared_ptr<TreeNode> node, std::vector<VertexID> &vids);
        void inOrderTraversal(std::shared_ptr<TreeNode> node) const;
        std::shared_ptr<TreeNode> buildTree(std::vector<VertexID> &vids, int start, int end, std::shared_ptr<TreeNode> parent = nullptr);

    public:
        OSTree();
        void buildTree(std::vector<VertexID>& vids);
        void insertFront(const VertexID& vid);
        void insertBack(const VertexID& vid);
        void erase(const VertexID& vid);
        uint getRank(const VertexID& vid);
        bool compare(const VertexID& v1, const VertexID& v2);
        bool hasVertex(const VertexID& vid) const;
        VertexID at(uint index);
        std::vector<VertexID> getVids();
        uint size() const;
        void display() const;
        void showMap() const;
};