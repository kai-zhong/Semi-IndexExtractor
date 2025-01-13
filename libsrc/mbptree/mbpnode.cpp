#include "mbptree/mbpnode.h"

MbpNode::MbpNode(MbpNode* _parent, MbpNode* _prev, MbpNode* _next, bool _isLeaf)
{
    parent = _parent;
    prev = _prev;
    next = _next;
    isLeaf = _isLeaf;

    isDigestComputed = false;
    memset(digest, 0, SHA256_DIGEST_LENGTH);

    if(next != nullptr)
    {
        next->prev = this;
    }
    if(prev != nullptr)
    {
        prev->next = this;
    }
}

MbpNode::~MbpNode(){}

uint MbpNode::indexofChild(const uint& key) const
{
    auto it = std::lower_bound(keys.begin(), keys.end(), key); // 返回指向第一个大于等于 key 的迭代器
    if (it != keys.end() && *it == key) // 等于 key 的情况的child index再加1
    {
        ++it;
    }
    return std::distance(keys.begin(), it);
}

uint MbpNode::indexofKey(const uint& key) const
{
    auto it = std::lower_bound(keys.begin(), keys.end(), key); // 返回指向第一个大于等于 key 的迭代器
    if(it != keys.end() && *it == key)
    {
        return std::distance(keys.begin(), it);
    }
    return keys.size();
}

bool MbpNode::hasKey(const uint& key) const
{
    return std::binary_search(keys.begin(), keys.end(), key);
}

MbpNode* MbpNode::getParent() const
{
    return parent;
}

MbpNode* MbpNode::getPrev() const
{
    return prev;
}

MbpNode* MbpNode::getNext() const
{
    return next;
}

MbpNode* MbpNode::getChild(const uint& key) const
{
    return children[indexofChild(key)];
}

void MbpNode::setParent(MbpNode* _parent)
{
    parent = _parent;
}

void MbpNode::setPrev(MbpNode* _prev)
{
    prev = _prev;
}

void MbpNode::setNext(MbpNode* _next)
{
    next = _next;
}

bool MbpNode::isLeafNode() const
{
    return isLeaf;
}

void MbpNode::setChild(const uint& key, const std::vector<MbpNode*>& value)
{
    uint index = indexofChild(key);
    keys.insert(keys.begin() + index, key);
    children.erase(children.begin() + index);
    children.insert(children.begin() + index, value.begin(), value.end());
}

std::tuple<uint, MbpNode*, MbpNode*> MbpNode::splitInternal()
{
    int mid = keys.size() / 2; // 计算分裂位置
    MbpNode* left = new MbpNode(parent, nullptr, nullptr, false); // 创建左分裂节点

    std::copy(keys.begin(), keys.begin() + mid, std::back_inserter(left->keys));
    std::copy(children.begin(), children.begin() + mid + 1, std::back_inserter(left->children));

    for(MbpNode* child : left->children)
    {
        child->parent = left;
    }

    uint key = keys[mid];
    keys.erase(keys.begin(), keys.begin() + mid + 1);
    children.erase(children.begin(), children.begin() + mid + 1);

    return std::make_tuple(key, left, this);
}

std::tuple<uint, MbpNode*, MbpNode*> MbpNode::splitLeaf()
{
    int mid = keys.size() / 2; // 计算分裂位置
    MbpNode* left = new MbpNode(parent, prev, this, true);

    left->keys = std::vector<uint>(keys.begin(), keys.begin() + mid);
    // left->values = std::vector<uint>(values.begin(), values.begin() + mid);
    left->vertexDigests = std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>>(vertexDigests.begin(), vertexDigests.begin() + mid);

    keys.erase(keys.begin(), keys.begin() + mid);
    // values.erase(values.begin(), values.begin() + mid);
    vertexDigests.erase(vertexDigests.begin(), vertexDigests.begin() + mid);

    return std::make_tuple(keys[0], left, this);
}

// uint MbpNode::get(const uint& key) const
// {
//     uint index = indexofKey(key);
//     if (index >= keys.size())
//     {
//         std::cout << key << " not found in node" << std::endl;
//         throw std::runtime_error("Key not found in node");
//     }

//     return values[index];
// }

std::array<unsigned char, SHA256_DIGEST_LENGTH> MbpNode::getVertexDigest(const VertexID& vid) const
{
    uint index = indexofKey(vid);

    if(index >= keys.size())
    {
        std::cout << vid << "not found in node" << std::endl;
        throw std::runtime_error("Key not found in node");
    }

    return vertexDigests[index];
}

