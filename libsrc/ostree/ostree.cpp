#include "ostree/ostree.h"

uint OSTree::getHeight(std::shared_ptr<TreeNode> node) const
{
    if (node == nullptr)
    {
        return 0;
    }
    return node->height;
}

int OSTree::getBalanceFactor(std::shared_ptr<TreeNode> node) const
{
    if (node == nullptr)
    {
        return 0;
    }
    return getHeight(node->left) - getHeight(node->right);
}

uint OSTree::getSize(std::shared_ptr<TreeNode> node) const
{
    if(node == nullptr)
    {
        return 0;
    }
    return node->size;
}

void OSTree::updateSize(std::shared_ptr<TreeNode> node)
{
    if (node == nullptr)
    {
        return ;
    }
    node->size = 1 + getSize(node->left) + getSize(node->right);
}

std::shared_ptr<TreeNode> OSTree::rightRotate(std::shared_ptr<TreeNode> y)
{
    std::shared_ptr<TreeNode> x = y->left;
    std::shared_ptr<TreeNode> T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;

    x->parent = y->parent;
    y->parent = x;

    if (T2!= nullptr)
    {
        T2->parent = y;
    }

    updateSize(y);
    updateSize(x);

    return x;
}

std::shared_ptr<TreeNode> OSTree::leftRotate(std::shared_ptr<TreeNode> x)
{
    std::shared_ptr<TreeNode> y = x->right;
    std::shared_ptr<TreeNode> T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;

    // 更新父节点指针
    y->parent = x->parent;
    x->parent = y;
    if (T2!= nullptr)
    {
        T2->parent = x;
    }

    updateSize(x);
    updateSize(y);

    return y;
}

