#include "mbptree/mbptree.h"

MbpTree::MbpTree(uint _maxCapcity)
{
    root = new MbpNode(nullptr, nullptr, nullptr, true);
    maxCapacity = _maxCapcity > 2 ? _maxCapcity : 2;
    minCapacity = maxCapacity / 2;
    depth = 0;
}

MbpTree::~MbpTree()
{
    deleteTree(root);
}

void MbpTree::deleteTree(MbpNode* node)
{
    if(node == nullptr)
    {
        return;
    }
    // 如果节点不是叶节点，递归删除所有子节点
    if(!node->isLeafNode())
    {
        for(MbpNode* child : node->children)
        {
            deleteTree(child);
        }
    }
    delete node;
}

MbpNode* MbpTree::getRoot() const
{
    return root;
}

MbpNode* MbpTree::findLeaf(uint key) const
{
    MbpNode* node = root;
    while(!node->isLeafNode())
    {
        node = node->getChild(key);
    }
    return node;
}

// uint MbpTree::get(uint key) const
// {
//     return findLeaf(key)->get(key);
// }

std::array<unsigned char, SHA256_DIGEST_LENGTH> MbpTree::getVertexDigest(const VertexID& vid) const
{
    return findLeaf(vid)->getVertexDigest(vid);
}

// void MbpTree::set(uint key, uint value)
// {
//     MbpNode* leaf = findLeaf(key);
//     leaf->set(key, value); // 如果key存在，则更新value；否则，插入新节点
//     // 如果叶节点超出最大容量
//     if(leaf->keys.size() > maxCapacity)
//     {
//         insert(leaf->splitLeaf());
//     }
// }

void MbpTree::setVertexDigest(const VertexID& vid, const std::array<unsigned char, SHA256_DIGEST_LENGTH>& _digest)
{
    MbpNode* leaf = findLeaf(vid);
    leaf->setVertexDigest(vid, _digest); // 如果key存在，则更新value；否则，插入新节点
    // 如果叶节点超出最大容量
    if(leaf->keys.size() > maxCapacity)
    {
        insert(leaf->splitLeaf());
    }
}

void MbpTree::insert(std::tuple<uint, MbpNode*, MbpNode*> result)
{
    uint key = std::get<0>(result);
    MbpNode* left = std::get<1>(result);
    MbpNode* right = std::get<2>(result);
    MbpNode* parent = left->getParent();

    // 如果父节点为空，说明是根节点，需要新建父节点
    if(parent == nullptr)
    {
        MbpNode* newRoot = new MbpNode(nullptr, nullptr, nullptr, false);
        left->setParent(newRoot);
        right->setParent(newRoot);
        root = newRoot;
        depth++;
        root->keys = {key};
        root->children = {left, right};
        return ;
    }

    // 将分裂结果重新插入父节点
    parent->setChild(key, {left, right});

    // 如果父节点超出最大容量，需要分裂父节点
    if(parent->keys.size() > maxCapacity)
    {
        insert(parent->splitInternal());
    }
}

void MbpTree::removefromLeaf(uint key, MbpNode* node)
{
    int index = node->indexofKey(key);
    if(index >= node->keys.size())
    {
        std::cout << key << " not found" << std::endl;
        return ;
    }
    node->keys.erase(node->keys.begin() + index);
    // node->values.erase(node->values.begin() + index);
    node->vertexDigests.erase(node->vertexDigests.begin() + index);

    MbpNode* parent = node->getParent();
    // 如果有父节点
    if(parent != nullptr)
    {
        int indexofNode = parent->indexofChild(key);
        if(indexofNode != 0)
        {
            parent->keys[indexofNode - 1] = node->keys.front();
        }
    }
}

void MbpTree::removefromInternal(uint key, MbpNode* node)
{
    int index = node->indexofKey(key);
    if(index < node->keys.size())
    {
        // 找到右子节点的最左叶子节点(即右边孩子叶子节点中的最小值)
        MbpNode* leftMostLeaf = node->children[index + 1]; 
        while(!leftMostLeaf->isLeafNode())
        {
            leftMostLeaf = leftMostLeaf->children.front();
        }
        node->keys[index] = leftMostLeaf->keys.front(); // 用右子节点的最左叶子节点的值替换掉当前节点的key
    }
}

void MbpTree::borrowKeyfromRightLeaf(MbpNode* node, MbpNode* next)
{
    MbpNode* parent = node->getParent();
    node->keys.push_back(next->keys.front()); // 从右侧节点借用第一个key
    next->keys.erase(next->keys.begin()); // 删除右侧节点的第一个key
    // node->values.push_back(next->values.front()); // 从右侧节点借用对应的value值
    // next->values.erase(next->values.begin()); // 删除右侧节点的对应的value
    node->vertexDigests.push_back(next->vertexDigests.front());
    next->vertexDigests.erase(next->vertexDigests.begin());
    for(int i = 0; i < parent->children.size(); i++)
    {
        if(parent->children[i] == next)
        {
            parent->keys[i - 1] = next->keys.front(); // 更新父节点的key
            break;
        }
    }
}