// void MbpNode::set(const uint& key, const uint& value)
// {
//     uint i = indexofChild(key);// 查找插入位置
//     if(!hasKey(key))
//     {
//         keys.insert(keys.begin() + i, key);
//         values.insert(values.begin() + i, value);
//     }
//     else
//     {
//         values[i - 1] = value;
//     }
// }

void MbpNode::setVertexDigest(const VertexID& vid, const std::array<unsigned char, SHA256_DIGEST_LENGTH>& _digest)
{
    uint i = indexofChild(vid);
    if(!hasKey(vid))
    {
        keys.insert(keys.begin() + i, vid);
        vertexDigests.insert(vertexDigests.begin() + i, _digest);
    }
    else
    {
        vertexDigests[i - 1] = _digest;
    }
}

void MbpNode::getDigest(unsigned char* _digest)
{
    // if(isDigestComputed == false)
    // {
    //     digestCompute();
    //     isDigestComputed = true;
    // }
    digestCompute();
    memcpy(_digest, digest, SHA256_DIGEST_LENGTH);
}

void MbpNode::setFalseDigestComputed()
{
    if(parent != nullptr && parent->isDigestComputed == true)
    {
        parent->setFalseDigestComputed();
    }
    isDigestComputed = false;
}

void MbpNode::digestCompute()
{
    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    if(isLeafNode())
    {
        for(int i = 0; i < keys.size(); i++)
        {
            unsigned char* data = vertexDigests[i].data();
            SHA256_Update(&ctx, data, SHA256_DIGEST_LENGTH);
        }
    }
    else
    {
        unsigned char childDigest[SHA256_DIGEST_LENGTH];
        for(int i = 0; i < children.size(); i++)
        {
            memset(childDigest, 0, SHA256_DIGEST_LENGTH);
            children[i]->getDigest(childDigest);
            SHA256_Update(&ctx, childDigest, SHA256_DIGEST_LENGTH);
        }
    }
    SHA256_Final(digest, &ctx);
}

void MbpNode::constructVO(std::vector<VOEntry>& vo, std::vector<VertexID>& subgraphVids, const std::map<VertexID, std::string>& serializedVertexInfo)
{
    VOEntry entryFront('[');
    VOEntry entryBack(']');
    vo.push_back(entryFront);
    if(isLeafNode())
    {
        for(size_t i = 0; i < keys.size(); i++)
        {
            if(subgraphVids.size() > 0 && keys[i] == subgraphVids.front())
            {
                vo.emplace_back(VOEntry(serializedVertexInfo.at(keys[i])));
                subgraphVids.erase(subgraphVids.begin());
            }
            else
            {
                vo.emplace_back(VOEntry(getVertexDigest(keys[i]).data(), SHA256_DIGEST_LENGTH));
            }
        }
    }
    else
    {
        for(size_t i = 0; i < keys.size(); i++)
        {
            
            std::vector<VertexID> nodePartVids;
            while(subgraphVids.size() > 0 && subgraphVids.front() < keys[i])
            {
                nodePartVids.emplace_back(subgraphVids.front());
                subgraphVids.erase(subgraphVids.begin());
            }
            if(nodePartVids.size() > 0)
            {
                children[i]->constructVO(vo, nodePartVids, serializedVertexInfo);
            }
            else
            {
                unsigned char childDigest[SHA256_DIGEST_LENGTH];
                children[i]->getDigest(childDigest);
                vo.emplace_back(VOEntry(childDigest, SHA256_DIGEST_LENGTH));
            }
        }
        if(subgraphVids.size() > 0)
        {
            children[keys.size()]->constructVO(vo, subgraphVids, serializedVertexInfo);
        }
        else
        {
            unsigned char childDigest[SHA256_DIGEST_LENGTH];
            children[keys.size()]->getDigest(childDigest);
            vo.emplace_back(VOEntry(childDigest, SHA256_DIGEST_LENGTH));
        }
    }
    vo.push_back(entryBack);
}

void MbpNode::printNodeInfo()
{
    std::cout<<"Keys: [";
    for(int i = 0; i < keys.size(); i++)
    {
        std::cout<<keys[i]<<" ";
    }
    std::cout<<"]"<<std::endl;
}