std::shared_ptr<TreeNode> OSTree::balance(std::shared_ptr<TreeNode> node, bool isFront)
{
    int balanceFactor = getBalanceFactor(node);

    if(balanceFactor > 1 && isFront)
    {
        return rightRotate(node);
    }
    else if(balanceFactor < -1 && !isFront)
    {
        return leftRotate(node);
    }
    else if(balanceFactor > 1 && !isFront)
    {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    else if(balanceFactor < -1 && isFront)
    {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

std::shared_ptr<TreeNode> OSTree::insertFront(std::shared_ptr<TreeNode> node, VertexID vid, std::shared_ptr<TreeNode> parent)
{
    if (node == nullptr)
    {
        std::shared_ptr<TreeNode> newNode(new TreeNode(vid));
        node_map[vid] = newNode;
        return newNode;
    }

    node->left = insertFront(node->left, vid, node);

    node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
    
    updateSize(node);
    return balance(node, true);
}

std::shared_ptr<TreeNode> OSTree::insertBack(std::shared_ptr<TreeNode> node, VertexID vid, std::shared_ptr<TreeNode> parent)
{
    if (node == nullptr)
    {
        std::shared_ptr<TreeNode> newNode(new TreeNode(vid, parent));
        node_map[vid] = newNode;
        return newNode;
    }

    node->right = insertBack(node->right, vid, node);

    node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;

    updateSize(node);
    return balance(node, false);
}

std::shared_ptr<TreeNode> OSTree::minValueNode(std::shared_ptr<TreeNode> node)
{
    std::shared_ptr<TreeNode> current = node;
    while(current->left != nullptr)
    {
        current = current->left;
    }
    return current;
}

std::shared_ptr<TreeNode> OSTree::erase(std::shared_ptr<TreeNode> node, uint vRank)
{
    if (node == nullptr)
    {
        return nullptr;
    }
    uint leftSize = node->left ? node->left->size : 0;
    if(vRank < leftSize + 1)
    {
        node->left = erase(node->left, vRank);
    }
    else if(vRank > leftSize + 1)
    {
        node->right = erase(node->right, vRank - leftSize - 1);
    }
    else
    {
        if(node->left == nullptr || node->right == nullptr)
        {
            std::shared_ptr<TreeNode> temp = node->left ? node->left : node->right;
            
            if(temp == nullptr) // 叶子节点
            {
                node_map.erase(node->vid);
                node = nullptr;
            }
            else // 只有一个孩子节点
            {
                node_map[temp->vid] = node_map[node->vid];
                node_map.erase(node->vid);
                // *node = *temp;
                node->height = temp->height;
                node->left = temp->left;
                node->right = temp->right;
                node->vid = temp->vid;
                node->size = temp->size;
            }
        }
        else // 有两个孩子
        {
            std::shared_ptr<TreeNode> temp = minValueNode(node->right);
            node_map.erase(node->vid);
            node->vid = temp->vid;
            node->right = erase(node->right, vRank - leftSize);
            node_map[node->vid] = node;
        }
    }
    if(node == nullptr)
    {
        return nullptr;
    }

    node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
    updateSize(node);

    int balance = getBalanceFactor(node);
    // Left Left Case
    if (balance > 1 && getBalanceFactor(node->left) >= 0)
    {
        return rightRotate(node);
    }
    // Left Right Case
    if (balance > 1 && getBalanceFactor(node->left) < 0) 
    {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    // Right Right Case
    if (balance < -1 && getBalanceFactor(node->right) <= 0)
    {
        return leftRotate(node);
    }
    // Right Left Case
    if (balance < -1 && getBalanceFactor(node->right) > 0) 
    {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

std::shared_ptr<TreeNode> OSTree::findNode(std::shared_ptr<TreeNode> node, uint vRank)
{
    if(node == nullptr)
    {
        return nullptr;
    }
    uint leftSize = node->left ? node->left->size : 0;
    if(vRank == leftSize + 1)
    {
        return node;
    }
    else if(vRank < leftSize + 1)
    {
        return findNode(node->left, vRank);
    }
    else
    {
        return findNode(node->right, vRank - leftSize - 1);
    }
}

uint OSTree::rank(std::shared_ptr<TreeNode> node) const
{
    uint vRank = (node->left ? node->left->size : 0) + 1;
    std::shared_ptr<TreeNode> current = node;
    while(auto _parent = current->parent.lock())
    {
        if(current == _parent->right)
        {
            vRank += (_parent->left ? _parent->left->size : 0) + 1;
        }
        current = _parent;
    }
    return vRank;
}

void OSTree::getVertexes(std::shared_ptr<TreeNode> node, std::vector<VertexID> &vids)
{
    if (node == nullptr)
    {
        return ;
    }
    getVertexes(node->left, vids);
    vids.emplace_back(node->vid);
    getVertexes(node->right, vids);
}

void OSTree::inOrderTraversal(std::shared_ptr<TreeNode> node) const
{
    if (node == nullptr)
    {
        return;
    }
    std::cout << node->vid;
    if(auto _parent = node->parent.lock())
    {
        std::cout << "[parent=" << _parent->vid << ", ";
    }
    else
    {
        std::cout << "[parent=root, ";
    }
    std::cout << "size=" << node->size << ", ";
    std::cout << "rank=" << rank(node) << "] ";
    inOrderTraversal(node->left);
    inOrderTraversal(node->right);
}

std::shared_ptr<TreeNode> OSTree::buildTree(std::vector<VertexID> &vids, int start, int end, std::shared_ptr<TreeNode> parent)
{
    if(start > end)
    {
        return nullptr;
    }

    int mid = (start + end) / 2;

    std::shared_ptr<TreeNode> node = std::make_shared<TreeNode>(vids[mid], parent);
    node_map[vids[mid]] = node;
    node->left = buildTree(vids, start, mid - 1, node);
    node->right = buildTree(vids, mid + 1, end, node);
    updateSize(node);
    node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;

    return node;
}

OSTree::OSTree() : root(nullptr) {}

void OSTree::buildTree(std::vector<VertexID> &vids)
{
    root = buildTree(vids, 0, vids.size() - 1);
}

void OSTree::insertFront(const VertexID& vid)
{
    root = insertFront(root, vid, nullptr);
}

void OSTree::insertBack(const VertexID& vid)
{
    root = insertBack(root, vid, nullptr);
}

void OSTree::erase(const VertexID& vid)
{
    root = erase(root, getRank(vid));
    if(root != nullptr)
    {
        if(auto _parent = root->parent.lock())
        {
            _parent = nullptr;    
        }
    }
}

uint OSTree::getRank(const VertexID& vid)
{ 
    // std::cout << "Get Vertex " << vid << " rank" << std::endl;
    if(node_map.find(vid) == node_map.end())
    {
        std::cerr << "OSTree Error: vertex " << vid << " not found" << std::endl;
        throw std::runtime_error("OSTree Error: vertex not found");
    }
    if(node_map[vid] == nullptr)
    {
        std::cout << "OSTree Warning: vertex " << vid << " not in tree" << std::endl;
    }
    // std::cout << "Vertex " << vid << " rank is " << rank(node_map[vid]) << std::endl;
    return rank(node_map[vid]);
}

bool OSTree::compare(const VertexID& v1, const VertexID& v2)
{
    return getRank(v1) < getRank(v2);
}

bool OSTree::hasVertex(const VertexID& vid) const
{
    return node_map.find(vid) != node_map.end();
}

VertexID OSTree::at(uint index)
{
    if(root == nullptr)
    {
        std::cerr << "Error: empty tree" << std::endl;
        throw std::runtime_error("Error: empty tree");
    }
    std::shared_ptr<TreeNode> node = findNode(root, index);
    return node->vid;
}

std::vector<VertexID> OSTree::getVids()
{
    std::vector<VertexID> vids;
    vids.reserve(root->size);
    getVertexes(root, vids);
    return vids;
}

uint OSTree::size() const
{
    return root->size;
}

void OSTree::display() const
{
    inOrderTraversal(root);
    std::cout << std::endl;
}

void OSTree::showMap() const
{
    for(auto it = node_map.begin(); it != node_map.end(); ++it)
    {
        std::cout << it->first << "->" << it->second->vid << "[rank=" << rank(it->second) << "]" << std::endl;
    }
}

void OSTree::check()
{
    std::unordered_set<std::shared_ptr<TreeNode>> visited;
    for(const std::pair<VertexID, std::shared_ptr<TreeNode>> &p : node_map)
    {
        std::shared_ptr<TreeNode> node = p.second;
        if(visited.find(node) != visited.end())
        {
            std::cerr << "OSTree Error: cycle detected" << std::endl;
            throw std::runtime_error("OSTree Error: cycle detected");
        }
        visited.insert(node);
    }
}