void MbpTree::borrowKeyfromLeftLeaf(MbpNode* node, MbpNode* prev)
{
    MbpNode* parent = node->getParent();
    node->keys.insert(node->keys.begin(), prev->keys.back()); // 从左侧节点借用最后一个key
    prev->keys.erase(prev->keys.end() - 1); // 删除左侧节点的最后一个key
    // node->values.insert(node->values.begin(), prev->values.back()); // 从左侧节点借用对应的value值
    // prev->values.erase(prev->values.end() - 1);
    node->vertexDigests.insert(node->vertexDigests.begin(), prev->vertexDigests.back());
    prev->vertexDigests.erase(prev->vertexDigests.end() - 1);
    for(int i = 0; i < parent->children.size(); i++)
    {
        if(parent->children[i] == node);
        {
            parent->keys[i - 1] = node->keys.front(); // 更新父节点的key
            break;
        }
    }
}

/**
 * mergeNodewithRightLeaf和mergeNodewithLeftLeaf这两个函数可以合并，
 * 合并左节点也可以使用mergenodewithRightLeaf函数，只需要将参数输入改为(prev, node)
 */

void MbpTree::mergeNodewithRightLeaf(MbpNode* node, MbpNode* next)
{
    MbpNode* parent = node->getParent();
    node->keys.insert(node->keys.end(), next->keys.begin(), next->keys.end()); // 将右侧节点的key合并到当前节点
    // node->values.insert(node->values.end(), next->values.begin(), next->values.end()); // 将右侧节点的value值合并到当前节点
    node->vertexDigests.insert(node->vertexDigests.end(), next->vertexDigests.begin(), next->vertexDigests.end());
    node->setNext(next->getNext()); // 更新当前节点的next指针

    if(node->getNext() != nullptr) // 不是最后一个节点
    {
        node->getNext()->setPrev(node); // 更新下一个节点的prev指针
    }

    for(int i = 0; i < parent->children.size(); i++)
    {
        if(parent->children[i] == next)
        {
            parent->keys.erase(parent->keys.begin() + i - 1);
            parent->children.erase(parent->children.begin() + i);
            break;
        }
    }

    delete next; // 删除右侧节点,防止内存泄漏
}

void MbpTree::mergeNodewithLeftLeaf(MbpNode* node, MbpNode* prev)
{
    MbpNode* prevParent = prev->getParent();
    prev->keys.insert(prev->keys.end(), node->keys.begin(), node->keys.end()); // 将当前节点的key合并到左侧节点
    // prev->values.insert(prev->values.end(), node->values.begin(), node->values.end()); // 将当前节点的value值合并到左侧节点
    prev->vertexDigests.insert(prev->vertexDigests.end(), node->vertexDigests.begin(), node->vertexDigests.end());
    prev->setNext(node->getNext()); // 更新左侧节点的next指针

    if(prev->getNext() != nullptr)
    {
        prev->getNext()->setPrev(prev); // 更新下一个节点的prev指针
    }

    for(int i = 0; i < prevParent->children.size(); i++)
    {
        if(prevParent->children[i] == node)
        {
            prevParent->keys.erase(prevParent->keys.begin() + i - 1);
            prevParent->children.erase(prevParent->children.begin() + i);
            break;
        }
    }
    delete node; // 删除当前节点,防止内存泄漏
}

void MbpTree::borrowKeyfromRightInternal(int posinParent, MbpNode* node, MbpNode* next)
{
    // 把父节点的key借给node,然后使用next节点的第一个key替换掉父节点的key,再将next节点的第一个孩子节点插入到node最右侧
    MbpNode* parent = node->getParent();
    node->keys.insert(node->keys.end(), parent->keys[posinParent]); // 从右侧节点借用父节点的key
    parent->keys[posinParent] = next->keys.front(); // 更新父节点的key
    next->keys.erase(next->keys.begin()); // 删除右侧节点的第一个key
    node->children.insert(node->children.end(), next->children.front()); // 从右侧节点借用右孩子
    next->children.erase(next->children.begin());
    node->children.back()->setParent(node); // 设置右孩子的父节点为node
}

void MbpTree::borrowKeyfromLeftInternal(int posinParent, MbpNode* node, MbpNode* prev)
{
    MbpNode* parent = node->getParent();
    node->keys.insert(node->keys.begin(), parent->keys[posinParent - 1]); // 从左侧节点借用父节点的key
    parent->keys[posinParent - 1] = prev->keys.back(); // 更新父节点的key
    prev->keys.erase(prev->keys.end() - 1); // 删除左侧节点的最后一个key
    node->children.insert(node->children.begin(), prev->children.back()); // 从左侧节点借用左孩子
    prev->children.erase(prev->children.end() - 1);
    node->children.front()->setParent(node); // 设置左孩子的父节点为node
}

