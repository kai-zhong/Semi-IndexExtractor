#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <iostream>
#include <algorithm>

#include "../configuration/types.h"


template <typename T>
class AVLTree 
{
private:
    struct Node 
    {
        T value;
        Node* left;
        Node* right;
        int height;

        Node(const T& val)
            : value(val), left(nullptr), right(nullptr), height(1) {}
    };

public:
    AVLTree() : root(nullptr) 
    {
        valueNum = 0;
    }
    ~AVLTree() { destroyTree(root); }

    void insert(const T& value) 
    {
        root = insert(root, value);
    }

    void remove(const T& value) 
    {
        root = remove(root, value);
    }

    void printInOrder() const 
    {
        printInOrder(root);
        std::cout << std::endl;
    }

    T getMin() const
    {
        if(!root)
        {
            std::cerr << "Error: Tree is empty" << std::endl;
            throw std::runtime_error("Tree is empty");
        }
        return getMin(root)->value;
    }

    Node* getMin(Node* node) const
    {
        while(node->left)
        {
            node = node->left;
        }
        return node;
    }

    T getMax() const 
    {
        if (!root) 
        {
            throw std::runtime_error("Tree is empty");
        }
        return getMax(root)->value;
    }

    Node* getMax(Node* node) const 
    {
        while (node->right) 
        {
            node = node->right;
        }
        return node;
    }

    uint getSize() const
    {
        return valueNum;
    }

private:

    Node* root;
    uint valueNum;

    // Helper functions
    int height(Node* node) const {
        return node ? node->height : 0;
    }

    int balanceFactor(Node* node) const {
        return node ? height(node->left) - height(node->right) : 0;
    }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;

        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;

        return y;
    }

    Node* insert(Node* node, const T& value) 
    {
        if (!node) 
        {
            valueNum++;
            return new Node(value);
        }

        if (value < node->value) 
        {
            node->left = insert(node->left, value);
        } 
        else if (value > node->value) 
        {
            node->right = insert(node->right, value);
        } 
        else 
        {
            return node; // Duplicate values not allowed
        }

        node->height = std::max(height(node->left), height(node->right)) + 1;

        int balance = balanceFactor(node);

        if (balance > 1 && value < node->left->value) 
        {
            return rotateRight(node);
        }
        if (balance < -1 && value > node->right->value) 
        {
            return rotateLeft(node);
        }
        if (balance > 1 && value > node->left->value) 
        {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (balance < -1 && value < node->right->value) 
        {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    Node* remove(Node* node, const T& value) {
        if (!node) 
        {
            return node;
        }

        if (value < node->value) 
        {
            node->left = remove(node->left, value);
        } 
        else if (value > node->value) 
        {
            node->right = remove(node->right, value);
        } 
        else 
        {
            if (!node->left || !node->right) 
            {
                Node* temp = node->left ? node->left : node->right;

                if (!temp) 
                {
                    temp = node;
                    node = nullptr;
                } 
                else 
                {
                    *node = *temp;
                }

                delete temp;
                --valueNum;  
            } 
            else 
            {
                Node* temp = minValueNode(node->right);
                node->value = temp->value;
                node->right = remove(node->right, temp->value);
            }
        }

        if (!node)
        {
            return node;
        }

        node->height = std::max(height(node->left), height(node->right)) + 1;

        int balance = balanceFactor(node);

        if (balance > 1 && balanceFactor(node->left) >= 0) 
        {
            return rotateRight(node);
        }
        if (balance > 1 && balanceFactor(node->left) < 0) 
        {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (balance < -1 && balanceFactor(node->right) <= 0) 
        {
            return rotateLeft(node);
        }
        if (balance < -1 && balanceFactor(node->right) > 0) 
        {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    Node* minValueNode(Node* node) const {
        Node* current = node;

        while (current->left) {
            current = current->left;
        }

        return current;
    }

    void printInOrder(Node* node) const {
        if (!node) {
            return;
        }

        printInOrder(node->left);
        std::cout << node->value << " ";
        printInOrder(node->right);
    }

    void destroyTree(Node* node) {
        if (!node) {
            return;
        }

        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
};

#endif // AVL_TREE_H
