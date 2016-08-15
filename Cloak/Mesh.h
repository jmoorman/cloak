#pragma once

#include "stdafx.h"

#include "geometry.h"

enum VertexAttribute {
    kVertexAttributeBegin = 0, //sentinel value
    kVertexAttributePosition = kVertexAttributeBegin,
    kVertexAttributeNormal,
    kVertexAttributeCount //sentinel value
};

class Mesh {
public:
    static const int kIndicesPerTriangle = 3;

	Mesh();
    ~Mesh();

	bool loadFromObj(const std::string &filename);

    unsigned int getVertexCount();
    unsigned int getIndexCount();
    unsigned int getTriangleCount();

    void addVertex(const MeshVertex &vertex);
    void addIndex(U32 index);

    void clear();

    void render();

private:
    std::vector<MeshVertex> mVertices;
    std::vector<U32> mIndices;
    U32 mVertexBuffers[kVertexAttributeCount];
    U32 mVertexArrayObject;

	VkBuffer *mIndexBuffer;
};