void MbpTree::mergeNodewithRightInternal(int posinParent, MbpNode* node, MbpNode* next)
{
    MbpNode* parent = node->getParent();
    node->keys.insert(node->keys.end(), parent->keys[posinParent]); // 将父节点的key合并到当前节点
    parent->keys.erase(parent->keys.begin() + posinParent); // 删除父节点的key
    parent->children.erase(parent->children.begin() + posinParent + 1); // 删除父节点的右孩子,即next节点
    node->keys.insert(node->keys.end(), next->keys.begin(), next->keys.end()); // 将next节点的key合并到当前节点
    node->children.insert(node->children.end(), next->children.begin(), next->children.end()); // 将next节点的孩子节点合并到当前节点
    for(MbpNode* child : node->children)
    {
        child->setParent(node); // 设置孩子节点的父节点为node
    }

    delete next; // 删除next节点,防止内存泄漏
}

void MbpTree::mergeNodewithLeftInternal(int posinParent, MbpNode* node, MbpNode* prev)
{
    MbpNode* prevParent = prev->getParent();
    prev->keys.insert(prev->keys.end(), prevParent->keys[posinParent - 1]);
    prevParent->keys.erase(prevParent->keys.begin() + posinParent - 1);
    prevParent->children.erase(prevParent->children.begin() + posinParent);
    prev->keys.insert(prev->keys.end(), node->keys.begin(), node->keys.end());
    prev->children.insert(prev->children.end(), node->children.begin(), node->children.end());
    for(MbpNode* child : prev->children)
    {
        child->setParent(prev);
    }

    delete node; // 删除当前节点,防止内存泄漏
}

void MbpTree::remove(uint key, MbpNode* node)
{
    if(node == nullptr)
    {
        node = findLeaf(key);
    }
    if(node->isLeafNode())
    {
        removefromLeaf(key, node);
    }
    else
    {
        removefromInternal(key, node);
    }

    MbpNode* parent = node->getParent();

    if(node->keys.size() < minCapacity)
    {
        if(node == root)
        {
            if(root->keys.empty() && !root->children.empty())
            {
                root = root->children.front();
                root->setParent(nullptr);
                depth--;
            }
            return ;
        }
        else if(node->isLeafNode())
        {
            MbpNode* next = node->getNext();
            MbpNode* prev = node->getPrev();

            if(next != nullptr && next->getParent() == parent)
            {
                if(next->keys.size() > minCapacity)
                {
                    borrowKeyfromRightLeaf(node, next);
                }
                else
                {
                    mergeNodewithRightLeaf(node, next);
                }
            }
            else if(prev != nullptr && prev->getParent() == parent)
            {
                if(prev->keys.size() > minCapacity)
                {
                    borrowKeyfromLeftLeaf(node, prev);
                }
                else
                {
                    mergeNodewithLeftLeaf(node, prev);
                }
            }
        }
        else
        {
            int posinParent = -1;
            MbpNode* next = nullptr;
            MbpNode* prev = nullptr;

            for(int i = 0; i < parent->children.size(); i++)
            {
                if(parent->children[i] == node)
                {
                    posinParent = i;
                    break;
                }
            }

            if(posinParent < parent->children.size() - 1) // node不是最后一个孩子节点
            {
                next = parent->children[posinParent + 1];
            }
            if(posinParent > 0) // node不是第一个孩子节点
            {
                prev = parent->children[posinParent - 1];
            }

            if(next != nullptr && next->getParent() == parent)
            {
                if(next->keys.size() > minCapacity)
                {
                    borrowKeyfromRightInternal(posinParent, node, next);
                }
                else
                {
                    mergeNodewithRightInternal(posinParent, node, next);
                }
            }
            else if(prev != nullptr && prev->getParent() == parent)
            {
                if(prev->keys.size() > minCapacity)
                {
                    borrowKeyfromLeftInternal(posinParent, node, prev);
                }
                else
                {
                    mergeNodewithLeftInternal(posinParent, node, prev);
                }
            }
        }
    }
    if(parent != nullptr)
    {
        remove(key, parent);
    }
}

void MbpTree::constructVO(std::vector<VOEntry>& vo, std::vector<VertexID> subgraphVids, const std::map<VertexID, std::string>& serializedVertexInfo)
{
    root->constructVO(vo, subgraphVids, serializedVertexInfo);
}

void MbpTree::printMbpTreeInfo(MbpNode* node, std::string _prefix, bool _last)
{
    if(node == nullptr)
    {
        std::cout<<std::endl;
        std::cout<<"MbpTree Info : "<<std::endl;
        node = root;
    }

    // std::cout << _prefix << "├ [";
    std::cout << _prefix << "|- [";

    for(int i = 0; i < node->keys.size(); i++)
    {
        std::cout << node->keys[i];
        if(i != node->keys.size() - 1)
        {
            std::cout<<", ";
        }
    }
    // std::cout << "]" << std::endl;
    std::cout << "] : ";
    unsigned char nodeDigest[SHA256_DIGEST_LENGTH];
    node->getDigest(nodeDigest);
    digestPrint(nodeDigest);

    // _prefix += _last ? "   " : "╎  ";
    _prefix += _last ? "   " : "|  ";

    if(!node->isLeafNode())
    {
        for(int i = 0; i < node->children.size(); i++)
        {
            _last = (i == node->children.size() - 1);
            printMbpTreeInfo(node->children[i], _prefix, _last);
        }
    }
}