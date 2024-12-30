#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <array>
#include <openssl/sha.h>

#include "../configuration/types.h"
#include "../configuration/config.h"
#include "../util/common.h"

class Vertex
{
    private:
        VertexID id;
        uint degree;
        std::vector<VertexID> neighbors;
        unsigned char digest[SHA256_DIGEST_LENGTH];

    public:
        Vertex();
        Vertex(VertexID vid);
        Vertex(const Vertex& other);
        ~Vertex();

        VertexID getVid() const;
        uint getDegree() const;
        VertexID getMaxDegreeNeighbor() const;
        std::array<unsigned char, SHA256_DIGEST_LENGTH> getDigest() const;
        const std::vector<VertexID>& getNeighbors() const;

        bool hasNeighbor(VertexID neighbor_vid) const;

        void addNeighbor(VertexID neighbor_vid);
        void removeNeighbor(VertexID neighbor_vid);

        void digestCompute();
        void printInfo() const;
        void printNeighbors() const;
        void printDigest() const;

        bool operator==(const Vertex& other) const;
        bool operator>(const Vertex& other) const;
        bool operator<(const Vertex& other) const;
};