#include "stdafx.h"

#include "Mesh.h"


Mesh::Mesh() {

}

Mesh::~Mesh() {

}

bool Mesh::loadFromObj(const std::string & filename)
{
	clear();

	//attempt to open the file
	std::ifstream file(filename);
	if (file.fail()) {
		return false;
	}

	//get file length
	file.seekg(0, std::ios::end);
	int fileLength = (int)file.tellg();
	file.seekg(std::ios::beg);

	std::string token;
	std::string ignore;

	std::vector<unsigned int> indexCounts;
	while (file >> token, !file.eof()) {
		if (token == "v") {
			MeshVertex vertex;
			file >> vertex.position.x >> vertex.position.y >> vertex.position.z;
			file.ignore(fileLength, '\n');
			mVertices.push_back(vertex);
			indexCounts.push_back(0);
		}
		else if (token == "f") {
			U32 indices[Mesh::kIndicesPerTriangle];
			MeshVertex *triangle[Mesh::kIndicesPerTriangle];
			//get each index that makes up a triangle face
			for (int i = 0; i < Mesh::kIndicesPerTriangle; i++) {
				file >> indices[i];
				indices[i]--; //.obj indices start with 1
				assert(indices[i] < mVertices.size());
				indexCounts[indices[i]]++;
				triangle[i] = &mVertices[indices[i]];
				addIndex(indices[i]);
			}
			//calculate the face normal
			glm::vec3 u = triangle[1]->position - triangle[0]->position;
			glm::vec3 v = triangle[2]->position - triangle[0]->position;
			glm::vec3 n = glm::cross(u, v);

			//add the face normal to each vertex normal
			for (int i = 0; i < Mesh::kIndicesPerTriangle; i++) {
				triangle[i]->normal = triangle[i]->normal + n;
			}
		}
	}
	//average the face normals to get per-vertex normals
	for (U32 i = 0; i < mVertices.size(); i++) {
		MeshVertex &vertex = mVertices[i];
		vertex.normal = glm::normalize(vertex.normal / (float)indexCounts[i]);
	}
	return true;
}

unsigned int Mesh::getVertexCount() {
    return mVertices.size();
}

unsigned int Mesh::getIndexCount() {
    return mIndices.size();
}

unsigned int Mesh::getTriangleCount() {
    assert(mIndices.size() % kIndicesPerTriangle == 0);
    return mIndices.size() / kIndicesPerTriangle;
}

void Mesh::addVertex(const MeshVertex &vertex) {
    mVertices.push_back(vertex);
}

void Mesh::addIndex(U32 index) {
    mIndices.push_back(index);
}

void Mesh::clear() {
    mVertices.clear();
    mIndices.clear();
}

void Mesh::render() {
	/*
    glBindVertexArray(mVertexArrayObject);
	    
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[kVertexAttributePosition]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * mVertices.size(), &mVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(kVertexAttributePosition);
	glVertexAttribPointer(kVertexAttributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), 0);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[kVertexAttributeNormal]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * mVertices.size(), &mVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(kVertexAttributeNormal);
    glVertexAttribPointer(kVertexAttributeNormal, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)sizeof(glm::vec3));
        
    //bind the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mIndices.size(), &mIndices[0], GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, NULL);
	*/
}
